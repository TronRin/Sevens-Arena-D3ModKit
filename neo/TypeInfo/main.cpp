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

#include "sys/platform.h"
#include "sys/sys_local.h"
#include "framework/FileSystem.h"
#include "idlib/containers/StrList.h"
#include "TypeInfo/TypeInfoGen.h"

/*
==============================================================

	main

==============================================================
*/

int main( int argc, char** argv ) {
	idStr fileName, sourcePath;
	idTypeInfoGen *generator;

	idLib::common = common;
	idLib::cvarSystem = cvarSystem;
	idLib::fileSystem = fileSystem;
	idLib::sys = sys;

	idLib::Init();
	cmdSystem->Init();
	cvarSystem->Init();
	idCVar::RegisterStaticVars();
	fileSystem->Init();

	generator = new idTypeInfoGen;

	// Change the direcotry base uppon the game that's being build uppon
#ifdef _D3XP
	sourcePath = "d3xp";
	fileName = "../neo/d3xp/gamesys/GameTypeInfo";
#else
	sourcePath = "game";
	fileName = "../neo/game/gamesys/GameTypeInfo";
#endif // _D3XP

	if ( argc > 3 ) {
		for ( int i = 3; i < argc; i++ ) {
			generator->AddDefine( argv[i] );
		}
	} else {
		generator->AddDefine( "__cplusplus" );
		generator->AddDefine( "GAME_DLL" );		
#ifdef _D3XP
		generator->AddDefine( "CTF" );
		generator->AddDefine( "_D3XP" );
#endif // _D3XP
		generator->AddDefine( "ID_TYPEINFO" );
	}

	generator->CreateTypeInfo( sourcePath );
	generator->WriteTypeInfo( fileName );

	delete generator;

	fileName.Clear();
	sourcePath.Clear();

	fileSystem->Shutdown( false );
	cvarSystem->Shutdown();
	cmdSystem->Shutdown();
	idLib::ShutDown();

	return 0;
}
