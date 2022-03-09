/*
 * linux/drivers/mtd/nand/s5p_nand_mlc.c
 *
 * Copyright (c) 2012 FriendlyARM (www.arm9.net)
 *
 * Copyright (c) 2009 Samsung Electronics Co., Ltd.
 *      http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/sched.h>
#include <asm/mach-types.h>
#include <asm/io.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>

#include <plat/regs-nand.h>
#include <plat/nand.h>

#include "regs-nand-ecc.h"

/* NAND register address (virtual) */
static void __iomem *regs = NULL;


/* Enable ECC on OOB [4~35] or not */
#define OOB_ECC					1
#define OOB_ECC_OFFSET			4
#define OOB_ECC_LEN				32

/* ECC type
 * 16bit as default, should be detect by chip ID5 */
const int ecc_type = 0x5;
const int ecclen[] = { 16, 20, 28 };

/* ECC message size
 * set to 511 for 512-byte message */
static int ecc_msg_len = 511;

/* ECC encode or decode */
static int cur_ecc_mode;


/*
 * Nand flash oob definitions for large page MLC, by FriendlyARM
 *  - 4K page size with 8bit ecc
 */
#if 1
static struct nand_ecclayout s5p_nand_oob_mlc_128 = {
	.eccbytes = 128,
	.eccpos = {
		 40,  41,  42,  43,  44,  45,  46,  47,		/* Generate 16bytes ECC for 512bytes */
		 48,  49,  50,  51,  52,  53,  54,  55,
		 56,  57,  58,  59,  60,  61,  62,  63,		/* ecc pos for next block */
		 64,  65,  66,  67,  68,  69,  70,  71,
		 72,  73,  74,  75,  76,  77,  78,  79,
		 80,  81,  82,  83,  84,  85,  86,  87,
		 88,  89,  90,  91,  92,  93,  94,  95,
		 96,  97,  98,  99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119,
		120, 121, 122, 123, 124, 125, 126, 127,
		128, 129, 130, 131, 132, 133, 134, 135,
		136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151,
		152, 153, 154, 155, 156, 157, 158, 159,
		160, 161, 162, 163, 164, 165, 166, 167,
	},
	.oobfree = {
		{.offset = OOB_ECC_OFFSET,
		 .length = OOB_ECC_LEN} }
};
#endif

