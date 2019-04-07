/* $Id: playlevel.cpp 104 2003-09-28 10:23:31Z Sirp $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "game_events.hpp"
#include "intro.hpp"
#include "language.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "sound.hpp"

#include <iostream>

LEVEL_RESULT play_level(game_data& gameinfo, config& terrain_config,
                        config* level, CVideo& video,
                        game_state& state_of_game,
						std::vector<config*>& story)
{
	const int num_turns = atoi(level->values["turns"].c_str());
	gamestatus status(num_turns);

	gamemap map(terrain_config,read_file("data/maps/" + level->values["map"]));

	CKey key;
	typedef std::map<gamemap::location,unit> units_map;
	units_map units;

	std::vector<team> teams;

	std::vector<config*>& unit_cfg = level->children["side"];
	for(std::vector<config*>::iterator ui = unit_cfg.begin();
				ui != unit_cfg.end(); ++ui) {
		unit new_unit(gameinfo, **ui);
		if(ui == unit_cfg.begin()) {
			for(std::vector<unit>::iterator it =
							state_of_game.available_units.begin();
			    it != state_of_game.available_units.end(); ++it) {
				if(it->can_recruit()) {
					new_unit = *it;
					state_of_game.available_units.erase(it);
					break;
				}
			}
		}

		std::string gold = (*ui)->values["gold"];
		if(gold.empty())
			gold = "100";

		int ngold = ::atoi(gold.c_str());
		if(ui == unit_cfg.begin() && state_of_game.gold >= 0)
			ngold = state_of_game.gold;

		units.insert(std::pair<gamemap::location,unit>(
					     map.starting_position(new_unit.side()), new_unit));
		teams.push_back(team(**ui,ngold));

		//if there are additional starting units on this side
		std::vector<config*>& starting_units = (*ui)->children["unit"];
		for(std::vector<config*>::iterator su = starting_units.begin();
		    su != starting_units.end(); ++su) {
			unit new_unit(gameinfo,**su);
			const std::string& x = (*su)->values["x"];
			const std::string& y = (*su)->values["y"];

			const gamemap::location loc(**su);
			if(x.size() == 0 || y.size() == 0 || !map.on_board(loc)) {
				state_of_game.available_units.push_back(new_unit);
			} else {
				units.insert(std::pair<gamemap::location,unit>(loc,new_unit));
			}
		}
	}

	display gui(units,video,map,status,teams);
	const preferences::display_manager prefs_disp_manager(&gui);

	if(recorder.skipping() == false) {
		for(std::vector<config*>::iterator story_i = story.begin();
		    story_i != story.end(); ++story_i) {
			show_intro(gui,**story_i);
		}

		show_map_scene(gui,*level);
	}

	const std::string& music = level->values["music"];
	if(!music.empty()) {
		sound::play_music(music);
	}

	game_events::manager events_manager(*level,gui,map,units,
	                                    state_of_game,gameinfo);

	//find a list of 'items' (i.e. overlays) on the level, and add them
	std::vector<config*>& overlays = level->children["item"];
	for(std::vector<config*>::iterator overlay = overlays.begin();
	    overlay != overlays.end(); ++overlay) {
		gui.add_overlay(gamemap::location(**overlay),
		                (*overlay)->values["image"]);
	}

	for(units_map::iterator i = units.begin(); i != units.end(); ++i) {
		i->second.new_turn();
	}

	gui.scroll_to_tile(map.starting_position(1).x,map.starting_position(1).y,
	                   display::WARP);

	bool replaying = (recorder.empty() == false);

	std::cout << "starting main loop\n";
	for(bool first_time = true; true; first_time = false) {
		try {

			if(first_time) {
				update_locker lock_display(gui,recorder.skipping());
				game_events::fire("start");
				gui.draw();
			}

			gui.invalidate_game_status();

			for(std::vector<team>::iterator team_it = teams.begin();
			    team_it != teams.end(); ++team_it) {
				const int player_number = (team_it - teams.begin()) + 1;

				calculate_healing(gui,map,units,player_number);

				//scroll the map to the leader
				const units_map::iterator leader =
				                   find_leader(units,player_number);

				if(leader != units.end() && !recorder.skipping()) {
					gui.scroll_to_tile(leader->first.x,leader->first.y);
				}

				if(replaying) {
					replaying = do_replay(gui,map,gameinfo,units,teams,
					                      player_number,status,state_of_game);
				}

				if(!replaying && team_it->is_human()) {

					if(first_time && team_it == teams.begin() &&
					   level->values["objectives"].empty() == false) {
						dialogs::show_objectives(gui,*level);
					}

					play_turn(gameinfo,state_of_game,status,terrain_config,
					          level, video, key, gui, events_manager, map,
							  teams, player_number, units);

					if(game_config::debug)
						display::clear_debug_highlights();

				} else if(!replaying) {
					ai::do_move(gui,map,gameinfo,units,teams,
					            player_number,status);

					gui.invalidate_unit();
					gui.invalidate_game_status();
					gui.invalidate_all();
					gui.draw();
					SDL_Delay(1000);
				}

				for(units_map::iterator uit = units.begin();
				    uit != units.end(); ++uit) {
					if(uit->second.side() == player_number)
						uit->second.end_turn();
				}

				game_events::pump();

				const int victory = check_victory(units);
				if(victory > 1) {
					throw end_level_exception(DEFEAT);
				} else if(victory == 1) {
					throw end_level_exception(VICTORY);
				}
			}

			//time has run out
			if(!status.next_turn()) {
				game_events::fire("time over");
				throw end_level_exception(DEFEAT);
			}

			std::stringstream event_stream;
			event_stream << "turn " << status.turn();

			{
				update_locker lock_display(gui,recorder.skipping());
				game_events::fire(event_stream.str());
			}

			std::map<int,int> expenditure;
			for(units_map::iterator i = units.begin();
			    i != units.end(); ++i) {
				i->second.new_turn();
				expenditure[i->second.side()]++;
			}

			int team_num = 1;
			for(std::vector<team>::iterator it = teams.begin();
			    it != teams.end(); ++it, ++team_num) {
				it->new_turn();
				it->spend_gold(expenditure[team_num]);
			}
		} catch(end_level_exception& end_level) {

			if(end_level.result == QUIT || end_level.result == REPLAY) {
				return end_level.result;
			} else if(end_level.result == DEFEAT) {
				try {
					game_events::fire("defeat");
				} catch(end_level_exception&) {
				}

				gui::show_dialog(gui,NULL,
				                 string_table["defeat_heading"],
				                 string_table["defeat_message"],
				                 gui::OK_ONLY);
				return DEFEAT;
			} else if(end_level.result == VICTORY) {
				try {
					game_events::fire("victory");
				} catch(end_level_exception&) {
				}

				//add all the units that survived the scenario
				for(std::map<gamemap::location,unit>::iterator un =
				    units.begin(); un != units.end(); ++un) {
					if(un->second.side() == 1) {
						un->second.new_level();
						state_of_game.available_units.
						                push_back(un->second);
					}
				}

				const int remaining_gold = teams[0].gold();
				const int finishing_bonus_per_turn =
				      map.towers().size()*game_config::tower_income;
				const int turns_left = status.number_of_turns() - status.turn();
				const int finishing_bonus = end_level.gold_bonus ?
				              (finishing_bonus_per_turn * turns_left) : 0;
				state_of_game.gold = (remaining_gold+finishing_bonus)/2;

				gui::show_dialog(gui,NULL,string_table["victory_heading"],
				                 string_table["victory_message"],gui::OK_ONLY);
				std::stringstream report;
				report << string_table["remaining_gold"] << ": "
				       << remaining_gold << "\n";
				if(end_level.gold_bonus) {
					report << string_table["early_finish_bonus"] << ": "
					       << finishing_bonus_per_turn
						   << " " << string_table["per_turn"] << "\n"
					       << string_table["turns_finished_early"] << ": "
					       << turns_left << "\n"
					       << string_table["bonus"] << ": "
						   << finishing_bonus << "\n"
					       << string_table["gold"] << ": "
						   << (remaining_gold+finishing_bonus);
				}

				report << "\n" << string_table["fifty_percent"] << "\n"
					   << string_table["retained_gold"] << ": "
					   << state_of_game.gold;

				gui::show_dialog(gui,NULL,"",report.str(),gui::OK_ONLY);
				return VICTORY;
			}
		} //end catch
		catch(replay::error& e) {
			gui::show_dialog(gui,NULL,"","The file you loaded is corrupt "
			        "or from a different version of the game",gui::OK_ONLY);
			return QUIT;
		}

	} //end for(;;)

	return QUIT;
}