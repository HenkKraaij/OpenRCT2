/*****************************************************************************
* Copyright (c) 2014 Ted John
* OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
*
* This file is part of OpenRCT2.
*
* OpenRCT2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "addresses.h"
#include "audio.h"
#include "date.h"
#include "scenario.h"
#include "strings.h"
#include "sprites.h"
#include "widget.h"
#include "window.h"

enum {
	WIDX_BACKGROUND,
	WIDX_TITLEBAR,
	WIDX_CLOSE,
	WIDX_TABCONTENT,
	WIDX_TAB1,
	WIDX_TAB2,
	WIDX_TAB3,
	WIDX_TAB4,
	WIDX_TAB5,
	WIDX_SCENARIOLIST
};

static rct_widget window_scenarioselect_widgets[] = {
	{ WWT_FRAME,	0,	0,		609,	0,		333,	-1,						STR_NONE },				// panel / background
	{ WWT_CAPTION,	0,	1,		608,	1,		14,		STR_SELECT_SCENARIO,	STR_WINDOW_TITLE_TIP },	// title bar
	{ WWT_CLOSEBOX,	0,	597,	607,	2,		13,		824,					STR_CLOSE_WINDOW_TIP },	// close x button
	{ WWT_IMGBTN,	1,	0,		609,	50,		333,	-1,						STR_NONE },				// tab content panel
	{ WWT_TAB,		1,	3,		93,		17,		50,		0x200015BC,				STR_NONE },				// tab 1
	{ WWT_TAB,		1,	94,		184,	17,		50,		0x200015BC,				STR_NONE },				// tab 2
	{ WWT_TAB,		1,	185,	275,	17,		50,		0x200015BC,				STR_NONE },				// tab 3
	{ WWT_TAB,		1,	276,	366,	17,		50,		0x200015BC,				STR_NONE },				// tab 4
	{ WWT_TAB,		1,	367,	457,	17,		50,		0x200015BC,				STR_NONE },				// tab 5
	{ WWT_SCROLL,	1,	3,		433,	54,		329,	2,						STR_NONE },				// level list
	{ WIDGETS_END },
};

static void window_scenarioselect_init_tabs();

static void window_scenarioselect_emptysub() { }
static void window_scenarioselect_mouseup();
static void window_scenarioselect_mousedown();
static void window_scenarioselect_scrollgetsize();
static void window_scenarioselect_scrollmousedown();
static void window_scenarioselect_scrollmouseover();
static void window_scenarioselect_invalidate();
static void window_scenarioselect_paint();
static void window_scenarioselect_scrollpaint();

static uint32 window_scenarioselect_events[] = {
	window_scenarioselect_emptysub,
	window_scenarioselect_mouseup,
	window_scenarioselect_emptysub,
	window_scenarioselect_mousedown,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_scrollgetsize,
	window_scenarioselect_scrollmousedown,
	window_scenarioselect_emptysub,
	window_scenarioselect_scrollmouseover,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_emptysub,
	window_scenarioselect_invalidate,
	window_scenarioselect_paint,
	window_scenarioselect_scrollpaint
};

/**
 * 
 *  rct2: 0x006781B5
 */
void window_scenarioselect_open()
{
	rct_window* window;

	if (window_bring_to_front_by_id(WC_SCENARIO_SELECT, 0) != NULL)
		return;

	// Load scenario list
	scenario_load_list();

	window = window_create(
		(RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, sint16) / 2) - 305,
		max(28, (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_HEIGHT, sint16) / 2) - 167),
		610,
		334,
		window_scenarioselect_events,
		WC_SCENARIO_SELECT,
		0x400 | 0x02
	);
	window->widgets = window_scenarioselect_widgets;
	
	window->enabled_widgets = 0x04 | 0x10 | 0x20 | 0x40 | 0x80 | 0x100;
	window_init_scroll_widgets(window);
	window->colours[0] = 1;
	window->colours[1] = 26;
	window->colours[2] = 26;
	window->var_480 = -1;
	window->var_494 = NULL;

	window_scenarioselect_init_tabs();

	window->var_4AC = 0;
}

/**
 * 
 *  rct2: 0x00677C8A
 */
