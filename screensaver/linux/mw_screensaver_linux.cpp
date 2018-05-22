/*

	@file mw_screensaver_linux.cpp

	$Id: mw_screensaver_linux.cpp,v 1.5 2005/04/21 13:01:43 james Exp $

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
 * $Log: mw_screensaver_linux.cpp,v $
 * Revision 1.5  2005/04/21 13:01:43  james
 * *** empty log message ***
 *
 * Revision 1.4  2005/04/21 12:59:18  james
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
 * OSX Screensaver
 *
 */

/*	Make this 1 is you want to see LOG functions to behave
    like they do in _DEBUG mode for this file.*/
#if 0 //_DEVELOPER
#ifndef _DEVELOPER
#	define _DEVELOPER
#endif //_DEVELOPER
#endif


#include <math.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "mw_screensaver_linux_x11driver.h"
#include "mw_screensaver_linux.h"

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

char *hack_name = (char *)"mw_screensaver_linux";

#define NUMCONSTS 18
#define MAXTEXSIZE 1024

// Globals
float aspectRatio;
float wide;
float high;
float c[NUMCONSTS];		// constant
float ct[NUMCONSTS];		// temporary value of constant
float cv[NUMCONSTS];		// velocity of constant
float ***position;
float ***mw_screensaver_linux;
unsigned int tex;
int texsize = 256;
int mw_screensaver_linuxWidth, mw_screensaver_linuxHeight;
float texright, textop;
float *mw_screensaver_linuxmap;

// Parameters edited in the dialog box
int dZoom;
int dFocus;
int dSpeed;
int dResolution;

// Find absolute value and truncate to 1.0
#define fabstrunc(f) (f >= 0.0f ? (f <= 1.0f ? f : 1.0f) : (f >= -1.0f ? -f : 1.0f))

/*
float fabstrunc (float f)
{
	if (f >= 0.0f) {
		return (f <= 1.0f ? f : 1.0f);
	} else {
		return (f >= -1.0f ? -f : 1.0f);
	}
}
*/

void hack_draw (xstuff_t * XStuff, double currentTime, float frameTime)
{
	int i, j;
	float rgb[3];
	float temp;
	static float focus = float (dFocus) / 50.0f + 0.3f;
	static float maxdiff = 0.004f * float (dSpeed);
	static int index;

	// Update constants
	for (i = 0; i < NUMCONSTS; i++) {
		ct[i] += cv[i];
		if (ct[i] > PIx2)
			ct[i] -= PIx2;
		c[i] = sin (ct[i]) * focus;
	}

	// Update colors
	for (i = 0; i < mw_screensaver_linuxHeight; i++) {
		for (j = 0; j < mw_screensaver_linuxWidth; j++) {
			// Calculate vertex colors
			rgb[0] = mw_screensaver_linux[i][j][0];
			rgb[1] = mw_screensaver_linux[i][j][1];
			rgb[2] = mw_screensaver_linux[i][j][2];
			mw_screensaver_linux[i][j][0] = 0.7f * (c[0] * position[i][j][0] + c[1] * position[i][j][1]
						  + c[2] * (position[i][j][0] * position[i][j][0] + 1.0f)
						  + c[3] * position[i][j][0] * position[i][j][1]
						  + c[4] * rgb[1] + c[5] * rgb[2]);
			mw_screensaver_linux[i][j][1] = 0.7f * (c[6] * position[i][j][0] + c[7] * position[i][j][1]
						  + c[8] * position[i][j][0] * position[i][j][0]
						  + c[9] * (position[i][j][1] * position[i][j][1] - 1.0f)
						  + c[10] * rgb[0] + c[11] * rgb[2]);
			mw_screensaver_linux[i][j][2] = 0.7f * (c[12] * position[i][j][0] + c[13] * position[i][j][1]
						  + c[14] * (1.0f - position[i][j][0] * position[i][j][1])
						  + c[15] * position[i][j][1] * position[i][j][1]
						  + c[16] * rgb[0] + c[17] * rgb[1]);

			// Don't let the colors change too much
			temp = mw_screensaver_linux[i][j][0] - rgb[0];
			if (temp > maxdiff)
				mw_screensaver_linux[i][j][0] = rgb[0] + maxdiff;
			if (temp < -maxdiff)
				mw_screensaver_linux[i][j][0] = rgb[0] - maxdiff;
			temp = mw_screensaver_linux[i][j][1] - rgb[1];
			if (temp > maxdiff)
				mw_screensaver_linux[i][j][1] = rgb[1] + maxdiff;
			if (temp < -maxdiff)
				mw_screensaver_linux[i][j][1] = rgb[1] - maxdiff;
			temp = mw_screensaver_linux[i][j][2] - rgb[2];
			if (temp > maxdiff)
				mw_screensaver_linux[i][j][2] = rgb[2] + maxdiff;
			if (temp < -maxdiff)
				mw_screensaver_linux[i][j][2] = rgb[2] - maxdiff;

			// Put colors into texture
			index = (i * texsize + j) * 3;
			mw_screensaver_linuxmap[index] = fabstrunc (mw_screensaver_linux[i][j][0]);
			mw_screensaver_linuxmap[index + 1] = fabstrunc (mw_screensaver_linux[i][j][1]);
			mw_screensaver_linuxmap[index + 2] = fabstrunc (mw_screensaver_linux[i][j][2]);
		}
	}

	// Update texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, texsize);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, mw_screensaver_linuxWidth, mw_screensaver_linuxHeight, GL_RGB, GL_FLOAT, mw_screensaver_linuxmap);

	// Draw it
	glBegin (GL_TRIANGLE_STRIP);
	glTexCoord2f (0.0f, 0.0f);
	glVertex2f (0.0f, 0.0f);
	glTexCoord2f (0.0f, texright);
	glVertex2f (1.0f, 0.0f);
	glTexCoord2f (textop, 0.0f);
	glVertex2f (0.0f, 1.0f);
	glTexCoord2f (textop, texright);
	glVertex2f (1.0f, 1.0f);
	glEnd ();
}

