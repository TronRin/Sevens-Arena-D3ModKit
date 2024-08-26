project( base )

set( src_game
	AF.cpp
	AFEntity.cpp
	Actor.cpp
	Actor_script.cpp
	AnimatedEntity.cpp
	AnimState.cpp
	Camera.cpp
	Entity.cpp
	Entity_script.cpp
	BrittleFracture.cpp
	Fx.cpp
	GameEdit.cpp
	Game_local.cpp
	Game_network.cpp
	Item.cpp
	IK.cpp
	Light.cpp
	Misc.cpp
	Mover.cpp
	Moveable.cpp
	MultiplayerGame.cpp
	Player.cpp
	PlayerIcon.cpp
	PlayerView.cpp
	Projectile.cpp
	Pvs.cpp
	SecurityCamera.cpp
	SmokeParticles.cpp
	Sound.cpp
	Target.cpp
	Trigger.cpp
	Weapon.cpp
	Weapon_script.cpp
	WorldSpawn.cpp
	aas/AAS.cpp
	aas/AAS_debug.cpp
	aas/AAS_pathing.cpp
	aas/AAS_routing.cpp
	ai/AI.cpp
	ai/AI_events.cpp
	ai/AI_pathing.cpp
	ai/AI_script.cpp
	gamesys/Class.cpp
	gamesys/Event.cpp
	gamesys/SaveGame.cpp
	gamesys/State.cpp
	gamesys/SysCmds.cpp
	gamesys/SysCvar.cpp
	anim/Anim.cpp
	anim/Anim_Blend.cpp
	anim/Anim_Testmodel.cpp
	script/Script_Compiler.cpp
	script/Script_Interpreter.cpp
	script/Script_Program.cpp
	script/Script_Thread.cpp
	physics/Clip.cpp
	physics/Force.cpp
	physics/Force_Constant.cpp
	physics/Force_Drag.cpp
	physics/Force_Field.cpp
	physics/Force_Spring.cpp
	physics/Physics.cpp
	physics/Physics_AF.cpp
	physics/Physics_Actor.cpp
	physics/Physics_Base.cpp
	physics/Physics_Monster.cpp
	physics/Physics_Liquid.cpp
	physics/Physics_Parametric.cpp
	physics/Physics_Player.cpp
	physics/Physics_RigidBody.cpp
	physics/Physics_Static.cpp
	physics/Physics_StaticMulti.cpp
	physics/Push.cpp
	CACHE INTERNAL ""
)

add_globbed_headers(src_game "game")

if( HARDLINK_GAME )
	add_library( ${PROJECT_NAME} STATIC ${src_game} )
else()
	add_library( ${PROJECT_NAME} SHARED ${src_game} )
endif()

if ( MSVC )
	target_sources( ${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/sys/win32/natvis/game.natvis" )
	source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_game} )

	if( HARDLINK_GAME )
		set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER "libs" )
	else()
		set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER "dlls" )
	endif()
endif()

target_precompile_headers( ${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/idlib/precompiled.h" )

set_target_properties( ${PROJECT_NAME} PROPERTIES PREFIX "" )
set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "GAME_DLL;_D3XP;CTF" )
target_include_directories( ${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/idlib" )
set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "${ldflags}" )
set_target_properties( ${PROJECT_NAME} PROPERTIES INSTALL_NAME_DIR "@executable_path" )
if( NOT HARDLINK_GAME )
	target_link_libraries( ${PROJECT_NAME} idlib )
endif()
set_target_properties( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:>${OUTPUT_FOLDER} LIBRARY_OUTPUT_DIRECTORY $<1:>${OUTPUT_FOLDER} )

if( NOT APPLE AND NOT WIN32 )
	install(TARGETS ${PROJECT_NAME}
			RUNTIME DESTINATION "${bindir}"
			LIBRARY DESTINATION "${libdir}"
			ARCHIVE DESTINATION "${libdir}"
	)
endif()