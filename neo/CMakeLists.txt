cmake_minimum_required( VERSION 3.27.8 )

# Check if vcpkg exist in our source folder
if ( EXISTS "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" )
	set( CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file" )
else()
	message( FATAL_ERROR "Vcpkg wasn't not found, aborting, make sure to git clone this with all submodules!")
endif()

# Drop the binaries in the output/ folder
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../output" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/../output" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/../output" )

# Enable folder groups
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER  "" )
set_property( GLOBAL PROPERTY USE_FOLDERS ON )

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" )

project( MyGame )

# build type
if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE "RelWithDebInfo" )
endif()

include( CheckCXXCompilerFlag )
include( GNUInstallDirs OPTIONAL RESULT_VARIABLE GNUINSTALLDIRS )
include( GitBranch )
include( GlobbedHeaders )

option( ENGINE						"Build the main executable." ON )
option( GAME_CODE					"Build the game code." ON )
option( TOOLS						"Build the tools game code." ON )
option( TYPEINFOGEN					"Build the TypeInfoGen tool (useful for C++ only mods/games)" ON )
option( DEDICATED					"Build the dedicated server." ON ) # OFF
option( ONATIVE						"Optimize for the host CPU." OFF )
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

if ( LINUX_RELEASE_BINS )
	message( STATUS "Setting RPATH to \$ORIGIN/libs/ so you can put dependencies in there" )
	set( CMAKE_SKIP_RPATH OFF CACHE BOOL "Skip RPATH" FORCE )
	set( CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE )
	set( CMAKE_BUILD_WITH_INSTALL_RPATH TRUE )
	set( CMAKE_INSTALL_RPATH "$ORIGIN/libs" )
	set( ldflags "${ldflags} -Wl,-z,origin" ) # not sure this is even needed, but shouldn't hurt
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

# libs
find_package( Stb REQUIRED )
include_directories( ${Stb_INCLUDE_DIR} )

find_package( ZLIB REQUIRED )
include_directories( ${ZLIB_INCLUDE_DIRS} )

find_package( OpenAL REQUIRED )
include_directories( ${OPENAL_INCLUDE_DIR} )

find_package( SDL2 REQUIRED )
include_directories( ${SDL2_INCLUDE_DIR} )
set( SDL2_LIBRARY SDL2::SDL2 )

if ( FREETYPE )
	find_package(Freetype REQUIRED)
	find_path( FREETYPE_INCLUDE_DIR "fttypes.h" PATH_SUFFIXES "freetype" )
	include_directories( ${FREETYPE_INCLUDE_DIR} )
	add_definitions( -DID_BUILD_FREETYPE )
endif()

