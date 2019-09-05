dirname = path.dirname (__file__)

tribes:new_productionsite_type {
   msgctxt = "amazons_building",
   name = "amazons_water_gatherers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Water Gatherer's Hut"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      log = 3,
      granite = 1,
      rubber = 1
   },
   return_on_dismantle = {
      log = 2,
   },

   animations = {
      idle = {
         pictures = path.list_files (dirname .. "idle_?.png"),
         hotspot = {23, 41},
      },
      working = {
         pictures = path.list_files (dirname .. "working_??.png"),
         hotspot = {23, 41},
         fps = 10,
      },
   },

   aihints = {
      collects_ware_from_map = "water",
      basic_amount = 1
   },

   working_positions = {
      amazons_carrier = 1
   },

   outputs = {
      "water"
   },

   indicate_workarea_overlaps = {
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "sleep=20000",
            "callworker=fetch_water",
            -- he carries 2 buckets so we need to create one now
            "produce=water",
         }
      },
   },

   out_of_resource_notification = {
      -- Translators: Short for "Out of ..." for a resource
      title = _"No Water",
      heading = _"Out of Water",
      message = pgettext ("amazons_building", "The carrier working at this water gatherer's hut can’t find any water in his vicinity."),
      productivity_threshold = 33
   },
}
