# I'm a bit sloppy with headers and just glob them in..
# they're only handled in CMake at all so they turn up in Visual Studio solutions..

# globs all the headers from ${PATHPREFIX}/ and adds them to ${SRCLIST}
function( add_globbed_headers SRCLIST PATHPREFIX )
	file( GLOB tmp_hdrs "${CMAKE_SOURCE_DIR}/${PATHPREFIX}/*.h" )
	set( ${SRCLIST} ${tmp_hdrs} ${${SRCLIST}} PARENT_SCOPE )
endfunction()