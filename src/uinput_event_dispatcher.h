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

#ifndef UINPUT_EVENT_DISPATCHER_H
#define UINPUT_EVENT_DISPATCHER_H

#include "event_dispatcher.h"

struct uinput_event_dispatcher
{
	struct event_dispatcher base;
	int uinput_dev_fd;
};

bool uinput_event_dispatcher_create(struct uinput_event_dispatcher *self, int real_input_dev_fd);

#endif // UINPUT_EVENT_DISPATCHER_H
