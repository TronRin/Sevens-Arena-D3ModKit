set( src_cm
	${CMAKE_SOURCE_DIR}/cm/CollisionModel_contacts.cpp
	${CMAKE_SOURCE_DIR}/cm/CollisionModel_contents.cpp
	${CMAKE_SOURCE_DIR}/cm/CollisionModel_debug.cpp
	${CMAKE_SOURCE_DIR}/cm/CollisionModel_files.cpp
	${CMAKE_SOURCE_DIR}/cm/CollisionModel_load.cpp
	${CMAKE_SOURCE_DIR}/cm/CollisionModel_rotate.cpp
	${CMAKE_SOURCE_DIR}/cm/CollisionModel_trace.cpp
	${CMAKE_SOURCE_DIR}/cm/CollisionModel_translate.cpp
)

add_globbed_headers( src_cm "cm" )

set( src_framework
	${CMAKE_SOURCE_DIR}/framework/CVarSystem.cpp
	${CMAKE_SOURCE_DIR}/framework/CmdSystem.cpp
	${CMAKE_SOURCE_DIR}/framework/Common.cpp
	${CMAKE_SOURCE_DIR}/framework/Compressor.cpp
	${CMAKE_SOURCE_DIR}/framework/Console.cpp
	${CMAKE_SOURCE_DIR}/framework/DemoFile.cpp
	${CMAKE_SOURCE_DIR}/framework/DeclAF.cpp
	${CMAKE_SOURCE_DIR}/framework/DeclEntityDef.cpp
	${CMAKE_SOURCE_DIR}/framework/DeclFX.cpp
	${CMAKE_SOURCE_DIR}/framework/DeclManager.cpp
	${CMAKE_SOURCE_DIR}/framework/DeclParticle.cpp
	${CMAKE_SOURCE_DIR}/framework/DeclSkin.cpp
	${CMAKE_SOURCE_DIR}/framework/DeclTable.cpp
	${CMAKE_SOURCE_DIR}/framework/Dhewm3SettingsMenu.cpp
	${CMAKE_SOURCE_DIR}/framework/EditField.cpp
	${CMAKE_SOURCE_DIR}/framework/EventLoop.cpp
	${CMAKE_SOURCE_DIR}/framework/File.cpp
	${CMAKE_SOURCE_DIR}/framework/FileSystem.cpp
	${CMAKE_SOURCE_DIR}/framework/KeyInput.cpp
	${CMAKE_SOURCE_DIR}/framework/UsercmdGen.cpp
	${CMAKE_SOURCE_DIR}/framework/Session_menu.cpp
	${CMAKE_SOURCE_DIR}/framework/Session.cpp
	${CMAKE_SOURCE_DIR}/framework/async/AsyncClient.cpp
	${CMAKE_SOURCE_DIR}/framework/async/AsyncNetwork.cpp
	${CMAKE_SOURCE_DIR}/framework/async/AsyncServer.cpp
	${CMAKE_SOURCE_DIR}/framework/async/MsgChannel.cpp
	${CMAKE_SOURCE_DIR}/framework/async/NetworkSystem.cpp
	${CMAKE_SOURCE_DIR}/framework/async/ServerScan.cpp
	${CMAKE_SOURCE_DIR}/framework/miniz/miniz.c
	${CMAKE_SOURCE_DIR}/framework/minizip/ioapi.c
	${CMAKE_SOURCE_DIR}/framework/minizip/unzip.cpp
)

add_globbed_headers( src_framework "framework" )

set( src_aas_file
	${CMAKE_SOURCE_DIR}/libs/aasfile/AASFile.cpp
	${CMAKE_SOURCE_DIR}/libs/aasfile/AASFile_optimize.cpp
	${CMAKE_SOURCE_DIR}/libs/aasfile/AASFile_sample.cpp
	${CMAKE_SOURCE_DIR}/libs/aasfile/AASFileManager.cpp
)

add_globbed_headers( src_aas_file "libs/aasfile" )

