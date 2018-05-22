/*

	@file mw_browser_extension_protocol_ie.cpp

	$Id: mw_screensaver_osx.h,v 1.2 2005/04/19 14:13:06 james Exp $
          
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
 * $Log: mw_screensaver_osx.h,v $
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


#ifndef __PLASMA__
#define __PLASMA__

#define NUMCONSTS 18
#define MAXTEXSIZE 1024

typedef struct mw_screensaver_osxSaverSettings
{
    float aspectRatio;
    float wide;
    float high;
    float c[NUMCONSTS];  // constant
    float ct[NUMCONSTS];  // temporary value of constant
    float cv[NUMCONSTS];  // velocity of constant
    float ***position;
    float ***plasma;
    int texsize;
    int plasmasize;
    float *plasmamap;
    // Parameters edited in the dialog box
    int dZoom;
    int dFocus;
    int dSpeed;
    int dResolution;
} mw_screensaver_osxSaverSettings;


__private_extern__ void draw(mw_screensaver_osxSaverSettings * inSettings);

__private_extern__ void cleanSettings(mw_screensaver_osxSaverSettings * inSettings);
__private_extern__ void initSaver(int width,int height,mw_screensaver_osxSaverSettings * inSettings);
__private_extern__ void setDefaults(mw_screensaver_osxSaverSettings * inSettings);

#endif
