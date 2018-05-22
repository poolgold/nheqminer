/*

    @file mw_secreensaver_windows_buffer.h

    $Id: mw_screensaver_windows.cpp,v 1.7 2005/04/19 07:28:45 james Exp $
     
    @author     James Mc Parlane
     
    PROJECT:    MetaWrap Server (Amphibian)
     
    COMPONENT:  -
   
    @date       11 September 2001
     

    GENERAL INFO:

    Massive Technologies
    PO Box 567
    Darlinghurst 2010
    NSW, Australia
    email: james@massive.com.au
    tel: (+61-2) 9331 8699
    fax: (+61-2) 9331 8699
    mob: (+61) 407-909-186


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
 * $Log: mw_screensaver_windows.cpp,v $
 * Revision 1.7  2005/04/19 07:28:45  james
 * code formatting
 *
 *
 * Revision 1.3  2005/04/18 13:32:15  james
 * Fixed over zealous global rename glitch
 *
 * Revision 1.2  2005/04/18 13:30:58  james
 * Started re-formatting up the code.
 * The original code is good - but only its mother could love it.
 *
 * Revision 1.1  2005/04/18 13:20:10  james
 * First import of minimal but feature complete windows screen saver originally written 
 * "(c) 2003 Lucian Wischik Anyone can use this code anyhow they want, except they can't 
 * sell it or claim ownership" Anyone can use this code anyhow they want, except they 
 * can't sell it or claim ownership
 *
*/

/* Make this 1 is you want to see LOG functions to behave
    like they do in _DEBUG mode for this file.*/
#if 0 //_DEVELOPER
#ifndef _DEVELOPER
# define _DEVELOPER
#endif //_DEVELOPER
#endif

/*! \page mw_secreensaver_windows MetaWrap - Screensaver - Windows
 *
 * \subsection mw_secreensaver_windows Overview
 *
 * Three Dee (OpenGL) Saver
 *
 * This saver uses graphics-hardware acceleration (via OpenGL) to draw a
 * spinning cube. If you want to use the same technique in your own code,
 * (1) #include <gl/gl.h> and <gl/glu.h>. (2) Under the Project Settings,
 * Linker, Input, add opengl32.lib and glu32.lib. (3) In the part of
 * WinMain which does RegisterClass, opengl requires that the saver window
 * have the style CS_OWNDC. (4) Copy out the block of code in the middle,
 * with functions ChoosePixelFormatEx(), EnsureGL(), EnsureMode() &c.
 * 
 * On some systems (especially Win95/98) it can take a long time for 3d
 * graphics drivers to load. This might be unacceptable for the preview
 * in the Display Properties > Screensaver dialog. You might consider not
 * using any 3d graphics in the preview -- instead, maybe just a still image.
 *
 * There's very little chance of getting 3d acceleration working on multiple
 * g_monitors in a portable way. Therefore, when running in full-screen mode,
 * this saver does its 3d thing only on the primary monitor, and leaves the
 * rest blank. If there happened to be an error initializing the 3d graphics
 * stuff, then it leaves the error message bouncing around the screen.
 *
 * If our window moves, certain hardware graphics cards require to be informed.
 * But consider the case when we are preview window inside the Display Settings
 * dialog. When the user moves the dialog, we never get any notification. Our
 * solution is to GetWindowRect() on every timer tick, and reset the viewport
 * for the graphics card. This is in the function EnsureProj().
 *
 * We really want hardware acceleration. The normal API function ChoosePixelFormat()
 * isn't as single-minded about hardware acceleration as we'd like. Therefore,
 * we use our own ChoosePixelFormatEx() function, which is.
 *
 * This saver has an option to change screen resolution for its full-screen mode.
 * There are a couple of issues. We don't change screen mode immediately, but
 * instead wait until our window has drawn itself (so hiding the normal desktop).
 * This is so that, when we change mode, the user doesn't see the existing windows
 * changing. The place we do this is in 'OnTimer', in response to the first timer
 * tick. This is an example of 'lazy evaluation', where initialization is only
 * done on the first occasion when it's actually needed, not before.
 *
 * To change screen resolution, you call the function ChangeDisplaySettings().
 * But on some systems, if you merely specify a width/height/bpp but without
 * specifying a refresh frequency, then it gives you an ugly low frequency.
 * To avoid this problem, we enumerate all the display modes, and pick the one
 * that has the nicest-looking frequency. This is in the function EnsureMode().
 *
 * When a mode-change happens, some spurious mouse-move messages get sent by the
 * system. Normally a mouse-move would cause a saver to terminate. We use a sleazy
 * way to avoid this: we set the global flag g_is_dialog_active, which is normally
 * just used for the password-verify-dialog that appears on screen. Our saver-window
 * WndProc ignores mouse-move messages when it is set. I encourage everyone
 * who uses screen-mode-changes to test it with SCRDEBUG set to false, and with
 * mouse sensitivity set to high.
 *
 * On Win95/98/ME, the saver itself invokes the password-verify dialog. But if we
 * are rendering 3d graphics at the same time as the dialog is up, and we're calling
 * SwapBuffers(), then the password-very dialog will get ruined. To avoid this,
 * we suspend animation while a dialog is active.
 *
 */

/* As long as we are not building the unit tests for this file.*/
#ifndef __TEST__

#pragma warning( disable: 4127 4800 4702 )
#include <string>
#include <vector>
#include <math.h>
#include <windows.h>
#include <commctrl.h>
#include <regstr.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <stdlib.h>
#include <tchar.h>
typedef std::basic_string<TCHAR> tstring;
using namespace std;

/*! \defgroup mw_secreensaver_windows MetaWrap - Screensaver - Windows
 *@{
 */

/// If you want debugging output, set this to "OutputDebugString" or a filename. The second line controls the saver's "friendliness"
const tstring g_debug_file = _T( "OutputDebugString" );

const bool SCRDEBUG = ( g_debug_file != _T( "" ) );

/// These global variables are loaded at the start of WinMain
BOOL g_mute_sound;

/// In pixels
DWORD g_mouse_threshold;

/// In seconds. Doesn't apply to NT/XP/Win2k.
DWORD g_password_delay;

/// also these, which are used by the General properties dialog

/// 0=seconds, 1=minutes. Purely a visual thing.
DWORD g_password_delay_index;

/// 0=high, 1=normal, 2=low, 3=ignore. Purely visual
DWORD g_mouse_threshold_index;

/// "-YN-" or something similar
TCHAR g_corners[ 5 ];

/// whether they're present or not
BOOL g_hot_services;

/// and these are created when the dialog/saver starts
POINT g_init_cursor_pos;

/// in ms
DWORD g_init_time;

bool g_is_dialog_active;

/// for NT, so we know if a WM_CLOSE came from us or it.
bool g_really_close;

/// Some other minor global variables and prototypes
HINSTANCE g_hinstance = 0;

HICON g_hiconsm = 0, g_hiconbg = 0;

/// bitmap for the monitor class
HBITMAP g_hbmmonitor = 0;

/// this is retrieved at the start of WinMain from String Resource 1
tstring g_saver_name;

/// the rectangles of each monitor (smSaver) or of the preview window (smPreview)
vector<RECT> g_monitors;


struct saver_window_s;

/// gets the client rectangle. 0=all, 1=topleft, 2=topright, 3=botright, 4=botleft corner
const UINT SCRM_GETMONITORAREA = WM_APP;
inline void Debug( const tstring s );
tstring GetLastErrorString();
void SetDlgItemUrl( HWND hdlg, int id, const TCHAR *url );
void RegSave( const tstring name, int val );
void RegSave( const tstring name, bool val );
void RegSave( const tstring name, tstring val );
int RegLoad( const tstring name, int def );
bool RegLoad( const tstring name, bool def );
tstring RegLoad( const tstring name, tstring def );

//
// IMPORTANT GLOBAL VARIABLES:
enum TScrMode {smNone, smConfig, smPassword, smPreview, smSaver, smInstall, smUninstall} ScrMode;
vector<saver_window_s*> g_saver_window;   // the saver windows, one per monitor. In preview mode there's just one.

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------


// These are the saver's global settings

/// this is our setting that we're storing in the registry
bool g_change_screen_mode = false;

/// To change mode. It uses the following variables...
void EnsureMode( HWND hwnd );

/// well, did we try to change it?
bool g_changed_mode = false;

/// and did we succeed?
bool g_changed_ok = false;

// Exit the curent mode
void ExitMode();

/// To initialize GL (lazily). It uses the following variables...
void EnsureGL( HDC p_hdc );

/// whether GL has been initialized (in which case, don't retry)
bool g_inited_gl = false;

/// whether it succeeded
bool g_glok = false;

/// if it didn't succeed, this is why
tstring g_glerr;

void ExitGL();

/// To tell the graphics card which screen-rectangle to draw into...
void ProjGL( const RECT rc );

/// the most recent rectangle we told it
RECT g_projrc;

void CommonInit()
{
    g_change_screen_mode = RegLoad( _T( "g_change_screen_mode" ), false );
}



/*!
    @fn         int ChoosePixelFormatEx(HDC p_hdc,int *p_bpp,int *p_depth,int *p_dbl,int *p_acc)   
    @param      p_hdc
    @param      p_bpp
    @param      p_depth
    @param      p_dbl
    @param      p_acc
    @return     int
    @brief      Like the regular WIN32 ChoosePixelFormat but more aggressive about using hardware acceleration. 
    @author     James Mc Parlane
    @date       18 April 2005
 
    Call it like this:
    int bpp=-1, depth=16, dbl=0, acc=1;
    ChoosePixelFormatEx(p_hdc,&bpp,&depth,&dbl,&acc);
    initial values are -1=don't care, 16=want this, 1=turn-on, 0=turn-off and it
    chooses the best pixel format, and updates the parameters with values chosen.
 
*/
int ChoosePixelFormatEx( HDC p_hdc, int *p_bpp, int *p_depth, int *p_dbl, int *p_acc )
{
    int wbpp;
    if ( p_bpp == NULL )
    {
        wbpp = -1;
    }
    else
    {
        wbpp = *p_bpp;
    }

    int wdepth;
    if ( p_depth == NULL )
    {
        wdepth = 16;
    }
    else
    {
        wdepth = *p_depth;
    }

    int wdbl;
    if ( p_dbl == NULL )
    {
        wdbl = -1;
    }
    else
    {
        wdbl = *p_dbl;
    }

    int wacc;
    if ( p_acc == NULL )
    {
        wacc = 1;
    }
    else
    {
        wacc = *p_acc;
    }


    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;

    int num = DescribePixelFormat( p_hdc, 1, sizeof( pfd ), &pfd );

    if ( num == 0 )
    {
        return 0;
    }

    unsigned int maxqual = 0;
    int maxindex = 0;

    int max_bpp = 0, max_depth = 0, max_dbl = 0, max_acc = 0;

    for ( int i = 1; i <= num; i++ )
    {
        ZeroMemory( &pfd, sizeof( pfd ) );
        pfd.nSize = sizeof( pfd );
        pfd.nVersion = 1;
        DescribePixelFormat( p_hdc, i, sizeof( pfd ), &pfd );
        int bpp = pfd.cColorBits;
        int depth = pfd.cDepthBits;

        // Pallet driven?
        bool l_pal = ( pfd.iPixelType == PFD_TYPE_COLORINDEX );

        //
        bool mcd = ( ( pfd.dwFlags & PFD_GENERIC_FORMAT ) && ( pfd.dwFlags & PFD_GENERIC_ACCELERATED ) );

        //
        bool soft = ( ( pfd.dwFlags & PFD_GENERIC_FORMAT ) && !( pfd.dwFlags & PFD_GENERIC_ACCELERATED ) );

        //
        bool icd = ( !( pfd.dwFlags & PFD_GENERIC_FORMAT ) && !( pfd.dwFlags & PFD_GENERIC_ACCELERATED ) );

        // OpenGL Supported?
        bool opengl = ( pfd.dwFlags & PFD_SUPPORT_OPENGL );

        // Able to draw to window?
        bool window = ( pfd.dwFlags & PFD_DRAW_TO_WINDOW );

        // Able to draw to bitmap?
        bool bitmap = ( pfd.dwFlags & PFD_DRAW_TO_BITMAP );

        // Can double buffer?
        bool dbuff = ( pfd.dwFlags & PFD_DOUBLEBUFFER );

        // Quality flags
        unsigned int q = 0;
        if ( opengl && window )
        {
            q = q + 0x8000;
        }

        if ( wdepth == -1 || ( wdepth > 0 && depth > 0 ) )
        {
            q = q + 0x4000;
        }

        if ( wdbl == -1 || ( wdbl == 0 && !dbuff ) || ( wdbl == 1 && dbuff ) )
        {
            q = q + 0x2000;
        }

        if ( wacc == -1 || ( wacc == 0 && soft ) || ( wacc == 1 && ( mcd || icd ) ) )
        {
            q = q + 0x1000;
        }

        if ( mcd || icd )
        {
            q = q + 0x0040;
        }

        if ( icd )
        {
            q = q + 0x0002;
        }

        if ( wbpp == -1 || ( wbpp == bpp ) )
        {
            q = q + 0x0800;
        }

        if ( bpp >= 16 )
        {
            q = q + 0x0020;
        }

        if ( bpp == 16 )
        {
            q = q + 0x0008;
        }

        if ( wdepth == -1 || ( wdepth == depth ) )
        {
            q = q + 0x0400;
        }

        if ( depth >= 16 )
        {
            q = q + 0x0010;
        }
        
        if ( depth == 16 )
        {
            q = q + 0x0004;
        }

        if ( !l_pal )
        {
            q = q + 0x0080;
        }

        if ( bitmap )
        {
            q = q + 0x0001;
        }

        if ( q > maxqual )
        {
            maxqual = q;
            maxindex = i;
            max_bpp = bpp;
            max_depth = depth;
            max_dbl = dbuff ? 1 : 0;
            max_acc = soft ? 0 : 1;
        }
    }

    if ( maxindex == 0 )
    {
        return maxindex;
    }

    if ( p_bpp != NULL )
    {
        *p_bpp = max_bpp;
    }

    if ( p_depth != NULL )
    {
        *p_depth = max_depth;
    }

    if ( p_dbl != NULL )
    {
        *p_dbl = max_dbl;
    }

    if ( p_acc != NULL )
    {
        *p_acc = max_acc;
    }

    return maxindex;
}



