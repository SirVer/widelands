/*
 * Copyright (C) 2011-2019 by the Widelands Development Team
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

#ifndef WL_ECONOMY_FLEET_H
#define WL_ECONOMY_FLEET_H

#include <boost/shared_ptr.hpp>

#include "base/macros.h"
#include "logic/map_objects/map_object.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

class Economy;
struct Flag;
class PortDock;
struct RoutingNodeNeighbour;
struct Ship;

class FleetDescr : public MapObjectDescr {
public:
	FleetDescr(char const* const init_name, char const* const init_descname)
	   : MapObjectDescr(MapObjectType::FLEET, init_name, init_descname, "") {
	}
	~FleetDescr() override {
	}

private:
	DISALLOW_COPY_AND_ASSIGN(FleetDescr);
};

constexpr int32_t kFleetInterval = 5000;
constexpr uint32_t kRouteNotCalculated = std::numeric_limits<uint32_t>::max();

/**
 * Manage all ships and ports of a player that are connected
 * by ocean.
 *
 * That is, two ports belong to the same fleet if - and only if - ships can
 * travel between them, and so on. Players may have several fleets, if they
 * build ports that cannot communicate (e.g. one port on the ocean, and another
 * in a lake).
 *
 * @paragraph Lifetime
 *
 * Fleet objects are created on-the-fly by @ref Ship and @ref PortDock,
 * and destroy themselves when they become empty.
 *
 * The intention is for fleet objects to merge automatically and separate
 * again in reaction to changes in the map. However, this may not work
 * properly at the moment.
 */
struct Fleet : MapObject {
	struct PortPath {
		int32_t cost;
		boost::shared_ptr<Path> path;

		PortPath() : cost(-1) {
		}
	};

	const FleetDescr& descr() const;

	explicit Fleet(Player* player);

	PortDock* get_dock(Flag& flag) const;
	PortDock* get_dock(EditorGameBase&, Coords) const;
	PortDock* get_arbitrary_dock() const;
	void set_economy(Economy* e);

	bool active() const;

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void update(EditorGameBase&);

	void add_ship(Ship* ship);
	void remove_ship(EditorGameBase& egbase, Ship* ship);
	void add_port(EditorGameBase& egbase, PortDock* port);
	void remove_port(EditorGameBase& egbase, PortDock* port);
	bool has_ports() const;

	void log_general_info(const EditorGameBase&) const override;

	bool get_path(const PortDock& start, const PortDock& end, Path& path);
	void add_neighbours(PortDock& pd, std::vector<RoutingNodeNeighbour>& neighbours);

	uint32_t count_ships() const;
	uint32_t count_ships_heading_here(EditorGameBase& egbase, PortDock* port) const;
	uint32_t count_ports() const;
	bool get_act_pending() const;

protected:
	void act(Game&, uint32_t data) override;

private:
	bool find_other_fleet(EditorGameBase& egbase);
	bool merge(EditorGameBase& egbase, Fleet* other);
	void check_merge_economy();
	void connect_port(EditorGameBase& egbase, uint32_t idx);

	PortPath& portpath(uint32_t i, uint32_t j);
	const PortPath& portpath(uint32_t i, uint32_t j) const;
	PortPath& portpath_bidir(uint32_t i, uint32_t j, bool& reverse);
	const PortPath& portpath_bidir(uint32_t i, uint32_t j, bool& reverse) const;

	bool penalize_route(Game&, PortDock&, const Ship&, uint32_t*);
	void check_push_destination(Game&, Ship&, const PortDock&, PortDock&, uint32_t);

	std::vector<Ship*> ships_;
	std::vector<PortDock*> ports_;

	bool act_pending_;

	/**
	 * Store all pairs shortest paths between port docks
	 *
	 * Let i < j, then the path from ports_[i] to ports_[j] is stored in
	 * portpaths_[binom(j,2) + i]
	 */
	std::vector<PortPath> portpaths_;

	// saving and loading
protected:
	struct Loader : MapObject::Loader {
		Loader();

		void load(FileRead&);
		void load_pointers() override;
		void load_finish() override;

	private:
		std::vector<uint32_t> ships_;
		std::vector<uint32_t> ports_;
	};

public:
	bool has_new_save_support() override {
		return true;
	}
	void save(EditorGameBase&, MapObjectSaver&, FileWrite&) override;

	static MapObject::Loader* load(EditorGameBase&, MapObjectLoader&, FileRead&);
	void push_next_destinations(Game&, Ship&, const PortDock& from_port);
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_FLEET_H