# Curl
find_package( CURL QUIET )
if( CURL_FOUND )
	set( ID_ENABLE_CURL ON )
	include_directories( ${CURL_INCLUDE_DIR} )
	message( STATUS "libcurl found and enabled" )
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
	#if( NOT WIN32 )
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
	#endif() # NOT WIN32

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
	set( CMAKE_C_FLAGS_RELEASE "-O2 -fno-math-errno -fno-trapping-math -fomit-frame-pointer" )
	set( CMAKE_C_FLAGS_RELWITHDEBINFO "-g -ggdb -O2 -fno-math-errno -fno-trapping-math -fno-omit-frame-pointer" )
	set( CMAKE_C_FLAGS_MINSIZEREL "-Os -fno-math-errno -fno-trapping-math -fomit-frame-pointer" )

	set( CMAKE_CXX_FLAGS_DEBUGALL ${CMAKE_C_FLAGS_DEBUGALL} )
	set( CMAKE_CXX_FLAGS_PROFILE ${CMAKE_C_FLAGS_PROFILE} )

	add_compile_options(-fno-strict-aliasing)

	# dear idiot compilers, don't fuck up math code with useless FMA "optimizations"
	# (https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100839)
	add_compile_options( -ffp-contract=off )
	
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

	if( NOT AROS )
		CHECK_CXX_COMPILER_FLAG( "-fvisibility=hidden" cxx_has_fvisibility )
		if( NOT cxx_has_fvisibility )
			message( FATAL_ERROR "Compiler does not support -fvisibility" )
		endif()
		add_compile_options( -fvisibility=hidden )
	endif()

	# TODO fix these warnings
	add_compile_options( -Wno-sign-compare )
	add_compile_options( -Wno-switch )

	CHECK_CXX_COMPILER_FLAG( "-Woverloaded-virtual" cxx_has_Woverload_virtual )
	if( cxx_has_Woverload_virtual )
		add_compile_options( -Woverloaded-virtual )
	endif()
	
	# shut up about using memcpy() on classes, in the cases doom3 uses it it seems to be fine
	CHECK_CXX_COMPILER_FLAG( "-Wno-class-memaccess" cxx_has_Wno-class-memaccess )
	if( cxx_has_Wno-class-memaccess )
		add_compile_options( -Wno-class-memaccess )
	endif()

	if( AROS )
		set( CMAKE_SHARED_LIBRARY_SUFFIX ".aros-${cpu}" )
		add_definitions( -DIOAPI_NO_64 )
	elseif( APPLE )
		add_definitions( -DMACOS_X=1 )

		if( cpu STREQUAL "x86_64" )
			add_compile_options( -arch x86_64 -mmacosx-version-min=10.9 )
			set( ldflags "${ldflags} -arch x86_64 -mmacosx-version-min=10.9" )
		elseif( cpu STREQUAL "arm64" )
			add_compile_options( -arch arm64 -mmacosx-version-min=11.0 )
			set( ldflags "${ldflags} -arch arm64 -mmacosx-version-min=11.0" )
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

		set( sys_libs ${sys_libs} "-framework Carbon -framework Cocoa -framework IOKit" )
	elseif( WIN32 )
		# use 8MB stack instead of 1MB, so big models don't overflow it with _alloca()
		set( ldflags "${ldflags} -static-libgcc -static-libstdc++ -Wl,--stack,8388608" )
	elseif( os STREQUAL "linux" )
		set( sys_libs ${sys_libs} dl )
	elseif( os STREQUAL "freebsd" )
		find_library( EXECINFO_LIBRARIES execinfo /usr/lib /usr/local/lib )
		set( sys_libs ${sys_libs} ${EXECINFO_LIBRARIES} )
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
	
	add_definitions( -D_ALLOW_KEYWORD_MACROS ) # because of the "#define private public" and "#define protected public" in TypeInfo.cpp
	
	set( CMAKE_C_FLAGS_DEBUG "-D_DEBUG /Od /Zi /MDd" )
	set( CMAKE_C_FLAGS_RELEASE "/Ox /Oy /MD" )
	set( CMAKE_C_FLAGS_RELWITHDEBINFO "/Ox /Oy /Zi /MD" )
	set( CMAKE_C_FLAGS_MINSIZEREL "/Ox /Oy /Os /MD" )
	
	# use 8MB stack instead of 1MB, so big models don't overflow it with _alloca()
	set( ldflags "${ldflags} /STACK:8388608" )
else()
	message( FATAL_ERROR "Unsupported compiler" )
endif()

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

set( bindir		"${CMAKE_INSTALL_FULL_BINDIR}" )
set( libdir		"${CMAKE_INSTALL_FULL_LIBDIR}/dhewm3" )
set( datadir	"${CMAKE_INSTALL_FULL_DATADIR}/dhewm3" )

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

set(src_renderer
	renderer/Cinematic.cpp
	renderer/GuiModel.cpp
	renderer/Image_files.cpp
	renderer/Image_init.cpp
	renderer/Image_load.cpp
	renderer/Image_process.cpp
	renderer/Image_program.cpp
	renderer/Interaction.cpp
	renderer/Material.cpp
	renderer/MegaTexture.cpp
	renderer/Model.cpp
	renderer/ModelDecal.cpp
	renderer/ModelManager.cpp
	renderer/ModelOverlay.cpp
	renderer/Model_beam.cpp
	renderer/Model_ase.cpp
	renderer/Model_liquid.cpp
	renderer/Model_lwo.cpp
	renderer/Model_ma.cpp
	renderer/Model_md3.cpp
	renderer/Model_md5.cpp
	renderer/Model_prt.cpp
	renderer/Model_sprite.cpp
	renderer/RenderEntity.cpp
	renderer/RenderSystem.cpp
	renderer/RenderSystem_init.cpp
	renderer/RenderWorld.cpp
	renderer/RenderWorld_demo.cpp
	renderer/RenderWorld_load.cpp
	renderer/RenderWorld_portals.cpp
	renderer/VertexCache.cpp
	renderer/draw_arb2.cpp
	renderer/draw_common.cpp
	renderer/tr_backend.cpp
	renderer/tr_deform.cpp
	renderer/tr_font.cpp
	renderer/tr_guisurf.cpp
	renderer/tr_light.cpp
	renderer/tr_lightrun.cpp
	renderer/tr_main.cpp
	renderer/tr_orderIndexes.cpp
	renderer/tr_polytope.cpp
	renderer/tr_render.cpp
	renderer/tr_rendertools.cpp
	renderer/tr_shadowbounds.cpp
	renderer/tr_stencilshadow.cpp
	renderer/tr_subview.cpp
	renderer/tr_trace.cpp
	renderer/tr_trisurf.cpp
	renderer/tr_turboshadow.cpp
)

