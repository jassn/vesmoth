/* $Id: game_config.hpp 28 2003-09-19 10:21:25Z zas $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef GAME_CONFIG_H_INCLUDED
#define GAME_CONFIG_H_INCLUDED

#include <string>

namespace game_config
{
	extern const int unit_cost;
	extern const int base_income;
	extern const int tower_income;
	extern const int heal_amount;
	extern const int healer_heals_per_turn;
	extern const int cure_amount;
	extern const int curer_heals_per_turn;
	extern const int recall_cost;
	extern const std::string version;

	extern bool debug;
}

#endif