/* ...8K page size with 16bit ecc */
static struct nand_ecclayout s5p_nand_oob_mlc_448 = {
	.eccbytes = 448,
	.eccpos = {
		 36,  37,  38,  39,
		 40,  41,  42,  43,  44,  45,  46,  47,		/* Generate 26bytes ECC for 512bytes */
		 48,  49,  50,  51,  52,  53,  54,  55,
		 56,  57,  58,  59,  60,  61,  62,  63,
		 64,  65,  66,  67,  68,  69,  70,  71,
		 72,  73,  74,  75,  76,  77,  78,  79,
		 80,  81,  82,  83,  84,  85,  86,  87,
		 88,  89,  90,  91,  92,  93,  94,  95,
		 96,  97,  98,  99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119,
		120, 121, 122, 123, 124, 125, 126, 127,
		128, 129, 130, 131, 132, 133, 134, 135,
		136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151,
		152, 153, 154, 155, 156, 157, 158, 159,
		160, 161, 162, 163, 164, 165, 166, 167,
		168, 169, 170, 171, 172, 173, 174, 175,
		176, 177, 178, 179, 180, 181, 182, 183,
		184, 185, 186, 187, 188, 189, 190, 191,
		192, 193, 194, 195, 196, 197, 198, 199,
		200, 201, 202, 203, 204, 205, 206, 207,
		208, 209, 210, 211, 212, 213, 214, 215,
		216, 217, 218, 219, 220, 221, 222, 223,
		224, 225, 226, 227, 228, 229, 230, 231,
		232, 233, 234, 235, 236, 237, 238, 239,
		240, 241, 242, 243, 244, 245, 246, 247,
		248, 249, 250, 251, 252, 253, 254, 255,
		256, 257, 258, 259, 260, 261, 262, 263,
		264, 265, 266, 267, 268, 269, 270, 271,
		272, 273, 274, 275, 276, 277, 278, 279,
		280, 281, 282, 283, 284, 285, 286, 287,
		288, 289, 290, 291, 292, 293, 294, 295,
		296, 297, 298, 299, 300, 301, 302, 303,
		304, 305, 306, 307, 308, 309, 310, 311,
		312, 313, 314, 315, 316, 317, 318, 319,
		320, 321, 322, 323, 324, 325, 326, 327,
		328, 329, 330, 331, 332, 333, 334, 335,
		336, 337, 338, 339, 340, 341, 342, 343,
		344, 345, 346, 347, 348, 349, 350, 351,
		352, 353, 354, 355, 356, 357, 358, 359,
		360, 361, 362, 363, 364, 365, 366, 367,
		368, 369, 370, 371, 372, 373, 374, 375,
		376, 377, 378, 379, 380, 381, 382, 383,
		384, 385, 386, 387, 388, 389, 390, 391,
		392, 393, 394, 395, 396, 397, 398, 399,
		400, 401, 402, 403, 404, 405, 406, 407,
		408, 409, 410, 411, 412, 413, 414, 415,
		416, 417, 418, 419, 420, 421, 422, 423,
		424, 425, 426, 427, 428, 429, 430, 431,
		432, 433, 434, 435, 436, 437, 438, 439,
		440, 441, 442, 443, 444, 445, 446, 447,
		448, 449, 450, 451, 452, 453, 454, 455,
		456, 457, 458, 459, 460, 461, 462, 463,
		464, 465, 466, 467, 468, 469, 470, 471,
		472, 473, 474, 475, 476, 477, 478, 479,
		480, 481, 482, 483,
#if 0
		484, 485, 486, 487,
#endif
	},
	.oobfree = {
		{.offset = OOB_ECC_OFFSET,
		 .length = OOB_ECC_LEN} }
};

/* Magic info. */
static unsigned int s5p_nand_oob_mode = 0x00fa0000;
static char __oob[] = {
	/* Encoded "FriendlyARM\0" */
	0xb9, 0x85, 0x90, 0x9c, 0x91, 0x9b, 0x93, 0x80,
	0xb8, 0xa5, 0xb4, 0xf7,
};


/* when Hz=200, jiffies interval 1/200=5mS,
 * waiting for 100mS 100/5 = 20 */
#define S5P_NAND_WAIT_TIME_MS   (100)
#define S5P_NAND_WAIT_INTERVAL  (S5P_NAND_WAIT_TIME_MS * HZ / 1000)

static inline void s5p_nand_wait_ecc_busy_8bit(void)
{
	unsigned long timeo = jiffies;

	timeo += S5P_NAND_WAIT_INTERVAL;

	while (time_before(jiffies, timeo)) {
		if (!(readl(regs + NFECCSTAT) & (1 << 31)))
			return;
		cond_resched();
	}

	printk("s5p-nand: ECC busy\n");
}

static inline void s5p_nand_wait_ecc_status(unsigned long stat)
{
	unsigned long timeo = jiffies;

	timeo += S5P_NAND_WAIT_INTERVAL;

	while (time_before(jiffies, timeo)) {
		if (readl(regs + NFECCSTAT) & stat)
			return;
		cond_resched();
	}

	printk("s5p-nand: ECC status error\n");
}

/* debug helper */
#ifdef S5P_MLC_DUMP
static void xdump(unsigned char *p, int len, char *prompt) {
	int i;

	printk("%s - %d:\n", prompt, len);
	for (i = 0; i < len; i++) {
		printk(" %02x", *p++);
		if ((i % 16) == 15) {
			printk("\n");
		}
	}
	if ((i % 16)) {
		printk("\n");
	}
}
#endif


/**
 * Functions for S5PV210 8/12/16 bit ECC support
 */

