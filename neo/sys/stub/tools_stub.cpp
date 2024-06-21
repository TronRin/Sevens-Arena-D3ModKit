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
#include "framework/Common.h"
#include "framework/FileSystem.h"
#include "framework/Session.h"
#include "framework/EventLoop.h"

#ifdef _WIN32
	#include <io.h>
	#include <direct.h>
#else
	#include <errno.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <sys/mman.h>
	#include <sys/time.h>
	#include <pwd.h>
	#include <dlfcn.h>
	#include <termios.h>
	#include <signal.h>
	#include <fcntl.h>
#endif // _WIN32

/*
==============================================================

	Misc

==============================================================
*/

idSession* session = NULL;
idDeclManager* declManager = NULL;
idEventLoop* eventLoop = NULL;

int idEventLoop::JournalLevel(void) const { return 0; }

/*
==============================================================

	idSys

==============================================================
*/

void	Sys_Mkdir( const char *path ) {}
ID_TIME_T	Sys_FileTimeStamp( FILE *fp ) { return 0; }

#ifdef _WIN32
/*
================
Sys_Cwd
================
*/
const char *Sys_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	_getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

/*
================
Sys_GetPath
================
*/
bool Sys_GetPath( sysPath_t type, idStr &path ) {
	switch(type) {
	case PATH_BASE:
		path = Sys_Cwd();
		return true;

	case PATH_CONFIG:
	case PATH_SAVE:
		path = cvarSystem->GetCVarString( "fs_basepath" );
		return true;

	case PATH_EXE:
		return false;
	}

	return false;
}

/*
================
Sys_ListFiles
================
*/
int Sys_ListFiles( const char *directory, const char *extension, idStrList &list ) {
	idStr		search;
	struct _finddata_t findinfo;
	intptr_t	findhandle;
	int			flag;

	if ( !extension) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	sprintf( search, "%s\\*%s", directory, extension );

	// search
	list.Clear();

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		return -1;
	}

	do {
		if ( flag ^ ( findinfo.attrib & _A_SUBDIR ) ) {
			list.Append( findinfo.name );
		}
	} while ( _findnext( findhandle, &findinfo ) != -1 );

	_findclose( findhandle );

	return list.Num();
}

#else
/*
================
Posix_Cwd
================
*/
const char *Posix_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	if (getcwd( cwd, sizeof( cwd ) - 1 ))
		cwd[MAX_OSPATH-1] = 0;
	else
		cwd[0] = 0;

	return cwd;
}

/*
================
Sys_GetPath
================
*/
bool Sys_GetPath( sysPath_t type, idStr &path ) {
	switch(type) {
	case PATH_BASE:
		path = Posix_Cwd();
		return true;

	case PATH_CONFIG:
	case PATH_SAVE:
		path = cvarSystem->GetCVarString( "fs_basepath" );
		return true;

	case PATH_EXE:
		return false;
	}

	return false;
}

/*
================
Sys_ListFiles
================
*/
int Sys_ListFiles( const char *directory, const char *extension, idStrList &list ) {
	struct dirent *d;
	DIR *fdir;
	bool dironly = false;
	char search[MAX_OSPATH];
	struct stat st;
	bool debug;

	list.Clear();

	debug = cvarSystem->GetCVarBool( "fs_debug" );

	if (!extension)
		extension = "";

	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == 0) {
		extension = "";
		dironly = true;
	}

	// search
	// NOTE: case sensitivity of directory path can screw us up here
	if ((fdir = opendir(directory)) == NULL) {
		if (debug) {
			common->Printf("Sys_ListFiles: opendir %s failed\n", directory);
		}
		return -1;
	}

	while ((d = readdir(fdir)) != NULL) {
		idStr::snPrintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if (!dironly) {
			idStr look(search);
			idStr ext;
			look.ExtractFileExtension(ext);
			if (extension[0] != '\0' && ext.Icmp(&extension[1]) != 0) {
				continue;
			}
		}
		if ((dironly && !(st.st_mode & S_IFDIR)) ||
			(!dironly && (st.st_mode & S_IFDIR)))
			continue;

		list.Append(d->d_name);
	}

	closedir(fdir);

	if ( debug ) {
		common->Printf( "Sys_ListFiles: %d entries in %s\n", list.Num(), directory );
	}

	return list.Num();
}

#endif

void	Sys_CreateThread( xthread_t function, void *parms, xthreadInfo &info, const char *name ) {}
void	Sys_DestroyThread( xthreadInfo& info ) {}

void	Sys_EnterCriticalSection( int index ) {}
void	Sys_LeaveCriticalSection( int index ) {}

void	Sys_WaitForEvent( int index ) {}
void	Sys_TriggerEvent( int index ) {}

/*
==============================================================

	idCommon

==============================================================
*/

#define STDIO_PRINT( pre, post )	\
	va_list argptr;					\
	va_start( argptr, fmt );		\
	printf( pre );					\
	vprintf( fmt, argptr );			\
	printf( post );					\
	va_end( argptr )

