/*
 * Copyright 2011 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef __LINUX_MMA7660_H
#define __LINUX_MMA7660_H

#include <linux/types.h>

#ifdef __KERNEL__

/* MMA7760 Registers */
#define MMA7660_XOUT			0x00	// 6-bit output value X
#define MMA7660_YOUT			0x01	// 6-bit output value Y
#define MMA7660_ZOUT			0x02	// 6-bit output value Z
#define MMA7660_TILT			0x03	// Tilt status
#define MMA7660_SRST			0x04	// Sampling Rate Status
#define MMA7660_SPCNT			0x05	// Sleep Count
#define MMA7660_INTSU			0x06	// Interrupt Setup
#define MMA7660_MODE			0x07	// Mode
#define MMA7660_SR				0x08	// Auto-Wake/Sleep and Debounce Filter
#define MMA7660_PDET			0x09	// Tap Detection
#define MMA7660_PD				0x0a	// Tap Debounce Count

struct mma7660_platform_data {
	/* eint connected to chip */
	int irq;

	/* parameters for input device */
	int poll_interval;
	int input_fuzz;
	int input_flat;
};

#endif /* __KERNEL__ */

#endif /* __LINUX_MMA7660_H */

