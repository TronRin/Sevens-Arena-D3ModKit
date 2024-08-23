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

#include "cm/CollisionModel_local.h"

/*
==========================
idCollisionModelLocal::GetName
==========================
*/
const char* idCollisionModelLocal::GetName( void ) const {
	return name.c_str();
}

/*
==========================
idCollisionModelLocal::GetBounds
==========================
*/
bool idCollisionModelLocal::GetBounds( idBounds &bounds ) const {
	bounds = this->bounds;
	return true;
}

/*
==========================
idCollisionModelLocal::GetBounds
==========================
*/
bool idCollisionModelLocal::GetContents( int &contents ) const {
	contents = this->contents;
	return true;
}

/*
==========================
idCollisionModelLocal::GetBounds
==========================
*/
bool idCollisionModelLocal::GetVertex( int vertexNum, idVec3 &vertex ) const {
	if ( vertexNum < 0 || vertexNum >= numVertices ) {
		common->Printf( "idCollisionModelManagerLocal::GetModelVertex: invalid vertex number\n" );
		return false;
	}

	vertex = vertices[vertexNum].p;

	return true;
}

/*
==========================
idCollisionModelLocal::GetBounds
==========================
*/
bool idCollisionModelLocal::GetEdge( int edgeNum, idVec3 &start, idVec3&end ) const {
	edgeNum = abs( edgeNum );

	if ( edgeNum >= numEdges ) {
		common->Printf( "idCollisionModelManagerLocal::GetModelEdge: invalid edge number\n" );
		return false;
	}

	start = vertices[edges[edgeNum].vertexNum[0]].p;
	end = vertices[edges[edgeNum].vertexNum[1]].p;

	return true;
}

/*
==========================
idCollisionModelLocal::GetPolygon
==========================
*/
bool idCollisionModelLocal::GetPolygon( int polygonNum, idFixedWinding &winding ) const {
	int i, edgeNum;
	cm_polygon_t* poly;

	poly = *reinterpret_cast<cm_polygon_t**>( &polygonNum );
	winding.Clear();
	for ( i = 0; i < poly->numEdges; i++ ) {
		edgeNum = poly->edges[i];
		winding += vertices[edges[abs(edgeNum)].vertexNum[INTSIGNBITSET(edgeNum)]].p;
	}

	return true;
}

/*
================
idCollisionModelLocal::DrawModel
================
*/
void idCollisionModelLocal::DrawModel( const idVec3 &modelOrigin, const idMat3 &modelAxis, const idVec3 &viewOrigin, const float radius ) {
	idVec3 viewPos;

	if ( cm_drawColor.IsModified() ) {
		sscanf( cm_drawColor.GetString(), "%f %f %f %f", &cm_color.x, &cm_color.y, &cm_color.z, &cm_color.w );
		cm_drawColor.ClearModified();
	}

	viewPos = (viewOrigin - modelOrigin) * modelAxis.Transpose();
	collisionModelManagerLocal.DrawNodePolygons( this, node, modelOrigin, modelAxis, viewPos, radius );
}


/*
================
idCollisionModelLocal::ModelInfo
================
*/
void idCollisionModelLocal::ModelInfo( void ) {
	idCollisionModelLocal modelInfo;

//	collisionModelManagerLocal.PrintModelInfo(this);
}