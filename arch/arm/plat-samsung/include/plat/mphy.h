/*
 * include/plat/mphy.h
 *
 * Copyright (C) 2012 FriendlyARM (www.arm9.net)
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

#ifndef __PLAT_MPHY_H__
#define __PLAT_MPHY_H__


/* command list */
#define PHY_CMD_EHCI		1
#define PHY_CMD_SDHCI		2

/* PHY init function */
extern int s5p_phy_init_ext(unsigned int cmd, unsigned long arg, void *p);


#endif	// __PLAT_MPHY_H__

