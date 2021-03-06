#ifndef CLIPBOARD_HPP_INCLUDED
#define CLIPBOARD_HPP_INCLUDED

#include <string>
#include "SDL.h"

void copy_to_clipboard(const std::string& text);
std::string copy_from_clipboard();

#ifdef _X11
void handle_system_event(const SDL_Event& ev);
#endif

#endif
