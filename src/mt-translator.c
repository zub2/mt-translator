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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>

#include <linux/input.h>
#include <mtdev.h>

#include <assert.h>

#ifdef HAVE_LINUX_UINPUT_H
	#include <linux/uinput.h>
#endif

#include "config.h"

#include "mt-translator.h"
#include "event_dispatcher.h"
#include "pipe_event_dispatcher.h"
#ifdef HAVE_LINUX_UINPUT_H
	#include "uinput_event_dispatcher.h"
#endif
#include "input_utils.h"

static const unsigned MAX_EVENTS = 10;
const char *progname;

int translate_loop(int fd, struct event_dispatcher *ed)
{
	struct mtdev mtd;
	if (mtdev_open(&mtd, fd) == 0)
	{
		struct input_event events[MAX_EVENTS];
		int i = 0;
		while (!mtdev_idle(&mtd, fd, -1))
		{
			while (i = mtdev_get(&mtd, fd, events, sizeof(events)/sizeof(events[0])), i > 0)
			{
				if (!ed->dispatch(ed, events, i))
				{
					fprintf(stderr, "dispatch_events failed!\n");
					break;
				}
			}
		}

		mtdev_close(&mtd);
	}
	else
		fprintf(stderr, "mtdev_open failed!\n");

	return 0;
}

static const struct option long_options[] =
{
	{"help",			no_argument,		0,	'h'},
	{"version",			no_argument,		0,	'V'},
	{"input",		required_argument,		0,	'i'},
	{"pipe",		required_argument,		0,	'p'},
	{"verbose",			no_argument,		0,	'v'},
#ifdef HAVE_LINUX_UINPUT_H
	{"uinput",			no_argument,		0,	'u'},
#endif

	{0, 0, 0, 0}
};

const char short_options[] = "hVi:p:v"
#ifdef HAVE_LINUX_UINPUT
		"u"
#endif
		;

int main(int argc, char **argv)
{
	const char *input_dev = NULL;
	const char *out_fifo = NULL;

	bool display_help = false;
	bool display_version = false;
	bool verbose = false;
#ifdef HAVE_LINUX_UINPUT_H
	bool use_uinput = false;
#endif

	progname = (argc > 0) ? argv[0] : PACKAGE_NAME;

	while (true)
	{
		int option_index = 0;
		int c = getopt_long(argc, argv, short_options, long_options, &option_index);
		if (c == -1)
			break;

		switch (c)
		{
		case 'h':
			display_help = true;
			break;
		case 'V':
			display_version = true;
			break;
		case 'i':
			input_dev = optarg;
			break;
		case 'p':
			out_fifo = optarg;
			break;
		case 'v':
			verbose = true;
			break;
#ifdef HAVE_LINUX_UINPUT_H
		case 'u':
			use_uinput = true;
			break;
#endif
		default:
			abort();
		}
	}

	if (display_help)
	{
		printf("Usage: %s "
#ifdef HAVE_LINUX_UINPUT_H
			"{-i input_dev {-u | {-p output_fifo} } | { [--help] [--version]} [--verbose]"
#else
			"{-i input_dev -p -o output_fifo } | { [--help] [--version]} [--verbose]"
#endif
			"\n", progname);
		return 0;
	}

	if (display_version)
	{
		printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
		return 0;
	}

	if (!input_dev)
	{
		fprintf(stderr, "Missing input device argument.\n");
		return 2;
	}

#ifdef HAVE_LINUX_UINPUT_H
	if (use_uinput && out_fifo)
	{
		printf("%s: can use only one of --uinput and --pipe\n", progname);
		return 1;
	}

	// default to pipe
	if (!use_uinput && !out_fifo)
	{
		printf("%s: need to specify one of --pipe or --uinput\n", progname);
		return 1;
	}
#else
	if (!out_fifo)
	{
		printf("%s: missing output fifo argument (--pipe)\n", progname);
		return 1;
	}
#endif

	if (verbose)
	{
		printf("opening input device '%s'\n", input_dev);
	}
	int fd = open(input_dev, O_RDONLY | O_NONBLOCK);
	if (fd < 0)
	{
		perror("can't open input device");
		return 1;
	}

	if (verbose)
	{
		if (!print_input_device_info(fd))
			return 1;
	}

	struct event_dispatcher *base = NULL;
	if (out_fifo)
	{
		struct pipe_event_dispatcher *ed = (struct pipe_event_dispatcher*)malloc(sizeof(ed));
		if (!ed)
		{
			fprintf(stderr, "can't allocate dispatcher instance\n");
			return 4;
		}
		if (!pipe_event_dispatcher_create(ed, out_fifo))
		{
			fprintf(stderr, "pipe_event_dispatcher_create failed!\n");
			return 4;
		}
		base = (struct event_dispatcher*)ed;
	}
#ifdef HAVE_LINUX_UINPUT_H
	else
	{
		// uinput
		struct uinput_event_dispatcher *ed = (struct uinput_event_dispatcher*)malloc(sizeof(ed));
		if (!ed)
		{
			fprintf(stderr, "can't allocate dispatcher instance\n");
			return 4;
		}
		if (!uinput_event_dispatcher_create(ed, out_fifo))
		{
			fprintf(stderr, "uinput_event_dispatcher_create failed!\n");
			return 4;
		}
		base = (struct event_dispatcher*)ed;
	}
#endif

	int r = translate_loop(fd, base);
	close(fd);

	base->destroy(base);
	free(base);

	return r;
}
