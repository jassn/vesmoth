/* $Id: key.hpp,v 1.11 2003/12/23 22:45:30 uid66289 Exp $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef KEY_HPP_INCLUDED
#define KEY_HPP_INCLUDED

#include "SDL.h"

//object which keeps track of all the keys on the keyboard, and
//whether any key is pressed or not can be found by using its
//operator[]. Note though that it is generally better to use
//key events to see when keys are pressed rather than poll using
//this object.
class CKey {
public:
	CKey();

	int operator[](int);
	void SetEnabled(bool enable);
private:
	Uint8 *key_list;
	bool is_enabled;
};

#endif