/*!
    @fn         void EnsureGL(HDC p_hdc)   
    @param      p_hdc
    @return     void
    @brief      Ensure that GL is ready and willing
    @author     James Mc Parlane
    @date       18 April 2005

    Question: When is it a safe time to create our GL context?
    Answer: OpenGL is very picky about this. If we try to create the context
    too early, it won't like it. This here InitGL function gets called
    the first time the Timer fires. This is an okay time to initialize.
*/
void EnsureGL( HDC p_hdc )
{
    // Already initiated? Leave then - no need to do it again - in fact its downright fatal.
    if ( g_inited_gl )
    {
        return;
    }

    // Ok - we are going to do this - or die..
    g_inited_gl = true;

    // Start not knowing the health of GL
    g_glok = false;


    // Our desired format
    int bpp = 16, depth = 16, dbl = 1, acc = 1;

    // This will contain our format descriptor
    PIXELFORMATDESCRIPTOR pfd;

    // clear it
    ZeroMemory( &pfd, sizeof( pfd ) );

    // Calculate it
    int pfi = ChoosePixelFormatEx( p_hdc, &bpp, &depth, &dbl, &acc );

    // Describe it
    DescribePixelFormat( p_hdc, pfi, sizeof( pfd ), &pfd );

    // Set it - *foom*
    bool res = SetPixelFormat( p_hdc, pfi, &pfd );
    if ( !res )
    {
        g_glerr = _T( "Graphics card doesn't support these display settings" );
        return ;
    }

    // Create GL HDC
    HGLRC hglrc = wglCreateContext( p_hdc );
    if ( hglrc == NULL )
    {
        g_glerr = _T( "Failed to start OpenGL" );
        return ;
    }

    // Make it the current GL context
    res = wglMakeCurrent( p_hdc, hglrc );
    if ( !res )
    {
        wglDeleteContext( hglrc );
        g_glerr = _T( "Failed to activate OpenGL" );
        return ;
    }


    // And away we go..
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    
    glClearDepth( 1.0 );
    
    // Superimpose smaller Z values over larger ones
    glDepthFunc( GL_LEQUAL );  
    
    // Smooth shading
    glShadeModel( GL_SMOOTH ); 
    glFrontFace( GL_CCW );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glAlphaFunc( GL_GEQUAL, 0.07f );
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_BLEND );
    glDisable( GL_CULL_FACE );

    // Z buffering is go...
    glEnable( GL_DEPTH_TEST ); 
    
    g_projrc.left = 0;
    g_projrc.top = 0;
    g_projrc.top = 0;
    g_projrc.bottom = 0;

    // We made it
    g_glok = true;
}


/*!
    @fn         void ProjGL( const RECT rc )
    @param      rc
    @param
    @param
    @return   
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005

    'prepare the projection' by telling the OpenGL system our
    window's size and screen position and camera parameters. Some hardware-
    accelerated drivers require that we tell them glViewport every time the
    window moves on the screen. But because we might be a child window inside
    DisplayProperties, we don't necessarily get WM_MOVE messages! So, how to
    tell when we've moved? Answer: every timer event, we poll the current
    window position on the screen. If it has changed, then we ProjGL again.

*/ 
void ProjGL( const RECT rc )
{
    if ( !g_inited_gl || !g_glok )
        return ;
    if ( rc.left == g_projrc.left && rc.top == g_projrc.top && rc.right == g_projrc.right && rc.bottom == g_projrc.bottom )
        return ;
    g_projrc = rc;
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    //
    double znear = 1;     // Put the screen 1 unit in front of the "eye"
    double zfar = 50.0;  // It's good to have a small far/near ratio
    double aspect = ( ( double ) w ) / ( ( double ) h );
    double fovX = 25.0;
    glViewport( 0, 0, w, h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( fovX / aspect, aspect, znear, zfar );
}


/*!
    @fn         void ExitGL()
    @return     void
    @brief      Exit GL
    @author     James Mc Parlane
    @date       18 April 2005

*/
void ExitGL()
{
    if ( !g_inited_gl || !g_glok )
    {
        return ;
    }

    HGLRC hglrc = wglGetCurrentContext();

    if ( hglrc != NULL )
    {
        wglMakeCurrent( 0, 0 );
        wglDeleteContext( hglrc );
    }

    g_inited_gl = false;
}


/*!
    @fn         void EnsureMode( HWND hwnd )
    @param      hwnd
    @return     void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void EnsureMode( HWND hwnd )
{
    if ( g_changed_mode )
        return ;
    g_changed_mode = true;
    g_changed_ok = false;
    // We want to change to 640x480. But if we merely change, without specifying
    // a refresh rate, then some systems give us a crummy one. So, we first
    // enum the available modes and pick the best one -- the least one above 70Hz,
    // or the largest if none are above 70. Some systems return only "0" and "1"
    // for refresh rates (i.e. referring to monitor-specific presets, rather than
    // frequencies in hertz). This possibility is already covered by our heuristic.
    int leastabove70 = -1, leastabove70i = -1, greatest = -1, greatesti = -1;
    for ( int i = 0; ; i++ )
    {
        DEVMODE dm;
        ZeroMemory( &dm, sizeof( dm ) );
        dm.dmSize = sizeof( dm );
        dm.dmDriverExtra = 0;
        BOOL res = EnumDisplaySettings( NULL, i, &dm );
        if ( !res )
            break;
        if ( dm.dmPelsWidth != 640 || dm.dmPelsHeight != 480 || dm.dmBitsPerPel != 16 )
            continue;
        if ( dm.dmDisplayFrequency > 70 && ( leastabove70 == -1 || ( int ) dm.dmDisplayFrequency < leastabove70 ) )
        {
            leastabove70 = dm.dmDisplayFrequency;
            leastabove70i = i;
        }
        if ( ( int ) dm.dmDisplayFrequency > greatest )
        {
            greatest = dm.dmDisplayFrequency;
            greatesti = i;
        }
    }
    int index = leastabove70i;
    if ( index == -1 )
        index = greatesti;
    if ( index == -1 )
        return ;
    DEVMODE dm;
    ZeroMemory( &dm, sizeof( dm ) );
    dm.dmSize = sizeof( dm );
    dm.dmDriverExtra = 0;
    EnumDisplaySettings( NULL, index, &dm );
    LONG res = ChangeDisplaySettings( &dm, CDS_TEST );
    if ( res != DISP_CHANGE_SUCCESSFUL )
        return ;
    // Changing the screen-mode causes some mouse-move messages to be sent.
    // In order that the saver doesn't respond by quitting, we lie to it:
    g_is_dialog_active = true;
    ChangeDisplaySettings( &dm, CDS_FULLSCREEN );
    MoveWindow( hwnd, 0, 0, 640, 480, FALSE );
    g_is_dialog_active = false;
    GetCursorPos( &g_init_cursor_pos );
    //
    g_changed_ok = true;
}


/*! 
    @fn         void ExitMode()
    @return     void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void ExitMode()
{
    if ( !g_changed_mode || !g_changed_ok )
    {
        return;
    }

    // this magically resizes our window
    ChangeDisplaySettings( 0, 0 ); 

    g_changed_mode = false;
}


/*!
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005

    saver_window_s: one is created for each saver window (be it preview, or the
    preview in the config dialog, or one for each monitor when running full-screen)
*/ 
struct saver_window_s
{
    HWND hwnd;
    int id;    // id=-1 for a preview, or 0..n for full-screen on the specified monitor
    HDC p_hdc;              // OpenGL requires the CS_OWNDC style. This is our own p_hdc
    float RotX, RotY, RotZ; // the current state of our animation
    bool timer;           // have we allocated a timer?
    
    //
    saver_window_s( HWND _hwnd, int _id ) : hwnd( _hwnd ), id( _id ), p_hdc( 0 )
    {
        CommonInit();
        RotX = 0;
        RotY = 0;
        RotZ = 0;
        SetTimer( hwnd, 1, 50, NULL );
        timer = true;
    }

    ~saver_window_s()
    {
        if ( timer )
            KillTimer( hwnd, 1 );
        timer = false;
        if ( p_hdc != 0 )
            ReleaseDC( hwnd, p_hdc );
        p_hdc = 0;
        if ( id == 0 )
        {
            ExitGL();
            ExitMode();
        }
    }

    void OtherWndProc( UINT, WPARAM, LPARAM )
    {
    }


    void OnPaint( HDC p_hdc, const RECT &rc )
    { // For a working opengl window, OnTimer will do a complete redraw.
        // Otherwise we have to do it ourselves.
        if ( id == 0 && g_glok && !g_is_dialog_active )
        {
            OnTimer();
            return ;
        }
        FillRect( p_hdc, &rc, ( HBRUSH ) GetStockObject( BLACK_BRUSH ) );
        if ( id == 0 && !g_is_dialog_active )
        {
            int y = ( GetTickCount() / 200 ) % rc.bottom, x = y;
            RECT trc;
            trc.left = x - 400;
            trc.top = y - 20;
            trc.right = x + 400;
            trc.bottom = y + 20;
            SelectObject( p_hdc, GetStockObject( DEFAULT_GUI_FONT ) );
            SetBkColor( p_hdc, RGB( 0, 0, 0 ) );
            SetTextColor( p_hdc, RGB( 255, 128, 128 ) );			
			if (g_glerr.length() > 0)
			{
				DrawText(p_hdc, g_glerr.c_str(), -1, &trc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}

			DrawText(p_hdc, "128 Sol/s", -1, &trc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            
        }
    }

    void OnTimer()
    { 
        // On secondary g_monitors, just do normal GDI painting:
        if ( id > 0 )
        {
            InvalidateRect( hwnd, NULL, FALSE );
            return ;
        }
        // If we're the primary monitor, change screen. We do this screen-change
        // lazily, here instead of in the constructor, so that our window
        // has had a change to paint itself (so hiding the desktop) before
        // we do the mode-change.
        if ( id == 0 && g_change_screen_mode )
            EnsureMode( hwnd );
        // Our window has CS_OWNDC. 'p_hdc' is our local copy of it.
        if ( p_hdc == 0 )
            p_hdc = GetDC( hwnd );
        // EnsureGL can be called multiple times:
        //EnsureGL( p_hdc );
        // We have to reproject ourselves every time our window moves (eg. in
        // the display properties dialog). This can't be done through a WM_MOVE
        // message since, as a child in the display properties, we won't even
        // get a WM_MOVE when the dialog is dragged around the screen.
        RECT rc;
        GetWindowRect( hwnd, &rc );
        ProjGL( rc );
        // If opengl didn't work, we resort to normal GDI painting
        if ( !g_glok )
        {
            InvalidateRect( hwnd, NULL, FALSE );
            return ;
        }
        // If the password-verify dialog is up (Win95/98/ME only) then we
        // shouldn't be calling SwapBuffers on top of it:
        if ( g_is_dialog_active )
            return ;
        //
        RotX += 5;
        RotY += 5;
        RotZ += 5;
        //
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // Wipe the color and Depth Buffers
        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glLoadIdentity();
        gluLookAt( 0, 0, -10, 0, 0, 0, 0, 1, 0 );  // put eye at (0,0,-10) looking at (0,0,0)
        glRotatef( RotX, 1.0f, 0.0f, 0.0f ); // rotate about the x unit vector
        glRotatef( RotY, 0.0f, 1.0f, 0.0f ); // rotate about the y unit vector
        glRotatef( RotZ, 0.0f, 0.0f, 1.0f ); // rotate about the z unit vector
        glBegin( GL_QUADS );
        // Six faces of a cube to draw
        glColor3f( 0.0f, 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, -1.0f );
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( -1.0f, 1.0f, -1.0f );
        glColor3f( 1.0f, 1.0f, 0.0f );
        glVertex3f( 1.0f, 1.0f, -1.0f );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex3f( 1.0f, -1.0f, -1.0f );
        //
        glColor3f( 1.0f, 0.0f, 1.0f );
        glVertex3f( 1.0f, -1.0f, 1.0f );
        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 1.0f, 1.0f, 1.0f );
        glColor3f( 0.0f, 1.0f, 1.0f );
        glVertex3f( -1.0f, 1.0f, 1.0f );
        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( -1.0f, -1.0f, 1.0f );
        //
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( -1.0f, 1.0f, -1.0f );
        glColor3f( 0.0f, 1.0f, 1.0f );
        glVertex3f( -1.0f, 1.0f, 1.0f );
        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 1.0f, 1.0f, 1.0f );
        glColor3f( 1.0f, 1.0f, 0.0f );
        glVertex3f( 1.0f, 1.0f, -1.0f );
        //
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex3f( 1.0f, -1.0f, -1.0f );
        glColor3f( 1.0f, 0.0f, 1.0f );
        glVertex3f( 1.0f, -1.0f, 1.0f );
        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( -1.0f, -1.0f, 1.0f );
        glColor3f( 0.0f, 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, -1.0f );
        //
        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( -1.0f, -1.0f, 1.0f );
        glColor3f( 0.0f, 1.0f, 1.0f );
        glVertex3f( -1.0f, 1.0f, 1.0f );
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( -1.0f, 1.0f, -1.0f );
        glColor3f( 0.0f, 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, -1.0f );
        //
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex3f( 1.0f, -1.0f, -1.0f );
        glColor3f( 1.0f, 1.0f, 0.0f );
        glVertex3f( 1.0f, 1.0f, -1.0f );
        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 1.0f, 1.0f, 1.0f );
        glColor3f( 1.0f, 0.0f, 1.0f );
        glVertex3f( 1.0f, -1.0f, 1.0f );
        glEnd();
        glPopMatrix();
        SwapBuffers( p_hdc );
    }
};