static void s5p_nand_enable_hwecc_8bit(struct mtd_info *mtd, int mode)
{
	u_long nfreg;

	cur_ecc_mode = mode;

	/* disable 1bit/4bit ecc */
	nfreg = readl(regs + S3C_NFCONF);
	if (!(nfreg & (1 << 23))) {
		printk("Warning: NFREG = %08lx\n", nfreg);
		nfreg |= (0x3 << 23);
		writel(nfreg, (regs + S3C_NFCONF));
	}

	/* set ecc type and message length */
	nfreg = (ecc_msg_len << 16) | ecc_type;
	writel(nfreg, (regs + NFECCCONF));

	/* set ecc direction */
	nfreg = readl(regs + NFECCCONT);
	if (cur_ecc_mode == NAND_ECC_WRITE) {
		nfreg |= (1 << 16);
	} else {
		nfreg &= ~(1 << 16);
	}
	writel(nfreg, (regs + NFECCCONT));

	/* clean status */
	nfreg = readl(regs + S3C_NFSTAT);
	nfreg |= (1 << 4) | (1 << 5);
	writel(nfreg, (regs + S3C_NFSTAT));

	nfreg = readl(regs + NFECCSTAT);
	nfreg |= (1 << 24) | (1 << 25);
	writel(nfreg, (regs + NFECCSTAT));

	/* Initialize & unlock */
	nfreg = readl(regs + S3C_NFCONT);
	nfreg &= ~S3C_NFCONT_MECCLOCK;
#if 0
	nfreg |= S3C_NFCONT_INITECC;
#endif
	writel(nfreg, (regs + S3C_NFCONT));

	/* reset ECC value */
	nfreg = readl(regs + NFECCCONT);
	nfreg |= (1 << 2);
	writel(nfreg, (regs + NFECCCONT));
}

static int s5p_nand_calculate_ecc_8bit(struct mtd_info *mtd,
		const u_char *dat, u_char *ecc_code)
{
	u_long nfreg;
	u_long prgecc[7];

	/* Lock */
	nfreg = readl(regs + S3C_NFCONT);
	nfreg |= S3C_NFCONT_MECCLOCK;
	writel(nfreg, (regs + S3C_NFCONT));

	if (cur_ecc_mode == NAND_ECC_READ) {
		s5p_nand_wait_ecc_status(1 << 24);

		/* clear 8/12/16bit ecc decode done */
		nfreg = readl(regs + NFECCSTAT);
		nfreg |= (1 << 24);
		writel(nfreg, (regs + NFECCSTAT));

		/* TODO: check illegal access */

	} else {
		s5p_nand_wait_ecc_status(1 << 25);

		/* clear 8/12/16bit ecc encode done */
		nfreg = readl(regs + NFECCSTAT);
		nfreg |= (1 << 25);
		writel(nfreg, (regs + NFECCSTAT));

		/* read ECC and save it */
		prgecc[0] = readl(regs + NFECCPRGECC(0));
		prgecc[1] = readl(regs + NFECCPRGECC(1));
		prgecc[2] = readl(regs + NFECCPRGECC(2));
		prgecc[3] = readl(regs + NFECCPRGECC(3));
		prgecc[4] = readl(regs + NFECCPRGECC(4));
		prgecc[5] = readl(regs + NFECCPRGECC(5));
		prgecc[6] = readl(regs + NFECCPRGECC(6)) | 0xffff0000;

		memcpy(ecc_code, prgecc, ecclen[ecc_type - 3]);
	}

	return 0;
}

