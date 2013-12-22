/*
 *  dma.h - ALSA PCM interface for the Samsung SoCs
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#ifndef _S3C_AUDIO_DMA_H
#define _S3C_AUDIO_DMA_H

#define ST_RUNNING			(1 << 0)
#define ST_OPENED			(1 << 1)

struct s3c_dma_params {
	struct s3c2410_dma_client *client;	/* Stream Identifier */
	int channel;						/* Channel ID */
	dma_addr_t dma_addr;
	int dma_size;						/* DMA transfer size */
};

extern struct snd_soc_platform_driver samsung_asoc_platform;

#ifdef DEBUG
#define pr_debug(fmt...)	printk(fmt)
#endif

#endif

