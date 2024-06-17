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
#include "renderer/tr_local.h"
#include "../../sys/win32/rc/resource.h"
#include "../comafx/DialogColorPicker.h"
#include "LightDlg.h"

void CLightInfo::Defaults() {
	pointLight = true;

	strTexture = "";
	equalRadius = true;
	explicitStartEnd = false;
	lightStart.Zero();
	lightEnd.Zero();
	lightUp.Zero();
	lightRight.Zero();
	lightTarget.Zero();
	lightCenter.Zero();
	color[0] = color[1] = color[2] = 1.0f;

	lightRadius.Zero();
	castShadows = true;
	skipSpecular = false;
	hasCenter = false;
	isParallel = false;

	//lightStyle = -1;
}

void CLightInfo::DefaultPoint()
{
	idVec3 oldColor = color;
	Defaults();
	color = oldColor;

	pointLight = true;
	lightRadius[0] = lightRadius[1] = lightRadius[2] = 300;
	equalRadius = true;
}

void CLightInfo::DefaultProjected()
{
	idVec3 oldColor = color;
	Defaults();
	color = oldColor;

	pointLight = false;
	lightTarget[2] = -256;
	lightUp[1] = -128;
	lightRight[0] = -128;
}

void CLightInfo::DefaultSun()
{
	idVec3 oldColor = color;
	Defaults();
	color = oldColor;

	isParallel = true;
	hasCenter = true;
	lightCenter.Set( 4, 4, 32 );
	lightRadius[0] = lightRadius[1] = 2048;
	lightRadius[2] = 1024;
	equalRadius = false;
}

void CLightInfo::FromDict( const idDict *e ) {

	e->GetVector( "origin", "", origin );

	lightRadius.Zero();
	lightTarget.Zero();
	lightRight.Zero();
	lightUp.Zero();
	lightStart.Zero();
	lightEnd.Zero();
	lightCenter.Zero();

	castShadows = !e->GetBool( "noshadows" );
	skipSpecular = e->GetBool( "nospecular" );

	strTexture = e->GetString( "texture" );

	bool isParallel = e->GetBool( "parallel" );

	if ( !e->GetVector("_color", "", color ) ) {
		color[0] = color[1] = color[2] = 1;
	}

	// windows needs 0-255 scale
	color[0] *= 255;
	color[1] *= 255;
	color[2] *= 255;

	if ( e->GetVector( "light_right", "", lightRight ) ) {
		// projected light
		pointLight = false;

		e->GetVector( "light_target", "", lightTarget );
		e->GetVector( "light_up", "", lightUp );

		if ( e->GetVector( "light_start", "", lightStart ) ) {
			// explicit start and end points
			explicitStartEnd = true;

			if ( !e->GetVector( "light_end", "", lightEnd ) ) {
				// no end, use target
				lightEnd = lightTarget;
			}
		} else {
			explicitStartEnd = false;

			// create a start a quarter of the way to the target
			lightStart = lightTarget * 0.25;
			lightEnd = lightTarget;
		}
	} else {
		pointLight = true;

		if ( e->GetVector( "light_radius", "", lightRadius ) ) {
			equalRadius = ( lightRadius.x == lightRadius.y && lightRadius.x == lightRadius.z );
		} else {
			float radius = e->GetFloat( "light" );
			if ( radius == 0 ) {
				radius = 300;
			}

			lightRadius[0] = lightRadius[1] = lightRadius[2] = radius;
			equalRadius = true;
		}

		if ( e->GetVector( "light_center", "", lightCenter ) ) {
			hasCenter = true;
		}
	}
}

void CLightInfo::ToDictFromDifferences ( idDict *e, const idDict *differences ) {
	for ( int i = 0 ; i < differences->GetNumKeyVals () ; i ++ ) {
		const idKeyValue *kv = differences->GetKeyVal( i );

		if ( kv->GetValue().Length() > 0 ) {
			e->Set ( kv->GetKey() ,kv->GetValue() );
		} else {
			e->Delete ( kv->GetKey() );
		}

		common->Printf( "Applied difference: %s %s\n" , kv->GetKey().c_str() , kv->GetValue().c_str() );
	}
}

