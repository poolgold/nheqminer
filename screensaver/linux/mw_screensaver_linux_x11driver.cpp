/*

	@file mw_screensaver_linux.cpp

	$Id: mw_screensaver_linux_x11driver.cpp,v 1.1 2005/04/21 13:01:43 james Exp $

	@author     James Mc Parlane

	PROJECT:    MetaWrap Server (Amphibian)

	COMPONENT:  -

	@date       11 September 2001


	GENERAL INFO:

		Massive Technologies
		PO Box 567
		Darlinghurst 2010
		NSW, Australia
		email:	james@massive.com.au
		tel:	(+61-2) 9331 8699
		fax:	(+61-2) 9331 8699
		mob:	(+61) 407-909-186


	LICENSE:

	Copyright (C) 2001  Massive Technologies, Pty Ltd.

	MetaWrap is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	In addition, as a special exception, Massive Technologies
	gives permission for parties to develop 'Plugins' via the
	'PluginManager'. Said party is free to develop a proprietary
	'Plugin' and will not be forced to distribute source code for that
	'Plugin', but we of course encourage them to do so. You must obey the GNU
	General Public License in all respects for all of the code used
	other than interfacing with the 'PluginManager'.  If you modify this
	file, you may extend this exception to your version of the file, but
	you are not obligated to do so.  If you do not wish to do so, delete
	this exception statement from your version.

*/

/*
 * $Log: mw_screensaver_linux_x11driver.cpp,v $
 * Revision 1.1  2005/04/21 13:01:43  james
 * *** empty log message ***
 *
 * Revision 1.3  2005/04/21 12:59:18  james
 * *** empty log message ***
 *
 * Revision 1.3  2005/04/21 12:49:54  james
 * *** empty log message ***
 *
 * Revision 1.2  2005/04/21 12:44:02  james
 * *** empty log message ***
 *
 * Revision 1.1  2005/04/21 12:42:44  james
 * *** empty log message ***
 *
 * Revision 1.2  2005/04/21 12:06:12  james
 * Edited to compile locally
 *
 * Revision 1.2  2005/04/19 14:13:06  james
 * Linux Screensaver
 *
 */

/*	Make this 1 is you want to see LOG functions to behave
    like they do in _DEBUG mode for this file.*/
#if 0 //_DEVELOPER
#ifndef _DEVELOPER
#	define _DEVELOPER
#endif //_DEVELOPER
#endif


#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "mw_screensaver_linux_x11driver.h"

#include "vroot.h"


/*! \page mw_screensaver_linux MetaWrap - Utility - Screensaver - Linux
 *
 * \subsection mw_screensaver_linux Overview
 *
 */

/* As long as we are not building the unit tests for this file.*/
#ifndef __TEST__

/*! \defgroup mw_screensaver_linux MetaWrap - Utility - Screensaver - Linux
 *@{
 */


xstuff_t *XStuff;

extern char *hack_name;

/*
 * display parameters
 */
int rootWindow = False;
int frameTime = 0;
int be_nice = 0;
int signalled = 0;

