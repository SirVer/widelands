/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "editor/ui_menus/player_menu.h"

#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/set_starting_pos_tool.h"
#include "graphic/graphic.h"
#include "graphic/playercolor.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/widelands.h"
#include "profile/profile.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/multilinetextarea.h"

namespace {
constexpr int kMargin = 4;
// Make room for 8 players
// If this ever gets changed, don't forget to change the strings in the warning box as well.
constexpr Widelands::PlayerNumber kMaxRecommendedPlayers = 8;
}  // namespace

class EditorPlayerMenuWarningBox : public UI::Window {
public:
	explicit EditorPlayerMenuWarningBox(UI::Panel* parent)
	   /** TRANSLATORS: Window title in the editor when a player has selected more than the
	      recommended number of players */
	   : Window(parent, "editor_player_menu_warning_box", 0, 0, 500, 220, _("Too Many Players")),
	     box_(this, 0, 0, UI::Box::Vertical, 0, 0, 2 * kMargin),
	     warning_label_(
	        &box_,
	        0,
	        0,
	        300,
	        0,
	        UI::PanelStyle::kWui,
	        /** TRANSLATORS: Info text in editor player menu when a player has selected more than the
	           recommended number of players. Choice is made by OK/Abort. */
	        _("We do not recommend setting more than 8 players except for testing "
	          "purposes. Are you sure that you want more than 8 players?"),
	        UI::Align::kLeft,
	        UI::MultilineTextarea::ScrollMode::kNoScrolling),
	     /** TRANSLATORS: Checkbox for: 'We do not recommend setting more than 8 players except for
	        testing purposes. Are you sure that you want more than 8 players?' */
	     reminder_choice_(&box_, Vector2i::zero(), _("Do not remind me again")),
	     button_box_(&box_, kMargin, kMargin, UI::Box::Horizontal, 0, 0, 2 * kMargin),
	     ok_(&button_box_, "ok", 0, 0, 120, 0, UI::ButtonStyle::kWuiPrimary, _("OK")),
	     cancel_(&button_box_, "cancel", 0, 0, 120, 0, UI::ButtonStyle::kWuiSecondary, _("Abort")) {

		set_center_panel(&box_);

		box_.add(&warning_label_, UI::Box::Resizing::kFullSize);
		box_.add(&reminder_choice_, UI::Box::Resizing::kFullSize);

		button_box_.add_inf_space();
		button_box_.add(&cancel_);
		button_box_.add_inf_space();
		button_box_.add(&ok_);
		button_box_.add_inf_space();
		box_.add_space(kMargin);
		box_.add(&button_box_, UI::Box::Resizing::kFullSize);
		box_.add_space(kMargin);

		ok_.sigclicked.connect(boost::bind(&EditorPlayerMenuWarningBox::ok, boost::ref(*this)));
		cancel_.sigclicked.connect(
		   boost::bind(&EditorPlayerMenuWarningBox::cancel, boost::ref(*this)));
	}

	void ok() {
		write_option();
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
	}

	void cancel() {
		write_option();
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	}

	void write_option() {
		if (reminder_choice_.get_state()) {
			g_options.pull_section("global").set_bool(
			   "editor_player_menu_warn_too_many_players", false);
		}
	}

private:
	UI::Box box_;
	UI::MultilineTextarea warning_label_;
	UI::Checkbox reminder_choice_;
	UI::Box button_box_;
	UI::Button ok_;
	UI::Button cancel_;
};

inline EditorInteractive& EditorPlayerMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