//write all info to a dict, regardless of light type
void CLightInfo::ToDictWriteAllInfo( idDict *e ) {
	e->SetVector( "origin", origin );

	// idGameEdit::EntityChangeSpawnArgs() will delete key/value from entity,
	// if value is "" => use DELETE_VAL for readability
	static const char* DELETE_VAL = "";

	e->Set( "light", DELETE_VAL ); // we always use "light_radius" instead

	e->Set( "noshadows", ( !castShadows ) ? "1" : "0" );
	e->Set( "nospecular", ( skipSpecular ) ? "1" : "0" );

	if ( strTexture.GetLength() > 0 ) {
		e->Set( "texture", strTexture );
	} else {
		e->Set( "texture", DELETE_VAL );
	}

	idVec3 temp = color;
	temp /= 255;
	e->SetVector( "_color", temp );

	if ( !equalRadius ) {
		e->SetVector( "light_radius", lightRadius );
	} else {
		idVec3 tmp( lightRadius[0], lightRadius[0], lightRadius[0] ); // x, y and z have the same value
		e->SetVector( "light_radius", tmp );
	}

	if ( hasCenter ) {
		e->SetVector( "light_center", lightCenter );
	} else {
		e->Set( "light_center", DELETE_VAL );
	}

	if ( isParallel ) {
		e->Set( "parallel", "1" );
	}

	e->SetVector( "light_target", lightTarget );
	e->SetVector( "light_up", lightUp );
	e->SetVector( "light_right", lightRight );
	e->SetVector( "light_start", lightStart );
	e->SetVector( "light_end", lightEnd );

	/*
	if ( lightStyle != -1 && lightType != LIGHT_SUN ) {
		e->SetInt( "style", lightStyle );
	} else {
		e->Set( "style", DELETE_VAL );
	}
	*/
}

void CLightInfo::ToDict( idDict *e ) {

	e->SetVector( "origin", origin );

	// idGameEdit::EntityChangeSpawnArgs() will delete key/value from entity,
	// if value is "" => use DELETE_VAL for readability
	static const char* DELETE_VAL = "";

	e->Set( "light", DELETE_VAL ); // we always use "light_radius" instead

	e->Set( "noshadows", ( !castShadows ) ? "1" : "0" );
	e->Set( "nospecular", ( skipSpecular ) ? "1" : "0" );

	if ( strTexture.GetLength() > 0 ) {
		e->Set( "texture", strTexture );
	} else {
		e->Set( "texture", DELETE_VAL );
	}

	idVec3 temp = color;
	temp /= 255;
	e->SetVector( "_color", temp );

	if ( pointLight ) {
		if ( !equalRadius ) {
			e->SetVector( "light_radius", lightRadius );
		} else {
			idVec3 tmp( lightRadius[0], lightRadius[0], lightRadius[0] ); // x, y and z have the same value
			e->SetVector( "light_radius", tmp );
		}

		if ( hasCenter ) {
			e->SetVector( "light_center", lightCenter );
		} else {
			e->Set( "light_center", DELETE_VAL );
		}

		if ( isParallel ) {
			e->Set( "parallel", "1" );
		}

		// get rid of all the projected light specific stuff
		e->Set( "light_target", DELETE_VAL );
		e->Set( "light_up", DELETE_VAL );
		e->Set( "light_right", DELETE_VAL );
		e->Set( "light_start", DELETE_VAL );
		e->Set( "light_end", DELETE_VAL );
	} else {
		e->SetVector( "light_target", lightTarget );
		e->SetVector( "light_up", lightUp );
		e->SetVector( "light_right", lightRight );

		if ( explicitStartEnd ) {
			e->SetVector( "light_start", lightStart );
			e->SetVector( "light_end", lightEnd );
		}
		else
		{
			e->Set( "light_start", DELETE_VAL );
			e->Set( "light_end", DELETE_VAL );
		}

		// get rid of the pointlight specific stuff
		e->Set( "light_radius", DELETE_VAL );
		e->Set( "light_center", DELETE_VAL );
		e->Set( "parallel", DELETE_VAL );
	}

	/*
	if ( lightStyle != -1 && lightType != LIGHT_SUN ) {
		e->SetInt( "style", lightStyle );
	} else {
		e->Set( "style", DELETE_VAL );
	}
	*/
}

