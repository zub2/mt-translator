AM_CFLAGS = $(MTDEV_CFLAGS)

bin_PROGRAMS = mt-translator

mt_translator_SOURCES = \
	mt-translator.c \
	input_utils.c \
	pipe_event_dispatcher.c

if USE_UINPUT
	uinput_event_dispatcher.c
endif

mt_translator_LDFLAGS = -static-libtool-libs $(MTDEV_LIBS)