add_globbed_headers(src_renderer "renderer")

set(src_framework
	framework/CVarSystem.cpp
	framework/CmdSystem.cpp
	framework/Common.cpp
	framework/Compressor.cpp
	framework/Console.cpp
	framework/DemoFile.cpp
	framework/DeclAF.cpp
	framework/DeclEntityDef.cpp
	framework/DeclFX.cpp
	framework/DeclManager.cpp
	framework/DeclParticle.cpp
	framework/DeclPDA.cpp
	framework/DeclSkin.cpp
	framework/DeclTable.cpp
	framework/EditField.cpp
	framework/EventLoop.cpp
	framework/File.cpp
	framework/FileSystem.cpp
	framework/KeyInput.cpp
	framework/UsercmdGen.cpp
	framework/Session_menu.cpp
	framework/Session.cpp
	framework/async/AsyncClient.cpp
	framework/async/AsyncNetwork.cpp
	framework/async/AsyncServer.cpp
	framework/async/MsgChannel.cpp
	framework/async/NetworkSystem.cpp
	framework/async/ServerScan.cpp
	framework/minizip/ioapi.c
	framework/minizip/unzip.cpp
)

add_globbed_headers(src_framework "framework")

set(src_cm
	cm/CollisionModel_contacts.cpp
	cm/CollisionModel_contents.cpp
	cm/CollisionModel_debug.cpp
	cm/CollisionModel_files.cpp
	cm/CollisionModel_load.cpp
	cm/CollisionModel_rotate.cpp
	cm/CollisionModel_trace.cpp
	cm/CollisionModel_translate.cpp
)

add_globbed_headers(src_cm "cm")

set(src_dmap
	tools/compilers/dmap/dmap.cpp
	tools/compilers/dmap/facebsp.cpp
	tools/compilers/dmap/leakfile.cpp
	tools/compilers/dmap/map.cpp
	tools/compilers/dmap/optimize.cpp
	tools/compilers/dmap/output.cpp
	tools/compilers/dmap/portals.cpp
	tools/compilers/dmap/shadowopt3.cpp
	tools/compilers/dmap/tritjunction.cpp
	tools/compilers/dmap/tritools.cpp
	tools/compilers/dmap/ubrush.cpp
	tools/compilers/dmap/usurface.cpp
)

add_globbed_headers(src_dmap "tools/compilers/dmap")

set(src_aas
	tools/compilers/aas/AASBuild.cpp
	tools/compilers/aas/AASBuild_file.cpp
	tools/compilers/aas/AASBuild_gravity.cpp
	tools/compilers/aas/AASBuild_ledge.cpp
	tools/compilers/aas/AASBuild_merge.cpp
	tools/compilers/aas/AASCluster.cpp
	tools/compilers/aas/AASFile.cpp
	tools/compilers/aas/AASFile_optimize.cpp
	tools/compilers/aas/AASFile_sample.cpp
	tools/compilers/aas/AASReach.cpp
	tools/compilers/aas/AASFileManager.cpp
	tools/compilers/aas/Brush.cpp
	tools/compilers/aas/BrushBSP.cpp
)

add_globbed_headers(src_aas "tools/compilers/aas")

set(src_roq
	tools/compilers/roqvq/NSBitmapImageRep.cpp
	tools/compilers/roqvq/codec.cpp
	tools/compilers/roqvq/roq.cpp
	tools/compilers/roqvq/roqParam.cpp
)

add_globbed_headers(src_roq "tools/compilers/roqvq")

set(src_renderbump
	tools/compilers/renderbump/renderbump.cpp
)

add_globbed_headers(src_renderbump "tools/compilers/renderbump")