void createWindow (int argc, char **argv)
{
	XVisualInfo *visualInfo;
	GLXContext context;

	XStuff->screen_num = DefaultScreen (XStuff->display);
	XStuff->rootWindow = RootWindow (XStuff->display, XStuff->screen_num);

	if (rootWindow || XStuff->existingWindow) {
		XWindowAttributes gwa;
		Visual *visual;
		XVisualInfo templ;
		int outCount;

		XStuff->window = XStuff->existingWindow ? XStuff->existingWindow : XStuff->rootWindow;

		XGetWindowAttributes (XStuff->display, XStuff->window, &gwa);
		visual = gwa.visual;
		XStuff->windowWidth = gwa.width;
		XStuff->windowHeight = gwa.height;

		templ.screen = XStuff->screen_num;
		templ.visualid = XVisualIDFromVisual (visual);

		visualInfo = XGetVisualInfo (XStuff->display, VisualScreenMask | VisualIDMask, &templ, &outCount);

		if (!visualInfo) {
			fprintf (stderr, "%s: can't get GL visual for window 0x%lx.\n", XStuff->commandLineName, (unsigned long)XStuff->window);
			exit (1);
		}
	} else {
		int attributeList[] = {
			GLX_RGBA,
			GLX_RED_SIZE, 1,
			GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1,
			GLX_DEPTH_SIZE, 1,
			GLX_DOUBLEBUFFER,
			0
		};
		XSetWindowAttributes swa;
		XSizeHints hints;
		XWMHints wmHints;

		visualInfo = NULL;

		if (!(visualInfo = glXChooseVisual (XStuff->display, XStuff->screen_num, attributeList))) {
			fprintf (stderr, "%s: can't open GL visual.\n", XStuff->commandLineName);
			exit (1);
		}

		swa.colormap = XCreateColormap (XStuff->display, XStuff->rootWindow, visualInfo->visual, AllocNone);
		swa.border_pixel = swa.background_pixel = swa.backing_pixel = BlackPixel (XStuff->display, XStuff->screen_num);
		swa.event_mask = KeyPressMask | StructureNotifyMask;

		XStuff->windowWidth = DisplayWidth (XStuff->display, XStuff->screen_num) / 3;
		XStuff->windowHeight = DisplayHeight (XStuff->display, XStuff->screen_num) / 3;

		XStuff->window =
			XCreateWindow (XStuff->display, XStuff->rootWindow, 0, 0, XStuff->windowWidth, XStuff->windowHeight, 0, visualInfo->depth, InputOutput, visualInfo->visual,
				       CWBorderPixel | CWBackPixel | CWBackingPixel | CWColormap | CWEventMask, &swa);

		hints.flags = USSize;
		hints.width = XStuff->windowWidth;
		hints.height = XStuff->windowHeight;

		wmHints.flags = InputHint;
		wmHints.input = True;

		XmbSetWMProperties (XStuff->display, XStuff->window, hack_name, hack_name, argv, argc, &hints, &wmHints, NULL);
	}

	context = glXCreateContext (XStuff->display, visualInfo, 0, GL_TRUE);
	if (!context) {
		fprintf (stderr, "%s: can't open GLX context.\n", XStuff->commandLineName);
		exit (1);
	}

	if (!glXMakeCurrent (XStuff->display, XStuff->window, context)) {
		fprintf (stderr, "%s: can't set GL context.\n", XStuff->commandLineName);
		exit (1);
	}

	XFree (visualInfo);
	XMapWindow (XStuff->display, XStuff->window);
}

void clearBuffers() {
	int i;
	XEvent event;

	for (i = 0; i < 4; i++) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glXSwapBuffers (XStuff->display, XStuff->window);

		while (XPending (XStuff->display)) {
			XNextEvent (XStuff->display, &event);
		}
	}
}

