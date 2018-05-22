#ifndef DRIVER_H
#define DRIVER_H

#include <X11/Intrinsic.h>

#include "mw_screensaver_linux_config.h"

#define ATTR_ALIGN(x) __attribute__ ((aligned(x)))

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#ifdef HAVE_GETOPT
#include <unistd.h>
#endif
#endif

#define True 1
#define False 0

typedef struct xstuff {
	char *commandLineName;

	Display *display;

	int screen_num;

	Window rootWindow;
	Window window;
	Window existingWindow;

	unsigned int windowWidth, windowHeight;	/* dimensions in pixels */

	GC gc;			/* Graphics context. */

	Colormap colourMap;

	void *hackstuff;
} xstuff_t;

#define DRIVER_OPTIONS_LONG {"root", 0, 0, 'r'}, {"maxfps", 1, 0, 'x'}, {"nice", 0, 0, 'n'},
#define DRIVER_OPTIONS_SHORT "rx:n"
#define DRIVER_OPTIONS_HELP "\t--root/-r\n" "\t--maxfps/-x <arg>\n" "\t--nice/-n\n"
#define DRIVER_OPTIONS_CASES case 'r': case 'x': case 'n': handle_global_opts(c); break;

void handle_global_opts (int c);
int strtol_minmaxdef(char *optarg, int base, int min, int max, int type, int def, char *errmsg);

void hack_handle_opts (int argc, char **argv);
void hack_init (xstuff_t *);
void hack_reshape (xstuff_t *);
void hack_draw (xstuff_t *, double, float);
void hack_cleanup (xstuff_t *);

#endif
