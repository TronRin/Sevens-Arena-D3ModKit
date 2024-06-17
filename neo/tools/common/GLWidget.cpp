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

#include "tools/edit_gui_common.h"
#include "../../sys/win32/win_local.h"
#include "renderer/tr_local.h"
#include "GLWidget.h"

bool Sys_KeyDown( int key ) {
	return ( ( ::GetAsyncKeyState( key ) & 0x8000 ) != 0 );
}

idGLWidget::idGLWidget()
{
	initialized = false;
	drawable = NULL;
}

idGLWidget::~idGLWidget()
{
}

BEGIN_MESSAGE_MAP(idGLWidget, CWnd)
	//{{AFX_MSG_MAP(idGLWidget)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// idGLWidget message handlers

BOOL idGLWidget::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class

	return CWnd::PreCreateWindow(cs);
}

BOOL idGLWidget::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	if (CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext) == -1) {
		return FALSE;
	}

	CDC *dc = GetDC();
	ReleaseDC(dc);

	return TRUE;

}

void idGLWidget::OnPaint()
{

	if (!initialized) {
		CDC *dc = GetDC();
		ReleaseDC(dc);
		initialized = true;
	}
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(rect);

	if (!qwglMakeCurrent(dc.m_hDC, win32.hGLRC)) {
	}

	qglViewport(0, 0, rect.Width(), rect.Height());
	qglScissor(0, 0, rect.Width(), rect.Height());
	qglMatrixMode(GL_PROJECTION);
	qglLoadIdentity();
	qglClearColor (0.4f, 0.4f, 0.4f, 0.7f);
	qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	qglDisable(GL_DEPTH_TEST);
	qglDisable(GL_BLEND);
	qglOrtho(0, rect.Width(), 0, rect.Height(), -256, 256);

	if (drawable) {
		drawable->draw(1, 1, rect.Width()-1, rect.Height()-1);
	} else {
		qglViewport(0, 0, rect.Width(), rect.Height());
		qglScissor(0, 0, rect.Width(), rect.Height());
		qglMatrixMode(GL_PROJECTION);
		qglLoadIdentity();
		qglClearColor (0.4f, 0.4f, 0.4f, 0.7f);
		qglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	qwglSwapBuffers(dc);
	qglFlush();
	qwglMakeCurrent(win32.hDC, win32.hGLRC);

}

void idGLDrawable::buttonDown(int _button, float x, float y) {
	pressX = x;
	pressY = y;
	button = _button;
	if (button == MK_RBUTTON) {
		handleMove = true;
	}
}

void idGLDrawable::buttonUp(int button, float x, float y) {
	handleMove = false;
}

void idGLDrawable::mouseMove(float x, float y) {
	if (handleMove) {
		Update();
		if (Sys_KeyDown(VK_MENU)) {
			// scale
			float *px = &x;
			float *px2 = &pressX;

			if (idMath::Diff(y, pressY) > idMath::Diff(x, pressX)) {
				px = &y;
				px2 = &pressY;
			}

			if (*px > *px2) {
				// zoom in
				scale += 0.1f;
				if ( scale > 10.0f ) {
					scale = 10.0f;
				}
			} else if (*px < *px2) {
				// zoom out
				scale -= 0.1f;
				if ( scale <= 0.001f ) {
					scale = 0.001f;
				}
			}

			*px2 = *px;
			::SetCursorPos(pressX, pressY);

		} else if (Sys_KeyDown(VK_SHIFT)) {
			// rotate
		} else {
			// origin
			if (x != pressX) {
				xOffset += (x - pressX);
				pressX = x;
			}
			if (y != pressY) {
				yOffset -= (y - pressY);
				pressY = y;
			}
			//::SetCursorPos(pressX, pressY);
		}
	}
}

void idGLDrawable::draw(int x, int y, int w, int h) {
	GL_State( GLS_DEFAULT );
	qglViewport(x, y, w, h);
	qglScissor(x, y, w, h);
	qglMatrixMode(GL_PROJECTION);
	qglClearColor( 0.1f, 0.1f, 0.1f, 0.0f );
	qglClear(GL_COLOR_BUFFER_BIT);
	qglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	qglLineWidth(0.5);
	qglColor3f(1, 1, 1);
	globalImages->BindNull();
	qglBegin(GL_LINE_LOOP);
	qglColor3f(1, 0, 0);
	qglVertex2f(x + 3, y + 3);
	qglColor3f(0, 1, 0);
	qglVertex2f(x + 3, h - 3);
	qglColor3f(0, 0, 1);
	qglVertex2f(w - 3, h - 3);
	qglColor3f(1, 1, 1);
	qglVertex2f(w - 3, y + 3);
	qglEnd();

}

void idGLWidget::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	if (drawable) {
		if ( drawable->ScreenCoords() ) {
			ClientToScreen(&point);
		}
		drawable->buttonDown(MK_LBUTTON, point.x, point.y);
	}
}

void idGLWidget::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (drawable) {
		if ( drawable->ScreenCoords() ) {
			ClientToScreen(&point);
		}
		drawable->buttonUp(MK_LBUTTON, point.x, point.y);
	}
	ReleaseCapture();
}

void idGLWidget::OnMButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	if (drawable) {
		if ( drawable->ScreenCoords() ) {
			ClientToScreen(&point);
		}
		drawable->buttonDown(MK_MBUTTON, point.x, point.y);
	}
}

void idGLWidget::OnMButtonUp(UINT nFlags, CPoint point)
{
	if (drawable) {
		if ( drawable->ScreenCoords() ) {
			ClientToScreen(&point);
		}
		drawable->buttonUp(MK_MBUTTON, point.x, point.y);
	}
	ReleaseCapture();
}

void idGLWidget::OnMouseMove(UINT nFlags, CPoint point)
{
	if (drawable) {
		if ( drawable->ScreenCoords() ) {
			ClientToScreen(&point);
		}
		drawable->mouseMove(point.x, point.y);
		RedrawWindow();
	}
}

BOOL idGLWidget::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (drawable) {
		float f = drawable->getScale();
		if ( zDelta > 0.0f ) {
			f += 0.1f;
		} else {
			f -= 0.1f;
		}
		if ( f <= 0.0f ) {
			f = 0.1f;
		}
		if ( f > 5.0f ) {
			f = 5.0f;
		}
		drawable->setScale(f);
	}
	return TRUE;
}

void idGLWidget::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	if (drawable) {
		if ( drawable->ScreenCoords() ) {
			ClientToScreen(&point);
		}
		drawable->buttonDown(MK_RBUTTON, point.x, point.y);
	}
}

void idGLWidget::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (drawable) {
		if ( drawable->ScreenCoords() ) {
			ClientToScreen(&point);
		}
		drawable->buttonUp(MK_RBUTTON, point.x, point.y);
	}
	ReleaseCapture();
}

void idGLWidget::setDrawable(idGLDrawable *d) {
	drawable = d;
	if (d->getRealTime()) {
		SetTimer(1, d->getRealTime(), NULL);
	}
}


void idGLWidget::OnTimer(UINT_PTR nIDEvent) {
	if (drawable && drawable->getRealTime()) {
		Invalidate(FALSE);
	} else {
		KillTimer(1);
	}
}


idGLDrawable::idGLDrawable() {
	scale = 1.0;
	xOffset = 0.0;
	yOffset = 0.0;
	handleMove = false;
	realTime = 0;

}

BOOL idGLWidget::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}