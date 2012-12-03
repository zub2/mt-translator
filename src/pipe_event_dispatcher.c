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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "pipe_event_dispatcher.h"

static bool pipe_event_dispatcher_dispatch(struct event_dispatcher *base, const struct input_event *events, int count);
static void pipe_event_dispatcher_destroy(struct event_dispatcher *base);

bool pipe_event_dispatcher_create(struct pipe_event_dispatcher *self, const char *fifo_name)
{
	assert(self != NULL);
	self->base.dispatch = pipe_event_dispatcher_dispatch;
	self->base.destroy = pipe_event_dispatcher_destroy;

	self->fifo_fd = -1;
	self->unlink_on_close = false;
	self->fifo_name = NULL;

	int fd = open(fifo_name, O_WRONLY);
	if (fd >= 0)
	{
		struct stat s;
		if (fstat(fd, &s) >= 0)
		{
			if (!S_ISFIFO(s.st_mode))
			{
				fprintf(stderr, "output is not a fifo!");
				close(fd);
				return false;
			}
		}
		else
		{
			perror("can't stat output fifo");
			close(fd);
			fd = -1;
		}
	}

	if (fd < 0)
	{
		int r = mkfifo(fifo_name, 0660);
		if (r >= 0)
		{
			fd = open(fifo_name, O_WRONLY);
			if (fd < 0)
			{
				unlink(fifo_name);
				fd = -1;
			}
			else
			{
				self->unlink_on_close = true;
				self->fifo_name = strdup(fifo_name);
			}
		}
		else
			fd = -1;
	}

	self->fifo_fd = fd;
	return fd >= 0;
}

static bool pipe_event_dispatcher_dispatch(struct event_dispatcher *base, const struct input_event *events, int count)
{
	assert(base != NULL);
	struct pipe_event_dispatcher* const self = (struct pipe_event_dispatcher*)base;

	const size_t N = sizeof(*events)*count;
	if (write(self->fifo_fd, events, N) != N)
	{
		perror("pipe_event_dispatcher_dispatch: write() failed!");
		return false;
	}
	return true;
}

static void pipe_event_dispatcher_destroy(struct event_dispatcher *base)
{
	assert(base != NULL);
	struct pipe_event_dispatcher* const self = (struct pipe_event_dispatcher*)base;

	if (self->unlink_on_close)
		unlink(self->fifo_name);

	close(self->fifo_fd);
	free(self->fifo_name);
}