EditorPlayerMenu::EditorPlayerMenu(EditorInteractive& parent,
                                   EditorSetStartingPosTool& tool,
                                   UI::UniqueWindow::Registry& registry)
   : EditorToolOptionsMenu(parent, registry, 0, 0, _("Player Options"), tool),
     box_(this, kMargin, kMargin, UI::Box::Vertical),
     no_of_players_(&box_,
                    "dropdown_map_players",
                    0,
                    0,
                    50,
                    kMaxRecommendedPlayers,
                    24,
                    _("Number of players"),
                    UI::DropdownType::kTextual,
                    UI::PanelStyle::kWui,
                    UI::ButtonStyle::kWuiSecondary),
     finalize_button_(&box_,
	    "finalize",
	    0,
	    0,
	    50,
	    24,
	    UI::ButtonStyle::kWuiSecondary,
	    parent.players_finalized() ? _("Players are finalized") : _("Finalize Players"),
	    parent.players_finalized() ?
	    		_("The players were finalized to enable scenario editor features. "
	    		"Their settings can not be changed anymore.") :
	    		_("Forbid changing the player settings. This is required for using the scenario editor functions.")) {
	box_.set_size(100, 100);  // Prevent assert failures
	box_.add(&finalize_button_, UI::Box::Resizing::kFullSize);
	box_.add_space(kMargin);
	box_.add(&no_of_players_, UI::Box::Resizing::kFullSize);
	box_.add_space(2 * kMargin);

	const bool finalized = parent.players_finalized();
	no_of_players_.set_enabled(!finalized);
	finalize_button_.set_enabled(!finalized);

	const Widelands::Map& map = eia().egbase().map();
	const Widelands::PlayerNumber nr_players = map.get_nrplayers();
	iterate_player_numbers(p, kMaxPlayers) {
		const bool map_has_player = p <= nr_players;

		no_of_players_.add(boost::lexical_cast<std::string>(static_cast<unsigned int>(p)), p, nullptr,
		                   p == nr_players);
		if (!finalized) {
			no_of_players_.selected.connect(
			   boost::bind(&EditorPlayerMenu::no_of_players_clicked, boost::ref(*this)));
		}

		UI::Box* row = new UI::Box(&box_, 0, 0, UI::Box::Horizontal);

		// Name
		UI::Panel* plr_name;
		if (finalized) {
			plr_name = new UI::Textarea(row, map_has_player ? map.get_scenario_player_name(p) : "");
		} else {
			UI::EditBox* e = new UI::EditBox(row, 0, 0, 0, UI::PanelStyle::kWui);
			if (map_has_player) {
				e->set_text(map.get_scenario_player_name(p));
			}
			e->changed.connect(boost::bind(&EditorPlayerMenu::name_changed, this, p - 1));
			plr_name = e;
		}

		// Tribe
		UI::Dropdown<std::string>* plr_tribe = new UI::Dropdown<std::string>(
		   row, (boost::format("dropdown_tribe%d") % static_cast<unsigned int>(p)).str(), 0, 0, 50,
		   16, plr_name->get_h(), _("Tribe"), UI::DropdownType::kPictorial, UI::PanelStyle::kWui,
		   UI::ButtonStyle::kWuiSecondary);
		{
			i18n::Textdomain td("tribes");
			for (const Widelands::TribeBasicInfo& tribeinfo : Widelands::get_all_tribeinfos()) {
				plr_tribe->add(_(tribeinfo.descname), tribeinfo.name,
				               g_gr->images().get(tribeinfo.icon), false, tribeinfo.tooltip);
			}
			plr_tribe->add(pgettext("tribe", "Random"), "",
			               g_gr->images().get("images/ui_fsmenu/random.png"), false,
			               _("The tribe will be selected at random"));
		}

		plr_tribe->select(
		   (p <= map.get_nrplayers() && Widelands::tribe_exists(map.get_scenario_player_tribe(p))) ?
		      map.get_scenario_player_tribe(p) :
		      "");
		if (finalized) {
			plr_tribe->set_enabled(false);
		} else {
			plr_tribe->selected.connect(
			   boost::bind(&EditorPlayerMenu::player_tribe_clicked, boost::ref(*this), p - 1));
		}

		// Starting position
		const Image* player_image =
		   playercolor_image(p - 1, "images/players/player_position_menu.png");
		assert(player_image);

		UI::Button* plr_position = new UI::Button(
		   row, "tribe", 0, 0, plr_tribe->get_h(), plr_tribe->get_h(), UI::ButtonStyle::kWuiSecondary,
		   /** TRANSLATORS: Button tooltip in the editor for using a player's starting position tool
		    */
		   player_image, _("Set this player’s starting position"));
		if (finalized) {
			plr_position->set_enabled(false);
		} else {
			plr_position->sigclicked.connect(
			   boost::bind(&EditorPlayerMenu::set_starting_pos_clicked, boost::ref(*this), p));
		}

		// Add the elements to the row
		row->add(plr_name, UI::Box::Resizing::kFillSpace);
		row->add_space(kMargin);

		row->add(plr_tribe);
		row->add_space(kMargin);

		row->add(plr_position);

		// Add the row itself
		box_.add(row, UI::Box::Resizing::kFullSize);
		box_.add_space(kMargin);
		row->set_visible(map_has_player);

		rows_.push_back(
		   std::unique_ptr<PlayerEditRow>(new PlayerEditRow(row, plr_name, plr_position, plr_tribe)));
	}

	no_of_players_.select(nr_players);
	if (!finalized) {
		finalize_button_.sigclicked.connect(boost::bind(&EditorPlayerMenu::finalize_clicked, this));
		// Init button states
		set_starting_pos_clicked(1);
	}
	layout();
}