static void window_scenarioselect_init_tabs()
{
	int i, x, show_pages;
	rct_widget* widget;
	rct_scenario_basic* scenario;

	show_pages = 0;
	for (i = 0; i < RCT2_GLOBAL(RCT2_ADDRESS_NUM_SCENARIOS, sint32); i++) {
		scenario = &(RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_LIST, rct_scenario_basic*)[i]);
		if (scenario->flags & SCENARIO_FLAGS_VISIBLE)
			show_pages |= 1 << scenario->category;
	}

	x = 3;
	for (i = 0; i < 5; i++) {
		widget = &window_scenarioselect_widgets[i + 4];
		if (!(show_pages & (1 << i))) {
			widget->type = WWT_EMPTY;
			continue;
		}

		widget->type = WWT_TAB;
		widget->left = x;
		widget->right = x + 90;
		x += 91;
	}
}

static void window_scenarioselect_mouseup()
{
	short widgetIndex;
	rct_window *w;

	__asm mov widgetIndex, dx
	__asm mov w, esi

	if (widgetIndex == WIDX_CLOSE)
		window_close(w);
}

static void window_scenarioselect_mousedown()
{
	short widgetIndex;
	rct_window *w;

	__asm mov widgetIndex, dx
	__asm mov w, esi

	if (widgetIndex >= WIDX_TAB1 && widgetIndex <= WIDX_TAB5) {
		w->var_4AC = widgetIndex - 4;
		w->var_494 = NULL;
		window_invalidate(w);
		RCT2_CALLPROC_X(w->event_handlers[WE_UNKNOWN_02], 0, 0, 0, 0, w, 0, 0);
		RCT2_CALLPROC_X(w->event_handlers[WE_INVALIDATE], 0, 0, 0, 0, w, 0, 0);
		window_init_scroll_widgets(w);
		window_invalidate(w);
	}
}

static void window_scenarioselect_scrollgetsize()
{
	int i, height;
	rct_window *w;
	rct_scenario_basic *scenario;

	__asm mov w, esi

	height = 0;
	for (i = 0; i < RCT2_GLOBAL(RCT2_ADDRESS_NUM_SCENARIOS, sint32); i++) {
		scenario = &(RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_LIST, rct_scenario_basic*)[i]);
		if (scenario->category != w->var_4AC)
			continue;
		if (scenario->flags & SCENARIO_FLAGS_VISIBLE)
			height += 24;
	}

	__asm mov ecx, 0
	__asm mov edx, height
}

static void window_scenarioselect_scrollmousedown()
{
	int i;
	short x, y;
	rct_window *w;
	rct_scenario_basic *scenario;

	__asm mov x, cx
	__asm mov y, dx
	__asm mov w, esi

	for (i = 0; i < RCT2_GLOBAL(RCT2_ADDRESS_NUM_SCENARIOS, sint32); i++) {
		scenario = &(RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_LIST, rct_scenario_basic*)[i]);
		if (scenario->category != w->var_4AC)
			continue;
		if (!(scenario->flags & SCENARIO_FLAGS_VISIBLE))
			continue;

		y -= 24;
		if (y >= 0)
			continue;

		sound_play_panned(4, w->width / 2 + w->x);
		scenario_load_and_play(scenario);
		break;
	}
}

static void window_scenarioselect_scrollmouseover()
{
	int i;
	short x, y;
	rct_window *w;
	rct_scenario_basic *scenario, *selected;

	__asm mov x, cx
	__asm mov y, dx
	__asm mov w, esi

	selected = NULL;
	for (i = 0; i < RCT2_GLOBAL(RCT2_ADDRESS_NUM_SCENARIOS, sint32); i++) {
		scenario = &(RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_LIST, rct_scenario_basic*)[i]);
		if (scenario->category != w->var_4AC)
			continue;
		if (!(scenario->flags & SCENARIO_FLAGS_VISIBLE))
			continue;

		y -= 24;
		if (y >= 0)
			continue;

		selected = scenario;
		break;
	}
	if (w->var_494 != selected) {
		w->var_494 = selected;
		window_invalidate(w);
	}
}

static void window_scenarioselect_invalidate()
{
	rct_window *w;

	__asm mov w, esi

	w->pressed_widgets &= ~(0x10 | 0x20 | 0x40 | 0x80 | 0x100);
	w->pressed_widgets |= 1 << (w->var_4AC + 4);
}

