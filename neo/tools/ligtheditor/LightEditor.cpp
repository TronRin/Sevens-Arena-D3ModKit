/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2015 Daniel Gibson
Copyright (C) 2020-2023 Robert Beckebans

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

#include "precompiled.h"
#pragma hdrstop

#include "LightEditor.h"

#include "renderer/Material.h"
#include "renderer/Image.h"

namespace ImGuiTools
{

void LightInfo::Defaults()
{
	lightType = LIGHT_POINT;

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
}

void LightInfo::DefaultPoint()
{
	idVec3 oldColor = color;
	Defaults();
	color = oldColor;
	lightType = LIGHT_POINT;
	lightRadius[0] = lightRadius[1] = lightRadius[2] = 300;
	equalRadius = true;
}

void LightInfo::DefaultProjected()
{
	idVec3 oldColor = color;
	Defaults();
	color = oldColor;

	lightType = LIGHT_SPOT;
	lightTarget[2] = -256;
	lightUp[1] = -128;
	lightRight[0] = -128;
}

void LightInfo::DefaultSun()
{
	idVec3 oldColor = color;
	Defaults();
	color = oldColor;

	lightType = LIGHT_SUN;
	lightCenter.Set( 4, 4, 32 );
	lightRadius[0] = lightRadius[1] = 2048;
	lightRadius[2] = 1024;
	equalRadius = false;
}

void LightInfo::FromDict( const idDict* e )
{
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

	if ( !e->GetVector( "_color", "", color ) ) {
		// NOTE: like the game, imgui uses color values between 0.0 and 1.0
		//       even though it displays them as 0 to 255
		color[0] = color[1] = color[2] = 1.0f;
	}

	if ( e->GetVector( "light_right","", lightRight ) ) {
		// projected light
		lightType = LIGHT_SPOT;
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
		lightType = isParallel ? LIGHT_SUN : LIGHT_POINT;
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

// the returned idDict is supposed to be used by idGameEdit::EntityChangeSpawnArgs()
// and thus will contain pairs with value "" if the key should be removed from entity
void LightInfo::ToDict( idDict *e ) {
	e->SetVector( "origin", origin );

	// idGameEdit::EntityChangeSpawnArgs() will delete key/value from entity,
	// if value is "" => use DELETE_VAL for readability
	static const char* DELETE_VAL = "";

	e->Set( "light", DELETE_VAL ); // we always use "light_radius" instead

	e->Set( "noshadows", ( !castShadows ) ? "1" : "0" );
	e->Set( "nospecular", ( skipSpecular ) ? "1" : "0" );

	if ( strTexture.Length() > 0 ) {
		e->Set( "texture", strTexture );
	} else {
		e->Set( "texture", DELETE_VAL );
	}

	// NOTE: e->SetVector() uses precision of 2, not enough for color
	e->Set( "_color", color.ToString( 4 ) );

	if ( lightType == LIGHT_POINT || lightType == LIGHT_SUN ) {
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

		if ( lightType == LIGHT_SUN ) {
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
		} else {
			e->Set( "light_start", DELETE_VAL );
			e->Set( "light_end", DELETE_VAL );
		}

		// get rid of the pointlight specific stuff
		e->Set( "light_radius", DELETE_VAL );
		e->Set( "light_center", DELETE_VAL );
		e->Set( "parallel", DELETE_VAL );
	}
}

LightInfo::LightInfo()
{
	Defaults();
}


// ########### LightEditor #############

LightEditor& LightEditor::Instance()
{
	static LightEditor instance;
	return instance;
}


// static
void LightEditor::ReInit( const idDict* dict, idEntity* light )
{
	Instance().Init( dict, light );
}

void LightEditor::Init( const idDict* dict, idEntity* light )
{
	Reset();

	if( textureNames.Num() == 0 )
	{
		LoadLightTextures();
	}

	if( dict )
	{
		original.FromDict( dict );
		cur.FromDict( dict );

		gameEdit->EntityGetOrigin( light, entityPos );

		const char* name = dict->GetString( "name", NULL );
		if( name )
		{
			entityName = name;
			//title.Format( "Light Editor: %s at (%s)", name, entityPos.ToString() );
		}
		else
		{
			//idassert( 0 && "LightEditor::Init(): Given entity has no 'name' property?!" );
			entityName = gameEdit->GetUniqueEntityName( "light" );

			//title.Format( "Light Editor: <unnamed> light at (%s)", entityPos.ToString() );
		}

		title = "Light Editor";

		currentTextureIndex = 0;
		currentTexture = NULL;
		if( original.strTexture.Length() > 0 )
		{
			const char* curTex = original.strTexture.c_str();
			for( int i = 0; i < textureNames.Num(); ++i )
			{
				if( textureNames[i] == curTex )
				{
					currentTextureIndex = i + 1; // remember, 0 is "<No Texture>"
					LoadCurrentTexture();
					break;
				}
			}
		}
	}

	this->lightEntity = light;
}

void LightEditor::Reset()
{
	title = "Light Editor: no Light selected!";
	entityPos.x = idMath::INFINITY;
	entityPos.y = idMath::INFINITY;
	entityPos.z = idMath::INFINITY;

	original.Defaults();
	cur.Defaults();

	lightEntity = NULL;
	currentTextureIndex = 0;
	currentTexture = NULL;
	currentTextureMaterial = NULL;

	//mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	//mCurrentGizmoMode = ImGuizmo::WORLD;

	useSnap = false;
	//snap = { 1.f, 1.f, 1.f };
	//bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
	//boundsSnap[] = { 0.1f, 0.1f, 0.1f };
	boundSizing = false;
	boundSizingSnap = false;

	shortcutSaveMapEnabled = true;
	shortcutDuplicateLightEnabled = true;
}

void LightEditor::LoadLightTextures()
{
	textureNames.Clear();

	int count = declManager->GetNumDecls( DECL_MATERIAL );

	for ( int i = 0; i < count; i++ ) {

		// just get the name of the light material
		const idMaterial *mat = declManager->MaterialByIndex( i, false );

		idStr matName = mat->GetName();
		matName.ToLower();

		if ( matName.Icmpn( "lights/", strlen("lights/" ) ) == 0 || matName.Icmpn( "fogs/", strlen( "fogs/" ) ) == 0 ) {
			// actually load the material
			const idMaterial *material = declManager->FindMaterial( matName, false );
			if( material != NULL ) {
				// check if the material has textures or is just a leftover from the development
				idImage* editorImage = mat->GetStage( 0 )->texture.image;
				if ( !editorImage->defaulted ) {
					textureNames.Append( matName );
				}
			}
		}
	}

	textureNames.Sort();
}

// static
bool LightEditor::TextureItemsGetter( void* data, int idx, const char** outText )
{
	LightEditor* self = static_cast<LightEditor*>( data );
	if( idx == 0 )
	{
		*outText = "<No Texture>";
		return true;
	}

	// as index 0 has special purpose, the "real" index is one less
	--idx;

	if( idx < 0 || idx >= self->textureNames.Num() )
	{
		*outText = "<Invalid Index!>";
		return false;
	}

	*outText = self->textureNames[idx].c_str();

	return true;
}

void LightEditor::LoadCurrentTexture()
{
	currentTexture = NULL;

	if( currentTextureIndex > 0 && cur.strTexture.Length() > 0 )
	{
		const idMaterial* mat = declManager->FindMaterial( cur.strTexture, false );
		if( mat != NULL )
		{
			currentTexture = mat->GetStage( 0 )->texture.image;
			if( currentTexture )
			{
				// RB: create extra 2D material of the image for UI rendering

				// HACK that deserves being called a hack
				idStr uiName( "lighteditor/" );
				uiName += currentTexture->imgName;

				currentTextureMaterial = declManager->FindMaterial( uiName, true );
			}
		}
	}
}

void LightEditor::TempApplyChanges()
{
	if( lightEntity != NULL )
	{
		idDict d;
		cur.ToDict( &d );

		gameEdit->EntityChangeSpawnArgs( lightEntity, &d );
		gameEdit->EntityUpdateChangeableSpawnArgs( lightEntity, NULL );
	}
}

void LightEditor::SaveChanges( bool saveMap )
{
	idDict d;
	cur.ToDict( &d );
	if( entityName[0] != '\0' )
	{
		gameEdit->MapCopyDictToEntity( entityName, &d );
	}
	else if( entityPos.x != idMath::INFINITY )
	{
		entityName = gameEdit->GetUniqueEntityName( "light" );
		d.Set( "name", entityName );

		// RB: this is really HACKY
		gameEdit->MapCopyDictToEntityAtOrigin( entityPos, &d );
	}

	original = cur;

	if( saveMap )
	{
		gameEdit->MapSave();
	}
}

void LightEditor::CancelChanges()
{
	if( lightEntity != NULL )
	{
		idDict d;
		original.ToDict( &d );

		gameEdit->EntityChangeSpawnArgs( lightEntity, &d );
		gameEdit->EntityUpdateChangeableSpawnArgs( lightEntity, NULL );
	}

	imguiLocal.CloseWindow( (idImGuiWindow)EDITOR_LIGHT );
}

void LightEditor::DuplicateLight()
{
	if( lightEntity != NULL )
	{
		// store current light properties to game idMapFile
		SaveChanges( false );

		// spawn the new light
		idDict d;
		cur.ToDict( &d );
		d.DeleteEmptyKeys();

		entityName = gameEdit->GetUniqueEntityName( "light" );
		d.Set( "name", entityName );
		d.Set( "classname", "light" );

		idEntity* light = NULL;
		gameEdit->SpawnEntityDef( d, &light );

		if( light )
		{
			gameEdit->MapAddEntity( &d );
			gameEdit->ClearEntitySelection();
			gameEdit->AddSelectedEntity( light );

			Init( &d, light );
		}
	}
}

// a kinda ugly hack to get a float* (as used by imgui) from idVec3
static float* vecToArr( idVec3& v )
{
	return &v.x;
}



void LightEditor::Draw()
{
	bool changes = false;
	bool showTool = isShown;
	bool isOpen;

	if( ImGui::Begin( title, &isOpen ) )
	{
		// RB: handle arrow key inputs like in TrenchBroom
		ImGuiIO& io = ImGui::GetIO();

		// TODO use view direction like just global values
		if( io.KeyCtrl )
		{
			if( io.KeysDown['s'] && shortcutSaveMapEnabled )
			{
				SaveChanges( true );
				shortcutSaveMapEnabled = false;
			}
			else if( io.KeysDown['d'] && shortcutDuplicateLightEnabled )
			{
				DuplicateLight();
				shortcutDuplicateLightEnabled = false;
			}
		}
		else if( io.KeyAlt )
		{
			if( io.KeysDown[K_UPARROW] )
			{
				cur.origin.z += 1;
				changes = true;
			}
			else if( io.KeysDown[K_DOWNARROW] )
			{
				cur.origin.z -= 1;
				changes = true;
			}
		}
		else if( io.KeysDown[K_RIGHTARROW] )
		{
			cur.origin.x += 1;
			changes = true;
		}
		else if( io.KeysDown[K_LEFTARROW] )
		{
			cur.origin.x -= 1;
			changes = true;
		}
		else if( io.KeysDown[K_UPARROW] )
		{
			cur.origin.y += 1;
			changes = true;
		}
		else if( io.KeysDown[K_DOWNARROW] )
		{
			cur.origin.y -= 1;
			changes = true;
		}

		// reenable commands if keys were released
		if( ( !io.KeyCtrl || !io.KeysDown['s'] ) && !shortcutSaveMapEnabled )
		{
			shortcutSaveMapEnabled = true;
		}

		if( ( !io.KeyCtrl || !io.KeysDown['d'] ) && !shortcutDuplicateLightEnabled )
		{
			shortcutDuplicateLightEnabled = true;
		}

		if( !entityName.IsEmpty() )
		{
			ImGui::SeparatorText( entityName.c_str() );
		}

		ImGui::SeparatorText( "Light Volume" );

		ImGui::Spacing();

		int lightSelectionRadioBtn = cur.lightType;

		changes |= ImGui::RadioButton( "Point Light", &lightSelectionRadioBtn, 0 );
		ImGui::SameLine();
		changes |= ImGui::RadioButton( "Spot Light", &lightSelectionRadioBtn, 1 );
		ImGui::SameLine();
		changes |= ImGui::RadioButton( "Sun Light", &lightSelectionRadioBtn, 2 );

		ImGui::Indent();

		ImGui::Spacing();

		if( lightSelectionRadioBtn == LIGHT_POINT || lightSelectionRadioBtn == LIGHT_SUN )
		{
			if( lightSelectionRadioBtn == LIGHT_POINT && lightSelectionRadioBtn != cur.lightType )
			{
				cur.DefaultPoint();
				changes = true;
			}
			else if( lightSelectionRadioBtn == LIGHT_SUN && lightSelectionRadioBtn != cur.lightType )
			{
				cur.DefaultSun();
				changes = true;
			}

			ImGui::PushItemWidth( -1.0f ); // align end of Drag* with right window border

			changes |= ImGui::Checkbox( "Equilateral Radius", &cur.equalRadius );
			ImGui::Text( "Radius:" );
			ImGui::Indent();
			if( cur.equalRadius )
			{
				if( ImGui::DragFloat( "##radEquil", &cur.lightRadius.x, 1.0f, 0.0f, 10000.0f, "%.1f" ) )
				{
					cur.lightRadius.z = cur.lightRadius.y = cur.lightRadius.x;
					changes = true;
				}
			}
			else
			{
				changes |= ImGui::DragVec3( "##radXYZ", cur.lightRadius );
			}
			ImGui::Unindent();

			ImGui::Spacing();

			//changes |= ImGui::Checkbox( "Parallel", &cur.isParallel );

			//ImGui::Spacing();

			changes |= ImGui::Checkbox( "Center", &cur.hasCenter );
			if( cur.hasCenter )
			{
				ImGui::Indent();
				changes |= ImGui::DragVec3( "##centerXYZ", cur.lightCenter, 1.0f, 0.0f, 10000.0f, "%.1f" );
				ImGui::Unindent();
			}
			ImGui::PopItemWidth(); // back to default alignment on right side
		}
		else if( lightSelectionRadioBtn == LIGHT_SPOT )
		{
			if( cur.lightType != lightSelectionRadioBtn )
			{
				cur.DefaultProjected();
				changes = true;
			}

			changes |= ImGui::DragVec3( "Target", cur.lightTarget, 1.0f, 0.0f, 0.0f, "%.1f" );
			changes |= ImGui::DragVec3( "Right", cur.lightRight, 1.0f, 0.0f, 0.0f, "%.1f" );
			changes |= ImGui::DragVec3( "Up", cur.lightUp, 1.0f, 0.0f, 0.0f, "%.1f" );

			ImGui::Spacing();

			changes |= ImGui::Checkbox( "Explicit start/end points", &cur.explicitStartEnd );

			ImGui::Spacing();
			if( cur.explicitStartEnd )
			{
				changes |= ImGui::DragVec3( "Start", cur.lightStart, 1.0f, 0.0f, 0.0f, "%.1f" );
				changes |= ImGui::DragVec3( "End", cur.lightEnd, 1.0f, 0.0f, 0.0f, "%.1f" );
			}
		}

		cur.lightType = ELightType( lightSelectionRadioBtn );

		ImGui::Unindent();

		ImGui::SeparatorText( "Transform" );

		if( io.KeysDown['g'] )
		{
			//mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		}

		if( io.KeysDown['r'] )
		{
			//mCurrentGizmoOperation = ImGuizmo::ROTATE;
		}

		//if( ImGui::IsKeyPressed( ImGuiKey_S ) )
		if( io.KeysDown['s'] )
		{
			//mCurrentGizmoOperation = ImGuizmo::SCALE;
		}

		/*
		if( mCurrentGizmoOperation != ImGuizmo::SCALE )
		{
			if( ImGui::RadioButton( "Local", mCurrentGizmoMode == ImGuizmo::LOCAL ) )
			{
				mCurrentGizmoMode = ImGuizmo::LOCAL;
			}
			ImGui::SameLine();
			if( ImGui::RadioButton( "World", mCurrentGizmoMode == ImGuizmo::WORLD ) )
			{
				mCurrentGizmoMode = ImGuizmo::WORLD;
			}
		}
		else
		{
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		}

		if( ImGui::RadioButton( "Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE ) )
		{
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE ) )
		{
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "Scale", mCurrentGizmoOperation == ImGuizmo::SCALE ) )
		{
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		}
		*/
		//if( ImGui::RadioButton( "Universal", mCurrentGizmoOperation == ImGuizmo::UNIVERSAL ) )
		//{
		//	mCurrentGizmoOperation = ImGuizmo::UNIVERSAL;
		//}

		changes |= ImGui::DragVec3( "Origin", cur.origin, 1.0f, 0.0f, 0.0f, "%.1f" );

		ImGui::SeparatorText( "Snapping" );

		ImGui::Checkbox( "Use Snapping", &useSnap );
		//ImGui::SameLine();

		if( useSnap )
		{
			/*
			switch( mCurrentGizmoOperation )
			{
				case ImGuizmo::TRANSLATE:
					ImGui::InputFloat3( "Grid Snap", &gridSnap[0] );
					break;
				case ImGuizmo::ROTATE:
					ImGui::InputFloat( "Angle Snap", &angleSnap );
					break;
				case ImGuizmo::SCALE:
					ImGui::InputFloat( "Scale Snap", &scaleSnap );
					break;
			}
			*/
		}

#if 0
		ImGui::Checkbox( "Bound Sizing", &boundSizing );
		if( boundSizing )
		{
			ImGui::PushID( 3 );
			ImGui::Checkbox( "##BoundSizing", &boundSizingSnap );
			ImGui::SameLine();
			ImGui::InputFloat3( "Snap", boundsSnap );
			ImGui::PopID();
		}
#endif

		ImGui::SeparatorText( "Color & Texturing" );

		changes |= ImGui::ColorEdit3( "Color", vecToArr( cur.color ) );

		ImGui::Spacing();

		if( ImGui::Combo( "Texture", &currentTextureIndex, TextureItemsGetter, this, textureNames.Num() + 1 ) )
		{
			changes = true;

			// -1 because 0 is "<No Texture>"
			cur.strTexture = ( currentTextureIndex > 0 ) ? textureNames[currentTextureIndex - 1] : "";
			LoadCurrentTexture();
		}

		if( currentTextureMaterial != nullptr && currentTexture != nullptr )
		{
			ImVec2 size( currentTexture->uploadWidth, currentTexture->uploadHeight );

			ImGui::Image( ( void* )currentTextureMaterial, size, ImVec2( 0, 0 ), ImVec2( 1, 1 ),
						  ImColor( 255, 255, 255, 255 ), ImColor( 255, 255, 255, 128 ) );
		}

		ImGui::SeparatorText( "Misc Options" );

		changes |= ImGui::Checkbox( "Cast Shadows", &cur.castShadows );
		changes |= ImGui::Checkbox( "Skip Specular", &cur.skipSpecular );

		// TODO: allow multiple lights selected at the same time + "apply different" button?
		//       then only the changed attribute (e.g. color) would be set to all lights,
		//       but they'd keep their other individual properties (eg radius)

		ImGui::Spacing();

		if( ImGui::Button( "Save to .map" ) )
		{
			SaveChanges( true );
			showTool = false;
		}
		else if( ImGui::SameLine(), ImGui::Button( "Cancel" ) )
		{
			CancelChanges();
			showTool = false;
		}

		/*
		viewDef_t viewDef;
		if( gameEdit->PlayerGetRenderView( viewDef.renderView ) )
		{
			ImGui::Separator();

			ImGui::Text( "X: %f Y: %f", io.MousePos.x, io.MousePos.y );
			if( ImGuizmo::IsUsing() )
			{
				ImGui::Text( "Using gizmo" );
			}
			else
			{
				ImGui::Text( ImGuizmo::IsOver() ? "Over gizmo" : "" );
				ImGui::SameLine();
				ImGui::Text( ImGuizmo::IsOver( ImGuizmo::TRANSLATE ) ? "Over translate gizmo" : "" );
				ImGui::SameLine();
				ImGui::Text( ImGuizmo::IsOver( ImGuizmo::ROTATE ) ? "Over rotate gizmo" : "" );
				ImGui::SameLine();
				ImGui::Text( ImGuizmo::IsOver( ImGuizmo::SCALE ) ? "Over scale gizmo" : "" );
			}
		}
		*/

		static bool use_work_area = true;
		static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
										| ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;// | ImGuiWindowFlags_MenuBar;

		// We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
		// Based on your use case you may want one or the other.
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos( use_work_area ? viewport->WorkPos : viewport->Pos );
		ImGui::SetNextWindowSize( use_work_area ? viewport->WorkSize : viewport->Size );

		if( ImGui::Begin( "Example: Fullscreen window", &showTool, flags ) )
		{
			if( ImGui::BeginMainMenuBar() )
			{
				if( ImGui::BeginMenu( "File" ) )
				{
					//ShowExampleMenuFile();
					if( ImGui::MenuItem( "Save Map", "Ctrl+S" ) )
					{
						SaveChanges( true );
					}
					ImGui::EndMenu();
				}
				if( ImGui::BeginMenu( "Edit" ) )
				{
					//if( ImGui::MenuItem( "Undo", "CTRL+Z" ) ) {}
					//if( ImGui::MenuItem( "Redo", "CTRL+Y", false, false ) ) {} // Disabled item

					//ImGui::Separator();

					//if( ImGui::MenuItem( "Cut", "CTRL+X" ) ) {}
					//if( ImGui::MenuItem( "Copy", "CTRL+C" ) ) {}
					//if( ImGui::MenuItem( "Paste", "CTRL+V" ) ) {}

					if( ImGui::MenuItem( "Duplicate", "CTRL+D" ) )
					{
						DuplicateLight();
					}

					//if( ImGui::MenuItem( "Delete", "Backspace" ) )
					//{
					// TODO
					//	goto exitLightEditor;
					//}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}

			// backup state before moving the light
			/*
			if( !ImGuizmo::IsUsing() )
			{
				curNotMoving = cur;
			}
			*/
			//
			// GIZMO
			//

			//ImGuiIO& io = ImGui::GetIO();
			//ImGuizmo::SetRect( 0, 0, io.DisplaySize.x, io.DisplaySize.y );
			//ImGuizmo::SetOrthographic( false );
			//ImGuizmo::SetDrawlist();
			//ImGuizmo::SetID( 0 );


			//viewDef_t viewDef;
			//if( gameEdit->PlayerGetRenderView( viewDef.renderView ) )
			/*
			{
				R_SetupViewMatrix( &viewDef );
				R_SetupProjectionMatrix( &viewDef, false );

				float* cameraView = viewDef.worldSpace.modelViewMatrix;
				float* cameraProjection = viewDef.unjitteredProjectionMatrix;

				idMat3 rotateMatrix = cur.angles.ToMat3();
				idMat3 scaleMatrix = mat3_identity;
				scaleMatrix[0][0] = 16;
				scaleMatrix[1][1] = 16;
				scaleMatrix[2][2] = 16;

				idMat4 objectMatrix( scaleMatrix * rotateMatrix,  cur.origin );
				ImGuizmo::DrawCubes( cameraView, cameraProjection, objectMatrix.Transpose().ToFloatPtr(), 1 );

				scaleMatrix[0][0] = 1;
				scaleMatrix[1][1] = 1;
				scaleMatrix[2][2] = 1;

				idMat4 gizmoMatrix( scaleMatrix * rotateMatrix,  cur.origin );
				idMat4 manipMatrix = gizmoMatrix.Transpose();

				const float* snap = NULL;
				if( useSnap )
				{
					switch( mCurrentGizmoOperation )
					{
						case ImGuizmo::TRANSLATE:
							snap = &gridSnap[0];
							break;
						case ImGuizmo::ROTATE:
							snap = &angleSnap;
							break;
						case ImGuizmo::SCALE:
							snap = &scaleSnap;
							break;
					}
				}

				ImGuizmo::Manipulate( cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, manipMatrix.ToFloatPtr(), NULL, useSnap ? snap : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL );

				if( ImGuizmo::IsUsing() )
				{
					//if( mCurrentGizmoOperation == ImGuizmo::TRANSLATE )
					{
						gizmoMatrix = manipMatrix.Transpose();
						cur.origin = gizmoMatrix.GetTranslation();

						changes = true;
					}

					if( ( mCurrentGizmoOperation & ImGuizmo::SCALE ) == 0 )
					{
						idMat3 axis = gizmoMatrix.ToMat3();
						cur.angles = axis.ToAngles();

						changes = true;
					}

					if( mCurrentGizmoOperation == ImGuizmo::SCALE )
					{
						// Use DecomposeMatrixToComponents just for the scaling
						float matrixTranslation[3], matrixRotation[3], matrixScale[3];
						ImGuizmo::DecomposeMatrixToComponents( &manipMatrix[0][0], matrixTranslation, matrixRotation, matrixScale );

						cur.scale.x = matrixScale[0];
						cur.scale.y = matrixScale[1];
						cur.scale.z = matrixScale[2];

						if( matrixScale[0] != 1.0f || matrixScale[1] != 1.0f || matrixScale[2] != 1.0f )
						{
							if( cur.lightType == LIGHT_SPOT )
							{
								cur.lightRight = curNotMoving.lightRight * matrixScale[0];
								cur.lightUp = curNotMoving.lightUp * matrixScale[1];
								cur.lightTarget = curNotMoving.lightTarget * matrixScale[2];
							}
							else //if( cur.lightType == LIGHT_POINT )
							{
								cur.lightRadius.x = curNotMoving.lightRadius.x * matrixScale[0];
								cur.lightRadius.y = curNotMoving.lightRadius.y * matrixScale[1];
								cur.lightRadius.z = curNotMoving.lightRadius.z * matrixScale[2];

								if( matrixScale[0] != matrixScale[1] || matrixScale[1] != matrixScale[2] )
								{
									cur.equalRadius = false;
								}
							}

							changes = true;
						}
					}
				}
			}
			*/
		}
		ImGui::End();
	}
	ImGui::End();

	if( changes )
	{
		TempApplyChanges();
	}

//exitLightEditor:

	if( isShown && !showTool )
	{
		isShown = showTool;
		//imguiTools->SetReleaseToolMouse( false );
		imguiLocal.CloseWindow( (idImGuiWindow)EDITOR_LIGHT );
	}
}

} //namespace ImGuiTools