int s5p_nand_correct_data_8bit(struct mtd_info *mtd, u_char *dat,
		u_char *read_ecc, u_char *calc_ecc)
{
	u_long eccsecstat;
	u_long el[8];
	u_long ep[4];
	unsigned short *err_byte_loc;
	unsigned char *err_bit_pattern;
	int err_num, pos;
	int i;

	s5p_nand_wait_ecc_busy_8bit();

	eccsecstat = readl(regs + NFECCSECSTAT);
	if (eccsecstat & (1 << 29)) {
		printk("s5p-nand: NFECCSECSTAT = %08lx\n", eccsecstat);

		/* No error, If free page (all 0xff) ...!!!TODO: undocomented */
		err_num = 0;
	} else {
		err_num = eccsecstat & 0x1f;
	}

	if (unlikely(err_num > ((ecc_type - 1) << 2))) {
		/* ECCType (3,4,5) ==> 8, 12, 16 ecc bits */
		printk("s5p-nand: ECC uncorrectable error detected\n");
		return -1;

	} else if (err_num) {
		//printk("s5p-nand: NFECCSECSTAT = %08lx\n", eccsecstat);

		el[0] = readl(regs + NFECCERL(0));
		el[1] = readl(regs + NFECCERL(1));
		el[2] = readl(regs + NFECCERL(2));
		el[3] = readl(regs + NFECCERL(3));
		el[4] = readl(regs + NFECCERL(4));
		el[5] = readl(regs + NFECCERL(5));
		el[6] = readl(regs + NFECCERL(6));
		el[7] = readl(regs + NFECCERL(7));
		err_byte_loc = (unsigned short *) el;

		ep[0] = readl(regs + NFECCERP(0));
		ep[1] = readl(regs + NFECCERP(1));
		ep[2] = readl(regs + NFECCERP(2));
		ep[3] = readl(regs + NFECCERP(3));
		err_bit_pattern = (unsigned char *) ep;

#ifdef S5P_MLC_DUMP
		xdump((unsigned char *) err_byte_loc, 28, "Location");
		xdump(err_bit_pattern, 16, "Pattern");
#endif

		for (i = 0; i < err_num; i++) {
			pos = err_byte_loc[i] & 0x3ff;

			if (pos > ecc_msg_len) {
				/* ignore error(s) in ECC itself (not data area) */
				continue;
			}

#if 0
			if (ecc_msg_len < 511) {
				printk("Correct OOB %d: dat[%3d] = %02x --> %02x\n", i, pos,
						dat[pos], dat[pos] ^ err_bit_pattern[i]);
			}
#endif

			dat[pos] ^= err_bit_pattern[i];
		}
	}

	return err_num;
}

void s5p_nand_write_page_8bit(struct mtd_info *mtd, struct nand_chip *chip,
		const uint8_t *buf)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	const uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint32_t *mecc_pos = chip->ecc.layout->eccpos;
#ifdef OOB_ECC
	uint8_t oob_ecc[28];
#endif
#ifdef S5P_MLC_PERF
	struct timeval t1, t2;
	static int count = 0;

	do_gettimeofday(&t1);
#endif

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);
	}

	if (unlikely(mtd->writesize == 512)) {
		chip->oob_poi[NAND_SMALL_BADBLOCK_POS] = 0xff;
	} else {
		chip->oob_poi[NAND_LARGE_BADBLOCK_POS] = 0xff;
	}

#if 0
	for (i = 0; i < chip->ecc.total; i++) {
		chip->oob_poi[mecc_pos[i]] = ecc_calc[i];
	}
#else
	memcpy(chip->oob_poi + mecc_pos[0], ecc_calc, chip->ecc.total);
#endif

#ifdef OOB_ECC
	chip->write_buf(mtd, chip->oob_poi, OOB_ECC_OFFSET);

	ecc_msg_len = (OOB_ECC_LEN - 1);
	chip->ecc.hwctl(mtd, NAND_ECC_WRITE);

	chip->write_buf(mtd, chip->oob_poi + OOB_ECC_OFFSET, OOB_ECC_LEN);
	chip->ecc.calculate(mtd, p, oob_ecc);
	memcpy(chip->oob_poi + mecc_pos[0] + chip->ecc.total,
			oob_ecc, ecclen[ecc_type - 3]);

	chip->write_buf(mtd, chip->oob_poi + OOB_ECC_OFFSET + OOB_ECC_LEN,
			mtd->oobsize - OOB_ECC_OFFSET - OOB_ECC_LEN);

	ecc_msg_len = 511;
#else
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
#endif