void EditorPlayerMenu::layout() {
	assert(!rows_.empty());
	const Widelands::PlayerNumber nr_players = eia().egbase().map().get_nrplayers();
	box_.set_size(310, no_of_players_.get_h() + finalize_button_.get_h() + 2 * kMargin +
	                      nr_players * (rows_.front()->name->get_h() + kMargin));
	set_inner_size(box_.get_w() + 2 * kMargin, box_.get_h() + 2 * kMargin);
}

void EditorPlayerMenu::no_of_players_clicked() {
	EditorInteractive& menu = eia();
	assert(!menu.players_finalized());
	Widelands::Map* map = menu.egbase().mutable_map();
	Widelands::PlayerNumber const old_nr_players = map->get_nrplayers();
	Widelands::PlayerNumber const nr_players = no_of_players_.get_selected();
	assert(1 <= nr_players);
	assert(nr_players <= kMaxPlayers);

	if (old_nr_players == nr_players) {
		return;
	}

	// Display a warning if there are too many players
	if (nr_players > kMaxRecommendedPlayers) {
		if (g_options.pull_section("global").get_bool(
		       "editor_player_menu_warn_too_many_players", true)) {
			EditorPlayerMenuWarningBox warning(get_parent());
			if (warning.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kBack) {
				// Abort setting of players
				no_of_players_.select(std::min(old_nr_players, kMaxRecommendedPlayers));
			}
		}
	}

	if (old_nr_players < nr_players) {
		// Add new players
		map->set_nrplayers(nr_players);

		for (Widelands::PlayerNumber pn = old_nr_players + 1; pn <= nr_players; ++pn) {
			map->set_scenario_player_ai(pn, "");
			map->set_scenario_player_closeable(pn, false);

			// Register new default name and tribe for these players
			const std::string name =
			   /** TRANSLATORS: Default player name, e.g. Player 1 */
			   (boost::format(_("Player %u")) % static_cast<unsigned int>(pn)).str();
			map->set_scenario_player_name(pn, name);
			UI::EditBox* name_box = dynamic_cast<UI::EditBox*>(rows_.at(pn - 1)->name);
			assert(name_box);
			name_box->set_text(name);

			const std::string& tribename = rows_.at(pn - 1)->tribe->get_selected();
			assert(tribename.empty() || Widelands::tribe_exists(tribename));
			map->set_scenario_player_tribe(pn, tribename);
			rows_.at(pn - 1)->box->set_visible(true);
		}
		// Update button states
		set_starting_pos_clicked(menu.tools()->set_starting_pos.get_current_player());
	} else {
		// If a removed player was selected, switch starting pos tool to the highest available player
		if (old_nr_players >= menu.tools()->set_starting_pos.get_current_player()) {
			set_starting_pos_clicked(nr_players);
		}

		// Hide extra players
		map->set_nrplayers(nr_players);
		for (Widelands::PlayerNumber pn = nr_players; pn < old_nr_players; ++pn) {
			rows_.at(pn)->box->set_visible(false);
		}
	}
	menu.set_need_save(true);
	layout();
}

