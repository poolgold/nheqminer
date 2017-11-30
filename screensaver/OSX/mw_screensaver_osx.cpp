/*

	@file mw_screensaver_osx.cpp

	$Id: mw_screensaver_osx.cpp,v 1.2 2005/04/19 14:13:06 james Exp $
          
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
 * $Log: mw_screensaver_osx.cpp,v $
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


#include "mw_screensaver_osx.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/*! \page mw_screensaver_osx MetaWrap - Utility - Screensaver - OSX
 *
 * \subsection mw_screensaver_osx Overview
 *
 */

/* As long as we are not building the unit tests for this file.*/
#ifndef __TEST__

/*! \defgroup mw_screensaver_osx MetaWrap - Utility - Screensaver - OSX
 *@{
 */

#define PIx2 6.28318530718f

// useful random functions

static float myRandf(float x)
{
    return(((double) rand() * x) / RAND_MAX);
}


// Find absolute value and truncate to 1.0

static float fabstrunc(float f)
{
	if(f >= 0.0f)
		return((f <= 1.0f) ? f : 1.0f);
	else
		return((f >= -1.0f) ? -f : 1.0f);
}


__private_extern__ void draw(mw_screensaver_osxSaverSettings * inSettings)
{
	int i, j;
	float rgb[3];
	float temp;
	float focus = ((float) inSettings->dFocus)/ 50.0f + 0.3f;
	float maxdiff = 0.004f * (float) (inSettings->dSpeed);
	static int index;
    float * c=inSettings->c;
    float *** position=inSettings->position;
    register float * tPosition;
    float ***plasma=inSettings->plasma;
    float *plasmamap=inSettings->plasmamap;
    int maxj;
    int plasmasize=inSettings->plasmasize;
    
	//Update constants
	for(i=0; i<NUMCONSTS; i++)
    {
		inSettings->ct[i] += inSettings->cv[i];
		if(inSettings->ct[i] > PIx2)
			inSettings->ct[i] -= PIx2;
		c[i] = ((float) sin((double)inSettings->ct[i])) * focus;
	}

	// Update colors
    
    maxj=(int)(((float)plasmasize) / inSettings->aspectRatio);
    
	for(i=0; i<plasmasize; i++)
    {
		for(j=0; j<maxj; j++)
        {
			// Calculate vertex colors
			rgb[0] = inSettings->plasma[i][j][0];
			rgb[1] = inSettings->plasma[i][j][1];
			rgb[2] = inSettings->plasma[i][j][2];
			
            tPosition=position[i][j];
            
            plasma[i][j][0] = 0.7f
							* (c[0] * tPosition[0] + c[1] * tPosition[1]
							+ c[2] * (tPosition[0] * tPosition[0] + 1.0f)
							+ c[3] * tPosition[0] * tPosition[1]
							+ c[4] * rgb[1] + c[5] * rgb[2]);
			plasma[i][j][1] = 0.7f
							* (c[6] * tPosition[0] + c[7] * tPosition[1]
							+ c[8] * tPosition[0] * tPosition[0]
							+ c[9] * (tPosition[1] * tPosition[1] - 1.0f)
							+ c[10] * rgb[0] + c[11] * rgb[2]);
			plasma[i][j][2] = 0.7f
							* (c[12] * tPosition[0] + c[13] * tPosition[1]
							+ c[14] * (1.0f - tPosition[0] * tPosition[1])
							+ c[15] * tPosition[1] * tPosition[1]
							+ c[16] * rgb[0] + c[17] * rgb[1]);

			// Don't let the colors change too much
            
			temp = plasma[i][j][0] - rgb[0];
            
			if(temp > maxdiff)
				plasma[i][j][0] = rgb[0] + maxdiff;
            else
            {
                if(temp < -maxdiff)
                    plasma[i][j][0] = rgb[0] - maxdiff;
			}
            
            temp = plasma[i][j][1] - rgb[1];
            
			if(temp > maxdiff)
				plasma[i][j][1] = rgb[1] + maxdiff;
			else
            {
                if(temp < -maxdiff)
            
                    plasma[i][j][1] = rgb[1] - maxdiff;
			}
            
            temp = plasma[i][j][2] - rgb[2];
            
			if(temp > maxdiff)
				plasma[i][j][2] = rgb[2] + maxdiff;
			else
            {
                if(temp < -maxdiff)
                    plasma[i][j][2] = rgb[2] - maxdiff;
            }
            
			// Put colors into texture
			
            index = (i * inSettings->texsize + j) * 3;
			plasmamap[index] = fabstrunc(plasma[i][j][0]);
			plasmamap[index+1] = fabstrunc(plasma[i][j][1]);
			plasmamap[index+2] = fabstrunc(plasma[i][j][2]);
		}
	}

	// Update texture
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (int)(((float)plasmasize) / inSettings->aspectRatio), plasmasize,
		GL_RGB, GL_FLOAT, plasmamap);

	// Draw it
	// The "- 1" cuts off right and top edges to get rid of blending to black
	
    float texright = (((float)plasmasize) - 1) / ((float)inSettings->texsize);
	float textop = texright / inSettings->aspectRatio;
	
    glBindTexture(GL_TEXTURE_2D, 1);
    
    glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, texright);
		glVertex2f(1.0f, 0.0f);
		glTexCoord2f(textop, 0.0f);
		glVertex2f(0.0f, 1.0f);
		glTexCoord2f(textop, texright);
		glVertex2f(1.0f, 1.0f);
	glEnd();
}