CLightInfo::CLightInfo() {
	Defaults();
}

CLightDlg *g_LightDialog = NULL;

CLightDlg::CLightDlg( CWnd* pParent ) : CDialog( CLightDlg::IDD, pParent )
{
	m_bEqualRadius = FALSE;
	m_bExplicitFalloff = FALSE;
	m_bPointLight = FALSE;
	m_bCheckProjected = FALSE;
	m_bShadows = FALSE;
	m_bSkipSpecular = FALSE;

	m_fEndX = 0.0f;
	m_fEndY = 0.0f;
	m_fEndZ = 0.0f;

	m_fRadiusX = 0.0f;
	m_fRadiusY = 0.0f;
	m_fRadiusZ = 0.0f;

	m_fRightX = 0.0f;
	m_fRightY = 0.0f;
	m_fRightZ = 0.0f;

	m_fRotate = 0.0f;

	m_fStartX = 0.0f;
	m_fStartY = 0.0f;
	m_fStartZ = 0.0f;

	m_fTargetX = 0.0f;
	m_fTargetY = 0.0f;
	m_fTargetZ = 0.0f;

	m_fUpX = 0.0f;
	m_fUpY = 0.0f;
	m_fUpZ = 0.0f;

	m_hasCenter = FALSE;

	m_centerX = 0.0f;
	m_centerY = 0.0f;
	m_centerZ = 0.0f;

	m_bIsParallel = FALSE;
}

CLightDlg::~CLightDlg() {
}

void CLightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_TEXTURE, m_wndLights);
	DDX_Check(pDX, IDC_CHECK_EQUALRADIUS, m_bEqualRadius);
	DDX_Check(pDX, IDC_CHECK_EXPLICITFALLOFF, m_bExplicitFalloff);
	DDX_Check(pDX, IDC_CHECK_POINT, m_bPointLight);
	DDX_Check(pDX, IDC_CHECK_PROJECTED, m_bCheckProjected);
	DDX_Check(pDX, IDC_CHECK_SHADOWS, m_bShadows);
	DDX_Check(pDX, IDC_CHECK_SPECULAR, m_bSkipSpecular);
	DDX_Check(pDX , IDC_CHECK_PARALLEL , m_bIsParallel );
	DDX_Text(pDX, IDC_EDIT_ENDX, m_fEndX);
	DDX_Text(pDX, IDC_EDIT_ENDY, m_fEndY);
	DDX_Text(pDX, IDC_EDIT_ENDZ, m_fEndZ);
	DDX_Text(pDX, IDC_EDIT_RADIUSX, m_fRadiusX);
	DDX_Text(pDX, IDC_EDIT_RADIUSY, m_fRadiusY);
	DDX_Text(pDX, IDC_EDIT_RADIUSZ, m_fRadiusZ);
	DDX_Text(pDX, IDC_EDIT_RIGHTX, m_fRightX);
	DDX_Text(pDX, IDC_EDIT_RIGHTY, m_fRightY);
	DDX_Text(pDX, IDC_EDIT_RIGHTZ, m_fRightZ);
	DDX_Text(pDX, IDC_EDIT_STARTX, m_fStartX);
	DDX_Text(pDX, IDC_EDIT_STARTY, m_fStartY);
	DDX_Text(pDX, IDC_EDIT_STARTZ, m_fStartZ);
	DDX_Text(pDX, IDC_EDIT_TARGETX, m_fTargetX);
	DDX_Text(pDX, IDC_EDIT_TARGETY, m_fTargetY);
	DDX_Text(pDX, IDC_EDIT_TARGETZ, m_fTargetZ);
	DDX_Text(pDX, IDC_EDIT_UPX, m_fUpX);
	DDX_Text(pDX, IDC_EDIT_UPY, m_fUpY);
	DDX_Text(pDX, IDC_EDIT_UPZ, m_fUpZ);
	DDX_Check(pDX, IDC_CHECK_CENTER, m_hasCenter);
	DDX_Text(pDX, IDC_EDIT_CENTERX, m_centerX);
	DDX_Text(pDX, IDC_EDIT_CENTERY, m_centerY);
	DDX_Text(pDX, IDC_EDIT_CENTERZ, m_centerZ);
}

