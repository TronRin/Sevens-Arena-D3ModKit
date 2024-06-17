project( MayaImport${MAYA_VERSION} )

set( src_mayaimport 
	exporter.h
	maya_main.cpp
	maya_main.h
)

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

add_library( ${PROJECT_NAME} SHARED
	${src_mayaimport}
	${src_toolsupportsrc}
	${src_enginestub}
)

add_dependencies( ${PROJECT_NAME} MayaDevKit )

source_group( TREE ${CMAKE_SOURCE_DIR} PREFIX neo FILES ${src_mayaimport} ${src_toolsupportsrc} ${src_enginestub} )

if( HARDLINK_GAME )
	if ( MSVC )
		set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "ID_MAYA_IMPORT;ID_MAYA_IMPORT_API=__declspec(dllexport)" )
	else()
		set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "ID_MAYA_IMPORT;ID_MAYA_IMPORT_API=__attribute__ ((visibility (\"default\")))" )
	endif()
	target_include_directories( ${PROJECT_NAME} PRIVATE ${game_includes} )
else()
	if ( MSVC )
		set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "ID_MAYA_IMPORT;__DOOM_DLL__;ID_MAYA_IMPORT_API=__declspec(dllexport)" )
	else()
		set_target_properties( ${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "ID_MAYA_IMPORT;__DOOM_DLL__;ID_MAYA_IMPORT_API=__attribute__ ((visibility (\"default\")))" )
	endif()
endif()
set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "${ldflags}" )
target_link_libraries( ${PROJECT_NAME} idlib ZLIB::ZLIB )
target_include_directories( ${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libs/MayaSDKDevKit/include" )
target_link_directories( ${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/libs/MayaSDKDevKit/lib" )
target_link_libraries( ${PROJECT_NAME} idlib Foundation OpenMaya OpenMayaAnim )
set_target_properties( ${PROJECT_NAME} PROPERTIES FOLDER "dlls" )

if( NOT APPLE AND NOT WIN32 )
	install( TARGETS ${PROJECT_NAME}
			RUNTIME DESTINATION "${bindir}"
			LIBRARY DESTINATION "${libdir}"
			ARCHIVE DESTINATION "${libdir}"
	)
endif()