__private_extern__ void initSaver(int width,int height,mw_screensaver_osxSaverSettings * inSettings)
{
	int i, j;

	srand((unsigned)time(NULL));
	rand(); rand(); rand(); rand(); rand();
	rand(); rand(); rand(); rand(); rand();
	rand(); rand(); rand(); rand(); rand();
	rand(); rand(); rand(); rand(); rand();

	// Window initialization
	
	glViewport(0,0, width,height);
	inSettings->aspectRatio = ((float)width) / ((float)height);
	
    if(inSettings->aspectRatio >= 1.0f)
    {
		inSettings->wide = 30.0f / inSettings->dZoom;
		inSettings->high = inSettings->wide / inSettings->aspectRatio;
	}
	else
    {
		inSettings->high = 30.0f / inSettings->dZoom;
		inSettings->wide = inSettings->high * inSettings->aspectRatio;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0f, 1.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set resolution of plasma
    
	if(inSettings->aspectRatio >= 1.0f)
		inSettings->plasmasize = (inSettings->dResolution * MAXTEXSIZE) / 100;
	else
		inSettings->plasmasize = (int)(((float)inSettings->dResolution * MAXTEXSIZE) * inSettings->aspectRatio * 0.01f);
        
	// Set resolution of texture
    
	inSettings->texsize = 16;
	if(inSettings->aspectRatio >= 1.0f)
    {
		while(inSettings->plasmasize > inSettings->texsize)
			inSettings->texsize *= 2;
	}
    else
    {
		while(int(((float) inSettings->plasmasize) / inSettings->aspectRatio) > inSettings->texsize)
			inSettings->texsize *= 2;
    }
    
	// Initialize memory and positions
    
    inSettings->plasmamap = new float[inSettings->texsize*inSettings->texsize*3];
    
	for(i=0; i<inSettings->texsize*inSettings->texsize*3; i++)
    {
		inSettings->plasmamap[i] = 0.0f;
    }
    
	inSettings->plasma = new float**[inSettings->plasmasize];
	inSettings->position = new float**[inSettings->plasmasize];
    
	for(i=0; i<inSettings->plasmasize; i++)
    {
		int maxj=(int)(((float)inSettings->plasmasize) / inSettings->aspectRatio);
        
        inSettings->plasma[i] = new float*[maxj];
		inSettings->position[i] = new float*[maxj];
        
		for(j=0; j<maxj; j++)
        {
			inSettings->plasma[i][j] = new float[3];
			inSettings->position[i][j] = new float[2];
            
			inSettings->plasma[i][j][0] = 0.0f;
			inSettings->plasma[i][j][1] = 0.0f;
			inSettings->plasma[i][j][2] = 0.0f;
            
			inSettings->position[i][j][0] = (((float) i) * inSettings->wide) / (((float)inSettings->plasmasize) - 1) - (inSettings->wide * 0.5f);
			inSettings->position[i][j][1] = (((float) j) * inSettings->high) / ((((float)inSettings->plasmasize)) / inSettings->aspectRatio - 1.0f) - (inSettings->high * 0.5f);
		}
	}
	// Initialize constants
	for(i=0; i<NUMCONSTS; i++)
    {
		inSettings->ct[i] = myRandf(PIx2);
		inSettings->cv[i] = myRandf(0.005f * ((float) inSettings->dSpeed)) + 0.0001f;
	}
    
    // Make texture
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, inSettings->texsize);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, inSettings->texsize, inSettings->texsize, 0,GL_RGB, GL_FLOAT, inSettings->plasmamap);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	
}

__private_extern__ void cleanSettings(mw_screensaver_osxSaverSettings * inSettings)
{
    int i, j;
    GLuint texnum=1;
    
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1,&texnum);
    
    glDisable(GL_TEXTURE_2D);
    
    if (inSettings->plasmamap!=NULL)
    {
        delete[] inSettings->plasmamap;
        inSettings->plasmamap=NULL;
    }
    
    if (inSettings->plasma!=NULL)
    {
        for(i=0; i<inSettings->plasmasize; i++)
        {
			int maxj=(int) (inSettings->plasmasize / inSettings->aspectRatio);
            
            for(j=0; j<maxj; j++)
            {
                delete[] inSettings->plasma[i][j];
                delete[] inSettings->position[i][j];
            }
            
            delete[] inSettings->plasma[i];
            delete[] inSettings->position[i];
        }
        
        delete[] inSettings->plasma;
        delete[] inSettings->position;
        
        inSettings->plasma=NULL;
        inSettings->position=NULL;
    }
}

__private_extern__ void setDefaults(mw_screensaver_osxSaverSettings * inSettings)
{
	inSettings->dZoom = 10;
	inSettings->dFocus = 30;
	inSettings->dSpeed = 20;
	inSettings->dResolution = 25;
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