/*!
    @fn         BOOL CALLBACK OptionsDlgProc( HWND hwnd, UINT msg, WPARAM, LPARAM lParam )
    @param      hwnd
    @param      msg
    @param      wParam
    @param      lParam
    @return   
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
BOOL CALLBACK OptionsDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch ( msg )
    {
        case WM_INITDIALOG:
            {
                CommonInit();
                CheckDlgButton( hwnd, 102, g_change_screen_mode ? BST_CHECKED : BST_UNCHECKED );
            }
            return TRUE;
        case WM_NOTIFY:
            {
                LPNMHDR nmh = ( LPNMHDR ) lParam;
                UINT code = nmh->code;
                switch ( code )
                {
                    case ( PSN_APPLY ) :
                        {
                            g_change_screen_mode = ( IsDlgButtonChecked( hwnd, 102 ) == BST_CHECKED );
                            RegSave( _T( "g_change_screen_mode" ), g_change_screen_mode );
                            SetWindowLong( hwnd, DWLP_MSGRESULT, PSNRET_NOERROR );
                        }
                        return TRUE;
                }
            }
            return FALSE;
    }
    return FALSE;
}


/*!
    @fn         BOOL VerifyPassword( HWND hwnd )
    @param      hwnd
    @return     BOOL
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
BOOL VerifyPassword( HWND hwnd )
{ 
    // Under NT, we return TRUE immediately. This lets the saver quit, and the system manages passwords.
    // Under '95, we call VerifyScreenSavePwd. This checks the appropriate registry key and, if necessary, pops up a verify dialog
    OSVERSIONINFO osv;
    osv.dwOSVersionInfoSize = sizeof( osv );
    GetVersionEx( &osv );
    if ( osv.dwPlatformId == VER_PLATFORM_WIN32_NT )
        return TRUE;
    HINSTANCE hpwdcpl = ::LoadLibrary( _T( "PASSWORD.CPL" ) );
    if ( hpwdcpl == NULL )
    {
        Debug( _T( "Unable to load PASSWORD.CPL. Aborting" ) );
        return TRUE;
    }
    typedef BOOL ( WINAPI * VERIFYSCREENSAVEPWD ) ( HWND hwnd );
    VERIFYSCREENSAVEPWD VerifyScreenSavePwd;
    VerifyScreenSavePwd = ( VERIFYSCREENSAVEPWD ) GetProcAddress( hpwdcpl, "VerifyScreenSavePwd" );
    if ( VerifyScreenSavePwd == NULL )
    {
        Debug( _T( "Unable to get VerifyPwProc address. Aborting" ) );
        FreeLibrary( hpwdcpl );
        return TRUE;
    }
    Debug( _T( "About to call VerifyPwProc" ) );
    BOOL bres = VerifyScreenSavePwd( hwnd );
    FreeLibrary( hpwdcpl );
    return bres;
}

/*!
    @fn         void ChangePassword( HWND hwnd ) 
    @param      hwnd
    @return     void
    @brief      
    @author     James Mc Parlane
    @date       18 April 2005

    This only ever gets called under '95, when started with the /a option.
*/
void ChangePassword( HWND hwnd )
{ 
    HINSTANCE hmpr = ::LoadLibrary( _T( "MPR.DLL" ) );

    if ( hmpr == NULL )
    {
        Debug( _T( "MPR.DLL not found: cannot change password." ) );
        return ;
    }

    typedef VOID ( WINAPI * PWDCHANGEPASSWORD ) ( LPCSTR lpcRegkeyname, HWND hwnd, UINT uiReserved1, UINT uiReserved2 );

    PWDCHANGEPASSWORD PwdChangePassword = ( PWDCHANGEPASSWORD ) ::GetProcAddress( hmpr, "PwdChangePasswordA" );

    if ( PwdChangePassword == NULL )
    {
        FreeLibrary( hmpr );
        Debug( _T( "PwdChangeProc not found: cannot change password" ) );
        return ;
    }

    PwdChangePassword( "SCRSAVE", hwnd, 0, 0 );

    FreeLibrary( hmpr );
}


/*!
    @fn         void ReadGeneralRegistry()
    @return     void
    @brief      Read settings from registry
    @author     James Mc Parlane
    @date       18 April 2005
*/
void ReadGeneralRegistry()
{
    g_password_delay = 15;
    g_password_delay_index = 0;
    g_mouse_threshold = 50;
    g_is_dialog_active = false; // default values in case they're not in registry

    LONG res;

    HKEY skey;

    DWORD valtype, valsize, val;

    res = RegOpenKeyEx( HKEY_CURRENT_USER, REGSTR_PATH_SETUP _T( "\\Screen Savers" ), 0, KEY_READ, &skey );
    if ( res != ERROR_SUCCESS )
    {
        return ;
    }

    valsize = sizeof( val );

    res = RegQueryValueEx( skey, _T( "Password Delay" ), 0, &valtype, ( LPBYTE ) & val, &valsize );

    if ( res == ERROR_SUCCESS )
    {
        g_password_delay = val;
    }

    valsize = sizeof( val );

    res = RegQueryValueEx( skey, _T( "Password Delay Index" ), 0, &valtype, ( LPBYTE ) & val, &valsize );

    if ( res == ERROR_SUCCESS )
    {
        g_password_delay_index = val;
    }

    valsize = sizeof( val );

    res = RegQueryValueEx( skey, _T( "Mouse Threshold" ), 0, &valtype, ( LPBYTE ) & val, &valsize );

    if ( res == ERROR_SUCCESS )
    {
        g_mouse_threshold = val;
    }

    valsize = sizeof( val );

    res = RegQueryValueEx( skey, _T( "Mouse Threshold Index" ), 0, &valtype, ( LPBYTE ) & val, &valsize );

    if ( res == ERROR_SUCCESS )
    {
        g_mouse_threshold_index = val;
    }

    valsize = sizeof( val );

    res = RegQueryValueEx( skey, _T( "Mute Sound" ), 0, &valtype, ( LPBYTE ) & val, &valsize );

    if ( res == ERROR_SUCCESS )
    {
        g_mute_sound = val;
    }

    valsize = 5 * sizeof( TCHAR );

    res = RegQueryValueEx( skey, _T( "Mouse g_corners" ), 0, &valtype, ( LPBYTE ) g_corners, &valsize );

    for ( int i = 0; i < 4; i++ )
    {
        if ( g_corners[ i ] != 'Y' && g_corners[ i ] != 'N' )
        {
            g_corners[ i ] = '-';
        }
    }
    g_corners[ 4 ] = 0;
    RegCloseKey( skey );
    //
    g_hot_services = FALSE;
    HINSTANCE sagedll = LoadLibrary( _T( "sage.dll" ) );
    if ( sagedll == 0 )
        sagedll = LoadLibrary( _T( "scrhots.dll" ) );
    if ( sagedll != 0 )
    {
        typedef BOOL ( WINAPI * SYSTEMAGENTDETECT ) ();
        SYSTEMAGENTDETECT detectproc = ( SYSTEMAGENTDETECT ) GetProcAddress( sagedll, "System_Agent_Detect" );
        if ( detectproc != NULL )
            g_hot_services = detectproc();
        FreeLibrary( sagedll );
    }
}

