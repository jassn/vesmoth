#include "font.hpp"
#include "show_dialog.hpp"
#include "tooltips.hpp"
#include "sdl_utils.hpp"

#include <vector>

namespace {

bool rectangles_overlap(const SDL_Rect& a, const SDL_Rect& b)
{
	const bool xoverlap = a.x >= b.x && a.x < b.x + b.w ||
	                      b.x >= a.x && b.x < a.x + a.w;

	const bool yoverlap = a.y >= b.y && a.y < b.y + b.h ||
	                      b.y >= a.y && b.y < a.y + a.h;
	
	return xoverlap && yoverlap;
}

display* display_ = NULL;

struct tooltip
{
	tooltip(const SDL_Rect& r, const std::string& msg) : rect(r), message(msg)
	{
		gui::text_to_lines(message,60);
	}
	SDL_Rect rect;
	std::string message;
};

static const int font_size = 12;

std::vector<tooltip> tips;
std::vector<tooltip>::const_iterator current_tooltip = tips.end();

int tooltip_handle = 0;

SDL_Rect current_rect;
SDL_Surface* current_background = NULL;

SDL_Rect get_text_size(const std::string& msg)
{
	SDL_Rect area = {0,0,10000,10000};
	return font::draw_text(NULL,area,font_size,font::BLACK_COLOUR,msg,0,0);
}

void clear_tooltip()
{
	if(tooltip_handle != 0) {
		font::remove_floating_label(tooltip_handle);
		tooltip_handle = 0;
	}
}

void show_tooltip(const tooltip& tip)
{
	if(display_ == NULL) {
		return;
	}

	clear_tooltip();

	const SDL_Color bgcolour = {0,0,0,128};
	SDL_Rect area = display_->screen_area();
	tooltip_handle = font::add_floating_label(tip.message,font_size,font::NORMAL_COLOUR,
	                                          0,0,0,0,-1,area,font::LEFT_ALIGN,&bgcolour,10);

	SDL_Rect rect = font::get_floating_label_rect(tooltip_handle);

	//see if there is enough room to fit it above the tip area
	if(tip.rect.y > rect.h) {
		rect.y = tip.rect.y - rect.h;
	} else {
		rect.y = tip.rect.y + tip.rect.h;
	}

	rect.x = tip.rect.x;
	if(rect.x < 0) {
		rect.x = 0;
	} else if(rect.x + rect.w > area.w) {
		rect.x = area.w - rect.w;
	}

	font::move_floating_label(tooltip_handle,rect.x,rect.y);
}

}

namespace tooltips {

manager::manager(display& disp)
{
	clear_tooltips();
	display_ = &disp;
}

manager::~manager()
{
	clear_tooltips();
	display_ = NULL;
}

void clear_tooltips()
{
	clear_tooltip();
	tips.clear();
	current_tooltip = tips.end();
}

void clear_tooltips(const SDL_Rect& rect)
{
	clear_tooltip();
	for(std::vector<tooltip>::iterator i = tips.begin(); i != tips.end(); ) {
		if(rectangles_overlap(i->rect,rect)) {
			i = tips.erase(i);
			current_tooltip = tips.end();
		} else {
			++i;
		}
	}
}

void add_tooltip(const SDL_Rect& rect, const std::string& message)
{
	for(std::vector<tooltip>::iterator i = tips.begin(); i != tips.end(); ++i) {
		if(rectangles_overlap(i->rect,rect)) {
			*i = tooltip(rect,message);
			return;
		}
	}

	tips.push_back(tooltip(rect,message));
	current_tooltip = tips.end();
}

void process(int mousex, int mousey, bool lbutton)
{
	for(std::vector<tooltip>::const_iterator i = tips.begin(); i != tips.end(); ++i) {
		if(mousex > i->rect.x && mousey > i->rect.y &&
		   mousex < i->rect.x + i->rect.w && mousey < i->rect.y + i->rect.h) {
			if(current_tooltip != i) {
				show_tooltip(*i);
				current_tooltip = i;
			}

			return;
		}
	}

	clear_tooltip();
	current_tooltip = tips.end();
}

SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& text,
                   int x, int y, SDL_Surface* bg)
{
	return font::draw_text(gui,area,size,colour,text,x,y,bg,true);
}

}
