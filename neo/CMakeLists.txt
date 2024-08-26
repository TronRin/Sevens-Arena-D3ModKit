cmake_minimum_required( VERSION 3.20 )

# Check if vcpkg exist in our source folder
if ( EXISTS "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" )
	set( CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file" )
else()
	message( FATAL_ERROR "Vcpkg wasn't not found, aborting, make sure to git clone this with all submodules!")
endif()

# Enable folder groups
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER  "" )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" )

project( MyGame CXX C)

# build type
if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE "RelWithDebInfo" )
endif()

include( CheckCXXCompilerFlag )
include( GNUInstallDirs OPTIONAL RESULT_VARIABLE GNUINSTALLDIRS )
include( GitBranch )
include( GlobbedHeaders )

option( ENGINE						"Build the main executable." ON )
option( DEDICATED					"Build the dedicated server executable." ON )
option( GAME_CODE					"Build the game code." ON )
option( TOOLS						"Build the include tools into the main executable." ON )
option( ONATIVE						"Optimize for the host CPU." OFF )
option( IMGUI						"Build with Dear ImGui integration - requires SDL2 and C++11" ON )
option( REPRODUCIBLE_BUILD			"Replace __DATE__ and __TIME__ by hardcoded values for reproducible builds." OFF )
option( LINUX_RELEASE_BINS			"Set RPATH to \$ORIGIN/libs/ for Linux binary releases." OFF )
option( HARDLINK_GAME				"Compile gamecode into executable (no game DLLs)" OFF )
option( FREETYPE					"Enable Freetype support." ON )
if( NOT MSVC ) # GCC/clang or compatible, hopefully
	option( FORCE_COLORED_OUTPUT	"Always produce ANSI-colored compiler warnings/errors (GCC/Clang only; esp. useful with ninja)." OFF )
	option( ASAN					"Enable GCC/Clang Adress Sanitizer (ASan)" OFF) # TODO: MSVC might also support this, somehow?
	option( UBSAN					"Enable GCC/Clang Undefined Behavior Sanitizer (UBSan), implies HARDLINK_GAME." OFF )

	if( UBSAN AND NOT HARDLINK_GAME )
		message( STATUS "UBSAN requires linking the gamecode into the executable, will enable HARDLINK_GAME" )
		set( HARDLINK_GAME ON )
	endif()
endif()

set( ENGINE_EXECUTABLE "engine" CACHE STRING "The name of the main executable that's build." )
set( DEDICATED_EXECUTABLE "dedicated" CACHE STRING "The name of the dedicate server executable that's build." )

set( GAME_CONFG_NAME "My Game" CACHE STRING "The name of the game (path) whe're building." )
set( GAME_CONFG_FOLDER "base" CACHE STRING "The name of the game (path) whe're building." )
set( GAME_CONFG_FILE "config.cfg" CACHE STRING "The name of the file that gets written into." )
set( GAME_CONFG_URL "https://github.com/FriskTheFallenHuman/D3ModdingKit" CACHE STRING "The url on where the game can be downloaded." )

if ( LINUX_RELEASE_BINS )
	message( STATUS "Setting RPATH to \$ORIGIN/libs/ so you can put dependencies in there" )
	set( CMAKE_SKIP_RPATH OFF CACHE BOOL "Skip RPATH" FORCE )
	set( CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE )
	set( CMAKE_BUILD_WITH_INSTALL_RPATH TRUE )
	set( CMAKE_INSTALL_RPATH "$ORIGIN/libs" )
	set( ldflags "${ldflags} -Wl,--large-address-aware,-z,origin" ) # not sure this is even needed, but shouldn't hurt
else()
	set( CMAKE_SKIP_RPATH ON CACHE BOOL "Skip RPATH" FORCE )
endif()

if( REPRODUCIBLE_BUILD )
	# don't use __DATE__ and __TIME__ macros so builds are reproducible
	add_definitions( -DID_REPRODUCIBLE_BUILD )
endif()

# Set these if we found either MFC or any UI toolkit, turn off TOOLS if we didn't found neither of theses
set( MFC_TOOLS OFF ) # (Visual Studio + SDL2 only)

# Define our cpu variable
include( CPUDefine )

# Nicely tied our thirdparty libraries here
set( sys_thirdparty_libs )

# libs
find_package( Stb REQUIRED )
include_directories( ${Stb_INCLUDE_DIR} )

