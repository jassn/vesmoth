/* $Id: mouse.cpp,v 1.10 2003/12/23 22:45:30 uid66289 Exp $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "mouse.hpp"

namespace {

int scrollamount_ = 0;

}

namespace gui {

// Current scroll pending
int scrollamount()	{ return scrollamount_; }

// Reset scroll
void scroll_reset()	{ scrollamount_ = 0; }

// Scrolling UP or LEFT
void scroll_dec() { --scrollamount_; }

// Scrolling DOWN or RIGHT
void scroll_inc()  { ++scrollamount_; }
	
// Reducing scroll amount
void scroll_reduce() { 
	if(scrollamount_ > 0) {
		--scrollamount_;
	} else {
		++scrollamount_;
	}
}

}