/*!
    @fn         void WriteGeneralRegistry()
    @return     void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void WriteGeneralRegistry()
{
    LONG res;
    HKEY skey;
    DWORD val;
    res = RegCreateKeyEx( HKEY_CURRENT_USER, REGSTR_PATH_SETUP _T( "\\Screen Savers" ), 0, 0, 0, KEY_ALL_ACCESS, 0, &skey, 0 );
    if ( res != ERROR_SUCCESS )
        return ;
    val = g_password_delay;
    RegSetValueEx( skey, _T( "Password Delay" ), 0, REG_DWORD, ( const BYTE* ) & val, sizeof( val ) );
    val = g_password_delay_index;
    RegSetValueEx( skey, _T( "Password Delay Index" ), 0, REG_DWORD, ( const BYTE* ) & val, sizeof( val ) );
    val = g_mouse_threshold;
    RegSetValueEx( skey, _T( "Mouse Threshold" ), 0, REG_DWORD, ( const BYTE* ) & val, sizeof( val ) );
    val = g_mouse_threshold_index;
    RegSetValueEx( skey, _T( "Mouse Threshold Index" ), 0, REG_DWORD, ( const BYTE* ) & val, sizeof( val ) );
    val = g_mute_sound ? 1 : 0;
    RegSetValueEx( skey, _T( "Mute Sound" ), 0, REG_DWORD, ( const BYTE* ) & val, sizeof( val ) );
    RegSetValueEx( skey, _T( "Mouse g_corners" ), 0, REG_SZ, ( const BYTE* ) g_corners, 5 * sizeof( TCHAR ) );
    RegCloseKey( skey );
    //
    HINSTANCE sagedll = LoadLibrary( _T( "sage.dll" ) );
    if ( sagedll == 0 )
        sagedll = LoadLibrary( _T( "scrhots.dll" ) );
    if ( sagedll != 0 )
    {
        typedef VOID ( WINAPI * SCREENSAVERCHANGED ) ();
        SCREENSAVERCHANGED changedproc = ( SCREENSAVERCHANGED ) GetProcAddress( sagedll, "Screen_Saver_Changed" );
        if ( changedproc != NULL )
            changedproc();
        FreeLibrary( sagedll );
    }
}


/*!
    @fn         LRESULT CALLBACK SaverWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
    @param      hwnd
    @param      msg 
    @param      wParam
    @param      lParam
    @return     LRESULT
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
LRESULT CALLBACK SaverWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    saver_window_s * sav;
    int id;
    HWND hmain;
#pragma warning( push )
#pragma warning( disable: 4244 4312 )

    if ( msg == WM_CREATE )
    {
        CREATESTRUCT * cs = ( CREATESTRUCT* ) lParam;
        id = *( int* ) cs->lpCreateParams;
        SetWindowLong( hwnd, 0, id );
        Debug( _T( "WM_CREATE" ) );
        sav = new saver_window_s( hwnd, id );
        SetWindowLongPtr( hwnd, GWLP_USERDATA, ( LONG_PTR ) sav );
        g_saver_window.push_back( sav );
    }
    else
    {
        sav = ( saver_window_s* ) GetWindowLongPtr( hwnd, GWLP_USERDATA );
        id = GetWindowLong( hwnd, 0 );
    }
#pragma warning( pop )
    if ( id <= 0 )
        hmain = hwnd;
    else
        hmain = g_saver_window[ 0 ] ->hwnd;
    //
    if ( msg == WM_TIMER )
        sav->OnTimer();
    else
        if ( msg == WM_PAINT )
        {
            PAINTSTRUCT ps;
            BeginPaint( hwnd, &ps );
            RECT rc;
            GetClientRect( hwnd, &rc );
            if ( sav != 0 )
                sav->OnPaint( ps.hdc, rc );
            EndPaint( hwnd, &ps );
        }
        else
            if ( sav != 0 )
                sav->OtherWndProc( msg, wParam, lParam );
    //
    switch ( msg )
    {
        case WM_ACTIVATE:
        case WM_ACTIVATEAPP:
        case WM_NCACTIVATE:
            {
                TCHAR pn[ 100 ];
                GetClassName( ( HWND ) lParam, pn, 100 );
                bool ispeer = ( _tcsncmp( pn, _T( "ScrClass" ), 8 ) == 0 );
                if ( ScrMode == smSaver && !g_is_dialog_active && LOWORD( wParam ) == WA_INACTIVE && !ispeer && !SCRDEBUG )
                {
                    Debug( _T( "WM_ACTIVATE: about to inactive window, so sending close" ) );
                    g_really_close = true;
                    PostMessage( hmain, WM_CLOSE, 0, 0 );
                    return 0;
                }
            }
            break;
        case WM_SETCURSOR:
            {
                if ( ScrMode == smSaver && !g_is_dialog_active && !SCRDEBUG )
                    SetCursor( NULL );
                else
                    SetCursor( LoadCursor( NULL, IDC_ARROW ) );
            }
            return 0;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_KEYDOWN:
            {
                if ( ScrMode == smSaver && !g_is_dialog_active )
                {
                    Debug( _T( "WM_BUTTONDOWN: sending close" ) );
                    g_really_close = true;
                    PostMessage( hmain, WM_CLOSE, 0, 0 );
                    return 0;
                }
            }
            break;
        case WM_MOUSEMOVE:
            {
                if ( ScrMode == smSaver && !g_is_dialog_active && !SCRDEBUG )
                {
                    POINT pt;
                    GetCursorPos( &pt );
                    int dx = pt.x - g_init_cursor_pos.x;
                    if ( dx < 0 )
                        dx = -dx;
                    int dy = pt.y - g_init_cursor_pos.y;
                    if ( dy < 0 )
                        dy = -dy;
                    if ( dx > ( int ) g_mouse_threshold || dy > ( int ) g_mouse_threshold )
                    {
                        Debug( _T( "WM_MOUSEMOVE: gone beyond threshold, sending close" ) );
                        g_really_close = true;
                        PostMessage( hmain, WM_CLOSE, 0, 0 );
                    }
                }
            }
            return 0;
        case ( WM_SYSCOMMAND ) :
            {
                if ( ScrMode == smSaver )
                {
                    if ( wParam == SC_SCREENSAVE )
                    {
                        Debug( _T( "WM_SYSCOMMAND: gobbling up a SC_SCREENSAVE to stop a new saver from running." ) );
                        return 0;
                    }
                    if ( wParam == SC_CLOSE && !SCRDEBUG )
                    {
                        Debug( _T( "WM_SYSCOMMAND: gobbling up a SC_CLOSE" ) );
                        return 0;
                    }
                }
            }
            break;
        case ( WM_CLOSE ) :
            {
                if ( id > 0 )
                    return 0; // secondary windows ignore this message
                if ( id == -1 )
                {
                    DestroyWindow( hwnd );
                    return 0;
                } // preview windows close obediently
                if ( g_really_close && !g_is_dialog_active )
                {
                    Debug( _T( "WM_CLOSE: maybe we need a password" ) );
                    BOOL CanClose = TRUE;
                    if ( GetTickCount() - g_init_time > 1000 * g_password_delay )
                    {
                        g_is_dialog_active = true;
                        SendMessage( hwnd, WM_SETCURSOR, 0, 0 );
                        CanClose = VerifyPassword( hwnd );
                        g_is_dialog_active = false;
                        SendMessage( hwnd, WM_SETCURSOR, 0, 0 );
                        GetCursorPos( &g_init_cursor_pos );
                    }
                    // note: all secondary g_monitors are owned by the primary. And we're the primary (id==0)
                    // therefore, when we destroy ourself, windows will destroy the others first
                    if ( CanClose )
                    {
                        Debug( _T( "WM_CLOSE: doing a DestroyWindow" ) );
                        DestroyWindow( hwnd );
                    }
                    else
                    {
                        Debug( _T( "WM_CLOSE: but failed password, so doing nothing" ) );
                    }
                }
            }
            return 0;
        case ( WM_DESTROY ) :
            {
                Debug( _T( "WM_DESTROY." ) );
                SetWindowLong( hwnd, 0, 0 );
                SetWindowLong( hwnd, GWLP_USERDATA, 0 );
                for ( vector<saver_window_s*>::iterator i = g_saver_window.begin(); i != g_saver_window.end(); i++ )
                {
                    if ( sav == *i )
                        * i = 0;
                }
                delete sav;
                if ( ( id == 0 && ScrMode == smSaver ) || ScrMode == smPreview )
                    PostQuitMessage( 0 );
            }
            break;
    }
    return DefWindowProc( hwnd, msg, wParam, lParam );
}




#ifndef SM_CMONITORS
DECLARE_HANDLE( HMONITOR );
#endif 

/*!
    @fn         BOOL CALLBACK EnumMonitorCallback( HMONITOR, HDC, LPRECT rc, LPARAM )   
    @param      p_hmonitor
    @param      p_hdc
    @param      rc
    @param      p_lparam
    @return     BOOL   
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005

    ENUM-MONITOR-CALLBACK is part of DoSaver. Its stuff is in windef.h but
    requires WINVER>=0x0500. Well, we're doing LoadLibrary, so we know it's
    safe...
*/
BOOL CALLBACK EnumMonitorCallback( HMONITOR p_hmonitor , HDC p_hdc, LPRECT rc, LPARAM  p_lparam)
{
    if ( rc->left == 0 && rc->top == 0 )
        g_monitors.insert( g_monitors.begin(), *rc );
    else
        g_monitors.push_back( *rc );
    return TRUE;
}


/*!
    @fn         void DoSaver( HWND hparwnd, bool fakemulti )
    @param      p_hmonitor
    @param      p_hdc
    @param      rc
    @param      p_lparam
    @return     BOOL   
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005

*/
void DoSaver( HWND hparwnd, bool fakemulti )
{
    if ( ScrMode == smPreview )
    {
        RECT rc;
        GetWindowRect( hparwnd, &rc );
        g_monitors.push_back( rc );
    }
    else
        if ( fakemulti )
        {
            int w = GetSystemMetrics( SM_CXSCREEN ), x1 = w / 4, x2 = w * 2 / 3, h = x2 - x1;
            RECT rc;
            rc.left = x1;
            rc.top = x1;
            rc.right = x1 + h;
            rc.bottom = x1 + h;
            g_monitors.push_back( rc );
            rc.left = 0;
            rc.top = x1;
            rc.right = x1;
            rc.bottom = x1 + x1;
            g_monitors.push_back( rc );
            rc.left = x2;
            rc.top = x1 + h + x2 - w;
            rc.right = w;
            rc.bottom = x1 + h;
            g_monitors.push_back( rc );
        }
        else
        {
            int num_monitors = GetSystemMetrics( 80 ); // 80=SM_CMONITORS
            if ( num_monitors > 1 )
            {
                typedef BOOL ( CALLBACK * LUMONITORENUMPROC ) ( HMONITOR, HDC, LPRECT, LPARAM );
                typedef BOOL ( WINAPI * LUENUMDISPLAYMONITORS ) ( HDC, LPCRECT, LUMONITORENUMPROC, LPARAM );
                HINSTANCE husr = LoadLibrary( _T( "user32.dll" ) );
                LUENUMDISPLAYMONITORS pEnumDisplayMonitors = 0;
                if ( husr != NULL )
                    pEnumDisplayMonitors = ( LUENUMDISPLAYMONITORS ) GetProcAddress( husr, "EnumDisplayMonitors" );
                if ( pEnumDisplayMonitors != NULL )
                    ( *pEnumDisplayMonitors ) ( NULL, NULL, EnumMonitorCallback, NULL );
                if ( husr != NULL )
                    FreeLibrary( husr );
            }
            if ( g_monitors.size() == 0 )
            {
                RECT rc;
                rc.left = 0;
                rc.top = 0;
                rc.right = GetSystemMetrics( SM_CXSCREEN );
                rc.bottom = GetSystemMetrics( SM_CYSCREEN );
                g_monitors.push_back( rc );
            }
        }
    //
    HWND hwnd = 0;
    if ( ScrMode == smPreview )
    {
        RECT rc;
        GetWindowRect( hparwnd, &rc );
        int w = rc.right - rc.left, h = rc.bottom - rc.top;
        int id = -1;
        hwnd = CreateWindowEx( 0, _T( "ScrClass" ), _T( "" ), WS_CHILD | WS_VISIBLE, 0, 0, w, h, hparwnd, NULL, g_hinstance, &id );
    }
    else
    {
        GetCursorPos( &g_init_cursor_pos );
        g_init_time = GetTickCount();
        for ( int i = 0; i < ( int ) g_monitors.size(); i++ )
        {
            const RECT &rc = g_monitors[ i ];
            DWORD exstyle = WS_EX_TOPMOST;
            if ( SCRDEBUG )
                exstyle = 0;
            HWND hthis = CreateWindowEx( exstyle, _T( "ScrClass" ), _T( "" ), WS_POPUP | WS_VISIBLE, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hwnd, NULL, g_hinstance, &i );
            if ( i == 0 )
                hwnd = hthis;
        }
    }
    if ( hwnd != NULL )
    {
        UINT oldval;
        if ( ScrMode == smSaver )
            SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, 1, &oldval, 0 );
        MSG msg;
        while ( GetMessage( &msg, NULL, 0, 0 ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        if ( ScrMode == smSaver )
            SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, 0, &oldval, 0 );
    }
    //
    g_saver_window.clear();
    return ;
}