BEGIN_MESSAGE_MAP( CLightDlg, CDialog )
	ON_BN_CLICKED( IDC_BTN_SAVEMAP, OnBtnSavemap )
	ON_BN_CLICKED( IDC_BTN_SAVEMAPAS, OnBtnSavemapas )
	ON_BN_CLICKED( IDC_CHECK_EQUALRADIUS, OnCheckEqualradius )
	ON_BN_CLICKED( IDC_CHECK_EXPLICITFALLOFF, OnCheckExplicitfalloff )
	ON_BN_CLICKED( IDC_CHECK_POINT, OnCheckPoint )
	ON_BN_CLICKED( IDC_CHECK_PROJECTED, OnCheckProjected )
	ON_BN_CLICKED( IDC_APPLY, OnApply)
	ON_BN_CLICKED( IDC_APPLY_DIFFERENT, OnApplyDifferences )
	ON_BN_CLICKED( IDC_BTN_COLOR, OnBtnColor )
	ON_BN_CLICKED( IDC_BTN_YUP, OnBtnYup )
	ON_BN_CLICKED( IDC_BTN_YDN, OnBtnYdn )
	ON_BN_CLICKED( IDC_BTN_XDN, OnBtnXdn )
	ON_BN_CLICKED( IDC_BTN_XUP, OnBtnXup )
	ON_BN_CLICKED( IDC_BTN_ZUP, OnBtnZup )
	ON_BN_CLICKED( IDC_BTN_ZDN, OnBtnZdn )
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE( IDC_COMBO_TEXTURE, OnSelchangeComboTexture )
	ON_BN_CLICKED( IDC_CHECK_CENTER, OnCheckCenter )
	ON_BN_CLICKED(IDC_CHECK_PARALLEL, OnCheckParallel )
END_MESSAGE_MAP()

void CLightDlg::SetSpecifics() {
	if ( lightInfo.pointLight ) {
		GetDlgItem( IDC_EDIT_RADIUSY )->EnableWindow( !lightInfo.equalRadius );
		GetDlgItem( IDC_EDIT_RADIUSZ )->EnableWindow( !lightInfo.equalRadius );
		GetDlgItem( IDC_EDIT_CENTERX )->EnableWindow( lightInfo.hasCenter );
		GetDlgItem( IDC_EDIT_CENTERY )->EnableWindow( lightInfo.hasCenter );
		GetDlgItem( IDC_EDIT_CENTERZ )->EnableWindow( lightInfo.hasCenter );
	} else {
		GetDlgItem( IDC_EDIT_STARTX )->EnableWindow( lightInfo.explicitStartEnd );
		GetDlgItem( IDC_EDIT_STARTY )->EnableWindow( lightInfo.explicitStartEnd );
		GetDlgItem( IDC_EDIT_STARTZ )->EnableWindow( lightInfo.explicitStartEnd );
		GetDlgItem( IDC_EDIT_ENDX )->EnableWindow( lightInfo.explicitStartEnd );
		GetDlgItem( IDC_EDIT_ENDY )->EnableWindow( lightInfo.explicitStartEnd );
		GetDlgItem( IDC_EDIT_ENDZ )->EnableWindow( lightInfo.explicitStartEnd );
	}
}