#ifdef S5P_MLC_PERF
	do_gettimeofday(&t2);
	if (count++ < 300) {
		printk("Write page: %ld, %ld\n",
				t2.tv_sec - t1.tv_sec, t2.tv_usec - t1.tv_usec);
	}
#endif
}

int s5p_nand_read_page_8bit(struct mtd_info *mtd, struct nand_chip *chip,
		uint8_t *buf, int page)
{
	uint32_t *mecc_pos = chip->ecc.layout->eccpos;
	uint8_t *p = buf;
	int eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	int col, i;
#ifdef S5P_MLC_PERF
	struct timeval t1, t2;
	static int count = 0;

	do_gettimeofday(&t1);
#endif

	col = mtd->writesize;
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

#ifdef OOB_ECC
	/* Re-read protected OOB area with ECC enabled */
	col += OOB_ECC_OFFSET;
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);

	ecc_msg_len = (OOB_ECC_LEN - 1);
	chip->ecc.hwctl(mtd, NAND_ECC_READ);
	chip->read_buf(mtd, chip->oob_poi + OOB_ECC_OFFSET, OOB_ECC_LEN);

	chip->write_buf(mtd, chip->oob_poi + mecc_pos[0] + chip->ecc.total, eccbytes);
	chip->ecc.calculate(mtd, 0, 0);
	if (chip->ecc.correct(mtd, chip->oob_poi + OOB_ECC_OFFSET, NULL, NULL) < 0) {
		mtd->ecc_stats.failed++;
	}

	/* Set ECC message length for main data area */
	ecc_msg_len = 511;
#endif

	for (col = 0, i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);
		chip->ecc.hwctl(mtd, NAND_ECC_READ);
		chip->read_buf(mtd, p, eccsize);

		chip->write_buf(mtd, chip->oob_poi + mecc_pos[i], eccbytes);
		chip->ecc.calculate(mtd, 0, 0);
		if (chip->ecc.correct(mtd, p, NULL, NULL) < 0) {
			mtd->ecc_stats.failed++;
		}

		col += eccsize;
	}

#ifdef S5P_MLC_PERF
	do_gettimeofday(&t2);
	if (count++ < 300) {
		printk("Read page %8d: %ld, %ld\n", page,
				t2.tv_sec - t1.tv_sec, t2.tv_usec - t1.tv_usec);
	}
#endif

	return 0;
}

static int s5p_nand_read_oob_8bit(struct mtd_info *mtd, struct nand_chip *chip,
		int page, int sndcmd)
{
	uint8_t oob_ecc[28];
	int n = chip->ecc.layout->eccpos[0];
	int eccbytes = chip->ecc.bytes;
	u16 bad;
	int col, ret;

	if (sndcmd) {
		chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
		sndcmd = 0;
	}

#ifdef OOB_ECC
	chip->read_buf(mtd, chip->oob_poi, OOB_ECC_OFFSET);

	/* check bad block at first */
	bad = chip->oob_poi[0];
	if (unlikely(chip->badblockbits == 8))
		ret = bad != 0xFF;
	else
		ret = hweight8(bad) < chip->badblockbits;
	if (ret) 
		return sndcmd;

	/* read oob ecc for correction */
	col = mtd->writesize + n + chip->ecc.total;
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);
	chip->read_buf(mtd, oob_ecc, eccbytes);

	/* read protected OOB area with ECC enabled */
	col = mtd->writesize + OOB_ECC_OFFSET;
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);

	ecc_msg_len = (OOB_ECC_LEN - 1);
	chip->ecc.hwctl(mtd, NAND_ECC_READ);
	chip->read_buf(mtd, chip->oob_poi + OOB_ECC_OFFSET, OOB_ECC_LEN);

	chip->write_buf(mtd, oob_ecc, eccbytes);
	chip->ecc.calculate(mtd, 0, 0);
	if (chip->ecc.correct(mtd, chip->oob_poi + OOB_ECC_OFFSET, NULL, NULL) < 0) {
		mtd->ecc_stats.failed++;
	}

	ecc_msg_len = 511;
#else
	chip->read_buf(mtd, chip->oob_poi, n);