/*!
    @fn         BOOL CALLBACK GeneralDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )   
    @param      hwnd
    @param      msg
    @param      wParam
    @param      lParam
    @return     BOOL
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
BOOL CALLBACK GeneralDlgProc( HWND hwnd, UINT msg, WPARAM, LPARAM lParam )
{
    switch ( msg )
    {
        case ( WM_INITDIALOG ) :
            {
                ShowWindow( GetDlgItem( hwnd, g_hot_services ? 102 : 101 ), SW_HIDE );
                SetDlgItemText( hwnd, 112, g_corners );
                SendDlgItemMessage( hwnd, 109, CB_ADDSTRING, 0, ( LPARAM ) _T( "seconds" ) );
                SendDlgItemMessage( hwnd, 109, CB_ADDSTRING, 0, ( LPARAM ) _T( "minutes" ) );
                SendDlgItemMessage( hwnd, 109, CB_SETCURSEL, g_password_delay_index, 0 );
                int n = g_password_delay;
                if ( g_password_delay_index == 1 )
                    n /= 60;
                TCHAR c[ 16 ];
                wsprintf( c, _T( "%i" ), n );
                SetDlgItemText( hwnd, 107, c );
                SendDlgItemMessage( hwnd, 108, UDM_SETRANGE, 0, MAKELONG( 99, 0 ) );
                SendDlgItemMessage( hwnd, 105, CB_ADDSTRING, 0, ( LPARAM ) _T( "High" ) );
                SendDlgItemMessage( hwnd, 105, CB_ADDSTRING, 0, ( LPARAM ) _T( "Normal" ) );
                SendDlgItemMessage( hwnd, 105, CB_ADDSTRING, 0, ( LPARAM ) _T( "Low" ) );
                SendDlgItemMessage( hwnd, 105, CB_ADDSTRING, 0, ( LPARAM ) _T( "Keyboard only (ignore mouse movement)" ) );
                SendDlgItemMessage( hwnd, 105, CB_SETCURSEL, g_mouse_threshold_index, 0 );
                if ( g_mute_sound )
                    CheckDlgButton( hwnd, 113, BST_CHECKED );
                OSVERSIONINFO ver;
                ZeroMemory( &ver, sizeof( ver ) );
                ver.dwOSVersionInfoSize = sizeof( ver );
                GetVersionEx( &ver );
                for ( int i = 106; i < 111 && ver.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS; i++ )
                    ShowWindow( GetDlgItem( hwnd, i ), SW_HIDE );
            }
            return TRUE;
        case ( WM_NOTIFY ) :
            {
                LPNMHDR nmh = ( LPNMHDR ) lParam;
                UINT code = nmh->code;
                switch ( code )
                {
                    case ( PSN_APPLY ) :
                        {
                            GetDlgItemText( hwnd, 112, g_corners, 5 );
                            g_password_delay_index = SendDlgItemMessage( hwnd, 109, CB_GETCURSEL, 0, 0 );
                            TCHAR c[ 16 ];
                            GetDlgItemText( hwnd, 107, c, 16 );
                            int n = _ttoi( c );
                            if ( g_password_delay_index == 1 )
                                n *= 60;
                            g_password_delay = n;
                            g_mouse_threshold_index = SendDlgItemMessage( hwnd, 105, CB_GETCURSEL, 0, 0 );
                            if ( g_mouse_threshold_index == 0 )
                                g_mouse_threshold = 0;
                            else
                                if ( g_mouse_threshold_index == 1 )
                                    g_mouse_threshold = 200;
                                else
                                    if ( g_mouse_threshold_index == 2 )
                                        g_mouse_threshold = 400;
                                    else
                                        g_mouse_threshold = 999999;
                            g_mute_sound = ( IsDlgButtonChecked( hwnd, 113 ) == BST_CHECKED );
                            WriteGeneralRegistry();
                            SetWindowLong( hwnd, DWLP_MSGRESULT, PSNRET_NOERROR );
                        }
                        return TRUE;
                }
            }
            return FALSE;
    }
    return FALSE;
}


/*!
    @fn         LRESULT CALLBACK MonitorWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )   
    @param      hwnd
    @param      msg
    @param      wParam
    @param      lParam
    @return     LRESULT
    @brief       either corners or a preview
    @author     James Mc Parlane
    @date       18 April 2005
*/
LRESULT CALLBACK MonitorWndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch ( msg )
    {
        case WM_CREATE:
            {
                TCHAR c[ 5 ];
                GetWindowText( hwnd, c, 5 );
                if ( *c != 0 )
                    return 0;
                int id = -1;
                RECT rc;
                SendMessage( hwnd, SCRM_GETMONITORAREA, 0, ( LPARAM ) & rc );
                CreateWindow( _T( "ScrClass" ), _T( "" ), WS_CHILD | WS_VISIBLE, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hwnd, NULL, g_hinstance, &id );
            }
            return 0;
        case WM_PAINT:
            {
                if ( g_hbmmonitor == 0 )
                    g_hbmmonitor = LoadBitmap( g_hinstance, _T( "Monitor" ) );
                RECT rc;
                GetClientRect( hwnd, &rc );
                //
                PAINTSTRUCT ps;
                BeginPaint( hwnd, &ps );
                HBITMAP hback = ( HBITMAP ) GetWindowLong( hwnd, GWLP_USERDATA );
                if ( hback != 0 )
                {
                    BITMAP bmp;
                    GetObject( hback, sizeof( bmp ), &bmp );
                    if ( bmp.bmWidth != rc.right || bmp.bmHeight != rc.bottom )
                    {
                        DeleteObject( hback );
                        hback = 0;
                    }
                }
                if ( hback == 0 )
                {
                    hback = CreateCompatibleBitmap( ps.hdc, rc.right, rc.bottom );
                    SetWindowLongPtr( hwnd, GWLP_USERDATA, ( LONG_PTR ) hback );
                }
                HDC backdc = CreateCompatibleDC( ps.hdc );
                HGDIOBJ holdback = SelectObject( backdc, hback );
                BitBlt( backdc, 0, 0, rc.right, rc.bottom, ps.hdc, 0, 0, SRCCOPY );
                //
                TCHAR corners[ 5 ];
                GetWindowText( hwnd, corners, 5 );
                HDC p_hdc = CreateCompatibleDC( ps.hdc );
                HGDIOBJ hold = SelectObject( p_hdc, g_hbmmonitor );
                StretchBlt( backdc, 0, 0, rc.right, rc.bottom, p_hdc, 0, 0, 184, 170, SRCAND );
                StretchBlt( backdc, 0, 0, rc.right, rc.bottom, p_hdc, 184, 0, 184, 170, SRCINVERT );
                RECT crc;
                SendMessage( hwnd, SCRM_GETMONITORAREA, 0, ( LPARAM ) & crc );
                //
                if ( *corners != 0 )
                    FillRect( backdc, &crc, GetSysColorBrush( COLOR_DESKTOP ) );
                for ( int i = 0; i < 4 && *corners != 0; i++ )
                {
                    RECT crc;
                    SendMessage( hwnd, SCRM_GETMONITORAREA, i + 1, ( LPARAM ) & crc );
                    int y = 0;
                    if ( corners[ i ] == 'Y' )
                        y = 22;
                    else
                        if ( corners[ i ] == 'N' )
                            y = 44;
                    BitBlt( backdc, crc.left, crc.top, crc.right - crc.left, crc.bottom - crc.top, p_hdc, 368, y, SRCCOPY );
                    if ( !g_hot_services )
                    {
                        DWORD col = GetSysColor( COLOR_DESKTOP );
                        for ( int y = crc.top; y < crc.bottom; y++ )
                        {
                            for ( int x = crc.left + ( y & 1 ); x < crc.right; x += 2 )
                                SetPixel( backdc, x, y, col );
                        }
                    }
                }
                SelectObject( p_hdc, hold );
                DeleteDC( p_hdc );
                //
                BitBlt( ps.hdc, 0, 0, rc.right, rc.bottom, backdc, 0, 0, SRCCOPY );
                SelectObject( backdc, holdback );
                DeleteDC( backdc );
                EndPaint( hwnd, &ps );
            }
            return 0;
        case SCRM_GETMONITORAREA:
            {
                RECT *prc = ( RECT* ) lParam;
                if ( g_hbmmonitor == 0 )
                    g_hbmmonitor = LoadBitmap( g_hinstance, _T( "Monitor" ) );
                // those are the client coordinates unscalled
                RECT wrc;
                GetClientRect( hwnd, &wrc );
                int ww = wrc.right, wh = wrc.bottom;
                RECT rc;
                rc.left = 16 * ww / 184;
                rc.right = 168 * ww / 184;
                rc.top = 17 * wh / 170;
                rc.bottom = 130 * wh / 170;
                *prc = rc;
                if ( wParam == 0 )
                    return 0;
                if ( wParam == 1 )
                {
                    prc->right = rc.left + 24;
                    prc->bottom = rc.top + 22;
                }
                else
                    if ( wParam == 2 )
                    {
                        prc->left = rc.right - 24;
                        prc->bottom = rc.top + 22;
                    }
                    else
                        if ( wParam == 3 )
                        {
                            prc->left = rc.right - 24;
                            prc->top = rc.bottom - 22;
                        }
                        else
                            if ( wParam == 4 )
                            {
                                prc->right = rc.left + 24;
                                prc->top = rc.bottom - 22;
                            }
            }
            return 0;
        case WM_LBUTTONDOWN:
            {
                if ( !g_hot_services )
                    return 0;
                int x = LOWORD( lParam ), y = HIWORD( lParam );
                TCHAR corners[ 5 ];
                GetWindowText( hwnd, corners, 5 );
                if ( corners[ 0 ] == 0 )
                    return 0;
                int click = -1;
                for ( int i = 0; i < 4; i++ )
                {
                    RECT rc;
                    SendMessage( hwnd, SCRM_GETMONITORAREA, i + 1, ( LPARAM ) & rc );
                    if ( x >= rc.left && y >= rc.top && x < rc.right && y < rc.bottom )
                    {
                        click = i;
                        break;
                    }
                }
                if ( click == -1 )
                    return 0;
                for ( int j = 0; j < 4; j++ )
                {
                    if ( corners[ j ] != '-' && corners[ j ] != 'Y' && corners[ j ] != 'N' )
                        corners[ j ] = '-';
                }
                corners[ 4 ] = 0;
                //
                HMENU hmenu = CreatePopupMenu();
                MENUITEMINFO mi;
                ZeroMemory( &mi, sizeof( mi ) );
                mi.cbSize = sizeof( MENUITEMINFO );
                mi.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE | MIIM_DATA;
                mi.fType = MFT_STRING | MFT_RADIOCHECK;
                mi.wID = 'N';
                mi.fState = MFS_ENABLED;
                if ( corners[ click ] == 'N' )
                    mi.fState |= MFS_CHECKED;
                mi.dwTypeData = _T( "Never" );
                mi.cch = sizeof( TCHAR ) * _tcslen( mi.dwTypeData );
                InsertMenuItem( hmenu, 0, TRUE, &mi );
                mi.wID = 'Y';
                mi.fState = MFS_ENABLED;
                if ( corners[ click ] == 'Y' )
                    mi.fState |= MFS_CHECKED;
                mi.dwTypeData = _T( "Now" );
                mi.cch = sizeof( TCHAR ) * _tcslen( mi.dwTypeData );
                InsertMenuItem( hmenu, 0, TRUE, &mi );
                mi.wID = '-';
                mi.fState = MFS_ENABLED;
                if ( corners[ click ] != 'Y' && corners[ click ] != 'N' )
                    mi.fState |= MFS_CHECKED;
                mi.dwTypeData = _T( "Default" );
                mi.cch = sizeof( TCHAR ) * _tcslen( mi.dwTypeData );
                InsertMenuItem( hmenu, 0, TRUE, &mi );
                POINT pt;
                pt.x = x;
                pt.y = y;
                ClientToScreen( hwnd, &pt );
                int cmd = TrackPopupMenuEx( hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, hwnd, NULL );
                if ( cmd != 0 )
                    corners[ click ] = ( char ) cmd;
                corners[ 4 ] = 0;
                SetWindowText( hwnd, corners );
                InvalidateRect( hwnd, NULL, FALSE );
            }
            return 0;
        case WM_DESTROY:
            {
                HBITMAP hback = ( HBITMAP ) SetWindowLong( hwnd, GWLP_USERDATA, 0 );
                if ( hback != 0 )
                    DeleteObject( hback );
            }
            return 0;
    }
    return DefWindowProc( hwnd, msg, wParam, lParam );
}