void CLightDlg::EnableControls() {
	bool bLigthType = lightInfo.pointLight;

	GetDlgItem( IDC_CHECK_EQUALRADIUS )->EnableWindow( bLigthType );
	GetDlgItem( IDC_EDIT_RADIUSX )->EnableWindow( bLigthType );
	GetDlgItem( IDC_EDIT_RADIUSY )->EnableWindow( bLigthType );
	GetDlgItem( IDC_EDIT_RADIUSZ )->EnableWindow( bLigthType );
	GetDlgItem( IDC_EDIT_TARGETX )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_TARGETY )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_TARGETZ )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_RIGHTX )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_RIGHTY )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_RIGHTZ )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_UPX )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_UPY )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_UPZ )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_STARTX )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_STARTY )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_STARTZ )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_ENDX )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_ENDY )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_EDIT_ENDZ )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_CHECK_EXPLICITFALLOFF )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_CHECK_POINT )->EnableWindow( !bLigthType );
	GetDlgItem( IDC_CHECK_PROJECTED )->EnableWindow( bLigthType );
	GetDlgItem( IDC_EDIT_CENTERX )->EnableWindow( bLigthType );
	GetDlgItem( IDC_EDIT_CENTERY )->EnableWindow( bLigthType );
	GetDlgItem( IDC_EDIT_CENTERZ )->EnableWindow( bLigthType );
	GetDlgItem( IDC_CHECK_CENTER )->EnableWindow( bLigthType );

	reinterpret_cast<CButton*>( GetDlgItem( IDC_CHECK_PROJECTED ) )->SetCheck( !bLigthType );
	reinterpret_cast<CButton*>( GetDlgItem( IDC_CHECK_POINT ) )->SetCheck( bLigthType );

	SetSpecifics();
}

void CLightDlg::UpdateDialogFromLightInfo( void ) {
	m_hasCenter = lightInfo.hasCenter;
	m_bEqualRadius = lightInfo.equalRadius;
	m_bExplicitFalloff = lightInfo.explicitStartEnd;
	m_bPointLight = lightInfo.pointLight;
	m_bCheckProjected = !lightInfo.pointLight;

	m_bShadows = lightInfo.castShadows;
	m_bSkipSpecular = lightInfo.skipSpecular;

	int sel = m_wndLights.FindStringExact( -1, lightInfo.strTexture );
	m_wndLights.SetCurSel( sel );

	m_fEndX = lightInfo.lightEnd[0];
	m_fEndY = lightInfo.lightEnd[1];
	m_fEndZ = lightInfo.lightEnd[2];

	m_fRadiusX = lightInfo.lightRadius[0];
	m_fRadiusY = lightInfo.lightRadius[1];
	m_fRadiusZ = lightInfo.lightRadius[2];

	m_fRightX = lightInfo.lightRight[0];
	m_fRightY = lightInfo.lightRight[1];
	m_fRightZ = lightInfo.lightRight[2];

	m_fStartX = lightInfo.lightStart[0];
	m_fStartY = lightInfo.lightStart[1];
	m_fStartZ = lightInfo.lightStart[2];

	m_fTargetX = lightInfo.lightTarget[0];
	m_fTargetY = lightInfo.lightTarget[1];
	m_fTargetZ = lightInfo.lightTarget[2];

	m_fUpX = lightInfo.lightUp[0];
	m_fUpY = lightInfo.lightUp[1];
	m_fUpZ = lightInfo.lightUp[2];

	VectorCopy(lightInfo.color, color);

	m_centerX = lightInfo.lightCenter[0];
	m_centerY = lightInfo.lightCenter[1];
	m_centerZ = lightInfo.lightCenter[2];

	//jhefty - added parallel light updating
	m_bIsParallel = lightInfo.isParallel;

	UpdateData( FALSE );
}

