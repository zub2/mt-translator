/*****************************************************************************
 *
 * mt-translator - Multitouch Protocol Translation Tool (MIT license)
 *
 * Copyright (C) 2012 David Kozub <zub@linux.fjfi.cvut.cz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <linux/input.h>

#include "input_utils.h"

static bool get_bit(uint8_t const* bits, size_t size, size_t bitIndex)
{
	assert((bitIndex+8-1)/8 < size);
	return bits[bitIndex/8] & (1 << (bitIndex%8));
}

bool foreach_capability(int fd, FOREACH_CAPABILITY_CB cb, void *user_data)
{
	// query EV_* bits
	uint8_t evBits[EV_MAX];
	memset(evBits, 0, sizeof(evBits));
	if (ioctl(fd, EVIOCGBIT(0, sizeof(evBits)), evBits) < 0)
	{
		perror("can't get device capability bits");
		return false;
	}
	else
	{
		for (uint32_t idx = 0; idx <= EV_MAX; ++idx)
		{
			if (get_bit(evBits, sizeof(evBits)/sizeof(evBits[0]), idx))
			{
				if (!cb(fd, idx, user_data))
					break;
			}
		}
		return true;
	}
}

typedef struct
{
	bool first;
} dump_state;

static bool dump_single_capability(int fd, uint32_t value, void *user_data)
{
	(void)fd; // unused
	dump_state *state = (dump_state*)user_data;

	if (state->first)
	{
		state->first = false;
		printf("capabilities: ");
	}
	else
	{
		printf(", ");
	}


	switch(value)
	{
	case EV_SYN:
		printf("EV_SYN");
		break;
	case EV_KEY:
		printf("EV_KEY");
		break;
	case EV_REL:
		printf("EV_REL");
		break;
	case EV_ABS:
		printf("EV_ABS");
		break;
	case EV_MSC:
		printf("EV_MSC");
		break;
	case EV_SW:
		printf("EV_SW");
		break;
	case EV_LED:
		printf("EV_LED");
		break;
	case EV_SND:
		printf("EV_SND");
		break;
	case EV_REP:
		printf("EV_REP");
		break;
	case EV_FF:
		printf("EV_FF");
		break;
	case EV_PWR:
		printf("EV_PWR");
		break;
	case EV_FF_STATUS:
		printf("EV_FF_STATUS");
		break;
	default:
		printf("<unknown> 0x%0x", value);
	}

	return true;
}

static void dump_device_caps(int fd)
{
	dump_state state;
	state.first = true;
	if (foreach_capability(fd, dump_single_capability, &state))
	{
		putchar('\n');
	}
}

bool print_input_device_info(int fd)
{
	int version = 0;
	if (ioctl(fd, EVIOCGVERSION, &version) != 0)
	{
		perror("can't get device version");
		return false;
	}
	printf("input driver version: %d.%d.%d\n", version >> 16, (version >> 8) & 0xff, version & 0xff);

	struct input_id id;
	if (ioctl(fd, EVIOCGID, &id) != 0)
	{
		perror("can't get device id");
		return false;
	}
	printf("input device id: bustype=0x%x, vendor=0x%x, product=0x%x, version=0x%x\n",
			(int)id.bustype, (int)id.vendor, (int)id.product, (int)id.version);

	char s[256];
	if (ioctl(fd, EVIOCGNAME(sizeof(s)), s) < 0)
		perror("cant'get device name");
	else
		printf("device name: '%s'\n", s);

	if (ioctl(fd, EVIOCGPHYS(sizeof(s)), s) < 0)
		perror("cant'get device name");
	else
		printf("physical path: '%s'\n", s);

	dump_device_caps(fd);

	return true;
}
