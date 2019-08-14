/*
 * Copyright (C) 2010-2019 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_GAME_RENDERER_H
#define WL_GRAPHIC_GAME_RENDERER_H

#include <map>
#include <memory>

#include "base/macros.h"
#include "base/vector.h"
#include "graphic/gl/fields_to_draw.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"

// Draw the terrain only.
void draw_terrain(const Widelands::EditorGameBase& egbase,
                  const FieldsToDraw& fields_to_draw,
                  const float scale,
                  Workareas workarea,
                  bool grid,
                  RenderTarget* dst);

// Draw the border stones for 'field' if it is a border and 'visibility' is
// correct.
void draw_border_markers(const FieldsToDraw::Field& field,
                         const float scale,
                         const FieldsToDraw& fields_to_draw,
                         RenderTarget* dst,
                         const Widelands::EditorGameBase* = nullptr);

#endif  // end of include guard: WL_GRAPHIC_GAME_RENDERER_H