void CLightDlg::UpdateLightInfoFromDialog( void ) {
	UpdateData( TRUE );

	lightInfo.pointLight = ( m_bPointLight != FALSE );
	lightInfo.equalRadius = ( m_bEqualRadius != FALSE );
	lightInfo.explicitStartEnd = ( m_bExplicitFalloff != FALSE );

	lightInfo.castShadows = ( m_bShadows != FALSE );
	lightInfo.skipSpecular = ( m_bSkipSpecular != FALSE );

	VectorCopy( color, lightInfo.color );
	lightInfo.isParallel = ( m_bIsParallel == TRUE );

	int sel = m_wndLights.GetCurSel();
	CString str("");
	if ( sel >= 0 ) {
		m_wndLights.GetLBText( sel, str );
	}
	lightInfo.strTexture = str;

	lightInfo.lightEnd[0] = m_fEndX;
	lightInfo.lightEnd[1] = m_fEndY;
	lightInfo.lightEnd[2] = m_fEndZ;

	lightInfo.lightRadius[0] = m_fRadiusX;
	lightInfo.lightRadius[1] = m_fRadiusY;
	lightInfo.lightRadius[2] = m_fRadiusZ;

	lightInfo.lightRight[0] = m_fRightX;
	lightInfo.lightRight[1] = m_fRightY;
	lightInfo.lightRight[2] = m_fRightZ;

	lightInfo.lightStart[0] = m_fStartX;
	lightInfo.lightStart[1] = m_fStartY;
	lightInfo.lightStart[2] = m_fStartZ;

	lightInfo.lightTarget[0] = m_fTargetX;
	lightInfo.lightTarget[1] = m_fTargetY;
	lightInfo.lightTarget[2] = m_fTargetZ;

	lightInfo.lightUp[0] = m_fUpX;
	lightInfo.lightUp[1] = m_fUpY;
	lightInfo.lightUp[2] = m_fUpZ;

	lightInfo.hasCenter = ( m_hasCenter != FALSE );

	lightInfo.lightCenter[0] = m_centerX;
	lightInfo.lightCenter[1] = m_centerY;
	lightInfo.lightCenter[2] = m_centerZ;
}

void CLightDlg::SaveLightInfo( const idDict *differences ) {

	idList<idEntity *> list;

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	for ( int i = 0; i < count; i++ ) {
		if ( differences ) {
			gameEdit->EntityChangeSpawnArgs( list[i], differences );
			gameEdit->EntityUpdateChangeableSpawnArgs( list[i], NULL );
		} else {
			idDict newArgs;
			lightInfo.ToDict( &newArgs );
			gameEdit->EntityChangeSpawnArgs( list[i], &newArgs );
			gameEdit->EntityUpdateChangeableSpawnArgs( list[i], NULL );
		}
		gameEdit->EntityUpdateVisuals( list[i] );
	}
}

void CLightDlg::ColorButtons() {
	CRect r;

	CClientDC dc(this);

	CButton *pBtn = (CButton *)GetDlgItem(IDC_BTN_COLOR);
	pBtn->GetClientRect(&r);
	colorBitmap.DeleteObject();
	colorBitmap.CreateCompatibleBitmap(&dc, r.Width(), r.Height());
	CDC MemDC;
	MemDC.CreateCompatibleDC(&dc);
	CBitmap *pOldBmp = MemDC.SelectObject(&colorBitmap);
	{
		CBrush br(RGB(color[0], color[1], color[2]));
		MemDC.FillRect(r,&br);
	}
	dc.SelectObject(pOldBmp);
	pBtn->SetBitmap(HBITMAP(colorBitmap));
}


void CLightDlg::LoadLightTextures() {
	int count = declManager->GetNumDecls( DECL_MATERIAL );

	for ( int i = 0; i < count; i++ ) {
		// just get the name of the light material
		const idMaterial* mat = declManager->MaterialByIndex( i, false );

		idStr matName = mat->GetName();
		matName.ToLower();

		if ( matName.Icmpn( "lights/", strlen( "lights/" ) ) == 0 ){
			m_wndLights.AddString( mat->GetName() );
		}
	}
}

BOOL CLightDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	com_editors |= EDITOR_LIGHT;

	UpdateDialog( true );

	LoadLightTextures();

	return TRUE;
}

void CLightDlg::OnDestroy() {

	com_editors &= ~EDITOR_LIGHT;

	return CDialog::OnDestroy();
}

void CLightDlg::OnCheckEqualradius()
{
	lightInfo.equalRadius = ( reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_EQUALRADIUS))->GetCheck() != 0 );
	SetSpecifics();
}

void CLightDlg::OnCheckExplicitfalloff()
{
	lightInfo.explicitStartEnd = ( reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_EXPLICITFALLOFF))->GetCheck() != 0 );
	SetSpecifics();
}