/*!
    @fn         BOOL CALLBACK AboutDlgProc( HWND hdlg, UINT msg, WPARAM wParam, LPARAM )
    @param      hdlg
    @param      msg
    @param      wParam
    @param      lParam
    @return     BOOL
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
BOOL CALLBACK AboutDlgProc( HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if ( msg == WM_INITDIALOG )
    {
        SetDlgItemText( hdlg, 101, g_saver_name.c_str() );
        SetDlgItemUrl( hdlg, 102, _T( "http://www.wischik.com/lu/scr/" ) );
        SetDlgItemText( hdlg, 102, _T( "www.wischik.com/scr" ) );
        return TRUE;
    }
    else
        if ( msg == WM_COMMAND )
        {
            int id = LOWORD( wParam );
            if ( id == IDOK || id == IDCANCEL )
                EndDialog( hdlg, id );
            return TRUE;
        }
        else
            return FALSE;
}


WNDPROC OldSubclassProc = 0;

/*!
    @fn         LRESULT CALLBACK PropertysheetSubclassProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )   
    @param      hwnd
    @param      msg
    @param      wParam
    @param      lParam
    @return     LRESULT
    @brief      this is to stick an "About" option on the sysmenu.
    @author     James Mc Parlane
    @date       18 April 2005

    PROPERTY SHEET SUBCLASSING -- this is to stick an "About" option on the sysmenu.

*/ 
LRESULT CALLBACK PropertysheetSubclassProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if ( msg == WM_SYSCOMMAND && wParam == 3500 )
    {
        DialogBox( g_hinstance, _T( "DLG_ABOUT" ), hwnd, (DLGPROC)AboutDlgProc );
        return 0;
    }
    if ( OldSubclassProc != NULL )
        return CallWindowProc( OldSubclassProc, hwnd, msg, wParam, lParam );
    else
        return DefWindowProc( hwnd, msg, wParam, lParam );
}

int CALLBACK PropertysheetCallback( HWND hwnd, UINT msg, LPARAM )
{
    if ( msg != PSCB_INITIALIZED )
        return 0;
    HMENU hsysmenu = GetSystemMenu( hwnd, FALSE );
    AppendMenu( hsysmenu, MF_SEPARATOR, 1, _T( "-" ) );
    AppendMenu( hsysmenu, MF_STRING, 3500, _T( "About..." ) );
    OldSubclassProc = ( WNDPROC ) SetWindowLongPtr( hwnd, GWLP_WNDPROC, ( LONG_PTR ) PropertysheetSubclassProc );
    return 0;
}

/*!
    @fn         void DoConfig( HWND hpar )
    @param      hpar
    @return     void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void DoConfig( HWND hpar )
{
    g_hiconbg = ( HICON ) LoadImage( g_hinstance, MAKEINTRESOURCE( 1 ), IMAGE_ICON, GetSystemMetrics( SM_CXICON ), GetSystemMetrics( SM_CYICON ), 0 );
    g_hiconsm = ( HICON ) LoadImage( g_hinstance, MAKEINTRESOURCE( 1 ), IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ), 0 );
    //
    PROPSHEETHEADER psh;
    ZeroMemory( &psh, sizeof( psh ) );
    PROPSHEETPAGE psp[ 2 ];
    ZeroMemory( psp, 2 * sizeof( PROPSHEETPAGE ) );
    psp[ 0 ].dwSize = sizeof( psp[ 0 ] );
    psp[ 0 ].dwFlags = PSP_DEFAULT;
    psp[ 0 ].hInstance = g_hinstance;
    psp[ 0 ].pszTemplate = _T( "DLG_GENERAL" );
    psp[ 0 ].pfnDlgProc = (DLGPROC)GeneralDlgProc;
    psp[ 1 ].dwSize = sizeof( psp[ 1 ] );
    psp[ 1 ].dwFlags = PSP_DEFAULT;
    psp[ 1 ].hInstance = g_hinstance;
    psp[ 1 ].pszTemplate = _T( "DLG_OPTIONS" );
    psp[ 1 ].pfnDlgProc = (DLGPROC)OptionsDlgProc;
    psh.dwSize = sizeof( psh );
    psh.dwFlags = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE | PSH_USEHICON | PSH_USECALLBACK;
    psh.hwndParent = hpar;
    psh.hInstance = g_hinstance;
    psh.hIcon = g_hiconsm;
    tstring cap = _T( "Options for " ) + g_saver_name;
    psh.pszCaption = cap.c_str();
    psh.nPages = 2;
    psh.nStartPage = 1;
    psh.ppsp = psp;
    psh.pfnCallback = PropertysheetCallback;
    Debug( _T( "Config..." ) );
    PropertySheet( &psh );
    Debug( _T( "Config done." ) );
    if ( g_hiconbg != 0 )
        DestroyIcon( g_hiconbg );
    g_hiconbg = 0;
    if ( g_hiconsm != 0 )
        DestroyIcon( g_hiconsm );
    g_hiconsm = 0;
    if ( g_hbmmonitor != 0 )
        DeleteObject( g_hbmmonitor );
    g_hbmmonitor = 0;
}

/*!
    @fn         HWND CheckForScrprev()
    @return   
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005

    This routine is for using ScrPrev. It's so that you can start the saver
    with the command line /p scrprev and it runs itself in a preview window.
    You must first copy ScrPrev somewhere in your search path
*/ 
HWND CheckForScrprev()
{
    HWND hwnd = FindWindow( _T( "Scrprev" ), NULL ); // looks for the Scrprev class
    if ( hwnd == NULL )      // try to load it
    { 
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory( &si, sizeof( si ) );
        ZeroMemory( &pi, sizeof( pi ) );
        si.cb = sizeof( si );
        TCHAR cmd[ MAX_PATH ];

        // unicode CreateProcess requires it writeable
        _tcscpy( cmd, _T( "Scrprev" ) ); 
        BOOL cres = CreateProcess( NULL, cmd, 0, 0, FALSE, CREATE_NEW_PROCESS_GROUP | CREATE_DEFAULT_ERROR_MODE,0, 0, &si, &pi );

        if ( !cres )
        {
            Debug( _T( "Error creating scrprev process" ) );
            return NULL;
        }

        DWORD wres = WaitForInputIdle( pi.hProcess, 2000 );
        if ( wres == WAIT_TIMEOUT )
        {
            Debug( _T( "Scrprev never becomes idle" ) );
            return NULL;
        }

        if ( wres == 0xFFFFFFFF )
        {
            Debug( _T( "ScrPrev, misc error after ScrPrev execution" ) );
            return NULL;
        }

        hwnd = FindWindow( _T( "Scrprev" ), NULL );
    }

    if ( hwnd == NULL )
    {
        Debug( _T( "Unable to find Scrprev window" ) );
        return NULL;
    }

    ::SetForegroundWindow( hwnd );

    hwnd = GetWindow( hwnd, GW_CHILD );
    if ( hwnd == NULL )
    {
        Debug( _T( "Couldn't find Scrprev child" ) );
        return NULL;
    }
    return hwnd;
}

