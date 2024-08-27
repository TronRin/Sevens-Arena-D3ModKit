/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __SYS_IMGUI_H__
#define __SYS_IMGUI_H__

#include "../libs/imgui/imgui.h"

enum idImGuiWindow {
	WINDOW_NONE        = 0,
	WINDOW_SETTINGS    = 1, // advanced settings menu
	WINDOW_OVERLAYS    = 2, // overlays
	WINDOW_DEMO        = 4, // ImGui demo window
	// next should be 4, then 8, etc so a bitmask can be used
};

enum idImGuiStyle {
	Dhewm3,
	ImGui_Default,
	User
};

/*
===============================================================================

	Public imgui interface with methods to display dialogs.

===============================================================================
*/

class idImGui {
public:
	virtual			~idImGui() {}

					// Initialize imgui for the first time.
	virtual bool	Init( void *sdlWindow, void *sdlGlContext ) = 0;

					// Shut down imgui.
	virtual void	Shutdown( void ) = 0;

	virtual bool	IsEnabled( void ) = 0;

	virtual void	OpenWindow( idImGuiWindow win ) = 0;

	virtual void	CloseWindow( idImGuiWindow win ) = 0;

					// enum idImGuiWindow values of all currently open imgui windows or-ed together
					// (0 if none are open)
	virtual int		GetOpenWindowsMask( void ) = 0;

					// called with every SDL event by Sys_GetEvent()
					// returns true if ImGui has handled the event (so it shouldn't be handled by the engine)
	virtual bool	ProcessEvent( const void *sdlEvent ) = 0;

					// returns true if the system cursor should be shown because an ImGui menu is active
	virtual bool	ShouldShowCursor( void ) = 0;

					// NewFrame() is called once per D3 frame, after all events have been gotten
					// => ProcessEvent() has already been called (probably multiple times)
	virtual void	NewFrame( void ) = 0;

					// called at the end of the D3 frame, when all other D3 rendering is done
					// renders ImGui menus then
	virtual void	EndFrame( void ) = 0;
};

extern idImGui *					imgui;


#ifndef IMGUI_DISABLE
class idImGuiLocal : public idImGui {
public:
					idImGuiLocal();

	virtual bool	Init( void *sdlWindow, void *sdlGlContext );
	virtual void	Shutdown( void );
	virtual bool	IsEnabled( void ) { return imguiCtx != NULL; }
	virtual void	OpenWindow( idImGuiWindow win );
	virtual void	CloseWindow( idImGuiWindow win );
	virtual int		GetOpenWindowsMask( void ) { return openImguiWindows; }
	virtual bool	ProcessEvent( const void *sdlEvent );
	virtual bool	ShouldShowCursor( void );
	virtual void	NewFrame( void );
	virtual void	EndFrame( void );

public:

			// for binding keys from an ImGui-based menu: send input events to dhewm3
			// even if ImGui window has focus
	void	SetKeyBindMode( bool enable );

	float	GetDefaultDPI( void );
	float	GetDefaultScale( void );
	float	GetScale( void );
	void	SetScale( float scale );

	void	UpdateWarningOverlay( void );

			// show a red overlay-window at the center of the screen that contains
			// a warning symbol (triangle with !) and the given text
			// disappears after a few seconds or when a key is pressed or the mouse is moved
	void	ShowWarningOverlay( const char *text );

			// set the overall style for ImGui: Both shape (sizes, roundings, etc) and colors
	void	SetImGuiStyle( idImGuiStyle d3style );

			// set the default dhewm3 imgui style colors
	void	SetDhewm3StyleColors( ImGuiStyle *dst = nullptr );
	void	SetUserStyleColors( void );

			// write current style settings (incl. colors) as userStyle
	bool	WriteUserStyle( void );
			
			// Gets the current style settings.
	idStr	GetUserStyleFilename( void );

	ImGuiStyle GetImGuiStyle( idImGuiStyle style );

			// copy current style to clipboard
	void	CopyCurrentStyle( bool onlyChanges );

private:
	ImGuiContext *imguiCtx;

	bool keybindModeEnabled = false;

	bool imgui_initialized = false;

	SDL_Window* sdlWindow = NULL;
	bool haveNewFrame = false;
	int openImguiWindows = 0; // or-ed enum D3ImGuiWindow values

	ImGuiStyle userStyle;

	// was there a key down or button (mouse/gamepad) down event this frame?
	// used to make the warning overlay disappear
	bool hadKeyDownEvent = false;

	idStr warningOverlayText;
	double warningOverlayStartTime = -100.0;
	ImVec2 warningOverlayStartPos;
};

extern idImGuiLocal				imguiLocal;
#else // IMGUI_DISABLE - just stub out everything
class idImGuiLocal : public idImGui {
public:
					idImGuiLocal() {}
	virtual			~idImGuiLocal() {}

	virtual bool	Init( void *sdlWindow, void *sdlGlContext ) { return false; }
	virtual void	Shutdown( void ) {}
	virtual bool	IsEnabled( void ) { return false; }
	virtual void	OpenWindow( idImGuiWindow win ) {}
	virtual void	CloseWindow( idImGuiWindow win ) {}
	virtual int		GetOpenWindowsMask( void ) { return 0; }
	virtual bool	ProcessEvent( const void *sdlEvent ) { return false; }
	virtual bool	ShouldShowCursor( void ) { return false; }
	virtual void	NewFrame( void ) {}
	virtual void	EndFrame( void ) {}
};

static idImGuiLocal			imguiLocal;
static idImGui*				imgui = &imguiLocal;	// statically pointed at an idImGuiLocal
#endif


#endif /* !__SYS_IMGUI_H__ */