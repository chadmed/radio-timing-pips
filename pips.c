/* SPDX-License-Identifier: GPL-3.0-only */
/*
 * Australian radio timing pips
 *
 * Play your pips and fanfare at the top of the hour
 *
 * Copyright (C) 2024 James Calligeros
 */

#include <alsa/asoundlib.h>

#include <alsa/pcm.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pips.h"

#define PCM_DEVICE "default"


char check_magic(const char *check, char *magic)
{
	int i = 4;
	while (i--) {
		if (*check++ != *magic++)
			return -EINVAL;
	}
	return 0;
}

void print_help()
{
	printf("\nSilly Little Radio Timing Pip Utility\n");
	printf("  This utility will play a WAV file to the default ALSA sink at HH:59:55.000, every hour.\n");
	printf("  The WAV file should contain 6x 0.5 sec 735 Hz pure sine waves, with 0.5 sec of silence\n");
	printf("  between them. The sixth pip plays at the exact top of the hour. Any fanfare should be addeed\n");
	printf("  after the final half-second of silence. That is, it should start at HH:00:00:500.\n\n");

	printf("  The ABC thought this was too expensive and technically challenging to achieve.\n\n");

	printf("Use:\n");
	printf("  -m: Path to a WAV file to play at 1200 and 1700\n");
	printf("  -o: Path to a WAV file to play at all other times\n");

	printf("\nCopyright (C) 2024 James Calligeros <jcalligeros99@gmail.com>\n\n");
}

int load_file(const char *path, wav_data_t *wav)
{
	int fd;
	iff_head_t header;
	iff_chunk_head_t chunk_head;
	wav_fmt_t fmt;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -ENOENT;

	if (read(fd, &header, sizeof(iff_head_t)) == sizeof(iff_head_t)) {
		if (check_magic(riff_magic, header.magic) || check_magic(wave_magic, header.type)) {
			printf("ERROR: %s is not a valid RIFF WAVE!\n", path);
			return -EINVAL;
		}
	}

	while (read(fd, &chunk_head, sizeof(iff_chunk_head_t)) == sizeof(iff_chunk_head_t)) {
		if (!check_magic(format_magic, chunk_head.magic)) {

			if (read(fd, &fmt, sizeof(wav_fmt_t)) != sizeof(wav_fmt_t))
				return -EINVAL;

			if (fmt.tag != 1)
				continue;

			wav->rate = fmt.rate;
			wav->channels = fmt.channels;
			wav->bit_depth = fmt.bit_depth;

		} else if (!check_magic(data_magic, chunk_head.magic)) {
			wav->buf = malloc(chunk_head.len);
			wav->sz = (chunk_head.len * 8) / (wav->bit_depth * wav->channels);
			if (!wav->buf)
				return -ENOMEM;

			if (read(fd, wav->buf, chunk_head.len) != chunk_head.len) {
				free(wav->buf);
				return -EINVAL;
			} else {
				printf("WAV %s loaded successfully:\n", path);
				printf("\tChannels: %d\n", wav->channels);
				printf("\tSample Rate: %d\n", wav->rate);
				printf("\tBit Depth: %d-bit\n", wav->bit_depth);
				close(fd);
				return 0;
			}
		} else {
			if (header.len & 1) {
				++header.len;
				lseek(fd, header.len, SEEK_CUR);
			}
		}
	}
	return -EINVAL;
}