void mainLoop (void)
{
	int bFPS = False;
	XEvent event;
	Atom XA_WM_PROTOCOLS = XInternAtom (XStuff->display, "WM_PROTOCOLS", False);
	Atom XA_WM_DELETE_WINDOW = XInternAtom (XStuff->display, "WM_DELETE_WINDOW", False);
	struct timeval then, now, fps_time;
	int fps = 0;

	if (!rootWindow) {
		XSetWMProtocols (XStuff->display, XStuff->window, &XA_WM_DELETE_WINDOW, 1);
	}

	clearBuffers();

	gettimeofday (&now, NULL);
	int frameTimeSoFar = 0;
	while (!signalled) {
		hack_draw (XStuff, now.tv_sec + now.tv_usec / 1000000.0f, frameTimeSoFar / 1000000.0f);

		glXSwapBuffers (XStuff->display, XStuff->window);

		if (bFPS) {
			if (fps != -1)
				fps++;

			gettimeofday (&now, NULL);

			if (now.tv_sec > fps_time.tv_sec) {
				if (fps != -1) {
					printf ("%d fps\n", fps);
				}

				fps = 0;
				fps_time.tv_sec = now.tv_sec;
				fps_time.tv_usec = now.tv_usec;
			}
		}

		while (XPending (XStuff->display)) {
			KeySym keysym;
			char c = 0;

			XNextEvent (XStuff->display, &event);
			switch (event.type) {
			case ConfigureNotify:
				if ((int)XStuff->windowWidth != event.xconfigure.width || (int)XStuff->windowHeight != event.xconfigure.height) {
					XStuff->windowWidth = event.xconfigure.width;
					XStuff->windowHeight = event.xconfigure.height;

					clearBuffers();

					hack_reshape (XStuff);
				}

				break;
			case KeyPress:
				XLookupString (&event.xkey, &c, 1, &keysym, 0);

				if (c == 'f') {
					bFPS = !bFPS;

					if (bFPS) {
						fps = -1;
						gettimeofday (&fps_time, NULL);
					}
				}

				if (c == 'q' || c == 'Q' || c == 3 || c == 27)
					return;

				break;
			case ClientMessage:
				if (event.xclient.message_type == XA_WM_PROTOCOLS) {
					if (event.xclient.data.l[0] == XA_WM_DELETE_WINDOW) {
						return;
					}
				}
				break;
			case DestroyNotify:
				return;
			}
		}

		then = now;
		gettimeofday (&now, NULL);
		frameTimeSoFar = (now.tv_sec - then.tv_sec) * 1000000 + now.tv_usec - then.tv_usec;

		if (frameTime) {
			while (frameTimeSoFar < frameTime) {
				if (be_nice) {
/* nanosleep on Linux/i386 seems completely ineffective for idling for < 20ms */
/*
#ifdef HAVE_NANOSLEEP
					struct timespec hundreth;

					hundreth.tv_sec = 0;
					hundreth.tv_nsec = frameTime - frameTimeSoFar;

					nanosleep(&hundreth, NULL);
#endif
*/

/*
					usleep(frameTime - frameTimeSoFar);
*/

					struct timeval tv;

					tv.tv_sec = 0;
					tv.tv_usec = frameTime - frameTimeSoFar;
					select (0, 0, 0, 0, &tv);
				}

				gettimeofday (&now, NULL);
				frameTimeSoFar = (now.tv_sec - then.tv_sec) * 1000000 + now.tv_usec - then.tv_usec;
			}
		} else if (be_nice) {
			struct timeval tv;

			tv.tv_sec = 0;
			tv.tv_usec = 1000;
			select (0, 0, 0, 0, &tv);
		}
	}
}

void handle_global_opts (int c)
{
	switch (c) {
	case 'r':
		rootWindow = 1;

		break;
	case 'x':
		c = strtol_minmaxdef (optarg, 10, 1, 10000, 1, 100, "--maxfps: ");

		frameTime = 1000000 / c;

		break;
	case 'n':
		be_nice = 1;

		break;
	}
}

int strtol_minmaxdef (char *optarg, int base, int min, int max, int type, int def, char *errmsg)
{
	int result = strtol (optarg, (char **)NULL, base);

	if (result < min) {
		if (errmsg) {
			fprintf (stderr, errmsg);
			fprintf (stderr, "%d < %d, using %d instead.\n", result, min, type ? min : def);
		}

		return type ? min : def;
	}

	if (result > max) {
		if (errmsg) {
			fprintf (stderr, errmsg);
			fprintf (stderr, "%d > %d, using %d instead.\n", result, max, type ? max : def);
		}

		return type ? max : def;
	}

	return result;
}

void signalHandler (int sig)
{
	signalled = 1;
}

