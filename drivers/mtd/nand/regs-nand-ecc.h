/* arch/arm/plat-s5p/include/plat/regs-nand-ecc.h
 *
 * Copyright (c) 2012 FriendlyARM (www.arm9.net)
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Register definition file for Samsung NAND driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARM_REGS_S5P_NAND_ECC__
#define __ASM_ARM_REGS_S5P_NAND_ECC__

/* S5PV210 nand: 8/ 12/ 16 bit ECC support */
#define S5P_NFECCREG(x) 	(0x00020000 + (x))


/* ...configuration and control */
#define NFECCCONF			S5P_NFECCREG(0x00)
#define NFECCCONT			S5P_NFECCREG(0x20)

/* ...ecc status */
#define NFECCSTAT			S5P_NFECCREG(0x30)
#define NFECCSECSTAT		S5P_NFECCREG(0x40)

/* ...parity code registers for page program */
#define NFECCPRGECC0		S5P_NFECCREG(0x90)
#define NFECCPRGECC1		S5P_NFECCREG(0x94)
#define NFECCPRGECC2		S5P_NFECCREG(0x98)
#define NFECCPRGECC3		S5P_NFECCREG(0x9C)
#define NFECCPRGECC4		S5P_NFECCREG(0xA0)
#define NFECCPRGECC5		S5P_NFECCREG(0xA4)
#define NFECCPRGECC6		S5P_NFECCREG(0xA8)

/* ...error byte locations */
#define NFECCERL0			S5P_NFECCREG(0xC0)
#define NFECCERL1			S5P_NFECCREG(0xC4)
#define NFECCERL2			S5P_NFECCREG(0xC8)
#define NFECCERL3			S5P_NFECCREG(0xCC)
#define NFECCERL4			S5P_NFECCREG(0xD0)
#define NFECCERL5			S5P_NFECCREG(0xD4)
#define NFECCERL6			S5P_NFECCREG(0xD8)
#define NFECCERL7			S5P_NFECCREG(0xDC)

/* ...error bit pattern */
#define NFECCERP0			S5P_NFECCREG(0xF0)
#define NFECCERP1			S5P_NFECCREG(0xF4)
#define NFECCERP2			S5P_NFECCREG(0xF8)
#define NFECCERP3			S5P_NFECCREG(0xFC)

/* ...error parity conversion code */
#define NFECCCONECC0		S5P_NFECCREG(0x0110)
#define NFECCCONECC1		S5P_NFECCREG(0x0114)
#define NFECCCONECC2		S5P_NFECCREG(0x0118)
#define NFECCCONECC3		S5P_NFECCREG(0x011C)
#define NFECCCONECC4		S5P_NFECCREG(0x0120)
#define NFECCCONECC5		S5P_NFECCREG(0x0124)
#define NFECCCONECC6		S5P_NFECCREG(0x0128)

/* ...register R/W by offset */
#define NFECCPRGECC(x)		S5P_NFECCREG(0x0090 + x * 4)
#define NFECCERL(x)			S5P_NFECCREG(0x00C0 + x * 4)
#define NFECCERP(x)			S5P_NFECCREG(0x00F0 + x * 4)
#define NFECCCONECC(x)		S5P_NFECCREG(0x0110 + x * 4)


#endif /* __ASM_ARM_REGS_NAND_ECC__ */

