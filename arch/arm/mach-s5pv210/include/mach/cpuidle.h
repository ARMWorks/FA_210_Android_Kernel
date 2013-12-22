/* arch/arm/mach-s5pv210/include/mach/cpuidle.h
 *
 * Copyright 2010 Samsung Electronics
 *	Jaecheol Lee <jc.lee@samsung>
 *
 * S5PV210 - CPUIDLE support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __S5P_CPUIDLE_H__
#define __S5P_CPUIDLE_H__		1

extern int  s5pv210_didle_save(unsigned long *saveblk);
extern void s5pv210_didle_resume(void);
extern void i2sdma_getpos(dma_addr_t *src);

#ifdef CONFIG_S5P_HIGH_RES_TIMERS
extern unsigned int get_rtc_cnt(void);
#endif

#endif