void CLightDlg::OnCheckPoint()
{
	lightInfo.DefaultPoint();
	UpdateDialogFromLightInfo();
	EnableControls();
}

void CLightDlg::OnCheckProjected()
{
	lightInfo.DefaultProjected();
	UpdateDialogFromLightInfo();
	EnableControls();
}

void CLightDlg::OnOK() {
	UpdateLightInfoFromDialog();
	SaveLightInfo( NULL );

	// turn off light debug drawing in the render backend
	r_singleLight.SetInteger( -1 );
	r_showLights.SetInteger( 0 );

	CDialog::OnOK();
}

void CLightDlg::UpdateDialog( bool updateChecks )
{
	CString title;

	lightInfo.Defaults();
	lightInfoOriginal.Defaults ();

	idList<idEntity *> list;

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count > 0 ) {
		lightInfo.FromDict( gameEdit->EntityGetSpawnArgs( list[count-1] ) );
		title = "Light Editor";
	} else {
		title = "Light Editor - (No ligths selected)";
	}

	SetWindowText( title );

	UpdateDialogFromLightInfo();
	ColorButtons();

	if ( updateChecks ) {
		EnableControls();
	}
}

void CLightDlg::OnApply() {
	UpdateLightInfoFromDialog();
	SaveLightInfo( NULL );
}

//jhefty - only apply settings that are different
void CLightDlg::OnApplyDifferences () {
	idDict differences, modifiedlight, originallight;

	UpdateLightInfoFromDialog();

	lightInfo.ToDict( &modifiedlight);
	lightInfoOriginal.ToDictWriteAllInfo( &originallight);

	differences = modifiedlight;

	// jhefty - compile a set of modified values to apply
	for ( int i = 0; i < modifiedlight.GetNumKeyVals (); i ++ ) {
		const idKeyValue* valModified = modifiedlight.GetKeyVal ( i );
		const idKeyValue* valOriginal = originallight.FindKey ( valModified->GetKey() );

		//if it hasn't changed, remove it from the list of values to apply
		if ( !valOriginal || ( valModified->GetValue() == valOriginal->GetValue() ) ) {
			differences.Delete ( valModified->GetKey() );
		}
	}

	SaveLightInfo( &differences );

	lightInfoOriginal.FromDict( &modifiedlight );
}

void CLightDlg::OnBtnSavemap()
{
	OnApplyDifferences();
	gameEdit->MapSave();
}

void CLightDlg::OnBtnSavemapas()
{
	CFileDialog dlgSave( FALSE,"map",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"Map Files (*.map)|*.map||",AfxGetMainWnd() );
	if ( dlgSave.DoModal() == IDOK ) {
		OnApplyDifferences();
		idStr work;
		work = fileSystem->OSPathToRelativePath( dlgSave.m_ofn.lpstrFile );
		gameEdit->MapSave( work );
	}
}

void UpdateLightDialog( float r, float g, float b, float a ) {
	g_LightDialog->UpdateColor( r, g, b, a );
}

void CLightDlg::UpdateColor( float r, float g, float b, float a ) {
	color[0] = a * r;
	color[1] = a * g;
	color[2] = a * b;
	ColorButtons();
	UpdateLightInfoFromDialog();
	SaveLightInfo( NULL );}

void CLightDlg::OnBtnColor() {
	int r, g, b;
	float ob;
	r = color[0];
	g = color[1];
	b = color[2];
	if ( DoColor( &r, &g, &b, &ob, UpdateLightDialog ) ) {
		color[0] = ob * r;
		color[1] = ob * g;
		color[2] = ob * b;
		ColorButtons();
	}
}

void CLightDlg::OnCancel() {
	// turn off light debug drawing in the render backend
	r_singleLight.SetInteger( -1 );
	r_showLights.SetInteger( 0 );

	CDialog::OnCancel();
}

HBRUSH CLightDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	return hbr;
}

BOOL CLightDlg::DestroyWindow()
{
	if (GetSafeHwnd())
	{
		CRect rct;
		GetWindowRect(rct);
	}
	return CDialog::DestroyWindow();
}

