#include <stdio.h>
#include <mtdev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

static const unsigned MAX_EVENTS = 10;
const char *progname;

struct event_dispatcher
{
	int fifo_fd;
	bool unlink_on_close;
	char *fifo_name;
};

bool event_dispatcher_init(struct event_dispatcher *ed, const char *fifo_name)
{
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

	ed->unlink_on_close = false;
	ed->fifo_name = NULL;
	if (fd < 0)
	{
		fd = mkfifo(fifo_name, 0330);
		ed->unlink_on_close = true;
		ed->fifo_name = strdup(fifo_name);
	}

	ed->fifo_fd = fd;
	return fd >= 0;
}

void event_dispatcher_destroy(struct event_dispatcher *ed)
{
	if (ed->unlink_on_close)
	{
		unlink(ed->fifo_name);
	}
	close(ed->fifo_fd);
	free(ed->fifo_name);
}

bool event_dispatcher_dispatch(struct event_dispatcher *ed, const struct input_event *events, int count)
{
	const size_t N = sizeof(*events)*count;
	if (write(ed->fifo_fd, events, N) != N)
	{
		perror("event_dispatcher_dispatch: write() failed!");
		return false;
	}
	return true;
}

static const struct option long_options[] =
{
	{"input",		required_argument,		0,	'i'},
	{"output",		required_argument,		0,	'o'},

	{"help",			no_argument,		0,	'h'},
	{"version",			no_argument,		0,	'V'},

	{0, 0, 0, 0}
};

const char short_options[] = "i:o:hV";

int main(int argc, char **argv)
{
	const char *input_dev = NULL;
	const char *out_fifo = NULL;

	bool display_help = false;
	bool display_version = false;

	progname = (argc > 0) ? argv[0] : PACKAGE_NAME;

	while (true)
	{
		int option_index = 0;
		int c = getopt_long(argc, argv, short_options, long_options, &option_index);
		if (c == -1)
			break;

		switch (c)
		{
		case 'i':
			input_dev = optarg;
			break;
		case 'o':
			out_fifo = optarg;
			break;
		case 'h':
			display_help = true;
			break;
		case 'V':
			display_version = true;
			break;
		default:
			abort();
		}
	}

	if (display_help)
	{
		printf("Usage: %s {-i input_dev -o output_fifo} | { [--help] [--version]}\n", progname);
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

	if (!out_fifo)
	{
		fprintf(stderr, "Missing output fifo argument.\n");
		return 3;
	}

	int fd = open(input_dev, O_RDONLY);
	if (fd < 0)
	{
		perror("can't open input device");
		return 0;
	}

	struct mtdev *mtd = mtdev_new_open(fd);
	if (mtd != 0)
	{
		struct event_dispatcher ed;
		if (event_dispatcher_init(&ed, out_fifo))
		{
			struct input_event events[MAX_EVENTS];
			int i = 0;
			while (i = mtdev_get(mtd, fd, events, sizeof(events)/sizeof(events[0])), i > 0)
			{
				if (!event_dispatcher_dispatch(&ed, events, i))
				{
					fprintf(stderr, "dispatch_events failed!\n");
					break;
				}
			}

			event_dispatcher_destroy(&ed);
		}
		else
		{
			fprintf(stderr, "event_dispatcher_init failed!\n");
		}
		
		mtdev_close_delete(mtd);
	}
	else
	{
		fprintf(stderr, "mtdev_new_open failed!\n");
	}

	close(fd);
	return 0;
}
