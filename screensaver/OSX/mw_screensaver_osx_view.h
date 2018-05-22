/*

	@file mw_browser_extension_protocol_ie.cpp

	$Id: mw_screensaver_osx_view.h,v 1.2 2005/04/19 14:13:06 james Exp $
          
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
 * $Log: mw_screensaver_osx_view.h,v $
 * Revision 1.2  2005/04/19 14:13:06  james
 * OSX Screensaver
 *
 *
*/

/*	Make this 1 is you want to see LOG functions to behave  
    like they do in _DEBUG mode for this file.*/
#if 0 //_DEVELOPER
#ifndef _DEVELOPER
#	define _DEVELOPER
#endif //_DEVELOPER
#endif


#import <AppKit/AppKit.h>
#import <ScreenSaver/ScreenSaver.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#include "mw_screensaver_osx.h"

@interface mw_screensaver_osx_view : ScreenSaverView
{
	NSOpenGLView *_view;
    
    mw_screensaver_osxSaverSettings settings_;
    
    int mainScreenOnly_;
    
    BOOL isConfiguring_;
    BOOL preview_;
    BOOL mainScreen_;
    
    IBOutlet id IBfocusSlider_;
    IBOutlet id IBfocusText_;
    IBOutlet id IBmagnificationSlider_;
    IBOutlet id IBmagnificationText_;
    IBOutlet id IBmainScreen_;
    IBOutlet id IBresolutionSlider_;
    IBOutlet id IBresolutionText_;
    IBOutlet id IBspeedSlider_;
    IBOutlet id IBspeedText_;
    IBOutlet id IBconfigureSheet_;
    
    IBOutlet id IBversion_;
}

- (IBAction)closeSheet:(id)sender;
- (IBAction)setFocus:(id)sender;
- (IBAction)setMagnification:(id)sender;
- (IBAction)setResolution:(id)sender;
- (IBAction)setSpeed:(id)sender;

- (IBAction)reset:(id)sender;

- (void) setDialogValue;

- (void) readDefaults:(ScreenSaverDefaults *) inDefaults;
- (void) writeDefaults;

@end