set(src_matbuild
	tools/compilers/matbuild/matbuild.cpp
)

add_globbed_headers(src_matbuild "tools/compilers/matbuild")

set(src_snd
	sound/snd_cache.cpp
	sound/snd_decoder.cpp
	sound/snd_efxfile.cpp
	sound/snd_emitter.cpp
	sound/snd_shader.cpp
	sound/snd_system.cpp
	sound/snd_wavefile.cpp
	sound/snd_world.cpp
)

add_globbed_headers(src_snd "sound")

set(src_ui
	ui/BindWindow.cpp
	ui/ChoiceWindow.cpp
	ui/DeviceContext.cpp
	ui/EditWindow.cpp
	ui/FieldWindow.cpp
	ui/GameBearShootWindow.cpp
	ui/GameBustOutWindow.cpp
	ui/GameSSDWindow.cpp
	ui/GuiScript.cpp
	ui/ListGUI.cpp
	ui/ListWindow.cpp
	ui/MarkerWindow.cpp
	ui/RegExp.cpp
	ui/RenderWindow.cpp
	ui/SimpleWindow.cpp
	ui/SliderWindow.cpp
	ui/UserInterface.cpp
	ui/Window.cpp
	ui/Winvar.cpp
)

add_globbed_headers(src_ui "ui")

set(src_idlib
	idlib/bv/Bounds.cpp
	idlib/bv/Frustum.cpp
	idlib/bv/Sphere.cpp
	idlib/bv/Box.cpp
	idlib/geometry/DrawVert.cpp
	idlib/geometry/Winding2D.cpp
	idlib/geometry/Surface_SweptSpline.cpp
	idlib/geometry/Winding.cpp
	idlib/geometry/Surface.cpp
	idlib/geometry/Surface_Patch.cpp
	idlib/geometry/TraceModel.cpp
	idlib/geometry/JointTransform.cpp
	idlib/hashing/CRC32.cpp
	idlib/hashing/MD4.cpp
	idlib/hashing/MD5.cpp
	idlib/math/Angles.cpp
	idlib/math/Lcp.cpp
	idlib/math/Math.cpp
	idlib/math/Matrix.cpp
	idlib/math/Ode.cpp
	idlib/math/Plane.cpp
	idlib/math/Pluecker.cpp
	idlib/math/Polynomial.cpp
	idlib/math/Quat.cpp
	idlib/math/Rotation.cpp
	idlib/math/Simd.cpp
	idlib/math/Simd_Generic.cpp
	idlib/math/Simd_AltiVec.cpp
	idlib/math/Simd_MMX.cpp
	idlib/math/Simd_3DNow.cpp
	idlib/math/Simd_SSE.cpp
	idlib/math/Simd_SSE2.cpp
	idlib/math/Simd_SSE3.cpp
	idlib/math/Vector.cpp
	idlib/BitMsg.cpp
	idlib/LangDict.cpp
	idlib/Lexer.cpp
	idlib/Lib.cpp
	idlib/containers/HashIndex.cpp
	idlib/Dict.cpp
	idlib/Str.cpp
	idlib/Parser.cpp
	idlib/MapFile.cpp
	idlib/CmdArgs.cpp
	idlib/Token.cpp
	idlib/Base64.cpp
	idlib/Timer.cpp
	idlib/Heap.cpp
)

add_globbed_headers(src_idlib "idlib")

# RBMIKKT_TANGENT...

set(src_mikktspace
	libs/mikktspace/mikktspace.c
)

add_globbed_headers(src_mikktspace "mikktspace")

# ...RBMIKKT_TANGENT

