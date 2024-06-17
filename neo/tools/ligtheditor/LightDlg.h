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
#ifndef __LIGHTDLG_H__
#define __LIGHTDLG_H__

#include "../common/GLWidget.h"

class CLightInfo {
public:
				CLightInfo();

	bool		pointLight;
	CString		strTexture;
	bool		equalRadius;
	bool		explicitStartEnd;
	idVec3		lightStart;
	idVec3		lightEnd;
	idVec3		lightUp;
	idVec3		lightRight;
	idVec3		lightTarget;
	idVec3		lightCenter;
	idVec3		color;

	idVec3		origin;

	idVec3		lightRadius;
	bool		castShadows;
	bool		skipSpecular;
	bool		hasCenter;
	bool		isParallel;
	//int			lightStyle;		// saved to map as "style"

	void		Defaults();
	void		DefaultPoint();
	void		DefaultProjected();
	void		DefaultSun();
	void		FromDict( const idDict *e );
	void		ToDict( idDict *e );
	void		ToDictFromDifferences( idDict *e, const idDict *differences );
	void		ToDictWriteAllInfo( idDict *e );
};

/////////////////////////////////////////////////////////////////////////////
// CLightDlg dialog

class CLightDlg : public CDialog {
public:
					CLightDlg(CWnd* pParent = NULL);   // standard constructor
					~CLightDlg();

	void			UpdateDialogFromLightInfo( void );
	void			UpdateDialog( bool updateChecks );
	void			UpdateLightInfoFromDialog( void );
	void			UpdateColor( float r, float g, float b, float a );
	void			SetSpecifics();
	void			EnableControls();
	void			LoadLightTextures();
	void			ColorButtons();
	void			SaveLightInfo( const idDict *differences );
	void			UpdateSelectedOrigin( float x, float y, float z );

	enum { IDD = IDD_DIALOG_LIGHT };
	CComboBox	m_wndLights;
	CSliderCtrl	m_wndFalloff;
	BOOL	m_bEqualRadius;
	BOOL	m_bExplicitFalloff;
	BOOL	m_bPointLight;
	BOOL	m_bCheckProjected;
	BOOL	m_bShadows;
	BOOL	m_bSkipSpecular;

	float	m_fEndX;
	float	m_fEndY;
	float	m_fEndZ;
	float	m_fRadiusX;
	float	m_fRadiusY;
	float	m_fRadiusZ;
	float	m_fRightX;
	float	m_fRightY;
	float	m_fRightZ;
	float	m_fRotate;
	float	m_fStartX;
	float	m_fStartY;
	float	m_fStartZ;
	float	m_fTargetX;
	float	m_fTargetY;
	float	m_fTargetZ;
	float	m_fUpX;
	float	m_fUpY;
	float	m_fUpZ;
	BOOL	m_hasCenter;
	float	m_centerX;
	float	m_centerY;
	float	m_centerZ;
	BOOL    m_bIsParallel;

public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:

	virtual BOOL OnInitDialog();

	afx_msg void OnDestroy();
	afx_msg void OnCheckEqualradius();
	afx_msg void OnCheckExplicitfalloff();
	afx_msg void OnCheckPoint();
	afx_msg void OnCheckProjected();
	virtual void OnOK();
	afx_msg void OnApply();
	afx_msg void OnBtnColor();
	virtual void OnCancel();
	afx_msg void OnBtnYup();
	afx_msg void OnBtnYdn();
	afx_msg void OnBtnXdn();
	afx_msg void OnBtnXup();
	afx_msg void OnBtnZup();
	afx_msg void OnBtnZdn();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelchangeComboTexture();
	afx_msg void OnCheckCenter();
	afx_msg void OnCheckParallel();
	afx_msg void OnApplyDifferences();
	afx_msg void OnBtnSavemap();
	afx_msg void OnBtnSavemapas();

	DECLARE_MESSAGE_MAP()

private:
	CBitmap					colorBitmap;
	CBitmap					fogBitmap;
	CLightInfo				lightInfo;
	CLightInfo				lightInfoOriginal;
	idVec3					color;
};

extern CLightDlg* g_LightDialog;

#endif // __LIGHTDLG_H__