/*!
    @fn         void DoInstall()
    @return     void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void DoInstall()
{
    TCHAR windir[ MAX_PATH ];
    GetWindowsDirectory( windir, MAX_PATH );

    TCHAR tfn[ MAX_PATH ];

    UINT ures = GetTempFileName( windir, _T( "pst" ), 0, tfn );

    if ( ures == 0 )
    {
        MessageBox( NULL, _T( "You must be logged on as system administrator to install screen savers" ), _T( "Saver Install" ), MB_ICONINFORMATION | MB_OK );
        return ;
    }

    DeleteFile( tfn );

    tstring fn = tstring( windir ) + _T( "\\" ) + g_saver_name + _T( ".scr" );

    DWORD attr = GetFileAttributes( fn.c_str() );

    bool exists = ( attr != 0xFFFFFFFF );

    tstring msg = _T( "Do you want to install '" ) + g_saver_name + _T( "' ?" );

    if ( exists )
    {
        msg += _T( "\r\n\r\n(This will replace the version that you have currently)" );
    }

    int res = MessageBox( NULL, msg.c_str(), _T( "Saver Install" ), MB_YESNOCANCEL );
    
    if ( res != IDYES )
    {
        return ;
    }
    
    TCHAR cfn[ MAX_PATH ];
    
    GetModuleFileName( g_hinstance, cfn, MAX_PATH );
    
    SetCursor( LoadCursor( NULL, IDC_WAIT ) );
    
    BOOL bres = CopyFile( cfn, fn.c_str(), FALSE );

    if ( !bres )
    {
        tstring msg = _T( "There was an error installing the saver.\r\n\r\n\"" ) + GetLastErrorString() + _T( "\"" );
        MessageBox( NULL, msg.c_str(), _T( "Saver Install" ), MB_ICONERROR | MB_OK );
        SetCursor( LoadCursor( NULL, IDC_ARROW ) );
        return ;
    }

    LONG lres;
    HKEY skey;
    DWORD disp;
    tstring val;
    tstring key = REGSTR_PATH_UNINSTALL _T( "\\" ) + g_saver_name;
    lres = RegCreateKeyEx( HKEY_LOCAL_MACHINE, key.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &skey, &disp );
    if ( lres == ERROR_SUCCESS )
    {
        val = g_saver_name + _T( " saver" );
        RegSetValueEx( skey, _T( "DisplayName" ), 0, REG_SZ, ( const BYTE* ) val.c_str(), sizeof( TCHAR ) * ( val.length() + 1 ) );
        
        val = _T( "\"" ) + fn + _T( "\" /u" );
        RegSetValueEx( skey, _T( "UninstallString" ), 0, REG_SZ, ( const BYTE* ) val.c_str(), sizeof( TCHAR ) * ( val.length() + 1 ) );
        RegSetValueEx( skey, _T( "UninstallPath" ), 0, REG_SZ, ( const BYTE* ) val.c_str(), sizeof( TCHAR ) * ( val.length() + 1 ) );
        
        val = _T( "\"" ) + fn + _T( "\"" );
        RegSetValueEx( skey, _T( "ModifyPath" ), 0, REG_SZ, ( const BYTE* ) val.c_str(), sizeof( TCHAR ) * ( val.length() + 1 ) );
        
        val = fn;
        RegSetValueEx( skey, _T( "DisplayIcon" ), 0, REG_SZ, ( const BYTE* ) val.c_str(), sizeof( TCHAR ) * ( val.length() + 1 ) );
        
        TCHAR url[ 1024 ];
        
        int ures = LoadString( g_hinstance, 2, url, 1024 );
        
        if ( ures != 0 )
        {
            RegSetValueEx( skey, _T( "HelpLink" ), 0, REG_SZ, ( const BYTE* ) url, sizeof( TCHAR ) * ( _tcslen( url ) + 1 ) );
        }
        RegCloseKey( skey );
    }
    SHELLEXECUTEINFO sex;
    ZeroMemory( &sex, sizeof( sex ) );
    sex.cbSize = sizeof( sex );
    sex.fMask = SEE_MASK_NOCLOSEPROCESS;
    sex.lpVerb = _T( "install" );
    sex.lpFile = fn.c_str();
    sex.nShow = SW_SHOWNORMAL;
    bres = ShellExecuteEx( &sex );
    
    if ( !bres )
    {
        SetCursor( LoadCursor( NULL, IDC_ARROW ) );
        MessageBox( NULL, _T( "The saver has been installed" ), g_saver_name.c_str(), MB_OK );
        return ;
    }

    WaitForInputIdle( sex.hProcess, 2000 );
    CloseHandle( sex.hProcess );
    SetCursor( LoadCursor( NULL, IDC_ARROW ) );
}

/*!
    @fn         void DoUninstall()
    @return     void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void DoUninstall()
{
    tstring key = REGSTR_PATH_UNINSTALL _T( "\\" ) + g_saver_name;
    RegDeleteKey( HKEY_LOCAL_MACHINE, key.c_str() );
    TCHAR fn[ MAX_PATH ];
    GetModuleFileName( g_hinstance, fn, MAX_PATH );
    SetFileAttributes( fn, FILE_ATTRIBUTE_NORMAL ); // remove readonly if necessary
    BOOL res = MoveFileEx( fn, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
    //
    const TCHAR *c = fn, *lastslash = c;
    while ( *c != 0 )
    {
        if ( *c == '\\' || *c == '/' )
            lastslash = c + 1;
        c++;
    }
    tstring cap = g_saver_name + _T( " uninstaller" );
    tstring msg;
    if ( res )
        msg = _T( "Uninstall completed. The saver will be removed next time you reboot." );
    else
        msg = _T( "There was a problem uninstalling.\r\n" )
              _T( "To complete the uninstall manually, you should go into your Windows " )
              _T( "directory and delete the file '" ) + tstring( lastslash ) + _T( "'" );
    MessageBox( NULL, msg.c_str(), cap.c_str(), MB_OK );
}




void SetDlgItemUrl( HWND hdlg, int id, const TCHAR *url );

// Implementation notes:
// We have to subclass both the static control (to set its cursor, to respond to click)
// and the dialog procedure (to set the font of the static control). Here are the two
// subclasses:
LRESULT CALLBACK UrlCtlProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK UrlDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

/*!
    @brief TUrlData     

    When the user calls SetDlgItemUrl, then the static-control-subclass is added
    if it wasn't already there, and the dialog-subclass is added if it wasn't
    already there. Both subclasses are removed in response to their respective
    WM_DESTROY messages. Also, each subclass stores a property in its window,
    which is a HGLOBAL handle to a GlobalAlloc'd structure:
*/
typedef struct
{
    TCHAR *url;
    WNDPROC oldproc;
    HFONT hf;
    HBRUSH hb;
}
TUrlData;




/*!
    @fn         void SetDlgItemUrl( HWND hdlg, int id, const TCHAR *url )
    @param      hdlg
    @param      id
    @param      url
    @return     void   
    @brief      this is the routine that sets up the subclassing
    @author     James Mc Parlane
    @date       18 April 2005

    SetDlgItemUrl(hwnd,IDC_MYSTATIC,"http://www.wischik.com/lu");
    This routine turns a dialog's static text control into an underlined hyperlink.
    You can call it in your WM_INITDIALOG, or anywhere.
    It will also set the text of the control... if you want to change the text
    back, you can just call SetDlgItemText() afterwards.

    I'm a miser and only defined a single structure, which is used by both
    the control-subclass and the dialog-subclass. Both of them use 'oldproc' of course.
    The control-subclass only uses 'url' (in response to WM_LBUTTONDOWN),
    and the dialog-subclass only uses 'hf' and 'hb' (in response to WM_CTLCOLORSTATIC)
    there is one sneaky thing to note. We create our underlined font *lazily*.
    Initially, hf is just NULL. But the first time the subclassed dialog received
    WM_CTLCOLORSTATIC, we sneak a peak at the font that was going to be used for
    the control, and we create our own copy of it but including the underline style.
    This way our code works fine on dialogs of any font.

    SetDlgItemUrl: this is the routine that sets up the subclassing.

*/ 
void SetDlgItemUrl( HWND hdlg, int id, const TCHAR *url )
{ // nb. vc7 has crummy warnings about 32/64bit. My code's perfect! That's why I hide the warnings.
#pragma warning( push )
 #pragma warning( disable: 4312 4244 )
    // First we'll subclass the edit control
    HWND hctl = GetDlgItem( hdlg, id );
    SetWindowText( hctl, url );

    HGLOBAL hold = ( HGLOBAL ) GetProp( hctl, _T( "href_dat" ) );

    if ( hold != NULL )      // if it had been subclassed before, we merely need to tell it the new url
    { 
        TUrlData * ud = ( TUrlData* ) GlobalLock( hold );
        delete[] ud->url;
        ud->url = new TCHAR[ _tcslen( url ) + 1 ];
        _tcscpy( ud->url, url );
    }
    else
    {
        HGLOBAL hglob = GlobalAlloc( GMEM_MOVEABLE, sizeof( TUrlData ) );
        TUrlData *ud = ( TUrlData* ) GlobalLock( hglob );
        ud->oldproc = ( WNDPROC ) GetWindowLong( hctl, GWLP_WNDPROC );
        ud->url = new TCHAR[ _tcslen( url ) + 1 ];
        _tcscpy( ud->url, url );
        ud->hf = 0;
        ud->hb = 0;
        GlobalUnlock( hglob );
        SetProp( hctl, _T( "href_dat" ), hglob );
        SetWindowLongPtr( hctl, GWLP_WNDPROC, ( LONG_PTR ) UrlCtlProc );
    }
    //
    // Second we subclass the dialog
    hold = ( HGLOBAL ) GetProp( hdlg, _T( "href_dlg" ) );

    if ( hold == NULL )
    {
        HGLOBAL hglob = GlobalAlloc( GMEM_MOVEABLE, sizeof( TUrlData ) );
        TUrlData *ud = ( TUrlData* ) GlobalLock( hglob );
        ud->url = 0;
        ud->oldproc = ( WNDPROC ) GetWindowLong( hdlg, GWLP_WNDPROC );
        ud->hb = CreateSolidBrush( GetSysColor( COLOR_BTNFACE ) );
        ud->hf = 0; // the font will be created lazilly, the first time WM_CTLCOLORSTATIC gets called
        GlobalUnlock( hglob );
        SetProp( hdlg, _T( "href_dlg" ), hglob );
		SetWindowLongPtr( hdlg, GWLP_WNDPROC, (LONG_PTR) UrlDlgProc );
    }
#pragma warning( pop )
}


/*!
    @fn         LRESULT CALLBACK UrlCtlProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )    
    @param      hwnd
    @param      msg
    @param      wParam
    @paran      lParam
    @return     LRESULT   
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005

  UrlCtlProc: this is the subclass procedure for the static control
*/ 
LRESULT CALLBACK UrlCtlProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HGLOBAL hglob = ( HGLOBAL ) GetProp( hwnd, _T( "href_dat" ) );
    if ( hglob == NULL )
    {
        return DefWindowProc( hwnd, msg, wParam, lParam );
    }
    TUrlData *oud = ( TUrlData* ) GlobalLock( hglob );
    TUrlData ud = *oud;
    GlobalUnlock( hglob ); // I made a copy of the structure just so I could GlobalUnlock it now, to be more local in my code
    switch ( msg )
    {
        case WM_DESTROY:
            {
                RemoveProp( hwnd, _T( "href_dat" ) );
                GlobalFree( hglob );
                if ( ud.url != 0 )
                {
                    delete[] ud.url;
                }
                // nb. remember that ud.url is just a pointer to a memory block. It might look weird
                // for us to delete ud.url instead of oud->url, but they're both equivalent.
            }
            break;
        case WM_LBUTTONDOWN:
            {
                HWND hdlg = GetParent( hwnd );
                if ( hdlg == 0 )
                {
                    hdlg = hwnd;
                }
                ShellExecute( hdlg, _T( "open" ), ud.url, NULL, NULL, SW_SHOWNORMAL );
            }
            break;
        case WM_SETCURSOR:
            {
                HCURSOR hc = LoadCursor( NULL, MAKEINTRESOURCE( 32649 ) ); // =IDC_HAND
                if ( hc == 0 )
                {
                    hc = LoadCursor( NULL, IDC_ARROW ); // systems before Win2k didn't have the hand
                }
                SetCursor( hc );
                return TRUE;
            }
        case WM_NCHITTEST:
            {
                return HTCLIENT; // because normally a static returns HTTRANSPARENT, so disabling WM_SETCURSOR
            }
    }
    return CallWindowProc( ud.oldproc, hwnd, msg, wParam, lParam );
}


/*!
    @fn         LRESULT CALLBACK UrlDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )   
    @param      hwnd  
    @param      msg
    @param      wParam
    @param      lParam
    @return     LRESULT
    @brief   
    @author   James Mc Parlane
    @date   18 April 2005

    UrlDlgProc: this is the subclass procedure for the dialog
*/ 
LRESULT CALLBACK UrlDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HGLOBAL hglob = ( HGLOBAL ) GetProp( hwnd, _T( "href_dlg" ) );

    if ( hglob == NULL )
    {
        return DefWindowProc( hwnd, msg, wParam, lParam );
    }

    TUrlData *oud = ( TUrlData* ) GlobalLock( hglob );

    TUrlData ud = *oud;

    GlobalUnlock( hglob );

    switch ( msg )
    {
        case WM_DESTROY:
            {
                RemoveProp( hwnd, _T( "href_dlg" ) );
                GlobalFree( hglob );
                if ( ud.hb != 0 )
                {
                    DeleteObject( ud.hb );
                }

                if ( ud.hf != 0 )
                {
                    DeleteObject( ud.hf );
                }
            }
            break;
        case WM_CTLCOLORSTATIC:
            {
                HDC p_hdc = ( HDC ) wParam;
                HWND hctl = ( HWND ) lParam;
                // To check whether to handle this control, we look for its subclassed property!
                HANDLE hprop = GetProp( hctl, _T( "href_dat" ) );
                
                if ( hprop == NULL )
                {
                    return CallWindowProc( ud.oldproc, hwnd, msg, wParam, lParam );
                }
                // There has been a lot of faulty discussion in the newsgroups about how to change
                // the text colour of a static control. Lots of people mess around with the
                // TRANSPARENT text mode. That is incorrect. The correct solution is here:
                // (1) Leave the text opaque. This will allow us to re-SetDlgItemText without it looking wrong.
                // (2) SetBkColor. This background colour will be used underneath each character cell.
                // (3) return HBRUSH. This background colour will be used where there's no text.
                SetTextColor( p_hdc, RGB( 0, 0, 255 ) );
                SetBkColor( p_hdc, GetSysColor( COLOR_BTNFACE ) );
                if ( ud.hf == 0 )
                { // we use lazy creation of the font. That's so we can see font was currently being used.
                    TEXTMETRIC tm;
                    GetTextMetrics( p_hdc, &tm );
                    LOGFONT lf;
                    lf.lfHeight = tm.tmHeight;
                    lf.lfWidth = 0;
                    lf.lfEscapement = 0;
                    lf.lfOrientation = 0;
                    lf.lfWeight = tm.tmWeight;
                    lf.lfItalic = tm.tmItalic;
                    lf.lfUnderline = TRUE;
                    lf.lfStrikeOut = tm.tmStruckOut;
                    lf.lfCharSet = tm.tmCharSet;
                    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
                    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
                    lf.lfQuality = DEFAULT_QUALITY;
                    lf.lfPitchAndFamily = tm.tmPitchAndFamily;
                    GetTextFace( p_hdc, LF_FACESIZE, lf.lfFaceName );
                    ud.hf = CreateFontIndirect( &lf );
                    TUrlData *oud = ( TUrlData* ) GlobalLock( hglob );
                    oud->hf = ud.hf;
                    GlobalUnlock( hglob );
                }
                SelectObject( p_hdc, ud.hf );
                // Note: the win32 docs say to return an HBRUSH, typecast as a BOOL. But they
                // fail to explain how this will work in 64bit windows where an HBRUSH is 64bit.
                // I have supressed the warnings for now, because I hate them...
#pragma warning( push )
#pragma warning( disable: 4311 )
                return ( BOOL ) ud.hb;
#pragma warning( pop )

            }
    }
    return CallWindowProc( ud.oldproc, hwnd, msg, wParam, lParam );
}