void EditorPlayerMenu::player_tribe_clicked(size_t row) {
	EditorInteractive& menu = eia();
	assert(!menu.players_finalized());
	const std::string& tribename = rows_.at(row)->tribe->get_selected();
	assert(tribename.empty() || Widelands::tribe_exists(tribename));
	menu.egbase().mutable_map()->set_scenario_player_tribe(row + 1, tribename);
	menu.set_need_save(true);
}

void EditorPlayerMenu::set_starting_pos_clicked(size_t row) {
	EditorInteractive& menu = eia();
	assert(!menu.players_finalized());
	//  jump to the current node
	Widelands::Map* map = menu.egbase().mutable_map();
	if (Widelands::Coords const sp = map->get_starting_pos(row)) {
		menu.map_view()->scroll_to_field(sp, MapView::Transition::Smooth);
	}

	//  select tool set mplayer
	menu.select_tool(menu.tools()->set_starting_pos, EditorTool::First);
	menu.tools()->set_starting_pos.set_current_player(row);

	//  reselect tool, so everything is in a defined state
	menu.select_tool(menu.tools()->current(), EditorTool::First);

	// Signal player position states via button states
	iterate_player_numbers(pn, map->get_nrplayers()) {
		if (pn == row) {
			rows_.at(pn - 1)->position->set_style(UI::ButtonStyle::kWuiPrimary);
			rows_.at(pn - 1)->position->set_perm_pressed(true);
		} else {
			rows_.at(pn - 1)->position->set_style(UI::ButtonStyle::kWuiSecondary);
			rows_.at(pn - 1)->position->set_perm_pressed(map->get_starting_pos(pn) !=
			                                             Widelands::Coords::null());
		}
	}
}

void EditorPlayerMenu::name_changed(size_t row) {
	//  Player name has been changed.
	EditorInteractive& menu = eia();
	assert(!menu.players_finalized());
	upcast(UI::EditBox, e, rows_.at(row)->name);
	assert(e);
	Widelands::Map* map = menu.egbase().mutable_map();
	map->set_scenario_player_name(row + 1, e->text());
	menu.set_need_save(true);
}

void EditorPlayerMenu::finalize_clicked() {
	EditorInteractive& menu = eia();
	assert(!menu.players_finalized());
	UI::WLMessageBox m(get_parent(),
			_("Finalize Players"),
			_("Are you sure you want to finalize the players? "
			"This means you will not be able to add or remove players, rename them, "
			"or change their tribe and starting position. "
			"Finalizing players is only required if you want to design a scenario with the editor."),
			UI::WLMessageBox::MBoxType::kOkCancel);
	if (m.run<UI::Panel::Returncodes>() != UI::Panel::Returncodes::kOk) {
		return;
	}
	const std::string result = menu.try_finalize_players();
	if (result.empty()) {
		// Success
		menu.select_tool(menu.tools()->info, EditorTool::ToolIndex::First);
		die();
		return;
	}
	UI::WLMessageBox error(get_parent(),
			_("Finalize Players Failed"),
			(boost::format(_("Finalizing the players failed! Reason: %s")) % result).str(),
			UI::WLMessageBox::MBoxType::kOk);
	error.run<UI::Panel::Returncodes>();
}

