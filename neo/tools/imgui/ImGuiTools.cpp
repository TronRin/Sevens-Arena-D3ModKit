/*
===========================================================================

Doom 3 BFG Edition GPL Source Code
Copyright (C) 2016 Daniel Gibson
Copyright (C) 2022 Stephen Pridham

This file is part of the Doom 3 BFG Edition GPL Source Code ("Doom 3 BFG Edition Source Code").

Doom 3 BFG Edition Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 BFG Edition Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 BFG Edition Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 BFG Edition Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 BFG Edition Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "precompiled.h"
#pragma hdrstop

#include "ImGuiTools.h"

#include "../sys/sys_imgui.h"
#include "../idlib/CmdArgs.h"
#include "../renderer/tr_local.h"

//#include "afeditor/AfEditor.h"
#include "../tools/ligtheditor/LightEditor.h"

extern void Com_OpenCloseSettingsMenu( bool open ); // ditto

class idImGuiToolsLocal final : public idImGuiTools
{
public:
	idImGuiToolsLocal();

	virtual void	InitTool( const toolFlag_t tool, const idDict* dict, idEntity* entity );
	virtual bool	IsActive();
	virtual void	DrawToolWindows( void );
	virtual void	OpenWindow( int win );
	virtual void	CloseWindow( int win );

private:
	bool g_releaseMouse;
};

idImGuiToolsLocal	imguiToolsLocal;
idImGuiTools* imguiTools = &imguiToolsLocal;

void LightEditorInit( const idDict *dict, idEntity *ent ) {
	/*
	if( dict == NULL || ent == NULL )
	{
		return;
	}

	// NOTE: we can't access idEntity (it's just a declaration), because it should
	// be game/mod specific. but we can at least check the spawnclass from the dict.
	idassert( idStr::Icmp( dict->GetString( "spawnclass" ), "idLight" ) == 0
			  && "LightEditorInit() must only be called with light entities or NULL!" );


	ImGuiTools::LightEditor::Instance().ShowIt( true );
	imguiTools->SetReleaseToolMouse( true );

	ImGuiTools::LightEditor::ReInit( dict, ent );
	*/

	//ImGuiTools::LightEditor::Instance().Draw();
	ImGuiTools::LightEditor::Instance().ShowIt( true );

	ImGuiTools::LightEditor::ReInit( dict, ent );
}

void AFEditorInit2( const idDict *dict ) {
	//ImGuiTools::AfEditor::Instance().ShowIt( true );
	//imguiTools->SetReleaseToolMouse( true );
	common->Printf( "TODO: Implement me" );
}

idImGuiToolsLocal::idImGuiToolsLocal( void ) {
}

void idImGuiToolsLocal::InitTool( const toolFlag_t tool, const idDict *dict, idEntity *entity ) {
	if ( tool & EDITOR_LIGHT ) {
		LightEditorInit( dict, entity );
		imgui->OpenWindow( (idImGuiWindow)EDITOR_LIGHT );
	} else if( tool & EDITOR_AF ) {
		//AFEditorInit( dict );
	}
}

bool idImGuiToolsLocal::IsActive( void ) {
	return cvarSystem->GetCVarInteger( "g_editEntityMode" ) > 0 || com_editors != 0;
}

void idImGuiToolsLocal::DrawToolWindows( void ) {
	if ( com_editors & EDITOR_LIGHT ) {
		if ( ImGuiTools::LightEditor::Instance().IsShown() ) {
			ImGuiTools::LightEditor::Instance().Draw();
		}
	}
	/*
	else if( ImGuiTools::AfEditor::Instance().IsShown() )
	{
		ImGuiTools::AfEditor::Instance().Draw();
	}
	*/
}

void idImGuiToolsLocal::OpenWindow( int win ) {
	switch ( win ) {
		case EDITOR_LIGHT: {
			com_editors |= EDITOR_LIGHT;
			Com_OpenCloseSettingsMenu( true );
			break;
		}
		// TODO: other windows that need explicit opening
	}
}

void idImGuiToolsLocal::CloseWindow( int win ) {
	switch ( win ) {
		case EDITOR_LIGHT: {
			Com_OpenCloseSettingsMenu( false );
			// turn off light debug drawing in the render backend
			r_singleLight.SetInteger( -1 );
			r_showLights.SetInteger( 0 );
			break;
		}
		// TODO: other windows that need explicit closing
	}
}