set(src_game
	game/AF.cpp
	game/AFEntity.cpp
	game/Actor.cpp
	game/Camera.cpp
	game/Entity.cpp
	game/BrittleFracture.cpp
	game/Fx.cpp
	game/GameEdit.cpp
	game/Game_local.cpp
	game/Game_network.cpp
	game/Item.cpp
	game/IK.cpp
	game/Light.cpp
	game/Misc.cpp
	game/Mover.cpp
	game/Moveable.cpp
	game/MultiplayerGame.cpp
	game/Player.cpp
	game/PlayerIcon.cpp
	game/PlayerView.cpp
	game/Projectile.cpp
	game/Pvs.cpp
	game/SecurityCamera.cpp
	game/SmokeParticles.cpp
	game/Sound.cpp
	game/Target.cpp
	game/Trigger.cpp
	game/Weapon.cpp
	game/WorldSpawn.cpp
	game/ai/AAS.cpp
	game/ai/AAS_debug.cpp
	game/ai/AAS_pathing.cpp
	game/ai/AAS_routing.cpp
	game/ai/AI.cpp
	game/ai/AI_events.cpp
	game/ai/AI_pathing.cpp
	game/ai/AI_Vagary.cpp
	game/gamesys/DebugGraph.cpp
	game/gamesys/Class.cpp
	game/gamesys/Event.cpp
	game/gamesys/SaveGame.cpp
	game/gamesys/SysCmds.cpp
	game/gamesys/SysCvar.cpp
	game/gamesys/TypeInfo.cpp
	game/anim/Anim.cpp
	game/anim/Anim_Blend.cpp
	game/anim/Anim_Import.cpp
	game/anim/Anim_Testmodel.cpp
	game/script/Script_Compiler.cpp
	game/script/Script_Interpreter.cpp
	game/script/Script_Program.cpp
	game/script/Script_Thread.cpp
	game/physics/Clip.cpp
	game/physics/Force.cpp
	game/physics/Force_Constant.cpp
	game/physics/Force_Drag.cpp
	game/physics/Force_Field.cpp
	game/physics/Force_Spring.cpp
	game/physics/Physics.cpp
	game/physics/Physics_AF.cpp
	game/physics/Physics_Actor.cpp
	game/physics/Physics_Base.cpp
	game/physics/Physics_Monster.cpp
	game/physics/Physics_Liquid.cpp
	game/physics/Physics_Parametric.cpp
	game/physics/Physics_Player.cpp
	game/physics/Physics_RigidBody.cpp
	game/physics/Physics_Static.cpp
	game/physics/Physics_StaticMulti.cpp
	game/physics/Push.cpp
	game/Grabber.cpp
	game/physics/Force_Grab.cpp
)

add_globbed_headers(src_game "game")

set(src_debuggerServer
	tools/debugger/DebuggerBreakpoint.h
	tools/debugger/DebuggerBreakpoint.cpp
	tools/debugger/DebuggerServer.h
	tools/debugger/DebuggerServer.cpp
	tools/debugger/DebuggerScript.h
	tools/debugger/DebuggerScript.cpp
	tools/debugger/DebuggerMessages.h
	tools/debugger/debugger.cpp
)

set(src_core
	${src_renderer}
	${src_framework}
	${src_cm}
	${src_dmap}
	${src_aas}
	${src_roq}
	${src_renderbump}
	${src_matbuild}
	${src_snd}
	${src_ui}
	${src_tools}
)

set(src_stub_openal		sys/stub/openal_stub.cpp)
set(src_stub_gl			sys/stub/stub_gl.cpp)


set(src_tools
	tools/guied/GEWindowWrapper_stub.cpp
)

# TODO: add_globbed_headers(src_tools "tools/guied")