class idCommonLocal : public idCommon {
public:
							idCommonLocal( void ) {}

	virtual void			Init( int argc, char **argv ) {}
	virtual void			Shutdown( void ) {}
	virtual void			Quit( void ) {}
	virtual bool			IsInitialized( void ) const { return true; }
	virtual void			Frame( void ) {}
	virtual void			GUIFrame( bool execCmd, bool network  ) {}
	virtual void			StartupVariable( const char *match, bool once ) {}
	virtual void			InitTool( const toolFlag_t tool, const idDict *dict ) {}
	virtual void			ActivateTool( bool active ) {}
	virtual void			WriteConfigToFile( const char *filename ) {}
	virtual void			WriteFlaggedCVarsToFile( const char *filename, int flags, const char *setCmd ) {}
	virtual void			BeginRedirect( char *buffer, int buffersize, void (*flush)( const char * ) ) {}
	virtual void			EndRedirect( void ) {}
	virtual void			SetRefreshOnPrint( bool set ) {}
	virtual void			Printf( const char *fmt, ... ) { STDIO_PRINT( "", "" ); }
	virtual void			VPrintf( const char *fmt, va_list arg ) { vprintf( fmt, arg ); }
	virtual void			DPrintf( const char *fmt, ... ) { /*STDIO_PRINT( "", "" );*/ }
	virtual void			VerbosePrintf( const char* fmt, ...) { STDIO_PRINT( "", "" ); }
	virtual void			Warning( const char *fmt, ... ) { STDIO_PRINT( "WARNING: ", "\n" ); }
	virtual void			DWarning( const char *fmt, ...) { /*STDIO_PRINT( "WARNING: ", "\n" );*/ }
	virtual void			PrintWarnings( void ) {}
	virtual void			ClearWarnings( const char *reason ) {}
	virtual void			Error( const char *fmt, ... ) { STDIO_PRINT( "ERROR: ", "\n" ); exit(0); }
	virtual void			FatalError( const char *fmt, ... ) { STDIO_PRINT( "FATAL ERROR: ", "\n" ); exit(0); }
	virtual const idLangDict *GetLanguageDict() { return NULL; }
	virtual const char *	KeysFromBinding( const char *bind ) { return NULL; }
	virtual const char *	BindingFromKey( const char *key ) { return NULL; }
	virtual int				ButtonState( int key ) { return 0; }
	virtual int				KeyState( int key ) { return 0; }
	virtual bool			SetCallback( idCommon::CallbackType cbt, idCommon::FunctionPointer cb, void* userArg ) { return true; }
	virtual bool			GetAdditionalFunction( idCommon::FunctionType ft, idCommon::FunctionPointer* out_fnptr, void** out_userArg ) { return true; }
	virtual float			Get_com_engineHz_latched( void ) { return 1.0f; }
	virtual int64_t			Get_com_engineHz_numerator( void ) { return NULL; }
	virtual int64_t			Get_com_engineHz_denominator( void ) { return NULL; }
};

idCVar com_developer( "developer", "0", CVAR_BOOL|CVAR_SYSTEM, "developer mode" );

idCommonLocal		commonLocal;
idCommon *			common = &commonLocal;


/*
==============
idSysLocal stub
==============
*/
void			idSysLocal::DebugPrintf( const char *fmt, ... ) {}
void			idSysLocal::DebugVPrintf( const char *fmt, va_list arg ) {}

unsigned int	idSysLocal::GetMilliseconds(void) { return 0; }

int				idSysLocal::GetProcessorId( void ) { return 0; }
void			idSysLocal::FPU_SetFTZ( bool enable ) {}
void			idSysLocal::FPU_SetDAZ( bool enable ) {}

bool			idSysLocal::LockMemory( void *ptr, int bytes ) { return false; }
bool			idSysLocal::UnlockMemory( void *ptr, int bytes ) { return false; }

uintptr_t		idSysLocal::DLL_Load( const char *dllName ) { return 0; }
void *			idSysLocal::DLL_GetProcAddress( uintptr_t dllHandle, const char *procName ) { return NULL; }
void			idSysLocal::DLL_Unload( uintptr_t dllHandle ) { }
void			idSysLocal::DLL_GetFileName( const char *baseName, char *dllName, int maxLength ) { }

sysEvent_t		idSysLocal::GenerateMouseButtonEvent( int button, bool down ) { sysEvent_t ev; memset( &ev, 0, sizeof( ev ) ); return ev; }
sysEvent_t		idSysLocal::GenerateMouseMoveEvent( int deltax, int deltay ) { sysEvent_t ev; memset( &ev, 0, sizeof( ev ) ); return ev; }

void			idSysLocal::OpenURL( const char *url, bool quit ) { }
void			idSysLocal::StartProcess( const char *exeName, bool quit ) { }

bool			idSysLocal::IsGameWindowVisible( void ) { return false; }

idSysLocal		sysLocal;
idSys *			sys = &sysLocal;