find_package( OpenAL REQUIRED )
include_directories( ${OPENAL_INCLUDE_DIR} )
list( APPEND sys_thirdparty_libs ${OPENAL_LIBRARY} )

find_package( SDL2 REQUIRED )
include_directories( ${SDL2_INCLUDE_DIR} )
list( APPEND sys_thirdparty_libs SDL2::SDL2 )

find_package( GLEW REQUIRED )
include_directories( ${GLEW_INCLUDE_DIRS} )
list( APPEND sys_thirdparty_libs GLEW::GLEW )

if ( FREETYPE )
	find_package( Freetype REQUIRED )
	find_path( FREETYPE_INCLUDE_DIR "fttypes.h" PATH_SUFFIXES "freetype" )
	include_directories( ${FREETYPE_INCLUDE_DIR} )
	add_definitions( -DID_BUILD_FREETYPE )
	list( APPEND sys_thirdparty_libs Freetype::Freetype )
endif()

if( IMGUI )
	# we need C++11 for ImGui
	set ( CMAKE_CXX_STANDARD 11 )
	message( STATUS "Dear ImGui integration enabled" )
else()
	message( STATUS "Dear ImGui integration disabled" )
	add_definitions( -DIMGUI_DISABLE )
endif()

# ImGUI
if ( IMGUI )
	# we need C++11 for ImGui
	set ( CMAKE_CXX_STANDARD 11 )
	message( STATUS "Dear ImGui integration enabled" )
else()
	message( STATUS "Dear ImGui integration disabled" )
	add_definitions( -DIMGUI_DISABLE )
endif()

# Curl
find_package( CURL QUIET )
if( CURL_FOUND )
	set( ID_ENABLE_CURL ON )
	include_directories( ${CURL_INCLUDE_DIR} )
	message( STATUS "libcurl found and enabled" )
	list( APPEND sys_thirdparty_libs ${CURL_LIBRARY} )
else()
	message( WARNING "libcurl not found, server downloads won't be available (apart from that the engine will work)" )
	set( ID_ENABLE_CURL OFF )
	set( CURL_LIBRARY "" )
endif()

if( TOOLS )
	if( MSVC )
		# This is required for tools on windows.
		find_package( MFC )
		if( NOT MFC_FOUND )
			message( WARNING "MFC ('Microsoft Foundation Classes for C++') couldn't be found, but is needed for TOOLS!" )
			message( WARNING "If you're using VS2013, you'll also need the 'Multibyte MFC Library for Visual Studio 2013': https://www.microsoft.com/en-us/download/details.aspx?id=40770 (VS2015 and 2017 include that in the default MFC package)" )
		else()
			set( MFC_TOOLS ON )
		endif()
	endif()
endif()

set( D3_COMPILER_IS_CLANG FALSE )
set( D3_COMPILER_IS_GCC_OR_CLANG FALSE )