void * play(void *wav)
{
	snd_pcm_t *dev;
	snd_pcm_hw_params_t *p;
	snd_pcm_uframes_t frames = 0;
	wav_data_t *w = wav;
	int ret, fmt, i = 0;

	ret = snd_pcm_open(&dev, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
	if (ret)
		return NULL;

	snd_pcm_hw_params_alloca(&p);
	snd_pcm_hw_params_any(dev, p);

	ret = snd_pcm_hw_params_set_access(dev, p, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret)
		return NULL;

	switch (w->bit_depth) {
		case 8:
			fmt = SND_PCM_FORMAT_U8;
			break;
		case 16:
			fmt = SND_PCM_FORMAT_S16;
			break;
		case 24:
			fmt = SND_PCM_FORMAT_S24;
			break;
		case 32:
			fmt = SND_PCM_FORMAT_S32;
			break;
		default:
			return NULL;
	}

	ret = snd_pcm_hw_params_set_format(dev, p, fmt);
	if (ret)
		return NULL;

	ret = snd_pcm_hw_params_set_channels(dev, p, w->channels);
	if (ret)
		return NULL;

	ret = snd_pcm_hw_params_set_rate(dev, p, w->rate, 0);
	if (ret)
		return NULL;

	ret = snd_pcm_hw_params(dev, p);
	if (ret)
		return NULL;

	while (i < w->sz) {
		frames = snd_pcm_writei(dev, w->buf + i , w->sz - i);

		if (frames < 0) {
			frames = snd_pcm_recover(dev, frames, 0);
		}

		i += frames;
	}

	if (frames == w->sz) {
		snd_pcm_drain(dev);
	}

	return 0;
}

int main(int argc, char **argv)
{
	struct tm *loc_time = NULL;
	struct timespec *ts = NULL;
	const char *path_midday = NULL;
	const char *path_other = NULL;
	wav_data_t *wav_midday = NULL;
	wav_data_t *wav_other;
	pthread_t tid;
	time_t time_now, time_old = 0;
	char *t = NULL;
	int ret, c;

	while ((c = getopt(argc, argv, "m:o:h")) != -1) {
		switch (c) {
			case 'm':
				path_midday = optarg;
				break;
			case 'o':
				path_other = optarg;
				break;
			case ':':
			case '?':
			case 'h':
			default:
				print_help();
				exit(EXIT_FAILURE);
		}
	}

	if (!path_midday || !path_other) {
		print_help();
		exit(EXIT_FAILURE);
	}

	loc_time = malloc(sizeof(struct tm));
	if (!loc_time)
		return -ENOMEM;

	ts = malloc(sizeof(struct timespec));
	if (!ts)
		return -ENOMEM;

	wav_midday = malloc(sizeof(wav_data_t));
	if (!wav_midday)
		return -ENOMEM;

	wav_other = malloc(sizeof(wav_data_t));
	if(!wav_other)
		return -ENOMEM;

	ret = load_file(path_midday, wav_midday);
	if (ret) {
		printf("ERROR: Could not load WAV %s: %d\n", path_midday, ret);
		return ret;
	}

	ret = load_file(path_other, wav_other);
	if (ret) {
		printf("ERROR: Could not load WAV %s: %d\n", path_other, ret);
		return ret;
	}

	while (1) {
		ret = clock_gettime(CLOCK_REALTIME, ts);
		time_now = ts->tv_sec;
		loc_time = localtime(&time_now);
		if (ts->tv_sec != time_old) {
			ret = asprintf(&t, "\r%02d:%02d:%02d", loc_time->tm_hour,
							loc_time->tm_min,
							loc_time->tm_sec);

			if (loc_time->tm_min == 59) {
				switch (loc_time->tm_sec) {
					case 55:
						pthread_create(&tid, NULL, play, wav_midday);
						printf("%s - BEEP --", t);
						fflush(stdout);
						break;
					case 56:
						printf("%s - BEEP -- BEEP", t);
						fflush(stdout);
						break;
					case 57:
						printf("%s - BEEP -- BEEP -- BEEP", t);
						fflush(stdout);
						break;
					case 58:
						printf("%s - BEEP -- BEEP -- BEEP -- BEEP", t);
						fflush(stdout);
						break;
					case 59:
						printf("%s - BEEP -- BEEP -- BEEP -- BEEP -- BEEP", t);
						fflush(stdout);
						break;
					default:
						break;
				}
			}

			if (loc_time->tm_min && loc_time->tm_sec == 00)
				printf("%s - BEEP -- BEEP -- BEEP -- BEEP -- BEEP -- BEEP!\n", t);
			time_old = ts->tv_sec;
		}

		usleep(100000);

	}

	return ret;
}