static void window_scenarioselect_paint()
{
	int i, x, y;
	rct_window *w;
	rct_drawpixelinfo *dpi;
	rct_widget *widget;
	rct_scenario_basic *scenario;

	__asm mov w, esi
	__asm mov dpi, edi

	window_draw_widgets(w, dpi);

	// Text for each tab
	for (i = 0; i < 5; i++) {
		widget = &window_scenarioselect_widgets[WIDX_TAB1 + i];
		if (widget->type == WWT_EMPTY)
			continue;

		x = (widget->left + widget->right) / 2 + w->x;
		y = (widget->top + widget->bottom) / 2 + w->y - 3;
		*((short*)(0x0013CE952 + 0)) = STR_BEGINNER_PARKS + i;
		gfx_draw_string_centred_wrapped(dpi, (void*)0x013CE952, x, y, 87, 1193, 10);
	}

	// Return if no scenario highlighted
	scenario = (rct_scenario_basic*)w->var_494;
	if (scenario == NULL)
		return;

	// Draw SixFlags image
	if (scenario->flags & SCENARIO_FLAGS_SIXFLAGS)
		gfx_draw_sprite(dpi, SPR_SIX_FLAGS, w->x + w->width - 55, w->y + w->height - 75);

	// Scenario name
	x = w->x + window_scenarioselect_widgets[WIDX_SCENARIOLIST].right + 4;
	y = w->y + window_scenarioselect_widgets[WIDX_TABCONTENT].top + 5;
	strcpy(0x009BC677, scenario->name);
	*((short*)(0x0013CE952 + 0)) = 3165;
	gfx_draw_string_centred_clipped(dpi, 1193, (void*)0x013CE952, 0, x + 85, y, 170);
	y += 15;

	// Scenario details
	strcpy(0x009BC677, scenario->details);
	*((short*)(0x0013CE952 + 0)) = 3165;
	y += gfx_draw_string_left_wrapped(dpi, (void*)0x013CE952, x, y, 170, 1191, 0) + 5;

	// Scenario objective
	*((short*)(0x0013CE952 + 0)) = scenario->objective_type + STR_OBJECTIVE_NONE;
	*((short*)(0x0013CE952 + 2)) = scenario->objective_arg_3;
	*((short*)(0x0013CE952 + 4)) = date_get_total_months(MONTH_OCTOBER, scenario->objective_arg_1);
	*((int*)(0x0013CE952 + 6)) = scenario->objective_arg_2;
	y += gfx_draw_string_left_wrapped(dpi, (void*)0x013CE952, x, y, 170, STR_OBJECTIVE, 0) + 5;

	// Scenario score
	if (scenario->flags & SCENARIO_FLAGS_COMPLETED) {
		strcpy(0x009BC677, scenario->completed_by);
		*((short*)(0x0013CE952 + 0)) = 3165;
		*((int*)(0x0013CE952 + 2)) = scenario->company_value;
		y += gfx_draw_string_left_wrapped(dpi, (void*)0x013CE952, x, y, 170, STR_COMPLETED_BY_WITH_COMPANY_VALUE, 0);
	}
}

static void window_scenarioselect_scrollpaint()
{
	int i, y, colour, highlighted;
	rct_window *w;
	rct_drawpixelinfo *dpi;
	rct_scenario_basic *scenario;

	__asm mov w, esi
	__asm mov dpi, edi

	colour = ((char*)0x0141FC48)[w->colours[1] * 8];
	colour = (colour << 24) | (colour << 16) | (colour << 8) | colour;
	gfx_clear(dpi, colour);

	y = 0;
	for (i = 0; i < RCT2_GLOBAL(RCT2_ADDRESS_NUM_SCENARIOS, sint32); i++) {
		scenario = &(RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_LIST, rct_scenario_basic*)[i]);
		if (scenario->category != w->var_4AC)
			continue;
		if (!(scenario->flags & SCENARIO_FLAGS_VISIBLE))
			continue;

		if (y > dpi->y + dpi->height)
			continue;

		highlighted = w->var_494 == (int)scenario;

		// Draw hover highlight
		if (highlighted)
			gfx_fill_rect(dpi, 0, y, w->width, y + 23, 0x02000031);

		// Draw scenario name
		strcpy((char*)0x009BC677, scenario->name);
		*((short*)0x013CE952) = 3165;
		gfx_draw_string_centred(dpi, highlighted ? 1193 : 1191, 210, y + 1, 0, (void*)0x013CE952);

		// Check if scenario is completed
		if (scenario->flags & SCENARIO_FLAGS_COMPLETED) {
			// Draw completion tick
			gfx_draw_sprite(dpi, 0x5A9F, 395, y + 1);

			// Draw completion score
			strcpy((char*)0x009BC677, scenario->completed_by);
			*((short*)0x013CE952) = 2793;
			*((short*)0x013CE954) = 3165;
			gfx_draw_string_centred(dpi, highlighted ? 1193 : 1191, 210, y + 11, 0, (void*)0x013CE952);
		}

		y += 24;
	}
}