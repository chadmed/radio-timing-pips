/* SPDX-License-Identifier: GPL-3.0-only */
/*
 * Australian radio timing pips
 *
 * Play your pips and fanfare at the top of the hour
 *
 * Takes a stereo S16_LE 44.1 kHz WAV
 *
 * Copyright (C) 2024 James Calligeros
 */

#include <unistd.h>

#ifndef __PIPS_H__
#define __PIPS_H__
#endif

typedef struct iff_head {
	char magic[4];
	int len;
	char type[4];
} iff_head_t;

typedef struct iff_chunk_head {
	char magic[4];
	int len;
} iff_chunk_head_t;

typedef struct wav_fmt {
	short tag;
	unsigned short channels;
	unsigned int rate;
	unsigned int byte_rate;
	unsigned short blk_align;
	unsigned short bit_depth;
} wav_fmt_t;

typedef struct wav_data {
	size_t sz;
	unsigned int rate;
	unsigned short channels;
	unsigned short bit_depth;
	char *buf;
} wav_data_t;

static const char riff_magic[4] = { 'R', 'I', 'F', 'F' };
static const char wave_magic[4] = { 'W', 'A', 'V', 'E' };
static const char format_magic[4] = { 'f', 'm', 't', ' ' };
static const char data_magic[4] = { 'd', 'a', 't', 'a' };

char check_magic(const char *check, char *magic);
void print_help();
int load_file(const char *path, wav_data_t *wav);
void * play(void *wav);