#endif

	return sndcmd;
}

static int s5p_nand_write_oob_8bit(struct mtd_info *mtd, struct nand_chip *chip,
		int page)
{
	printk("s5p_nand: write page oob %8d\n", page);

	return -EPERM;
}

/**
 * s5p_nand_block_bad - [MLC] Read bad block marker from the chip
 * @mtd:    MTD device structure
 * @ofs:    offset from device start
 * @getchip:    0, if the chip is already selected
 *
 * Check, if the block is bad.
 */
static int s5p_nand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
#ifdef S5P_MLC_VERBOSE
	static int bad_blocks = 1;
#endif
	int page, res = 0;
	struct nand_chip *chip = mtd->priv;
	u16 bad;

	page = (int)(ofs >> chip->page_shift);

	/* check 1st page */
	chip->cmdfunc(mtd, NAND_CMD_READOOB, chip->badblockpos, page);
	bad = chip->read_byte(mtd);

	if (unlikely(chip->badblockbits == 8))
		res = bad != 0xFF;
	else
		res = hweight8(bad) < chip->badblockbits;

	if (res) {
		goto __found;
	}

	/* try last page */
	page += ((mtd->erasesize - mtd->writesize) >> chip->page_shift);

	chip->cmdfunc(mtd, NAND_CMD_READOOB, chip->badblockpos, page);
	bad = chip->read_byte(mtd);

	if (unlikely(chip->badblockbits == 8))
		res = bad != 0xFF;
	else
		res = hweight8(bad) < chip->badblockbits;

#ifdef S5P_MLC_VERBOSE
	if (res) {
__found:
		if (unlikely(bad_blocks > 400)) {
			printk("s5p-nand: too many (>400) bad blocks\n");
			bad_blocks = -1;
		} else if (bad_blocks > 0) {
			printk("Bad block %d\n", page >> 7);
			bad_blocks++;
		}
	}
#else
__found:
#endif

	return res;
}


/*
 * Improved read_buf & write_buf
 * - Read:  +25%
 * - Write: +35%
 */
static void s5p_nand_fast_ior(struct mtd_info *mtd, u_char *buf, int len)
{
#ifdef S5P_MLC_PERF
	struct timeval t1, t2;
	static int count = 0;

	do_gettimeofday(&t1);
#endif

	readsl(regs + S3C_NFDATA, buf, len >> 2);

	/* cleanup if we've got less than a word to do */
	if (len & 3) {
		buf += len & ~3;

		for (; len & 3; len--)
			*buf++ = readb(regs + S3C_NFDATA);
	}

#ifdef S5P_MLC_PERF
	do_gettimeofday(&t2);
	if (count++ < 100) {
		printk("---->R %d: %ld, %ld\n", len,
				t2.tv_sec - t1.tv_sec, t2.tv_usec - t1.tv_usec);
	}
#endif
}

static void s5p_nand_fast_iow(struct mtd_info *mtd, const u_char *buf, int len)
{
	writesl(regs + S3C_NFDATA, buf, len >> 2);

	/* cleanup any fractional write */
	if (len & 3) {
		buf += len & ~3;

		for (; len & 3; len--, buf++)
			writeb(*buf, regs + S3C_NFDATA);
	}
}


/*
 * Extended initializer for MLC nand chip and speed up
 */

#define S5P_MLC_EAT		0x3628

static void s5p_nand_chip_reset(int chip)
{
	int nce_shift[4] = { 1, 2, 22, 23 };
	int i = 0;
	unsigned int val;

	if (chip < 0 || chip > 3)
		return;

	val = readl(regs + S3C_NFCONT);
	writel(val & ~(1 << nce_shift[chip]), regs + S3C_NFCONT);

	val = readl(regs + S3C_NFSTAT);
	writel(val | (1 << 4), regs + S3C_NFSTAT);

	/* Reset command */
	writel(0xff, regs + S3C_NFCMMD);

	while (i++ < 10000) {
		if ((readl(regs + S3C_NFSTAT) & 0x11) == 0x11)
			break;

		udelay(10);
	}

	val = readl(regs + S3C_NFCONT);
	writel(val | (1 << nce_shift[chip]), regs + S3C_NFCONT);
}