void hack_reshape (xstuff_t * XStuff)
{
	// Window initialization
	glViewport (0, 0, XStuff->windowWidth, XStuff->windowHeight);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (0.0f, 1.0f, 0.0f, 1.0f);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

void hack_init (xstuff_t * XStuff)
{
	int i, j;

	hack_reshape (XStuff);

	aspectRatio = float (XStuff->windowWidth) / float (XStuff->windowHeight);

	if (aspectRatio >= 1.0f) {
		wide = 30.0f / float (dZoom);

		high = wide / aspectRatio;
	} else {
		high = 30.0f / float (dZoom);

		wide = high * aspectRatio;
	}

	// Set resolution of mw_screensaver_linux
	if (aspectRatio >= 1.0f)
		mw_screensaver_linuxHeight = (dResolution * MAXTEXSIZE) / 100;
	else
		mw_screensaver_linuxHeight = int (float (dResolution * MAXTEXSIZE) * aspectRatio * 0.01f);

	mw_screensaver_linuxWidth = int (float (mw_screensaver_linuxHeight) / aspectRatio);

	// Set resolution of texture
	texsize = 16;
	if (aspectRatio >= 1.0f)
		while (mw_screensaver_linuxHeight > texsize)
			texsize *= 2;
	else
		while (mw_screensaver_linuxWidth > texsize)
			texsize *= 2;

	// The "- 1" cuts off right and top edges to get rid of blending to black
	texright = float (mw_screensaver_linuxHeight - 1) / float (texsize);
	textop = texright / aspectRatio;

	// Initialize memory and positions
	mw_screensaver_linuxmap = new float[texsize * texsize * 3];

	for (i = 0; i < texsize * texsize * 3; i++)
		mw_screensaver_linuxmap[i] = 0.0f;
	mw_screensaver_linux = new float **[mw_screensaver_linuxHeight];
	position = new float **[mw_screensaver_linuxHeight];

	for (i = 0; i < mw_screensaver_linuxHeight; i++) {
		mw_screensaver_linux[i] = new float *[mw_screensaver_linuxWidth];
		position[i] = new float *[mw_screensaver_linuxWidth];

		for (j = 0; j < mw_screensaver_linuxWidth; j++) {
			mw_screensaver_linux[i][j] = new float[3];
			position[i][j] = new float[2];

			mw_screensaver_linux[i][j][0] = 0.0f;
			mw_screensaver_linux[i][j][1] = 0.0f;
			mw_screensaver_linux[i][j][2] = 0.0f;
			position[i][j][0] = float (i * wide) / float (mw_screensaver_linuxHeight - 1) - (wide * 0.5f);
			position[i][j][1] = float (j * high) / (float(mw_screensaver_linuxHeight) / aspectRatio - 1.0f) - (high * 0.5f);
		}
	}
	// Initialize constants
	for (i = 0; i < NUMCONSTS; i++) {
		ct[i] = rsRandf (PIx2);
		cv[i] = rsRandf (0.005f * dSpeed) + 0.0001f;
	}

	// Make texture
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D (GL_TEXTURE_2D, 0, 3, texsize, texsize, 0, GL_RGB, GL_FLOAT, mw_screensaver_linuxmap);
	glEnable (GL_TEXTURE_2D);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

void hack_cleanup (xstuff_t * XStuff)
{
}

void hack_handle_opts (int argc, char **argv)
{
	dZoom = 10;
	dFocus = 30;
	dSpeed = 20;
	dResolution = 25;

	while (1) {
		int c;

#ifdef HAVE_GETOPT_H
		static struct option long_options[] = {
			{"help", 0, 0, 'h'},
			DRIVER_OPTIONS_LONG
			{"zoom", 1, 0, 'z'},
			{"focus", 1, 0, 'f'},
			{"speed", 1, 0, 's'},
			{"resolution", 1, 0, 'R'},
			{0, 0, 0, 0}
		};

		c = getopt_long (argc, argv, DRIVER_OPTIONS_SHORT "hz:f:s:R:", long_options, NULL);
#else
		c = getopt (argc, argv, DRIVER_OPTIONS_SHORT "hz:f:s:R:");
#endif
		if (c == -1)
			break;

		switch (c) {
			DRIVER_OPTIONS_CASES case 'h':printf ("%s:"
#ifndef HAVE_GETOPT_H
							      " Not built with GNU getopt.h, long options *NOT* enabled."
#endif
							      "\n" DRIVER_OPTIONS_HELP "\t--zoom/-z <arg>\n" "\t--focus/-f <arg>\n" "\t--speed/-s <arg>\n"
							      "\t--resolution/-R <arg>\n", argv[0]);
			exit (1);
		case 'z':
			dZoom = strtol_minmaxdef (optarg, 10, 1, 100, 1, 10, "--zoom: ");
			break;
		case 'f':
			dFocus = strtol_minmaxdef (optarg, 10, 1, 100, 1, 30, "--focus: ");
			break;
		case 's':
			dSpeed = strtol_minmaxdef (optarg, 10, 1, 100, 1, 20, "--speed: ");
			break;
		case 'R':
			dResolution = strtol_minmaxdef (optarg, 10, 1, 100, 1, 25, "--resolution: ");
			break;
		}
	}
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

