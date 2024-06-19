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

namespace D3 {
	namespace ImGuiHooks {
		enum D3ImGuiWindow {
			D3_ImGuiWin_None        = 0,
			D3_ImGuiWin_Settings    = 1, // advanced dhewm3 settings menu
			D3_ImGuiWin_Demo        = 2, // ImGui demo window
			// next should be 4, then 8, etc so a bitmask can be used
		};

#ifndef IMGUI_DISABLE
		extern ImGuiContext *imguiCtx; // this is only here so IsImguiEnabled() can use it inline

		inline bool IsImguiEnabled( void ) {
			return imguiCtx != NULL;
		}

		// using void* instead of SDL_Window and SDL_GLContext to avoid dragging SDL headers into sys_imgui.h
		extern bool Init( void *sdlWindow, void *sdlGlContext);

		extern void Shutdown( void );

		extern void OpenWindow( D3ImGuiWindow win );

		extern void CloseWindow( D3ImGuiWindow win );

		// enum D3ImGuiWindow values of all currently open imgui windows or-ed together
		// (0 if none are open)
		extern int GetOpenWindowsMask( void );

		// called with every SDL event by Sys_GetEvent()
		// returns true if ImGui has handled the event (so it shouldn't be handled by D3)
		extern bool ProcessEvent( const void *sdlEvent );

		// for binding keys from an ImGui-based menu: send input events to dhewm3
		// even if ImGui window has focus
		extern void SetKeyBindMode( bool enable );

		// returns true if the system cursor should be shown because an ImGui menu is active
		extern bool ShouldShowCursor( void );

		// NewFrame() is called once per D3 frame, after all events have been gotten
		// => ProcessEvent() has already been called (probably multiple times)
		extern void NewFrame( void );

		// called at the end of the D3 frame, when all other D3 rendering is done
		// renders ImGui menus then
		extern void EndFrame( void );

		extern float GetScale( void );
		extern void SetScale( float scale );

		// show a red overlay-window at the center of the screen that contains
		// a warning symbol (triangle with !) and the given text
		// disappears after a few seconds or when a key is pressed or the mouse is moved
		extern void ShowWarningOverlay( const char *text );

		enum Style {
			Dhewm3,
			ImGui_Default,
			User
		};

		// set the overall style for ImGui: Both shape (sizes, roundings, etc) and colors
		extern void SetImGuiStyle( Style style );

		// set the default dhewm3 imgui style colors
		extern void SetDhewm3StyleColors( ImGuiStyle *dst = nullptr );
		extern void SetUserStyleColors( void );

		// write current style settings (incl. colors) as userStyle
		extern bool WriteUserStyle( void );

		// copy current style to clipboard
		extern void CopyCurrentStyle( bool onlyChanges );
#else // IMGUI_DISABLE - just stub out everything
		inline bool IsImguiEnabled( void ) {
			return false;
		}

		// using void* instead of SDL_Window and SDL_GLContext to avoid dragging SDL headers into sys_imgui.h
		inline bool Init( void *sdlWindow, void *sdlGlContext ) {
			return false;
		}

		inline void Shutdown( void ) {}
		inline bool ProcessEvent( const void *sdlEvent ) { return false; }
		inline void SetKeyBindMode( bool enable ) {}
		inline bool ShouldShowCursor( void ) { return false; }
		inline void NewFrame( void ) {}
		inline void EndFrame( void ) {}
		inline void OpenWindow( D3ImGuiWindow win ) {}
		inline void CloseWindow( D3ImGuiWindow win ) {}
		inline int GetOpenWindowsMask( void ) { return 0; }
		inline float GetScale( void ) { return 1.0f; }
		inline void SetScale( float scale ) {}
		inline void ShowWarningOverlay( const char* text ) {}
		inline bool WriteUserStyle( void ) { return false; }
#endif
	}
} //namespace D3::ImGuiHooks

#endif /* !__SYS_IMGUI_H__ */