# Begin normal (MFC) tools code
if (TOOLS AND MFC_TOOLS AND MSVC)
	set(ALLOWRESOURCES ON)
	# Common files.
	file(GLOB src_comafx "tools/comafx/*.cpp")
	add_globbed_headers(src_comafx "tools/comafx")
	file(GLOB_RECURSE src_tools_common "tools/common/*.cpp")
	add_globbed_headers(src_tools_common "tools/common")

	#Compilers.
	# DG: this is always added, why add it again? file(GLOB src_compiler_renderbump "neo/tools/compilers/renderbump/*.cpp")
	file(GLOB src_compiler_roqvq	  "tools/compilers/roqvq/*.cpp") # FIXME: some of those were added before
	add_globbed_headers(src_compiler_roqvq "tools/compilers/roqvq")
	#tools
	# Articulated Frame editor.
	file(GLOB src_afeditor "tools/af/*.cpp")
	add_globbed_headers(src_afeditor "tools/af")
	# Declaration editor
	file(GLOB src_decleditor "tools/decl/*.cpp")
	add_globbed_headers(src_decleditor "tools/decl")
	# GUI Editor.
	file(GLOB src_gui_editor "tools/guied/*.cpp")
	add_globbed_headers(src_gui_editor "tools/guied")
	# Material editor
	file(GLOB src_materialeditor "tools/materialeditor/*.cpp")
	add_globbed_headers(src_materialeditor "tools/materialeditor")
	# Particle Editor
	file(GLOB src_particleeditor "tools/particle/*.cpp")
	add_globbed_headers(src_particleeditor "tools/particle")
	# PDA editor
	file(GLOB src_pdaeditor "tools/pda/*.cpp")
	add_globbed_headers(src_pdaeditor "tools/pda")
	# Radiant ( Map editor )
	file(GLOB src_map_editor "tools/radiant/*.cpp")
	add_globbed_headers(src_map_editor "tools/radiant")
	# Script editor
	file(GLOB src_script_editor "tools/script/*.cpp")
	add_globbed_headers(src_script_editor "tools/script")
	# Script Debugger
	set(src_debuggerClient
		tools/debugger/DebuggerClient.h
		tools/debugger/DebuggerClient.cpp
		tools/debugger/DebuggerApp.h
		tools/debugger/DebuggerApp.cpp
		tools/debugger/DebuggerQuickWatchDlg.h
		tools/debugger/DebuggerQuickWatchDlg.cpp
		tools/debugger/DebuggerWindow.h
		tools/debugger/DebuggerWindow.cpp
		tools/debugger/DebuggerFindDlg.h
		tools/debugger/DebuggerFindDlg.cpp
	)
	set(src_script_debugger
		${src_debuggerServer}
		${src_debuggerClient}
	)
	# sound editor?
	file(GLOB src_sound_editor "tools/sound/*.cpp")
	add_globbed_headers(src_sound_editor "tools/sound")
	

	# The numerous tools in a nice list.
	list(APPEND src_editor_tools
		${src_comafx}
		${src_tools_common}
		${src_compiler_renderbump}
		${src_afeditor}
		${src_decleditor}
		${src_gui_editor}
		${src_materialeditor}
		${src_particleeditor}
		${src_pdaeditor}
		${src_map_editor}
		${src_script_editor}
		${src_sound_editor}
		${src_script_debugger}
		"tools/edit_public.h"
		"tools/edit_gui_common.h"
		)
	SET(CMAKE_MFC_FLAG 2)
	set(TOOLS_DEFINES "ID_ALLOW_TOOLS;_AFXDLL")
else()
	set(src_editor_tools "tools/edit_stub.cpp" "tools/edit_public.h" "tools/debugger/debugger_common.h")
	list(APPEND src_editor_tools
		${src_debuggerServer}
	)
endif()

# Generate the TypeInfoGen
if (TYPEINFOGEN)
	set(src_typeinfogen 
		"TypeInfo/main.cpp"
		"TypeInfo/TypeInfoGen.cpp"
		"TypeInfo/TypeInfoGen.h"
	)

	set (src_toolsupportsrc 
		framework/CmdSystem.cpp
		framework/CVarSystem.cpp
		framework/File.cpp
		framework/FileSystem.cpp
		framework/minizip/ioapi.c
		framework/minizip/ioapi.h
		framework/minizip/MiniZip64_info.txt
		framework/minizip/unzip.cpp
		framework/minizip/unzip.h
	)
	
	set (src_enginestub 
		tools/tool_idcommon_stub.cpp
	)
endif()

if(AROS)
	set(sys_libs ${sys_libs} dll)

	set(src_arosdll
		sys/aros/dll/dllstartup.c
		sys/aros/dll/dll.c
		sys/aros/dll/dllimport.c
	)

	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/events.cpp
		sys/sys_local.cpp
		sys/aros/aros_net.cpp
		sys/aros/aros_signal.cpp
		sys/aros/aros_main.cpp
		sys/aros/aros_dos.cpp
	)

	set(src_sys_core
		sys/glimp.cpp
	)