int main (int argc, char *argv[])
{
	struct sigaction sa;
	char *display_name = NULL;	/* Server to connect to */
	int i, j;

	XStuff = (xstuff_t *) malloc (sizeof (xstuff_t));
	XStuff->commandLineName = argv[0];

#ifdef BENCHMARK
	srandom (1);
#else
	srandom ((unsigned)time (NULL));
#endif

	XStuff->existingWindow = 0;
	for (i = 0; i < argc; i++) {
		if (!strcmp (argv[i], "-window-id")) {
			if ((argv[i + 1][0] == '0') && ((argv[i + 1][1] == 'x') || (argv[i + 1][1] == 'X'))) {
				XStuff->existingWindow = strtol ((char *)(argv[i + 1] + 2), (char **)NULL, 16);
			} else {
				XStuff->existingWindow = strtol ((char *)(argv[i + 1]), (char **)NULL, 10);
			}

			for (j = i + 2; j < argc; j++) {
				argv[j - 2] = argv[j];
			}

			argc -= 2;

			break;
		}
	}

	hack_handle_opts (argc, argv);

	XStuff->display = NULL;
	XStuff->window = 0;

	memset ((void *)&sa, 0, sizeof (struct sigaction));
	sa.sa_handler = signalHandler;
	sigaction (SIGINT, &sa, 0);
	sigaction (SIGPIPE, &sa, 0);
	sigaction (SIGQUIT, &sa, 0);
	sigaction (SIGTERM, &sa, 0);

	/*
	 * Connect to the X server.
	 */
	if (NULL == (XStuff->display = XOpenDisplay (display_name))) {
		fprintf (stderr, "%s: can't connect to X server %s\n", XStuff->commandLineName, XDisplayName (display_name));
		exit (1);
	}

	createWindow (argc, argv);

	hack_init (XStuff);

	mainLoop ();

	/*
	 * Clean up.
	 */
	if (XStuff->display) {
		if (XStuff->window) {
			hack_cleanup (XStuff);

			if (!((rootWindow) || (XStuff->existingWindow)))
				XDestroyWindow (XStuff->display, XStuff->window);
		}

		XCloseDisplay (XStuff->display);
	}

	return 0;
}

/* end of mw_screensaver_osx*/
/*@}*/

#endif /* __TEST__ */


#ifdef __TEST__

#ifndef MW_TEST_LEGACY_H
#include "mw_test_legacy.h"
#endif /* MW_TEST_LEGACY_H */

#ifndef _MW_ARGUMENTS_H_
#include "mw_arguments.h"
#endif // _MW_ARGUMENTS_H_

int main(int argc, const char * argv[])
{
	// this will be our return value
	int l_retval = 0;

    // open the CRT lib
    generic_crt_open();

    // start using the server log functions for logging
    mw_init_logging();

    // set up our global objects
    g_globals = new MwGlobal();

    // Set up our globals - we can't even print to the console or log till this is opened
    g_globals->Open();

    // parse arguments for the test
    if (!mw_interpret_arguments(argc, argv))
    {
	    // Close down our global objects
	    g_globals->Close();

	    // Dispose of the global object holder
	    delete g_globals;
	    g_globals = 0;

	    /* init generic_crt */
	    generic_crt_close();

	    /* return test status*/
	    return l_retval;
    }

	LOG("MAIN","+ initialising metawrap engine.");

    // redirect test output to this standard function
    mw_set_print(logext);

	// start up the metawrap engine
	mw_open();

	// get the args
	mw_test_legacy_process_arguments(argc,argv);

	// start reporting
	mw_test_legacy_start();

	// report a successful test
	mw_test_legacy_testcase_PASSED("mw_screensaver_osx compiles","Basic test of MetaWrap - Utility - Screensaver - OSX ");

    //Test_MwW3ParserLexerBuilder_Simple();

	// start reporting
	mw_test_legacy_end();

	LOG("MAIN","+ closing metawrap.");

	// shutdown the metawrap engine
	mw_close();

	LOG("MAIN","+ closing metawrap instance");

	// Close down our global objects
	g_globals->Close();

	// Dispose of the global object holder
	delete g_globals;
	g_globals = 0;

	/* get the return status*/
	l_retval = test_return_status();

	/* init generic_crt */
	generic_crt_close();

	/* return test status*/
	return l_retval;
}

#endif /* __TEST__ */


