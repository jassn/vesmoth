/* $Id: dialogs.cpp,v 1.42 2004/06/24 15:13:20 Sirp Exp $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "dialogs.hpp"
#include "events.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "language.hpp"
#include "log.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "util.hpp"
#include "widgets/file_chooser.hpp"
#include "widgets/progressbar.hpp"

#include <cstdio>
#include <map>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

namespace dialogs
{

void advance_unit(const game_data& info,
				  const gamemap& map,
                  std::map<gamemap::location,unit>& units,
                  gamemap::location loc,
                  display& gui, bool random_choice)
{
	std::map<gamemap::location,unit>::iterator u = units.find(loc);
	if(u == units.end() || u->second.advances() == false)
		return;

	std::cerr << "advance_unit: " << u->second.type().name() << "\n";

	const std::vector<std::string>& options = u->second.type().advances_to();

	std::vector<std::string> lang_options;

	std::vector<unit> sample_units;
	for(std::vector<std::string>::const_iterator op = options.begin(); op != options.end(); ++op) {
		sample_units.push_back(::get_advanced_unit(info,units,loc,*op));
		const unit_type& type = sample_units.back().type();
		lang_options.push_back("&" + type.image() + "," + type.language_name());
	}

	const config::child_list& mod_options = u->second.get_modification_advances();

	for(config::child_list::const_iterator mod = mod_options.begin(); mod != mod_options.end(); ++mod) {
		sample_units.push_back(::get_advanced_unit(info,units,loc,u->second.type().name()));
		sample_units.back().add_modification("advance",**mod);
		const unit_type& type = sample_units.back().type();
		lang_options.push_back("&" + type.image() + "," + translate_string_default("advance_" + (**mod)["id"],(**mod)["description"]));
	}

	std::cerr << "options: " << options.size() << "\n";

	int res = 0;

	if(lang_options.empty()) {
		return;
	} else if(random_choice) {
		res = rand()%lang_options.size();
	} else if(lang_options.size() > 1) {

		const events::event_context dialog_events_context;
		unit_preview_pane unit_preview(gui,&map,sample_units);
		std::vector<gui::preview_pane*> preview_panes;
		preview_panes.push_back(&unit_preview);

		res = gui::show_dialog(gui,NULL,string_table["advance_unit_heading"],
		                       string_table["advance_unit_message"],
		                       gui::OK_ONLY, &lang_options, &preview_panes);
	}

	recorder.choose_option(res);

	std::cerr << "animating advancement...\n";
	animate_unit_advancement(info,units,loc,gui,size_t(res));
}

bool animate_unit_advancement(const game_data& info,unit_map& units, gamemap::location loc, display& gui, size_t choice)
{
	const command_disabler cmd_disabler(&gui);
	
	std::map<gamemap::location,unit>::iterator u = units.find(loc);
	if(u == units.end() || u->second.advances() == false) {
		return false;
	}

	const std::vector<std::string>& options = u->second.type().advances_to();
	const config::child_list& mod_options = u->second.get_modification_advances();

	if(choice >= options.size() + mod_options.size()) {
		return false;
	}
	
	//when the unit advances, it fades to white, and then switches to the
	//new unit, then fades back to the normal colour
	
	if(!gui.update_locked()) {
		for(double intensity = 1.0; intensity >= 0.0; intensity -= 0.05) {
			gui.set_advancing_unit(loc,intensity);
			events::pump();
			gui.draw(false);
			gui.update_display();
			SDL_Delay(30);
		}
	}

	const std::string& chosen_unit = choice < options.size() ? options[choice] : u->second.type().name();
	::advance_unit(info,units,loc,chosen_unit);

	u = units.find(loc);
	if(u != units.end() && choice >= options.size()) {
		u->second.add_modification("advance",*mod_options[choice - options.size()]);
	}

	gui.invalidate_unit();

	if(!gui.update_locked()) {
		for(double intensity = 0.0; intensity <= 1.0; intensity += 0.05) {
			gui.set_advancing_unit(loc,intensity);
			events::pump();
			gui.draw(false);
			gui.update_display();
			SDL_Delay(30);
		}
	}

	gui.set_advancing_unit(gamemap::location(),0.0);

	gui.invalidate_all();
	gui.draw();

	return true;
}

void show_objectives(display& disp, config& level_info)
{
	static const std::string no_objectives(string_table["no_objectives"]);
	const std::string& id = level_info["id"];
	const std::string& lang_name = string_table[id];
	const std::string& name = lang_name.empty() ? level_info["name"] :
	                                              lang_name;
	const std::string& lang_objectives = string_table[id + "_objectives"];

	const std::string& objectives = lang_objectives.empty() ?
	        level_info["objectives"] : lang_objectives;
	gui::show_dialog(disp, NULL, "", font::LARGE_TEXT + name + "\n" +
	         (objectives.empty() ? no_objectives : objectives), gui::OK_ONLY);

}

int get_save_name(display & disp,const std::string& caption, const std::string& message,
				  std::string* name, gui::DIALOG_TYPE dialog_type)
{
    int overwrite=0;
    int res=0;
    do {
        res = gui::show_dialog(disp,NULL,"",caption,dialog_type,NULL,NULL,message,name);
            if (res == 0 && save_game_exists(*name))
                overwrite = gui::show_dialog(disp,NULL,"",
                    string_table["save_confirm_overwrite"],gui::YES_NO);
        else overwrite = 0;
    } while ((res==0)&&(overwrite!=0));
	return res;
}

//a class to handle deleting a saved game
namespace {

class delete_save : public gui::dialog_button_action
{
public:
	delete_save(display& disp, std::vector<save_info>& saves, std::vector<config*>& save_summaries) : disp_(disp), saves_(saves), summaries_(save_summaries) {}
private:
	gui::dialog_button_action::RESULT button_pressed(int menu_selection);

	display& disp_;
	std::vector<save_info>& saves_;
	std::vector<config*>& summaries_;
};

gui::dialog_button_action::RESULT delete_save::button_pressed(int menu_selection)
{
	const size_t index = size_t(menu_selection);
	if(index < saves_.size()) {

		//see if we should ask the user for deletion confirmation
		if(preferences::ask_delete_saves()) {
			std::vector<gui::check_item> options;
			options.push_back(gui::check_item(string_table["dont_ask_again"],false));

			const int res = gui::show_dialog(disp_,NULL,"",string_table["really_delete_save"],gui::YES_NO,
			                                 NULL,NULL,"",NULL,NULL,&options);

			//see if the user doesn't want to be asked this again
			assert(options.empty() == false);
			if(options.front().checked) {
				preferences::set_ask_delete_saves(false);
			}

			if(res != 0) {
				return gui::dialog_button_action::NO_EFFECT;
			}
		}

		//delete the file
		delete_game(saves_[index].name);

		//remove it from the list of saves
		saves_.erase(saves_.begin() + index);

		if(index < summaries_.size()) {
			summaries_.erase(summaries_.begin() + index);
		}

		return gui::dialog_button_action::DELETE_ITEM;
	} else {
		return gui::dialog_button_action::NO_EFFECT;
	}
}

static const SDL_Rect save_preview_pane_area = {-200,-400,200,400};
static const int save_preview_border = 10;

class save_preview_pane : public gui::preview_pane
{
public:
	save_preview_pane(display& disp, const config& game_config, gamemap* map, const game_data& data,
	                  const std::vector<save_info>& info, const std::vector<config*>& summaries)
		: gui::preview_pane(disp), game_config_(&game_config), map_(map), data_(&data), info_(&info), summaries_(&summaries), index_(0)
	{
		set_location(save_preview_pane_area);
	}

	void draw();
	void set_selection(int index) {
		index_ = index;
		set_dirty();
	}

	bool left_side() const { return true; }

private:
	const config* game_config_;
	gamemap* map_;
	const game_data* data_;
	const std::vector<save_info>* info_;
	const std::vector<config*>* summaries_;
	int index_;
	std::map<std::string,shared_sdl_surface> map_cache_;
};

void save_preview_pane::draw()
{
	if(!dirty()) {
		return;
	}

	bg_restore();

	if(index_ < 0 || index_ >= int(summaries_->size()) || info_->size() != summaries_->size()) {
		return;
	}

	set_dirty(false);

	static int n = 0;

	const config& summary = *(*summaries_)[index_];

	SDL_Surface* const screen = disp().video().getSurface();

	const SDL_Rect area = { location().x+save_preview_border, location().y+save_preview_border,
	                        location().w-save_preview_border*2, location().h-save_preview_border*2 };
	SDL_Rect clip_area = area;
	const clip_rect_setter clipper(screen,clip_area);

	int ypos = area.y;

	const game_data::unit_type_map::const_iterator leader = data_->unit_types.find(summary["leader"]);
	if(leader != data_->unit_types.end()) {
		const scoped_sdl_surface image(image::get_image(leader->second.image(),image::UNSCALED));
		if(image != NULL) {
			SDL_Rect image_rect = {area.x,area.y,image->w,image->h};
			ypos += image_rect.h + save_preview_border;

			SDL_BlitSurface(image,NULL,screen,&image_rect);
		}
	}


	std::string map_data = summary["map_data"];
	if(map_data.empty()) {
		const config* const scenario = game_config_->find_child(summary["campaign_type"],"id",summary["scenario"]);
		if(scenario != NULL && scenario->find_child("side","shroud","yes") == NULL) {
			map_data = (*scenario)["map_data"];
			if(map_data.empty() && (*scenario)["map"].empty() == false) {
				try {
					map_data = read_map((*scenario)["map"]);
				} catch(io_exception& e) {
					std::cerr << "could not read map '" << (*scenario)["map"] << "': " << e.what() << "\n";
				}
			}
		}
	}

	SDL_Surface* map_surf = NULL;

	if(map_data.empty() == false) {
		const std::map<std::string,shared_sdl_surface>::const_iterator itor = map_cache_.find(map_data);
		if(itor != map_cache_.end()) {
			map_surf = itor->second;
		} else if(map_ != NULL) {
			try {
				map_->read(map_data);

				map_surf = image::getMinimap(100,100,*map_,0,NULL);
				if(map_surf != NULL) {
					map_cache_.insert(std::pair<std::string,shared_sdl_surface>(map_data,shared_sdl_surface(map_surf)));
				}
			} catch(gamemap::incorrect_format_exception&) {
			}
		}
	}

	if(map_surf != NULL) {
		SDL_Rect map_rect = {area.x + area.w - map_surf->w,area.y,map_surf->w,map_surf->h};
		ypos = maximum<int>(ypos,map_rect.y + map_rect.h + save_preview_border);
		SDL_BlitSurface(map_surf,NULL,screen,&map_rect);
	}

	char time_buf[256];
	const size_t res = strftime(time_buf,sizeof(time_buf),string_table["date_format"].c_str(),localtime(&((*info_)[index_].time_modified)));
	if(res == 0) {
		time_buf[0] = 0;
	}

	std::stringstream str;

	// escape all special characters in filenames
	std::string name = (*info_)[index_].name;
	str << font::BOLD_TEXT << config::escape(name) << "\n" << time_buf;

	if(summary["corrupt"] == "yes") {
		str << "\n" << string_table["save_invalid"];
	} else if(summary["campaign_type"] != "") {
		str << "\n";
			
		const std::string& campaign_type = summary["campaign_type"];
		if(campaign_type == "scenario") {
			str << translate_string("campaign_button");
		} else if(campaign_type == "multiplayer") {
			str << translate_string("multiplayer_button");
		} else if(campaign_type == "tutorial") {
			str << translate_string("tutorial_button");
		} else {
			str << translate_string(campaign_type);
		}

		str << "\n";
			
		if(summary["snapshot"] == "no" && summary["replay"] == "yes") {
			str << translate_string("replay");
		} else if(summary["turn"] != "") {
			str << translate_string("turn") << " " << summary["turn"];
		} else {
			str << string_table["scenario_start"];
		}

		str << "\n" << translate_string("difficulty") << ": " << translate_string(summary["difficulty"]);
	}

	font::draw_text(&disp(),area,12,font::NORMAL_COLOUR,str.str(),area.x,ypos,NULL,true);
}

} //end anon namespace

std::string load_game_dialog(display& disp, const config& game_config, const game_data& data, bool* show_replay)
{
	std::vector<save_info> games = get_saves_list();

	if(games.empty()) {
		gui::show_dialog(disp,NULL,
		                 string_table["no_saves_heading"],
						 string_table["no_saves_message"],
		                 gui::OK_ONLY);
		return "";
	}

	std::vector<size_t> no_summary; //all items that aren't in the summary table
	std::vector<config*> summaries;
	std::vector<save_info>::const_iterator i;
	for(i = games.begin(); i != games.end(); ++i) {
		config& cfg = save_summary(i->name);
		if(cfg["campaign_type"].empty() && cfg["corrupt"] != "yes" || lexical_cast<int>(cfg["mod_time"]) != static_cast<int>(i->time_modified)) {
			no_summary.push_back(i - games.begin());
		}

		summaries.push_back(&cfg);
	}

	delete_save save_deleter(disp,games,summaries);
	gui::dialog_button delete_button(&save_deleter,string_table["delete_save"]);
	std::vector<gui::dialog_button> buttons;
	buttons.push_back(delete_button);

	bool generate_summaries = !no_summary.empty();

	//if there are more than 5 saves without a summary, it may take a substantial
	//amount of time to convert them over. Ask the user if they want to do this
	if(no_summary.size() > 5) {

		if(preferences::cache_saves() == preferences::CACHE_SAVES_NEVER) {
			generate_summaries = false;
		} else if(preferences::cache_saves() == preferences::CACHE_SAVES_ALWAYS) {
			generate_summaries = true;
		} else {
			std::string caption = "import_saves_caption", message = "import_old_saves";

			//if there are already some cached games, then we assume that the user is importing new
			//games, and it's not a total import, so tailor the message accordingly
			if(no_summary.size() < games.size()) {
				message = "import_saves";
			}

			std::vector<gui::check_item> options;
			options.push_back(gui::check_item(string_table["dont_ask_again"],false));
			const int res = gui::show_dialog(disp,NULL,string_table[caption],string_table[message],
			                                 gui::YES_NO,NULL,NULL,"",NULL,NULL,&options);

			generate_summaries = res == 0;
			if(options.front().checked) {
				preferences::set_cache_saves(generate_summaries ? preferences::CACHE_SAVES_ALWAYS : preferences::CACHE_SAVES_NEVER);
			}
		}
	}

	const events::event_context context;

	if(generate_summaries) {
		gui::progress_bar bar(disp);
		const SDL_Rect bar_area = {disp.x()/2 - 100, disp.y()/2 - 20, 200, 40};
		bar.set_location(bar_area);

		for(std::vector<size_t>::const_iterator s = no_summary.begin(); s != no_summary.end(); ++s) {
			bar.set_progress_percent(((s - no_summary.begin())*100)/no_summary.size());
			events::raise_draw_event();
			events::pump();
			disp.update_display();

			log_scope("load");
			game_state state;

			config& summary = save_summary(games[*s].name);

			try {
				summary["mod_time"] = str_cast(lexical_cast<int>(games[*s].time_modified));
				load_game(data,games[*s].name,state);
				extract_summary_data_from_save(state,summary);
			} catch(io_exception&) {
				summary["corrupt"] = "yes";
				std::cerr << "save '" << games[*s].name << "' could not be loaded (io_exception)\n";
			} catch(config::error&) {
				summary["corrupt"] = "yes";
				std::cerr << "save '" << games[*s].name << "' could not be loaded (config parse error)\n";
			} catch(gamestatus::load_game_failed&) {
				summary["corrupt"] = "yes";
				std::cerr << "save '" << games[*s].name << "' could not be loaded (load_game_failed exception)\n";
			}
		}

		write_save_index();
	}

	std::vector<std::string> items;
	for(i = games.begin(); i != games.end(); ++i) {
		std::string name = i->name;
		name.resize(minimum<size_t>(name.size(),40));

		items.push_back(name);
	}

	gamemap map_obj(game_config,"");

	std::vector<gui::preview_pane*> preview_panes;
	save_preview_pane save_preview(disp,game_config,&map_obj,data,games,summaries);
	preview_panes.push_back(&save_preview);

	//create an option for whether the replay should be shown or not
	std::vector<gui::check_item> options;

	if(show_replay != NULL)
		options.push_back(gui::check_item(string_table["show_replay"],false));

	const int res = gui::show_dialog(disp,NULL,
					 string_table["load_game_heading"],
					 string_table["load_game_message"],
			         gui::OK_CANCEL,&items,&preview_panes,"",NULL,NULL,&options,-1,-1,NULL,&buttons);

	if(res == -1)
		return "";

	if(show_replay != NULL) {
		*show_replay = options.front().checked;

		const config& summary = *summaries[res];
		if(summary["replay"] == "yes" && summary["snapshot"] == "no") {
			*show_replay = true;
		}
	}

	return games[res].name;
}

void unit_speak(const config& message_info, display& disp, const unit_map& units)
{
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.matches_filter(message_info)) {
			disp.scroll_to_tile(i->first.x,i->first.y);
			const scoped_sdl_surface surface(image::get_image(i->second.type().image_profile(),image::UNSCALED));
			gui::show_dialog(disp,surface,i->second.underlying_description(),message_info["message"],gui::MESSAGE);
		}
	}
}


int show_file_chooser_dialog(display &disp, std::string &filename,
							 const std::string title,
							 int xloc, int yloc) {
	const events::event_context dialog_events_context;
	const gui::dialog_manager manager;
	const events::resize_lock prevent_resizing;

	CVideo& screen = disp.video();
	SDL_Surface* const scr = screen.getSurface();

	const int width = 400;
	const int height = 400;
	const int left_padding = 10;
	const int right_padding = 10;
	const int top_padding = 10;
	const int bot_padding = 10;

	// If not both locations were supplied, put the dialog in the middle
	// of the screen.
	if (yloc <= -1 || xloc <= -1) {
		xloc = scr->w / 2 - width / 2;
		yloc = scr->h / 2 - height / 2;
	}
	std::vector<gui::button*> buttons_ptr;
	gui::button ok_button_(disp, string_table["ok_button"]);
	gui::button cancel_button_(disp, string_table["cancel_button"]);
	buttons_ptr.push_back(&ok_button_);
	buttons_ptr.push_back(&cancel_button_);
	surface_restorer restorer;
	gui::draw_dialog(xloc, yloc, width, height, disp, title, NULL, &buttons_ptr, &restorer);

	gui::file_chooser fc(disp, filename);
	fc.set_location(xloc + left_padding, yloc + top_padding);
	fc.set_width(width - left_padding - right_padding);
	fc.set_height(height - top_padding - bot_padding);
	fc.set_dirty(true);
	
	events::raise_draw_event();
	screen.flip();
	disp.invalidate_all();

	CKey key;
	for (;;) {
		events::pump();
		events::raise_process_event();
		events::raise_draw_event();
		if (fc.choice_made()) {
			filename = fc.get_choice();
			return 0; // We know that the OK button is on index 0.
		}
		if (key[SDLK_ESCAPE]) {
			// Escape quits from the dialog.
			return -1;
		}
		for (std::vector<gui::button*>::iterator button_it = buttons_ptr.begin();
			 button_it != buttons_ptr.end(); button_it++) {
			if ((*button_it)->pressed()) {
				// Return the index of the pressed button.
				filename = fc.get_choice();
				return button_it - buttons_ptr.begin();
			}
		}
		screen.flip();
		SDL_Delay(10);
	}
}

namespace {
	static const SDL_Rect unit_preview_size = {-200,-370,200,370};
	static const SDL_Rect weaponless_unit_preview_size = {-190,-140,190,140};
	static const int unit_preview_border = 10;
}

unit_preview_pane::unit_preview_pane(display& disp, const gamemap* map, const unit& u, TYPE type, bool on_left_side)
                                        : gui::preview_pane(disp), details_button_(disp,translate_string("profile"),gui::button::TYPE_PRESS,"lite_small",gui::button::MINIMUM_SPACE),
										  map_(map), units_(&unit_store_), index_(0), left_(on_left_side),
										  weapons_(type == SHOW_ALL)
{
	set_location(weapons_ ? unit_preview_size : weaponless_unit_preview_size);
	unit_store_.push_back(u);
}

unit_preview_pane::unit_preview_pane(display& disp, const gamemap* map, const std::vector<unit>& units, TYPE type, bool on_left_side)
                                        : gui::preview_pane(disp), details_button_(disp,translate_string("profile"),gui::button::TYPE_PRESS,"lite_small",gui::button::MINIMUM_SPACE),
										  map_(map), units_(&units), index_(0), left_(on_left_side),
										  weapons_(type == SHOW_ALL)
{
	set_location(unit_preview_size);
}

bool unit_preview_pane::show_above() const
{
	return !weapons_;
}

bool unit_preview_pane::left_side() const
{
	return left_;
}

void unit_preview_pane::set_selection(int index)
{
	index = minimum<int>(int(units_->size()-1),index);
	if(index != index_ && index >= 0) {
		index_ = index;
		set_dirty();
		if(map_ != NULL) {
			details_button_.set_dirty();
		}
	}
}

void unit_preview_pane::draw()
{
	if(!dirty()) {
		return;
	}

	bg_restore();

	if(index_ < 0 || index_ >= int(units_->size())) {
		return;
	}

	const unit& u = (*units_)[index_];

	set_dirty(false);

	const bool right_align = left_side();

	SDL_Surface* const screen = disp().video().getSurface();

	const SDL_Rect area = { location().x+unit_preview_border, location().y+unit_preview_border,
	                        location().w-unit_preview_border*2, location().h-unit_preview_border*2 };
	SDL_Rect clip_area = area;
	const clip_rect_setter clipper(screen,clip_area);

	scoped_sdl_surface unit_image(image::get_image(u.type().image(),image::UNSCALED));
	if(left_side() == false && unit_image != NULL) {
		unit_image.assign(image::reverse_image(unit_image));
	}

	SDL_Rect image_rect = {area.x,area.y,0,0};

	if(unit_image != NULL) {
		SDL_Rect rect = {right_align ? area.x : area.x + area.w - unit_image->w,area.y,unit_image->w,unit_image->h};
		SDL_BlitSurface(unit_image,NULL,screen,&rect);
		image_rect = rect;
	}

	SDL_Rect description_rect = {image_rect.x,image_rect.y+image_rect.h,0,0};

	if(u.description().empty() == false) {
		std::stringstream desc;
		desc << font::BOLD_TEXT << u.description();
		const std::string description = desc.str();
		description_rect = font::text_area(description,12);
		description_rect = font::draw_text(&disp(),area,12,font::NORMAL_COLOUR,desc.str(),right_align ? image_rect.x : image_rect.x + image_rect.w - description_rect.w,image_rect.y+image_rect.h);
	}

	//place the 'unit profile' button
	if(map_ != NULL) {
		const SDL_Rect button_loc = {right_align ? area.x : area.x + area.w - details_button_.location().w,
		                             image_rect.y + image_rect.h + description_rect.h,
		                             details_button_.location().w,details_button_.location().h};
		details_button_.set_location(button_loc);
	}

	std::stringstream details;
	details << font::BOLD_TEXT << u.type().language_name()
			<< "\n" << font::SMALL_TEXT << "(" << string_table["level"] << " "
			<< u.type().level() << ")\n"
			<< translate_string(unit_type::alignment_description(u.type().alignment())) << "\n"
			<< u.traits_description() << " \n";

	const std::vector<std::string>& abilities = u.type().abilities();
	for(std::vector<std::string>::const_iterator a = abilities.begin(); a != abilities.end(); ++a) {
		details << translate_string_default("ability_" + *a, *a);
		if(a+1 != abilities.end()) {
			details << ",";
		}
	}

	details << " \n";

	//display in green/white/red depending on hitpoints
	if(u.hitpoints() <= u.max_hitpoints()/3)
		details << font::BAD_TEXT;
	else if(u.hitpoints() > 2*(u.max_hitpoints()/3))
		details << font::GOOD_TEXT;

	details << string_table["hp"] << ": " << u.hitpoints()
			<< "/" << u.max_hitpoints() << "\n";
	
	if(u.can_advance() == false) {
		details << string_table["xp"] << ": " << u.experience() << "/-";
	} else {
		//if killing a unit the same level as us would level us up,
		//then display in green
		if(u.max_experience() - u.experience() < game_config::kill_experience) {
			details << font::GOOD_TEXT;
		}

		details << string_table["xp"] << ": " << u.experience() << "/" << u.max_experience();
	}
	
	if(weapons_) {
		details << "\n"
				<< string_table["moves"] << ": " << u.movement_left() << "/"
				<< u.total_movement()
				<< "\n";

		const std::vector<attack_type>& attacks = u.attacks();
		for(std::vector<attack_type>::const_iterator at_it = attacks.begin();
		    at_it != attacks.end(); ++at_it) {

			const std::string& lang_weapon = string_table["weapon_name_" + at_it->name()];
			const std::string& lang_type = string_table["weapon_type_" + at_it->type()];
			const std::string& lang_special = string_table["weapon_special_" + at_it->special()];
			details << "\n"
					<< (lang_weapon.empty() ? at_it->name():lang_weapon) << " ("
					<< (lang_type.empty() ? at_it->type():lang_type) << ")\n"
					<< (lang_special.empty() ? at_it->special():lang_special)<<"\n"
					<< at_it->damage() << "-" << at_it->num_attacks() << " -- "
			        << (at_it->range() == attack_type::SHORT_RANGE ?
			            string_table["short_range"] :
						string_table["long_range"]);
	
			if(at_it->hexes() > 1) {
				details << " (" << at_it->hexes() << ")";
			}
		}
	}
	
	const std::string text = details.str();
	const SDL_Rect& text_area = font::text_area(text,12);
	
	const std::vector<std::string> lines = config::split(text,'\n');

	SDL_Rect cur_area = area;

	if(weapons_) {
		cur_area.y += image_rect.h + description_rect.h + details_button_.location().h;
	}

	for(std::vector<std::string>::const_iterator line = lines.begin(); line != lines.end(); ++line) {
		int xpos = cur_area.x;
		if(right_align && !weapons_) {
			const SDL_Rect& line_area = font::text_area(*line,12);
			xpos = cur_area.x + cur_area.w - line_area.w;
		}

		cur_area = font::draw_text(&disp(),location(),12,font::NORMAL_COLOUR,*line,xpos,cur_area.y);
		cur_area.y += cur_area.h;
	}
}

void unit_preview_pane::process()
{
	if(map_ != NULL && details_button_.pressed() && index_ >= 0 && index_ < int(units_->size())) {

		show_unit_description(disp(),*map_,(*units_)[index_]);
	}
}

void show_unit_description(display& disp, const gamemap& map, const unit& u)
{
	const std::string description = u.unit_description()
	                                + "\n\n" + string_table["see_also"];

	std::vector<std::string> options;

	options.push_back(string_table["terrain_info"]);
	options.push_back(string_table["attack_resistance"]);
	options.push_back(string_table["close_window"]);

	const scoped_sdl_surface profile(image::get_image(u.type().image_profile(),image::SCALED));

	const int res = gui::show_dialog(disp, profile, u.type().language_name(),
	                                 description,gui::MESSAGE, &options);
	if(res == 0) {
		show_unit_terrain_table(disp,map,u);
	} else if(res == 1) {
		show_unit_resistance(disp,u);
	}
}

void show_unit_resistance(display& disp, const unit& u)
{
	std::vector<std::string> items;
	items.push_back(string_table["attack_type"] + "," + string_table["attack_resistance"]);
	const std::map<std::string,std::string>& table = u.type().movement_type().damage_table();
	for(std::map<std::string,std::string>::const_iterator i = table.begin(); i != table.end(); ++i) {
		int resistance = 100 - atoi(i->second.c_str());

		//if resistance is less than 0, display in red
		const char prefix = resistance < 0 ? font::BAD_TEXT : font::NULL_MARKUP;

		const std::string& lang_weapon = string_table["weapon_type_" + i->first];
		const std::string& weap = lang_weapon.empty() ? i->first : lang_weapon;

		std::stringstream str;
		str << weap << "," << prefix << resistance << "%";
		items.push_back(str.str());
	}

	const events::event_context dialog_events_context;
	dialogs::unit_preview_pane unit_preview(disp,NULL,u);
	std::vector<gui::preview_pane*> preview_panes;
	preview_panes.push_back(&unit_preview);

	gui::show_dialog(disp,NULL,
	                 u.type().language_name(),
					 string_table["unit_resistance_table"],
					 gui::MESSAGE,&items,&preview_panes);
}

void show_unit_terrain_table(display& disp, const gamemap& map, const unit& u)
{
	std::vector<std::string> items;
	items.push_back(string_table["terrain"] + "," +
	                string_table["movement"] + "," +
					string_table["defense"]);

	const unit_type& type = u.type();
	const unit_movement_type& move_type = type.movement_type();
	const std::vector<gamemap::TERRAIN>& terrains = map.get_terrain_precedence();
	for(std::vector<gamemap::TERRAIN>::const_iterator t =
	    terrains.begin(); t != terrains.end(); ++t) {

		//exclude fog and shroud
		if(*t == gamemap::FOGGED || *t == gamemap::VOID_TERRAIN) {
			continue;
		}

		const terrain_type& info = map.get_terrain_info(*t);
		if(!info.is_alias()) {
			const std::string& name = map.terrain_name(*t);
			const std::string& lang_name = string_table[name];
			const int moves = move_type.movement_cost(map,*t);

			std::stringstream str;
			str << lang_name << ",";
			if(moves < 100)
				str << moves;
			else
				str << "--";

			const int defense = 100 - move_type.defense_modifier(map,*t);
			str << "," << defense << "%";

			items.push_back(str.str());
		}
	}

	const events::event_context dialog_events_context;
	dialogs::unit_preview_pane unit_preview(disp,NULL,u);
	std::vector<gui::preview_pane*> preview_panes;
	preview_panes.push_back(&unit_preview);

	gui::show_dialog(disp,NULL,u.type().language_name(),
					 string_table["terrain_info"],
					 gui::MESSAGE,&items,&preview_panes);
}

} //end namespace dialogs