set( src_renderer
	${CMAKE_SOURCE_DIR}/renderer/Cinematic.cpp
	${CMAKE_SOURCE_DIR}/renderer/GuiModel.cpp
	${CMAKE_SOURCE_DIR}/renderer/Image_files.cpp
	${CMAKE_SOURCE_DIR}/renderer/Image_init.cpp
	${CMAKE_SOURCE_DIR}/renderer/Image_load.cpp
	${CMAKE_SOURCE_DIR}/renderer/Image_process.cpp
	${CMAKE_SOURCE_DIR}/renderer/Image_program.cpp
	${CMAKE_SOURCE_DIR}/renderer/Interaction.cpp
	${CMAKE_SOURCE_DIR}/renderer/Material.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model.cpp
	${CMAKE_SOURCE_DIR}/renderer/ModelDecal.cpp
	${CMAKE_SOURCE_DIR}/renderer/ModelManager.cpp
	${CMAKE_SOURCE_DIR}/renderer/ModelOverlay.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_beam.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_ase.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_liquid.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_lwo.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_ma.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_md3.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_md5.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_obj.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_prt.cpp
	${CMAKE_SOURCE_DIR}/renderer/Model_sprite.cpp
	${CMAKE_SOURCE_DIR}/renderer/RenderEntity.cpp
	${CMAKE_SOURCE_DIR}/renderer/RenderSystem.cpp
	${CMAKE_SOURCE_DIR}/renderer/RenderSystem_init.cpp
	${CMAKE_SOURCE_DIR}/renderer/RenderWorld.cpp
	${CMAKE_SOURCE_DIR}/renderer/RenderWorld_demo.cpp
	${CMAKE_SOURCE_DIR}/renderer/RenderWorld_load.cpp
	${CMAKE_SOURCE_DIR}/renderer/RenderWorld_portals.cpp
	${CMAKE_SOURCE_DIR}/renderer/VertexCache.cpp
	${CMAKE_SOURCE_DIR}/renderer/draw_arb2.cpp
	${CMAKE_SOURCE_DIR}/renderer/draw_common.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_backend.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_deform.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_font.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_guisurf.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_light.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_lightrun.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_main.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_normals.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_orderIndexes.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_polytope.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_render.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_rendertools.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_shadowbounds.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_stencilshadow.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_subview.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_trace.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_trisurf.cpp
	${CMAKE_SOURCE_DIR}/renderer/tr_turboshadow.cpp
)

add_globbed_headers( src_renderer "renderer" )

# RBMIKKT_TANGENT...

set( src_mikktspace
	${CMAKE_SOURCE_DIR}/libs/mikktspace/mikktspace.c
)

add_globbed_headers( src_mikktspace "mikktspace" )

# ...RBMIKKT_TANGENT

