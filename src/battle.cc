/*
 * Copyright (C) 2002-2004 by The Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
// TODO: Create the load/save code, networkcode isn't needed (I think)

#include "battle.h"
#include "error.h"
#include "game.h"

class Battle_Descr : public Map_Object_Descr
{
   public:
      Battle_Descr() { }
      ~Battle_Descr() { }
};

Battle_Descr g_Battle_Descr;


Battle::Battle () : BaseImmovable(&g_Battle_Descr)
{
   m_first = 0;
   m_second = 0;
   m_last_try = 0;
}

Battle::~Battle ()
{
   if (m_first) log ("Battle : first wasn't removed yet!\n");
   if (m_second) log ("Battle : second wasn't removed yet!\n");
}

void Battle::init (Editor_Game_Base* eg, Soldier* s1, Soldier* s2)
{
   assert (eg);
   assert (s1);
   assert (s2);
   
   log ("Battle::init\n");
   Map_Object::init(eg);
   m_first = s1;
   m_second = s2;
   if (eg->is_game())
      m_next_assault = schedule_act ((Game*)eg, 1000); // Every round is 1000 ms
}

void Battle::init (Editor_Game_Base* eg)
{
   assert (eg);
   
   Map_Object::init(eg);
   
   if (eg->is_game())
      m_next_assault = schedule_act ((Game*)eg, 1000); // Every round is 1000 ms
}

void Battle::soldiers (Soldier* s1, Soldier* s2)
{
   assert (s1);
   assert (s2);
   log ("Battle::init\n");
   
   m_first = s1;
   m_second = s2;
  
}


void Battle::cleanup (Editor_Game_Base* eg)
{
   log ("Battle::cleanup\n");
   m_first = 0;
   m_second = 0;
   Map_Object::cleanup(eg); 
}

void Battle::act (Game* g, uint data)
{
   log ("Battle::act\n");
   
   if (!g->is_game())
      return;
   
   Soldier* attacker;
   Soldier* defender;

   m_last_try = !m_last_try;
   if (m_last_try)
   {
      attacker = m_first;
      defender = m_second;
   }

   else
   {
      attacker = m_second;
      defender = m_first;
   }

   if (attacker->get_current_hitpoints() < 1)
   {
      attacker->send_signal(g, "die");
      defender->send_signal(g, "end_combat");
      m_first = 0;
      m_second = 0;
      schedule_destroy (g);
      return;
   }
   if (defender->get_current_hitpoints() < 1)
   {
      defender->send_signal(g, "die");
      attacker->send_signal(g, "end_combat");
      m_first = 0;
      m_second = 0;
      schedule_destroy (g);
      return;
   }
   
   // Put attack animation
   //attacker->start_animation(g, "attack", 1000);
   uint hit = g->logic_rand() % 100;
log (" hit=%d ", hit);
   if (hit > defender->get_evade())
   {
      uint attack = attacker->get_min_attack() + 
            (g->logic_rand()% (attacker->get_max_attack()-attacker->get_min_attack()-1));
      uint defend = defender->get_defense();
      defend = (attack * defend) / 100;

log (" attack(%d)=%d ", attacker->get_serial(), attack);
log (" defense(%d)=%d ", defender->get_serial(), defend);
log (" damage=%d\n", attack-defend);
      
      defender->damage (attack-defend);
      // defender->start_animation(g, "defend", 1000);
   }
   else
   {
log (" evade(%d)=%d\n", defender->get_serial(), defender->get_evade());
      //defender->start_animation(g, "evade", 1000);
   }
   m_next_assault = schedule_act(g, 1000);
}