elseif(APPLE)
	set(OSX_RESOURCE_FILES
		"${CMAKE_SOURCE_DIR}/sys/osx/Doom3.icns"
		"${CMAKE_SOURCE_DIR}/sys/osx/Doom 3.rsrc"
	)

	set_source_files_properties(${OSX_RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/events.cpp
		sys/sys_local.cpp
		sys/posix/posix_net.cpp
		sys/posix/posix_main.cpp
	)

	set(src_sys_core
		sys/glimp.cpp
		sys/osx/DOOMController.mm
		sys/osx/macosx_misc.mm
		sys/osx/SDLMain.m
		${OSX_RESOURCE_FILES}
	)
elseif(WIN32)

	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/events.cpp
		sys/sys_local.cpp
		sys/win32/win_input.cpp
		sys/win32/win_main.cpp
		sys/win32/win_net.cpp
		sys/win32/win_shared.cpp
		sys/win32/win_syscon.cpp
		sys/win32/SDL_win32_main.c
	)

	# adding the few relevant headers in sys/ manually..
	set(src_sys_base ${src_sys_base}
		sys/platform.h
		sys/sys_local.h
		sys/sys_public.h
		sys/win32/win_local.h
	)

	set(src_sys_core
		sys/glimp.cpp
	)

	if(TOOLS)
		file(GLOB src_sys_tools_images "sys/win32/rc/res/*.bmp" "sys/win32/rc/res/*.ico" "sys/win32/rc/res/*.cur")
		set(src_sys_core ${src_sys_core} ${src_sys_tools_images}  "sys/win32/rc/doom.rc")
	else()
		# adding both .rc files breaks the build, but I think it only contains the icon
		# and doom.rc also contains that (+icons for editor etc, AFAIK), so this is fine
		set(src_sys_core ${src_sys_core} "${CMAKE_SOURCE_DIR}/sys/win32/rc/dhewm3.rc" )
	endif()

else()
	set(src_sys_base
		sys/cpu.cpp
		sys/threads.cpp
		sys/events.cpp
		sys/sys_local.cpp
		sys/posix/posix_net.cpp
		sys/posix/posix_main.cpp
		sys/linux/main.cpp
	)

	set(src_sys_core
		sys/glimp.cpp
	)
endif()

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_library(idlib STATIC ${src_idlib})
if (AROS)
	add_library(dll STATIC ${src_arosdll})
		if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")
		set(AROS_ARCH "x86")
		else()
		set(AROS_ARCH ${CMAKE_SYSTEM_PROCESSOR})
	endif()
else()
	if(D3_COMPILER_IS_GCC_OR_CLANG AND NOT MINGW)
		set_target_properties(idlib PROPERTIES COMPILE_FLAGS "-fPIC")
	endif()
endif()

if (MSVC)
	target_sources(idlib INTERFACE "sys/win32/natvis/idlib.natvis")
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_idlib})

set_target_properties(idlib PROPERTIES FOLDER "libs")

# RBMIKKT_TANGENT...

add_library(mikktspace STATIC ${src_mikktspace})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_mikktspace})

set_target_properties(mikktspace PROPERTIES FOLDER "libs")

# ...RBMIKKT_TANGENT

if(HARDLINK_GAME)
	if(GAME_CODE)
		set(src_core ${src_core} ${src_game})
		set(game_includes "${CMAKE_SOURCE_DIR}/game")
	endif()
endif()# HARDLINK_GAME

if(ENGINE)
	add_executable(${ENGINE_EXECUTABLE} WIN32 MACOSX_BUNDLE
		${src_core}
		${src_sys_base}
		${src_sys_core}
		${src_editor_tools}
	)
	
	if( MSVC )
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ENGINE_EXECUTABLE})
	endif()

	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_core} ${src_sys_base} ${src_sys_core} ${src_editor_tools})

if(HARDLINK_GAME)
	set_target_properties(${ENGINE_EXECUTABLE} PROPERTIES COMPILE_DEFINITIONS "${TOOLS_DEFINES}")
	target_include_directories(${ENGINE_EXECUTABLE} PRIVATE ${game_includes})
	
	# Hack to get .natvis added
	if (MSVC)
		target_sources(${ENGINE_EXECUTABLE} PRIVATE "sys/win32/natvis/game.natvis")
	endif()
else()
	set_target_properties(${ENGINE_EXECUTABLE} PROPERTIES COMPILE_DEFINITIONS "__DOOM_DLL__;${TOOLS_DEFINES}")
endif()

	if (MSVC)
		target_sources(${ENGINE_EXECUTABLE} PRIVATE "sys/win32/natvis/engine.natvis")
	endif()

	set_target_properties(${ENGINE_EXECUTABLE} PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(${ENGINE_EXECUTABLE} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/sys/osx/Info.plist)
	set_target_properties(${ENGINE_EXECUTABLE} PROPERTIES FOLDER "exes")

	target_link_libraries(${ENGINE_EXECUTABLE}
		idlib
		mikktspace # RBMIKKT_TANGENT
		$<$<BOOL:FREETYPE>:Freetype::Freetype>
		${OPENAL_LIBRARY}
		${CURL_LIBRARY}
		${ZLIB_LIBRARY}
		${SDL2_LIBRARY}
		${sys_libs}
	)

	if(NOT APPLE AND NOT WIN32)
		install(TARGETS ${ENGINE_EXECUTABLE}
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()

if(DEDICATED)
	add_executable(${DEDICATED_EXECUTABLE} WIN32 MACOSX_BUNDLE
		${src_core}
		${src_stub_openal}
		${src_stub_gl}
		${src_sys_base}
		${src_debuggerServer}
	)
	
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_core} ${src_sys_base} ${src_stub_openal} ${src_stub_gl} ${src_debuggerServer})