if( IMGUI )
	set( src_imgui
		${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_sdl2.cpp
		${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl2.cpp
		${CMAKE_SOURCE_DIR}/libs/imgui/imgui.h
		${CMAKE_SOURCE_DIR}/libs/imgui/imgui.cpp
		${CMAKE_SOURCE_DIR}/libs/imgui/imgui_draw.cpp
		${CMAKE_SOURCE_DIR}/libs/imgui/imgui_tables.cpp
		${CMAKE_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp
		${CMAKE_SOURCE_DIR}/libs/imgui/imgui_demo.cpp
		sys_imgui.h
		sys_imgui.cpp
		imgui_savestyle.cpp
	)
else()
	set( src_imgui
		sys_imgui.h
	)
endif()

set( src_sound
	${CMAKE_SOURCE_DIR}/sound/snd_cache.cpp
	${CMAKE_SOURCE_DIR}/sound/snd_decoder.cpp
	${CMAKE_SOURCE_DIR}/sound/snd_efxfile.cpp
	${CMAKE_SOURCE_DIR}/sound/snd_emitter.cpp
	${CMAKE_SOURCE_DIR}/sound/snd_reverb.cpp
	${CMAKE_SOURCE_DIR}/sound/snd_shader.cpp
	${CMAKE_SOURCE_DIR}/sound/snd_system.cpp
	${CMAKE_SOURCE_DIR}/sound/snd_wavefile.cpp
	${CMAKE_SOURCE_DIR}/sound/snd_world.cpp
)

add_globbed_headers( src_sound "sound" )

set( src_ui
	${CMAKE_SOURCE_DIR}/ui/BindWindow.cpp
	${CMAKE_SOURCE_DIR}/ui/ChoiceWindow.cpp
	${CMAKE_SOURCE_DIR}/ui/DeviceContext.cpp
	${CMAKE_SOURCE_DIR}/ui/EditWindow.cpp
	${CMAKE_SOURCE_DIR}/ui/FieldWindow.cpp
	${CMAKE_SOURCE_DIR}/ui/GuiScript.cpp
	${CMAKE_SOURCE_DIR}/ui/ListGUI.cpp
	${CMAKE_SOURCE_DIR}/ui/ListWindow.cpp
	${CMAKE_SOURCE_DIR}/ui/MarkerWindow.cpp
	${CMAKE_SOURCE_DIR}/ui/RegExp.cpp
	${CMAKE_SOURCE_DIR}/ui/RenderWindow.cpp
	${CMAKE_SOURCE_DIR}/ui/SimpleWindow.cpp
	${CMAKE_SOURCE_DIR}/ui/SliderWindow.cpp
	${CMAKE_SOURCE_DIR}/ui/UserInterface.cpp
	${CMAKE_SOURCE_DIR}/ui/Window.cpp
	${CMAKE_SOURCE_DIR}/ui/Winvar.cpp
)

add_globbed_headers( src_ui "ui" )

set( src_dmap
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/dmap.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/facebsp.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/leakfile.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/map.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/optimize.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/output.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/portals.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/shadowopt3.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/tritjunction.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/tritools.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/ubrush.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/dmap/usurface.cpp
)

add_globbed_headers( src_dmap "tools/compilers/dmap" )

set( src_aas
	${CMAKE_SOURCE_DIR}/tools/compilers/aas/AASBuild.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/aas/AASBuild_file.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/aas/AASBuild_gravity.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/aas/AASBuild_ledge.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/aas/AASBuild_merge.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/aas/AASCluster.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/aas/AASReach.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/aas/Brush.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/aas/BrushBSP.cpp
)

add_globbed_headers( src_aas "tools/compilers/aas" )

set( src_roq
	${CMAKE_SOURCE_DIR}/tools/compilers/roqvq/NSBitmapImageRep.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/roqvq/codec.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/roqvq/roq.cpp
	${CMAKE_SOURCE_DIR}/tools/compilers/roqvq/roqParam.cpp
)

add_globbed_headers( src_roq "tools/compilers/roqvq" )

set( src_matbuild
	${CMAKE_SOURCE_DIR}/tools/compilers/matbuild/matbuild.cpp
)

add_globbed_headers( src_matbuild "tools/compilers/matbuild" )

set( src_debuggerServer
	${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerBreakpoint.h
	${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerBreakpoint.cpp
	${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerServer.h
	${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerServer.cpp
	${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerScript.h
	${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerScript.cpp
	${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerMessages.h
	${CMAKE_SOURCE_DIR}/tools/debugger/debugger.cpp
)

set(src_core
	${src_cm}
	${src_framework}
	${src_aas_file}
	${src_renderer}
	${src_mikktspace}
	${src_sound}
	${src_ui}
	${src_dmap}
	${src_aas}
	${src_roq}
	${src_matbuild}
	${src_tools}
)

set( src_stub_openal		stub/openal_stub.cpp )
set( src_stub_gl			stub/stub_gl.cpp )

set( src_tools
	${CMAKE_SOURCE_DIR}/tools/guied/GEWindowWrapper_stub.cpp
)

# TODO: add_globbed_headers(src_tools "tools/guied")

# Begin normal tools code
if ( TOOLS AND MFC_FOUND AND MSVC )
	set( ALLOWRESOURCES ON )

	# Common files.
	file( GLOB src_comafx ${CMAKE_SOURCE_DIR}/tools/comafx/*.cpp )
	add_globbed_headers( src_comafx "tools/comafx" )

	file( GLOB_RECURSE src_tools_common ${CMAKE_SOURCE_DIR}/tools/common/*.cpp )
	add_globbed_headers( src_tools_common "tools/common" )

	# Compilers.

	file( GLOB src_compiler_roqvq	  ${CMAKE_SOURCE_DIR}/tools/compilers/roqvq/*.cpp ) # FIXME: some of those were added before
	add_globbed_headers( src_compiler_roqvq "tools/compilers/roqvq" )

	# Tools

	# Articulated Frame Editor.
	file( GLOB src_afeditor ${CMAKE_SOURCE_DIR}/tools/af/*.cpp )
	add_globbed_headers( src_afeditor "tools/af" )

	# Declaration Editor.
	file( GLOB src_decleditor ${CMAKE_SOURCE_DIR}/tools/decl/*.cpp )
	add_globbed_headers( src_decleditor "tools/decl" )

	# GUI Editor.
	file( GLOB src_gui_editor ${CMAKE_SOURCE_DIR}/tools/guied/*.cpp )
	add_globbed_headers(src_gui_editor "tools/guied" )

	# Material Editor.
	file( GLOB src_materialeditor ${CMAKE_SOURCE_DIR}/tools/materialeditor/*.cpp )
	add_globbed_headers( src_materialeditor "tools/materialeditor" )

	# Particle Editor.
	file( GLOB src_particleeditor ${CMAKE_SOURCE_DIR}/tools/particle/*.cpp )
	add_globbed_headers(src_particleeditor "tools/particle" )
	
	# Ligth Editor.
	file( GLOB src_ligth_editor ${CMAKE_SOURCE_DIR}/tools/ligtheditor/*.cpp )
	add_globbed_headers( src_gui_editor "tools/ligtheditor" )

	# Script Editor.
	file( GLOB src_script_editor ${CMAKE_SOURCE_DIR}/tools/script/*.cpp )
	add_globbed_headers(src_script_editor "tools/script" )
	
	# Sound Editor.
	file( GLOB src_sound_editor ${CMAKE_SOURCE_DIR}/tools/sound/*.cpp )
	add_globbed_headers( src_sound_editor "tools/sound" )

	# Script Debugger
	set( src_debuggerClient
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerClient.h
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerClient.cpp
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerApp.h
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerApp.cpp
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerQuickWatchDlg.h
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerQuickWatchDlg.cpp
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerWindow.h
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerWindow.cpp
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerFindDlg.h
		${CMAKE_SOURCE_DIR}/tools/debugger/DebuggerFindDlg.cpp
	)
	set( src_script_debugger
		${src_debuggerServer}
		${src_debuggerClient}
	)

	# The numerous tools in a nice list.
	list( APPEND src_editor_tools
		${src_comafx}
		${src_tools_common}
		${src_compiler_renderbump}
		${src_afeditor}
		${src_decleditor}
		${src_gui_editor}
		${src_materialeditor}
		${src_particleeditor}
		${src_pdaeditor}
		${src_ligth_editor}
		${src_script_editor}
		${src_sound_editor}
		${src_script_debugger}
		${CMAKE_SOURCE_DIR}/tools/edit_public.h
		${CMAKE_SOURCE_DIR}/tools/edit_gui_common.h
	)

	SET( CMAKE_MFC_FLAG 2 )
	set( TOOLS_DEFINES "ID_ALLOW_TOOLS;_AFXDLL" )
else()
	set( src_editor_tools ${CMAKE_SOURCE_DIR}/tools/edit_stub.cpp ${CMAKE_SOURCE_DIR}/tools/edit_public.h ${CMAKE_SOURCE_DIR}/tools/debugger/debugger_common.h )
	list( APPEND src_editor_tools
		${src_debuggerServer}
	)
endif()

if( APPLE )
	set( OSX_RESOURCE_FILES
		osx/Doom3.icns
		osx/Doom 3.rsrc
	)

	set_source_files_properties( ${OSX_RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )

	set( src_sys_base
		cpu.cpp
		threads.cpp
		events.cpp
		sys_local.cpp
		posix/posix_net.cpp
		posix/posix_main.cpp
	)

	set( src_sys_core
		glimp.cpp
		osx/DOOMController.mm
		osx/macosx_misc.mm
		osx/SDLMain.m
		${OSX_RESOURCE_FILES}
	)
elseif( WIN32 )
	set( src_sys_base
		cpu.cpp
		threads.cpp
		events.cpp
		sys_local.cpp
		win32/win_input.cpp
		win32/win_main.cpp
		win32/win_net.cpp
		win32/win_shared.cpp
		win32/win_syscon.cpp
		win32/SDL_win32_main.c
	)

	# adding the few relevant headers in sys/ manually..
	set( src_sys_base ${src_sys_base}
		platform.h
		sys_local.h
		sys_public.h
		win32/win_local.h
	)

	set( src_sys_core
		glimp.cpp
	)

	if( TOOLS )
		file( GLOB src_sys_tools_images "win32/rc/res/*.bmp" "win32/rc/res/*.ico" "win32/rc/res/*.cur" )
		set( src_sys_core ${src_sys_core} ${src_sys_tools_images}  "win32/rc/doom.rc" )
	else()
		# adding both .rc files breaks the build, but I think it only contains the icon
		# and doom.rc also contains that (+icons for editor etc, AFAIK), so this is fine
		set(src_sys_core ${src_sys_core} "win32/rc/dhewm3.rc" )
	endif()
else()
	set( src_sys_base
		cpu.cpp
		threads.cpp
		events.cpp
		sys_local.cpp
		posix/posix_net.cpp
		posix/posix_main.cpp
		linux/main.cpp
	)

	set( src_sys_core
		glimp.cpp
	)
endif()

# /********************************** engine ************************************************/

if( ENGINE )
	add_executable( ${ENGINE_EXECUTABLE} WIN32 MACOSX_BUNDLE
		${src_core}
		${src_sys_base}
		${src_sys_core}
		${src_imgui}
		${src_editor_tools}
	)
	
	set( src_engine
		${src_core}
		${src_sys_base}
		${src_sys_core}
		${src_imgui}
		${src_editor_tools}
	)

	if( MSVC )
		set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ENGINE_EXECUTABLE} )
		
		target_sources( ${ENGINE_EXECUTABLE} PRIVATE "win32/natvis/engine.natvis" )
		
		source_group( TREE ${CMAKE_SOURCE_DIR} PREFIX neo FILES ${src_core} ${src_sys_base} ${src_sys_core} ${src_editor_tools} )
	endif()
	
	target_include_directories( ${ENGINE_EXECUTABLE} PRIVATE "${CMAKE_SOURCE_DIR}/libs/imgui" )

	if( HARDLINK_GAME )
		set_target_properties( ${ENGINE_EXECUTABLE} PROPERTIES COMPILE_DEFINITIONS "${TOOLS_DEFINES}" )
		
		# Hack to get .natvis added
		if ( MSVC )
			target_sources( ${ENGINE_EXECUTABLE} PRIVATE "win32/natvis/game.natvis" )
		endif()
	else()
		set_target_properties( ${ENGINE_EXECUTABLE} PROPERTIES COMPILE_DEFINITIONS "__DOOM_DLL__;${TOOLS_DEFINES}" )
	endif()

	set_target_properties( ${ENGINE_EXECUTABLE} PROPERTIES LINK_FLAGS "${ldflags}" )
	set_target_properties( ${ENGINE_EXECUTABLE} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/sys/osx/Info.plist )
	set_target_properties( ${ENGINE_EXECUTABLE} PROPERTIES FOLDER "exes" )

	target_link_libraries( ${ENGINE_EXECUTABLE}
		idlib
		$<$<BOOL:FREETYPE>:Freetype::Freetype>
		${OPENAL_LIBRARY}
		${CURL_LIBRARY}
		${SDL2_LIBRARY}
		${sys_libs}
	)

	if( HARDLINK_GAME )
		target_link_libraries( ${ENGINE_EXECUTABLE}
			base
		)
	endif()

	if( NOT APPLE AND NOT WIN32 )
		install( TARGETS ${ENGINE_EXECUTABLE}
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()

# /********************************** dedicated ************************************************/

if( DEDICATED )
	add_executable( ${DEDICATED_EXECUTABLE} WIN32 MACOSX_BUNDLE
		${src_core}
		${src_stub_openal}
		${src_stub_gl}
		${src_sys_base}
		${src_debuggerServer}
	)
	
	set( src_engine
		${src_core}
		${src_stub_openal}
		${src_stub_gl}
		${src_sys_base}
		${src_debuggerServer}
	)
	
	if ( MSVC )
		target_sources( ${DEDICATED_EXECUTABLE} PRIVATE "win32/natvis/engine.natvis" )
		
		source_group( TREE ${CMAKE_SOURCE_DIR} PREFIX neo FILES ${src_core} ${src_sys_base} ${src_stub_openal} ${src_stub_gl} ${src_debuggerServer} )
	endif()

	if( HARDLINK_GAME )
		set_target_properties( ${DEDICATED_EXECUTABLE} PROPERTIES COMPILE_DEFINITIONS "IMGUI_DISABLE;ID_DEDICATED" )
		
		# Hack to get .natvis added
		if ( MSVC )
			target_sources( ${DEDICATED_EXECUTABLE} PRIVATE "win32/natvis/game.natvis" )
		endif()
	else()
		set_target_properties( ${DEDICATED_EXECUTABLE} PROPERTIES COMPILE_DEFINITIONS "IMGUI_DISABLE;ID_DEDICATED;__DOOM_DLL__" )
	endif()

	set_target_properties( ${DEDICATED_EXECUTABLE} PROPERTIES LINK_FLAGS "${ldflags}" )
	set_target_properties( ${DEDICATED_EXECUTABLE} PROPERTIES FOLDER "exes" )

	target_link_libraries( ${DEDICATED_EXECUTABLE}
		idlib
		$<$<BOOL:FREETYPE>:Freetype::Freetype>
		${CURL_LIBRARY}
		${SDL2_LIBRARY}
		${sys_libs}
	)

	if( HARDLINK_GAME )
		target_link_libraries( ${DEDICATED_EXECUTABLE}
			base
		)
	endif()

	if( NOT APPLE AND NOT WIN32 )
		install( TARGETS ${DEDICATED_EXECUTABLE}
				RUNTIME DESTINATION "${bindir}"
				LIBRARY DESTINATION "${libdir}"
				ARCHIVE DESTINATION "${libdir}"
		)
	endif()
endif()