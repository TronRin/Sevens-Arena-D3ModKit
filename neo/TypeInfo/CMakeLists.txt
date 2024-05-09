project( TypeInfO )

set(src_typeinfogen 
	main.cpp
	TypeInfoGen.cpp
)

add_globbed_headers( src_typeinfogen "TypeInfo" )

set ( src_toolsupportsrc 
	${CMAKE_SOURCE_DIR}/framework/CmdSystem.cpp
	${CMAKE_SOURCE_DIR}/framework/CVarSystem.cpp
	${CMAKE_SOURCE_DIR}/framework/File.cpp
	${CMAKE_SOURCE_DIR}/framework/FileSystem.cpp
	${CMAKE_SOURCE_DIR}/framework/minizip/ioapi.c
	${CMAKE_SOURCE_DIR}/framework/minizip/ioapi.h
	${CMAKE_SOURCE_DIR}/framework/minizip/MiniZip64_info.txt
	${CMAKE_SOURCE_DIR}/framework/minizip/unzip.cpp
	${CMAKE_SOURCE_DIR}/framework/minizip/unzip.h
)
	
set ( src_enginestub 
	${CMAKE_SOURCE_DIR}/sys/stub/tools_stub.cpp
)

add_executable( ${PROJECT_NAME}
	${src_typeinfogen}
	${src_toolsupportsrc}
	${src_enginestub}
)

if ( MSVC )
	source_group( TREE ${CMAKE_SOURCE_DIR} PREFIX code FILES ${src_typeinfogen} ${src_toolsupportsrc} ${src_enginestub} )
endif()

if( HARDLINK_GAME )
	set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "ID_TYPEINFO;ID_CONSOLE_TOOLS" )
	target_include_directories( ${PROJECT_NAME} PRIVATE ${game_includes} )
else()
	set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "ID_TYPEINFO;ID_CONSOLE_TOOLS;__DOOM_DLL__" )
endif()

set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "${ldflags}" )
target_link_libraries( ${PROJECT_NAME} idlib ZLIB::ZLIB )
set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER "exes" )

if( NOT APPLE AND NOT WIN32 )
	install(TARGETS ${PROJECT_NAME}
			RUNTIME DESTINATION "${bindir}"
			LIBRARY DESTINATION "${libdir}"
			ARCHIVE DESTINATION "${libdir}"
	)
endif()