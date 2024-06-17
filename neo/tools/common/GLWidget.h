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
#ifndef __GLWIDGET_H__
#define __GLWIDGET_H__

/////////////////////////////////////////////////////////////////////////////
// idGLWidget window

class idGLDrawable {
public:
			idGLDrawable();
	virtual ~idGLDrawable() {};

	virtual void draw( int x, int y, int w, int h );
	virtual void setMedia( const char *name ){}
	virtual void buttonDown( int button, float x, float y );
	virtual void buttonUp( int button, float x, float y );
	virtual void mouseMove( float x, float y );
	virtual int getRealTime() {
		return realTime;
	};

	virtual bool ScreenCoords() {
		return true;
	}

	void SetRealTime( int i ) {
		realTime = i;
	}

	virtual void Update() {};

	float getScale() {
		return scale;
	}

	void setScale( float f ) {
		scale = f;
	}

protected:
	float scale;
	float xOffset;
	float yOffset;
	float zOffset;
	float pressX;
	float pressY;
	bool  handleMove;
	int button;
	int realTime;
};

class idGLWidget : public CWnd
{
// Construction
public:
			idGLWidget();
	virtual ~idGLWidget();

	void setDrawable( idGLDrawable *d );
	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL );

protected:

	virtual BOOL PreCreateWindow( CREATESTRUCT& cs );

	idGLDrawable *drawable;

	bool initialized;

	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()
};

#endif // __GLWIDGET_H__