void CLightDlg::OnSelchangeComboTexture()
{
	UpdateData(TRUE);
	int sel = m_wndLights.GetCurSel();
	CString str;
	if (sel >= 0) {
		m_wndLights.GetLBText(sel, str);
	}
}

void CLightDlg::OnCheckCenter()
{
	if (reinterpret_cast<CButton*>(GetDlgItem(IDC_CHECK_CENTER))->GetCheck()) {
		lightInfo.hasCenter = true;
		lightInfo.lightCenter.x = 0;
		lightInfo.lightCenter.y = 0;
		lightInfo.lightCenter.z = 32;
	} else {
		lightInfo.hasCenter = false;
		lightInfo.lightCenter.Zero();
	}
	UpdateDialogFromLightInfo();
	SetSpecifics();
}

void CLightDlg::OnCheckParallel() {
	if ( reinterpret_cast<CButton*>( GetDlgItem( IDC_CHECK_PARALLEL ) )->GetCheck() ) {
		lightInfo.DefaultSun();
	} else {
		lightInfo.isParallel = false;
		lightInfo.hasCenter = false;
	}

	UpdateDialogFromLightInfo();
	SetSpecifics();
}

void CLightDlg::UpdateSelectedOrigin( float x, float y, float z ) {
	idList<idEntity*> list;
	idVec3 origin;
	idVec3 vec(x, y, z);

	list.SetNum( 128 );
	int count = gameEdit->GetSelectedEntities( list.Ptr(), list.Num() );
	list.SetNum( count );

	if ( count ) {
		for ( int i = 0; i < count; i++ ) {
			const idDict *dict = gameEdit->EntityGetSpawnArgs( list[i] );
			if ( dict == NULL ) {
				continue;
			}
			const char *name = dict->GetString( "name" );
			gameEdit->EntityTranslate( list[i], vec );
			gameEdit->EntityUpdateVisuals( list[i] );
			gameEdit->MapEntityTranslate( name, vec );
		}
	}
}

void CLightDlg::OnBtnYup()
{
	UpdateSelectedOrigin(0, 8, 0);
}

void CLightDlg::OnBtnYdn()
{
	UpdateSelectedOrigin(0, -8, 0);
}

void CLightDlg::OnBtnXdn()
{
	UpdateSelectedOrigin(-8, 0, 0);
}

void CLightDlg::OnBtnXup()
{
	UpdateSelectedOrigin(8, 0, 0);
}

void CLightDlg::OnBtnZup()
{
	UpdateSelectedOrigin(0, 0, 8);
}

void CLightDlg::OnBtnZdn()
{
	UpdateSelectedOrigin(0, 0, -8);
}

void LightEditorInit( const idDict *spawnArgs ) {
	if ( renderSystem->IsFullScreen() ) {
		common->Printf( "Cannot run the light editor in fullscreen mode.\n"
					"Set r_fullscreen to 0 and vid_restart.\n" );
		return;
	}

	if ( g_LightDialog == NULL ) {
		InitAfx();
		g_LightDialog = new CLightDlg();
	}

	if ( g_LightDialog->GetSafeHwnd() == NULL ) {
		g_LightDialog->Create( IDD_DIALOG_LIGHT );
	}

	idKeyInput::ClearStates();

	g_LightDialog->ShowWindow( SW_SHOW );
	g_LightDialog->SetFocus();
	g_LightDialog->UpdateDialog( true );

	if ( spawnArgs ) {
		// FIXME: select light based on spawn args
	}
}

void LightEditorRun( void ) {
#if _MSC_VER >= 1300
	MSG *msg = AfxGetCurrentMessage();			// TODO Robert fix me!!
#else
	MSG *msg = &m_msgCur;
#endif

	while( ::PeekMessage(msg, NULL, NULL, NULL, PM_NOREMOVE) ) {
		// pump message
		if ( !AfxGetApp()->PumpMessage() ) {
		}
	}
}

void LightEditorShutdown( void ) {
	delete g_LightDialog;
	g_LightDialog = NULL;
}
