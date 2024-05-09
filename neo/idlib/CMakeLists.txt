project( idlib )

set( src_idlib
	bv/Bounds.cpp
	bv/Frustum.cpp
	bv/Sphere.cpp
	bv/Box.cpp
	geometry/DrawVert.cpp
	geometry/Winding2D.cpp
	geometry/Surface_SweptSpline.cpp
	geometry/Winding.cpp
	geometry/Surface.cpp
	geometry/Surface_Patch.cpp
	geometry/TraceModel.cpp
	geometry/JointTransform.cpp
	hashing/CRC32.cpp
	hashing/MD4.cpp
	hashing/MD5.cpp
	math/Angles.cpp
	math/Lcp.cpp
	math/Math.cpp
	math/Matrix.cpp
	math/Ode.cpp
	math/Plane.cpp
	math/Pluecker.cpp
	math/Polynomial.cpp
	math/Quat.cpp
	math/Rotation.cpp
	math/Simd.cpp
	math/Simd_Generic.cpp
	math/Simd_AltiVec.cpp
	math/Simd_MMX.cpp
	math/Simd_3DNow.cpp
	math/Simd_SSE.cpp
	math/Simd_SSE2.cpp
	math/Simd_SSE3.cpp
	math/Vector.cpp
	BitMsg.cpp
	LangDict.cpp
	Lexer.cpp
	Lib.cpp
	containers/HashIndex.cpp
	Dict.cpp
	Str.cpp
	Parser.cpp
	MapFile.cpp
	CmdArgs.cpp
	Token.cpp
	Base64.cpp
	Timer.cpp
	Heap.cpp
)

add_globbed_headers( src_idlib "idlib" )

add_library( idlib STATIC ${src_idlib} )

if( D3_COMPILER_IS_GCC_OR_CLANG AND NOT MINGW )
	set_target_properties( idlib PROPERTIES COMPILE_FLAGS "-fPIC" )
endif()

if ( MSVC )
	target_sources( idlib INTERFACE "${CMAKE_SOURCE_DIR}/sys/win32/natvis/idlib.natvis" )

	source_group( TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX neo FILES ${src_idlib} )

	set_target_properties( idlib PROPERTIES FOLDER "libs" )
endif()