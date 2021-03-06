/* $Id: mouse.hpp,v 1.10 2003/12/23 22:45:30 uid66289 Exp $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MOUSE_H_INCLUDED
#define MOUSE_H_INCLUDED

namespace gui {

void scroll_inc();
void scroll_dec();

void scroll_reduce();

int scrollamount();

void scroll_reset();

}

#endif
