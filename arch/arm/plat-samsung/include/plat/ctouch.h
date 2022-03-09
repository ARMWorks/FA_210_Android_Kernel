/*
 * include/mach/ctouch.h
 *
 * Copyright (C) 2018 FriendlyARM (www.arm9.net)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __PLAT_CTOUCH_H__
#define __PLAT_CTOUCH_H__

enum {
	CTP_NONE = 0,
	CTP_GT80X,
	CTP_FT5306,
	CTP_FT5406,
	CTP_AUTO,
	CTP_GT9XX,
	CTP_MAX
};

extern unsigned int panel_get_touch_id(void);
extern void panel_set_touch_id(int type);

#if defined(CONFIG_MACH_MINI210)
extern unsigned int mini210_get_ctp(void);
#endif

#endif	// __PLAT_CTOUCH_H__

