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

#include "config.h"
#ifndef HAVE_LINUX_UINPUT_H
	#error "uinput.h needed to compile this source"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <linux/input.h>
#include <linux/uinput.h>

#include "uinput_event_dispatcher.h"
#include "input_utils.h"

static bool uinput_event_dispatcher_dispatch(struct event_dispatcher *base, const struct input_event *events, int count);
static void uinput_event_dispatcher_destroy(struct event_dispatcher *base);

static const char UINPUT_CONTROL_NODE[] = "/dev/input/uinput";

static void set_details_for_capability_from_real_device(int uinput_ctl_fd, uint32_t capability, int real_input_dev_fd)
{
	// TODO
}

static bool set_event_bits_cb(int fd, uint32_t capability, void *user_data)
{
	(void)fd; // unused
	int uinput_ctl_fd = *(int const*)user_data;

	ioctl(uinput_ctl_fd, UI_SET_EVBIT, capability);
	set_details_for_capability_from_real_device(uinput_ctl_fd, capability, fd);

	return true;
}

bool uinput_event_dispatcher_create(struct uinput_event_dispatcher *self, int real_input_dev_fd)
{
	assert(self != NULL);
	self->base.dispatch = uinput_event_dispatcher_dispatch;
	self->base.destroy = uinput_event_dispatcher_destroy;

	self->uinput_dev_fd = -1;

	// TODO - http://thiemonge.org/getting-started-with-uinput
	int uinput_ctl_fd = open(UINPUT_CONTROL_NODE, O_WRONLY | O_NONBLOCK);
	if (uinput_ctl_fd < 0)
	{
		perror("can't open uinput");
		return false;
	}

	if (!foreach_capability(real_input_dev_fd, set_event_bits_cb, &uinput_ctl_fd))
	{
		return false;
	}

	struct uinput_user_dev uinput_dev;
	memset(&uinput_dev, 0, sizeof(uinput_dev));
	snprintf(uinput_dev.name, UINPUT_MAX_NAME_SIZE, "mt-translator check");

	// TODO: axes ranges

	// FIXME: copy from real device
	uinput_dev.id.bustype = BUS_USB;
	uinput_dev.id.vendor = 0x1234;
	uinput_dev.id.product = 0x5678;
	uinput_dev.id.version = 1;

	if (write(uinput_ctl_fd, &uinput_dev, sizeof(uinput_dev)) != sizeof(uinput_dev))
	{
		perror("can't set uinput device properties");
		close(uinput_ctl_fd);
		return false;
	}

	if (ioctl(uinput_ctl_fd, UI_DEV_CREATE) == -1)
	{
		perror("can't create uinput device");
		close(uinput_ctl_fd);
		return false;
	}

	self->uinput_dev_fd = uinput_ctl_fd;

	return true;
}

static bool uinput_event_dispatcher_dispatch(struct event_dispatcher *base, const struct input_event *events, int count)
{
	assert(base != NULL);
	struct uinput_event_dispatcher* const self = (struct uinput_event_dispatcher*)base;

	const size_t N = sizeof(*events)*count;
	if (write(self->uinput_dev_fd, events, N) != N)
	{
		perror("uinput_event_dispatcher_dispatch: write() failed!");
		return false;
	}
	return true;
}

static void uinput_event_dispatcher_destroy(struct event_dispatcher *base)
{
	assert(base != NULL);
	struct uinput_event_dispatcher* const self = (struct uinput_event_dispatcher*)base;

	ioctl(self->uinput_dev_fd, UI_DEV_DESTROY);
	close(self->uinput_dev_fd);
}