if( NOT MSVC )
	if( NOT WIN32 )
		# libbacktrace support - TODO: might work with MinGW? we don't have a crash handler for win32 though..
		include( CheckCSourceCompiles )
		set( CMAKE_REQUIRED_LIBRARIES backtrace )
		check_c_source_compiles( "#include <backtrace.h>
		int main() { backtrace_create_state(NULL, 0, NULL, NULL); return 0; }" HAVE_LIBBACKTRACE )
		unset( CMAKE_REQUIRED_LIBRARIES )

		if( HAVE_LIBBACKTRACE )
			set( sys_libs ${sys_libs} backtrace )
			add_definitions( -DD3_HAVE_LIBBACKTRACE )
			message( STATUS "Using libbacktrace" )
		else()
			message( WARNING "libbacktrace wasn't found. It's not required but recommended, because it provides useful backtraces if engine/game crashes" )
		endif()
	endif() # NOT WIN32

	# check if our SDL2 supports X11 in SDL_syswm so we can use it for DPI scaling ImGui
	set( CMAKE_REQUIRED_LIBRARIES SDL2 )
	check_c_source_compiles( "#include <SDL_syswm.h>
		int main() { SDL_SysWMinfo wmInfo = {}; wmInfo.info.x11.display = NULL; return 0; }" HAVE_SDL_X11 )
	unset( CMAKE_REQUIRED_LIBRARIES )
		
	if ( HAVE_SDL_X11 )
		message( STATUS "This SDL2 has X11 support" )
		add_definitions( -DD3_SDL_X11 )
	endif()

	# check if our SDL2 supports X11 in SDL_syswm so we can use it for DPI scaling ImGui
	set( CMAKE_REQUIRED_LIBRARIES SDL2 )
	check_c_source_compiles( "#include <SDL_syswm.h>
		int main() { SDL_SysWMinfo wmInfo = {}; wmInfo.info.x11.display = NULL; return 0; }" HAVE_SDL_X11)
	unset( CMAKE_REQUIRED_LIBRARIES )
		
	if( HAVE_SDL_X11 )
		message( STATUS "This SDL2 has X11 support" )
		add_definitions( -DD3_SDL_X11 )
	endif()

	# check if this is some kind of clang (Clang, AppleClang, whatever)
	# (convert compiler ID to lowercase so we match Clang, clang, AppleClang etc, regardless of case)
	string( TOLOWER ${CMAKE_CXX_COMPILER_ID} compiler_id_lower )
	if( compiler_id_lower MATCHES ".*clang.*" )
		message( STATUS "Compiler \"${CMAKE_CXX_COMPILER_ID}\" detected as some kind of clang" )
		set( D3_COMPILER_IS_CLANG TRUE )
		set( D3_COMPILER_IS_GCC_OR_CLANG TRUE )
	elseif( CMAKE_COMPILER_IS_GNUCC )
		set( D3_COMPILER_IS_GCC_OR_CLANG TRUE )
	endif()
	unset( compiler_id_lower )
endif() # not MSVC

# compiler specific flags
if( D3_COMPILER_IS_GCC_OR_CLANG )
	add_compile_options( -pipe )
	add_compile_options( -Wall )

	if( NOT CMAKE_CROSSCOMPILING AND ONATIVE )
		add_compile_options( -march=native)
	elseif( NOT APPLE AND cpu STREQUAL "x86" )
		add_compile_options( -march=pentium3 )
	endif()

	if( FORCE_COLORED_OUTPUT)
		if( CMAKE_COMPILER_IS_GNUCC )
		   add_compile_options ( -fdiagnostics-color=always )
		elseif ( D3_COMPILER_IS_CLANG )
		   add_compile_options ( -fcolor-diagnostics )
		endif ()
	endif ()

	set( CMAKE_C_FLAGS_DEBUG "-g -ggdb -D_DEBUG -O0" )
	set( CMAKE_C_FLAGS_DEBUGALL "-g -ggdb -D_DEBUG" )
	set( CMAKE_C_FLAGS_PROFILE "-g -ggdb -D_DEBUG -O1 -fno-omit-frame-pointer" )
	set( CMAKE_C_FLAGS_RELEASE "-O2 -fno-math-errno -fno-trapping-math  -ffinite-math-only -fomit-frame-pointer" )
	set( CMAKE_C_FLAGS_RELWITHDEBINFO "-g -ggdb -O2 -fno-math-errno -fno-trapping-math  -ffinite-math-only -fno-omit-frame-pointer" )
	set( CMAKE_C_FLAGS_MINSIZEREL "-Os -fno-math-errno -fno-trapping-math  -ffinite-math-only -fomit-frame-pointer" )

	set( CMAKE_CXX_FLAGS_DEBUGALL ${CMAKE_C_FLAGS_DEBUGALL} )
	set( CMAKE_CXX_FLAGS_PROFILE ${CMAKE_C_FLAGS_PROFILE} )

	add_compile_options(-fno-strict-aliasing)

	# dear idiot compilers, don't fuck up math code with useless FMA "optimizations"
	# (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100839)
	CHECK_CXX_COMPILER_FLAG("-ffp-contract=off" cxx_has_fp-contract)
	if(cxx_has_fp-contract)
		add_compile_options(-ffp-contract=off)
	endif()
	
	if( ASAN )
		# if this doesn't work, ASan might not be available on your platform, don't set ASAN then..
		add_compile_options( -fsanitize=address )
		set( sys_libs ${sys_libs} -fsanitize=address )
	endif()

	if( UBSAN )
		# if this doesn't work, UBSan might not be available on your platform, don't set UBSAN then..
		add_compile_options( -fsanitize=undefined )
		set( sys_libs ${sys_libs} -fsanitize=undefined )
	endif()

	CHECK_CXX_COMPILER_FLAG( "-fvisibility=hidden" cxx_has_fvisibility )
	if( NOT cxx_has_fvisibility )
		message( FATAL_ERROR "Compiler does not support -fvisibility" )
	endif()
	add_compile_options( -fvisibility=hidden )

	# TODO fix these warnings
	add_compile_options( -Wno-sign-compare )
	add_compile_options( -Wno-switch )

	CHECK_CXX_COMPILER_FLAG( "-Woverloaded-virtual" cxx_has_Woverload_virtual )
	if( cxx_has_Woverload_virtual )
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual" )
	endif()

	# shut up about using memcpy() on classes, in the cases doom3 uses it it seems to be fine
	CHECK_CXX_COMPILER_FLAG( "-Wno-class-memaccess" cxx_has_Wno-class-memaccess )
	if( cxx_has_Wno-class-memaccess )
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess" )
	endif()

	# ignore warnings about variables named "requires" for now (in C++20 it's a keyword,
	#  but currently we don't even use C++11 features)
	CHECK_CXX_COMPILER_FLAG( "-Wno-c++20-compat" cxx_has_Wno-cpp20-compat )
	if( cxx_has_Wno-cpp20-compat )
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++20-compat" )
	endif()
	
	if ( NOT MSVC )
		find_package( OpenSSL REQUIRED )
	endif()

	if(APPLE)
		add_definitions(-DMACOS_X=1)

		if( cpu STREQUAL "x86_64" )
			add_compile_options( -arch x86_64 -mmacosx-version-min=10.7 )
			set( ldflags "${ldflags} -arch x86_64 -mmacosx-version-min=10.7" )
		elseif( cpu STREQUAL "arm64" )
			add_compile_options( -arch arm64 -mmacosx-version-min=10.7 )
			set( ldflags "${ldflags} -arch arm64 -mmacosx-version-min=10.7" )
		elseif( cpu STREQUAL "x86" )
			CHECK_CXX_COMPILER_FLAG( "-arch i386" cxx_has_arch_i386 )
			if( cxx_has_arch_i386 )
				add_compile_options( -arch i386 )
				set( ldflags "${ldflags} -arch i386" )
			endif()

			add_compile_options( -mmacosx-version-min=10.4 )
			set( ldflags "${ldflags} -mmacosx-version-min=10.4" )
		elseif( cpu STREQUAL "ppc" )
			CHECK_CXX_COMPILER_FLAG( "-arch ppc" cxx_has_arch_ppc )
			if( cxx_has_arch_ppc )
				add_compile_options( -arch ppc -mone-byte-bool )
				set( ldflags "${ldflags} -arch ppc -mone-byte-bool" )
			endif()

			add_compile_options( -mmacosx-version-min=10.4 )
			set( ldflags "${ldflags} -mmacosx-version-min=10.4" )
		else()
			message( FATAL_ERROR "Unsupported CPU architecture for OSX" )
		endif()

		set( sys_libs ${sys_libs} "OpenSSL::SSL OpenSSL::Crypto -framework Carbon -framework Cocoa -framework IOKit" )
	elseif( WIN32 )
		# use 8MB stack instead of 1MB, so big models don't overflow it with _alloca()
		set( ldflags "${ldflags} -static-libgcc -static-libstdc++ -Wl, --large-address-aware --stack,8388608" )
	elseif( os STREQUAL "linux" )
		set( sys_libs ${sys_libs} dl OpenSSL::SSL OpenSSL::Crypto rt )
	elseif( os STREQUAL "freebsd" )
		find_library( EXECINFO_LIBRARIES execinfo /usr/lib /usr/local/lib )
		set( sys_libs ${sys_libs} OpenSSL::SSL OpenSSL::Crypto rt ${EXECINFO_LIBRARIES} )
	endif()
elseif( MSVC )
	add_compile_options( /MP ) # parallel build (use all cores, or as many as configured in VS)
	add_compile_options( /W3 ) # TODO: was /W4, caused trouble with VS2019 (and/or its integrated CMake? or only HarrieVG's setup?)

	add_compile_options( /we4840 ) # treat as error when passing a class to a vararg-function (probably printf-like)
	add_compile_options( /we4306 /we4311 /we4312 /we4302 )	# treat several kinds of truncating int<->pointer conversions as errors (for more 64bit-safety)
	# ignore the following warnings:
	add_compile_options( /wd4100 ) # unreferenced formal parameter
	add_compile_options( /wd4127 ) # conditional expression is constant
	add_compile_options( /wd4244 ) # possible loss of data
	add_compile_options( /wd4245 ) # signed/unsigned mismatch
	add_compile_options( /wd4267 ) # possible loss of data
	add_compile_options( /wd4714 ) # 'function' marked as __forceinline not inlined
	add_compile_options( /wd4996 ) # 'function': was declared deprecated
	add_compile_options( /wd4068 ) # unknown pragma
	add_compile_options( /wd4458 ) # declaration of 'variable' hides class member
	add_compile_options( /wd4018 ) # 'math symbol': signed/unsigned mismatch
	add_compile_options( /wd4018 ) # 'math symbol': signed/unsigned mismatch
	add_compile_options( /wd26495 ) # variable 'variable' is uninitialized. Always initialize a member variable (type.6).
	add_compile_options( /wd4146 ) # unary minus operator applied to unsigned type, result still unsigned
	add_compile_options( /wd4805 ) # 'symbol' unsafe mix of type 'type a' and type 'type b' in operation
	add_compile_options( /wd4273 ) # 'function': inconsistent dll linkage
	add_compile_options( /wd4838 ) # conversion from 'type' to 'type' requires a narrowing conversion

	# Ignore warnings for Dedicate server build
	set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4286 /ignore:4217" )
	set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4286 /ignore:4217" )
	set( CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4286 /ignore:4217" )

	
	add_definitions( -D_ALLOW_KEYWORD_MACROS ) # because of the "#define private public" and "#define protected public" in TypeInfo.cpp
	
	set( CMAKE_C_FLAGS_DEBUG "-D_DEBUG /Od /Zi /MDd" )
	set( CMAKE_C_FLAGS_RELEASE "/Ox /Oy /MD" )
	set( CMAKE_C_FLAGS_RELWITHDEBINFO "/Ox /Oy /Zi /MD" )
	set( CMAKE_C_FLAGS_MINSIZEREL "/Ox /Oy /Os /MD" )
	
	# use 8MB stack instead of 1MB, so big models don't overflow it with _alloca()
	set( ldflags "${ldflags} /STACK:8388608" )
	
	# let the 32bit app use more than 2gb of ram
	set( ldflags "${ldflags} /LARGEADDRESSAWARE" )
	
	# Don't let the linker generate manifest files for shared dlls
	set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /MANIFEST:NO" )
else()
	message( FATAL_ERROR "Unsupported compiler" )
endif()

# Drop the binaries in the output/${os} folder
set( OUTPUT_FOLDER ${CMAKE_SOURCE_DIR}/../output/${os} )

set( CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} )

# disable assertions in release builds
set( CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG ${CMAKE_C_FLAGS_RELEASE}" )
set( CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG ${CMAKE_C_FLAGS_MINSIZEREL}" )

# mingw and msvc
if( WIN32 )
	add_definitions( -DWINVER=0x0501 )
	add_definitions( -D_WIN32_WINNT=0x0501 )

	set( sys_libs ${sys_libs}
		winmm
		iphlpapi
		wsock32
		ole32
	)
endif()

# Inform us of the current branch we're on.
get_git_branch( branch )
message( STATUS "Setting project on git branch: ${branch}" )

# Include our function to optmize sources.
include( OptimizedSources )

set( bindir		"${CMAKE_INSTALL_FULL_BINDIR}" )
set( libdir		"${CMAKE_INSTALL_FULL_LIBDIR}/${GAME_CONFG_NAME}" )
set( datadir	"${CMAKE_INSTALL_FULL_DATADIR}/${GAME_CONFG_NAME}" )

configure_file(
	"${CMAKE_SOURCE_DIR}/config.h.in"
	"${CMAKE_BINARY_DIR}/config.h"
)

if( NOT MSVC )
	message( STATUS "Building ${CMAKE_BUILD_TYPE} for ${os}-${cpu}" )
endif()

if( NOT APPLE AND NOT WIN32 )
	message( STATUS "The install target will use the following directories:" )
	message( STATUS "  Binary directory:  ${bindir}" )
	message( STATUS "  Library directory: ${libdir}" )
	message( STATUS "  Data directory:    ${datadir}" )
endif()

include_directories(${CMAKE_BINARY_DIR} PUBLIC)
include_directories(${CMAKE_SOURCE_DIR} PUBLIC)
include_directories("${CMAKE_SOURCE_DIR}/idlib" PUBLIC)

add_subdirectory(idlib)
add_subdirectory(game)
add_subdirectory(sys)