int s5p_nand_ext_finit(struct nand_chip *nand, void __iomem *nandregs)
{
	int val = s5p_nand_oob_mode;

	if (!nandregs)
		return -1;

	regs = nandregs;
	val |= (__machine_arch_type << 2);

	if ((val & 0xffff) == S5P_MLC_EAT) {
		nand->read_buf	= s5p_nand_fast_ior;
		nand->write_buf	= s5p_nand_fast_iow;
	}

	s5p_nand_chip_reset(0);
	s5p_nand_chip_reset(1);

	return 0;
}

int s5p_nand_mlc_probe(struct nand_chip *nand, void __iomem *nandregs, u_char *ids)
{
	const char *__mlc_n = "MLC nand";
	const char *__mlc_p = "2012 ported by ";

	int i;

	s5p_nand_oob_mode |= (__machine_arch_type << 2);
	for (i = 0; i < sizeof(__oob); i++) {
		__oob[i] = (__oob[i] + 0x03) ^ 0xfa;
	}

	if (!regs && nandregs) {
		regs = nandregs;
	}

	if ((s5p_nand_oob_mode & 0xffff) == S5P_MLC_EAT) {
		printk("%s initialized, %s", __mlc_n, __mlc_p);

		nand->block_bad = s5p_nand_block_bad;

		/* NOTE: support 8bit ECC only */
		nand->ecc.read_page = s5p_nand_read_page_8bit;
		nand->ecc.write_page = s5p_nand_write_page_8bit;
		nand->ecc.read_oob = s5p_nand_read_oob_8bit;
		nand->ecc.write_oob = s5p_nand_write_oob_8bit;
		nand->ecc.layout = &s5p_nand_oob_mlc_448;
		nand->ecc.hwctl = s5p_nand_enable_hwecc_8bit;
		nand->ecc.calculate = s5p_nand_calculate_ecc_8bit;
		nand->ecc.correct = s5p_nand_correct_data_8bit;
		nand->ecc.size = 512;
		nand->ecc.bytes = ecclen[ecc_type - 3];

		if (ecc_type == 3) {
			nand->ecc.layout = &s5p_nand_oob_mlc_128;
		}

		printk("%s\n", __oob);
		return 0;
	}

	printk("%s init failed, %d, %s", __mlc_n, -ENODEV, __mlc_p);
	printk("%s", __oob);
	printk(" http://www.arm9.net\n");
	return -1;
}


/*
 * misc initializer(s).
 */

#include <linux/platform_device.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdhci.h>
#include <linux/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/sdhci.h>

int s5p_phy_init_ext(unsigned int cmd, unsigned long arg, void *p)
{
	struct platform_device *pdev;
	struct s3c_sdhci_platdata *pdata;
	struct usb_hcd *hcd;
	void __iomem *r;
	unsigned int val = 0;
	unsigned int etc;

	pdev = (struct platform_device *) arg;

	val += cmd + 0x0e;
	if (val > 15) {
		if (pdev && pdev->id == 2) {
			pdata = pdev->dev.platform_data;

			/* disable SDHCI_QUIRK_MULTIBLOCK_READ_ACMD12 for SDIO Wi-Fi support */
			pdata->disable_acmd12 = 1;
		}

	} else {
		r = S5PV210_ETC2_BASE;
		etc = 0x5 << (2 * 6);
		writel(readl(r + 0x08) | etc, (r + 0x08));
		writel(readl(r + 0x0c) | etc, (r + 0x0c));

		hcd = (struct usb_hcd *) p;
		r = (hcd->regs + 0x40);

		if (hcd && hcd->regs && r) {
			val <<= (val + 1);
			/* DMA burst Enable by writel(0x000F0000, hcd->regs + 0x90); */
			writel(val, (r + 0x50));
		}
	}

	return 0;
}

// !End of file s5p_nand_mlc.c--/>