/*!
    @fn         inline void Debug( const tstring s )
    @param      s
    @return   
    @brief   
    @author   James Mc Parlane
    @date   18 April 2005
*/
inline void Debug( const tstring s )
{
    if ( g_debug_file == _T( "" ) )
    {
        return ;
    }
    if ( g_debug_file == _T( "OutputDebugString" ) )
    {
        tstring err = s + _T( "\r\n" );
        OutputDebugString( err.c_str() );
    }
    else
    {
        FILE *f = _tfopen( g_debug_file.c_str(), _T( "a+t" ) );
        _ftprintf( f, _T( "%s\n" ), s.c_str() );
        fclose( f );
    }
}


/*!
    @fn         tstring GetLastErrorString() 
    @return     tstring
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
tstring GetLastErrorString()
{
    LPVOID lpMsgBuf;
    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                   GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPTSTR ) & lpMsgBuf, 0, NULL );
    tstring s( ( TCHAR* ) lpMsgBuf );
    LocalFree( lpMsgBuf );
    return s;
}

/*!
    @fn         void RegSave( const tstring name, DWORD type, void*buf, int size ) 
    @param      name
    @param      type
    @param      buf
    @param      size
    @return   void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void RegSave( const tstring name, DWORD type, void*buf, int size )
{
    tstring path = _T( "Software\\Scrplus\\" ) + g_saver_name;
    HKEY skey;
    LONG res = RegCreateKeyEx( HKEY_CURRENT_USER, path.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &skey, 0 );
    if ( res != ERROR_SUCCESS )
        return ;
    RegSetValueEx( skey, name.c_str(), 0, type, ( const BYTE* ) buf, size );
    RegCloseKey( skey );
}

/*!
    @fn         bool RegLoadDword( const tstring name, DWORD *buf ) 
    @param      name
    @param      buf
    @return     bool
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
bool RegLoadDword( const tstring name, DWORD *buf )
{
    tstring path = _T( "Software\\Scrplus\\" ) + g_saver_name;
    HKEY skey;
    LONG res = RegOpenKeyEx( HKEY_CURRENT_USER, path.c_str(), 0, KEY_READ, &skey );
    if ( res != ERROR_SUCCESS )
        return false;
    DWORD size = sizeof( DWORD );
    res = RegQueryValueEx( skey, name.c_str(), 0, 0, ( LPBYTE ) buf, &size );
    RegCloseKey( skey );
    return ( res == ERROR_SUCCESS );
}

/*!
    @fn         void RegSave( const tstring name, int val )  
    @param      name
    @param      val
    @return     void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void RegSave( const tstring name, int val )
{
    DWORD v = val;
    RegSave( name, REG_DWORD, &v, sizeof( v ) );
}

/*!
    @fn         void RegSave( const tstring name, bool val )
    @param      name
    @param      val
    @return     void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void RegSave( const tstring name, bool val )
{
    RegSave( name, val ? 1 : 0 );
}

/*!
    @fn         void RegSave( const tstring name, tstring val )   
    @param      name
    @param      val
    @return     void
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
void RegSave( const tstring name, tstring val )
{
    RegSave( name, REG_SZ, ( void* ) val.c_str(), sizeof( TCHAR ) * ( val.length() + 1 ) );
}

/*!
     @fn        int RegLoad( const tstring name, int def )
     @param     name
     @param     def
     @return    int
     @brief   
     @author    James Mc Parlane
     @date      18 April 2005
*/
int RegLoad( const tstring name, int def )
{
    DWORD val;
    bool res = RegLoadDword( name, &val );
    return res ? val : def;
}

/*!
    @fn         bool RegLoad( const tstring name, bool def )  
    @param      name
    @param      def
    @return     bool
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
bool RegLoad( const tstring name, bool def )
{
    int b = RegLoad( name, def ? 1 : 0 );
    return ( b != 0 );
}

/*!
    @fn         tstring RegLoad( const tstring name, tstring def )   
    @param      name
    @param      def
    @return     tstring
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
tstring RegLoad( const tstring name, tstring def )
{
    tstring path = _T( "Software\\Scrplus\\" ) + g_saver_name;
    HKEY skey;
    LONG res = RegOpenKeyEx( HKEY_CURRENT_USER, path.c_str(), 0, KEY_READ, &skey );
    if ( res != ERROR_SUCCESS )
        return def;
    DWORD size = 0;
    res = RegQueryValueEx( skey, name.c_str(), 0, 0, 0, &size );
    if ( res != ERROR_SUCCESS )
    {
        RegCloseKey( skey );
        return def;
    }
    TCHAR *buf = new TCHAR[ size ];
    RegQueryValueEx( skey, name.c_str(), 0, 0, ( LPBYTE ) buf, &size );
    tstring s( buf );
    delete[] buf;
    RegCloseKey( skey );
    return s;
}


/*!
    @fn         int WINAPI WinMain( HINSTANCE h, HINSTANCE, LPSTR, int )
    @param      h
    @param      hi
    @param      lp
    @param      i
    @return   
    @brief   
    @author     James Mc Parlane
    @date       18 April 2005
*/
int WINAPI WinMain( HINSTANCE h, HINSTANCE hi, LPSTR lp, int i)
{
    g_hinstance = h;
    TCHAR name[ MAX_PATH ];
    int sres = LoadString( g_hinstance, 1, name, MAX_PATH );
    if ( sres == 0 )
    {
        MessageBox( NULL, _T( "Must store saver name as String Resource 1" ), _T( "Saver" ), MB_ICONERROR | MB_OK );
        return 0;
    }
    g_saver_name = name;
    //
    TCHAR mod[ MAX_PATH ];
    GetModuleFileName( g_hinstance, mod, MAX_PATH );
    tstring smod( mod );
    bool isexe = ( smod.find( _T( ".exe" ) ) != tstring::npos || smod.find( _T( ".EXE" ) ) != tstring::npos );
    //
    TCHAR *c = GetCommandLine();
    if ( *c == '\"' )
    {
        c++;
        while ( *c != 0 && *c != '\"' )
            c++;
        if ( *c == '\"' )
            c++;
    }
    else
    {
        while ( *c != 0 && *c != ' ' )
            c++;
    }
    while ( *c == ' ' )
        c++;
    HWND hwnd = NULL;
    bool fakemulti = false;
    if ( *c == 0 )
    {
        if ( isexe )
            ScrMode = smInstall;
        else
            ScrMode = smConfig;
        hwnd = NULL;
    }
    else
    {
        if ( *c == '-' || *c == '/' )
            c++;
        if ( *c == 'u' || *c == 'U' )
            ScrMode = smUninstall;
        if ( *c == 'p' || *c == 'P' || *c == 'l' || *c == 'L' )
        {
            c++;
            while ( *c == ' ' || *c == ':' )
                c++;
            if ( _tcsicmp( c, _T( "scrprev" ) ) == 0 )
                hwnd = CheckForScrprev();
            else
                hwnd = ( HWND ) _ttoi( c );
            ScrMode = smPreview;
        }
        else
            if ( *c == 's' || *c == 'S' )
            {
                ScrMode = smSaver;
                fakemulti = ( c[ 1 ] == 'm' || c[ 1 ] == 'M' );
            }
            else
                if ( *c == 'c' || *c == 'C' )
                {
                    c++;
                    while ( *c == ' ' || *c == ':' )
                        c++;
                    if ( *c == 0 )
                        hwnd = GetForegroundWindow();
                    else
                        hwnd = ( HWND ) _ttoi( c );
                    ScrMode = smConfig;
                }
                else
                    if ( *c == 'a' || *c == 'A' )
                    {
                        c++;
                        while ( *c == ' ' || *c == ':' )
                            c++;
                        hwnd = ( HWND ) _ttoi( c );
                        ScrMode = smPassword;
                    }
    }
    //
    if ( ScrMode == smInstall )
    {
        DoInstall();
        return 0;
    }
    if ( ScrMode == smUninstall )
    {
        DoUninstall();
        return 0;
    }
    if ( ScrMode == smPassword )
    {
        ChangePassword( hwnd );
        return 0;
    }
    //
    ReadGeneralRegistry();
    //
    INITCOMMONCONTROLSEX icx;
    ZeroMemory( &icx, sizeof( icx ) );
    icx.dwSize = sizeof( icx );
    icx.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx( &icx );
    //
    WNDCLASS wc;
    ZeroMemory( &wc, sizeof( wc ) );
    wc.hInstance = g_hinstance;
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.lpszClassName = _T( "ScrMonitor" );
    wc.lpfnWndProc = MonitorWndProc;
    RegisterClass( &wc );
    //
    wc.lpfnWndProc = SaverWndProc;
    wc.cbWndExtra = 8;
    wc.style = CS_OWNDC;
    wc.lpszClassName = _T( "ScrClass" );
    RegisterClass( &wc );
    //
    if ( ScrMode == smConfig )
        DoConfig( hwnd );
    else
        if ( ScrMode == smSaver || ScrMode == smPreview )
            DoSaver( hwnd, fakemulti );
    //
    return 0;
}


/* end of mw_secreensaver_windows*/
/*@}*/

#endif /* __TEST__ */

#ifdef __TEST__

#ifndef MW_TEST_LEGACY_H
#include "mw_test_legacy.h"
#endif /* MW_TEST_LEGACY_H */

#ifndef _MW_ARGUMENTS_H_
#include "mw_arguments.h"
#endif // _MW_ARGUMENTS_H_

/*!
 @fn    static void MXMLProfile_MwParser()
 @return   bool
 @brief   test and profile the MwParser Parser
 @author   James Mc Parlane
 @date   21 April 2003
*/
static void Test_MwW3ParserLexerBuilder_Simple()
{
    DEBUG_ENTER( Test_MwW3ParserLexerBuilder_Simple );


    DEBUG_EXIT();
}



int main( int argc, const char * argv[] )
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
    if ( !mw_interpret_arguments( argc, argv ) )
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

    LOG( "MAIN", "+ initialising metawrap engine." );

    // redirect test output to this standard function
    mw_set_print( logext );

    // start up the metawrap engine
    mw_open();

    // get the args
    mw_test_legacy_process_arguments( argc, argv );

    // start reporting
    mw_test_legacy_start();

    // report a successful test
    mw_test_legacy_testcase_PASSED( "mw_secreensaver_windows compiles", "Basic test of MetaWrap - Screensaver - Windows " );

    //Test_MwW3ParserLexerBuilder_Simple();

    // start reporting
    mw_test_legacy_end();

    LOG( "MAIN", "+ closing metawrap." );

    // shutdown the metawrap engine
    mw_close();

    LOG( "MAIN", "+ closing metawrap instance" );

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