if(HARDLINK_GAME)
	set_target_properties(${DEDICATED_EXECUTABLE} PROPERTIES COMPILE_DEFINITIONS "ID_DEDICATED")
	target_include_directories(${DEDICATED_EXECUTABLE} PRIVATE ${game_includes})
	
	# Hack to get .natvis added
	if (MSVC)
		target_sources(${DEDICATED_EXECUTABLE} PRIVATE "sys/win32/natvis/game.natvis")
	endif()
else()
	set_target_properties(${DEDICATED_EXECUTABLE} PROPERTIES COMPILE_DEFINITIONS "ID_DEDICATED;__DOOM_DLL__")
endif()

	if (MSVC)
		target_sources(${DEDICATED_EXECUTABLE} PRIVATE "sys/win32/natvis/engine.natvis")
	endif()

	set_target_properties(${DEDICATED_EXECUTABLE} PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(${DEDICATED_EXECUTABLE} PROPERTIES FOLDER "exes")

	target_link_libraries(${DEDICATED_EXECUTABLE}
		idlib
		mikktspace # RBMIKKT_TANGENT
		$<$<BOOL:FREETYPE>:Freetype::Freetype>
		${CURL_LIBRARY}
		${ZLIB_LIBRARY}
		${SDL2_LIBRARY}
		${sys_libs}
	)

	if(NOT APPLE AND NOT WIN32)
		install(TARGETS ${DEDICATED_EXECUTABLE}
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()

# Generate the TypeInfoGen
if (TYPEINFOGEN)
	add_executable(TypeInfoGen
		${src_typeinfogen}
		${src_toolsupportsrc}
		${src_enginestub}
	)

	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_typeinfogen} ${src_toolsupportsrc} ${src_enginestub})

if(HARDLINK_GAME)
	set_target_properties(TypeInfoGen PROPERTIES COMPILE_DEFINITIONS "ID_TYPEINFO;ID_CONSOLE_TOOLS")
	target_include_directories(TypeInfoGen PRIVATE ${game_includes})
else()
	set_target_properties(TypeInfoGen PROPERTIES COMPILE_DEFINITIONS "ID_TYPEINFO;ID_CONSOLE_TOOLS;__DOOM_DLL__")
endif()
	set_target_properties(TypeInfoGen PROPERTIES LINK_FLAGS "${ldflags}")
	target_link_libraries(TypeInfoGen idlib ZLIB::ZLIB )
	set_target_properties(TypeInfoGen PROPERTIES FOLDER "exes")

	if(NOT APPLE AND NOT WIN32)
		install(TARGETS TypeInfoGen
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()

if(GAME_CODE AND NOT HARDLINK_GAME)
	if (AROS)
		add_executable(base sys/aros/dll/dllglue.c ${src_game})
		set_target_properties(base PROPERTIES OUTPUT_NAME "base.aros-${AROS_ARCH}")
	else()
		add_library(base SHARED ${src_game})
	endif()
	
	if (MSVC)
		target_sources(base PRIVATE "sys/win32/natvis/game.natvis")
	endif()
	
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_game})
	
	set_target_properties(base PROPERTIES PREFIX "")
	set_target_properties(base PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_D3XP;CTF")
	target_include_directories(base PRIVATE "${CMAKE_SOURCE_DIR}/game")
	set_target_properties(base PROPERTIES LINK_FLAGS "${ldflags}")
	set_target_properties(base PROPERTIES INSTALL_NAME_DIR "@executable_path")
	if (AROS)
		target_link_libraries(base idlib dll)
	else()
		target_link_libraries(base idlib)
	endif()
	set_target_properties(base PROPERTIES FOLDER "dlls")

	if(NOT APPLE AND NOT WIN32)
		install(TARGETS base
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()
