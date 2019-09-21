// ******************************************************************************
// Filename:    VoxRender.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 27/10/15
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "VoxGame.h"

#include <glm/detail/func_geometric.hpp>


// Rendering
void VoxGame::PreRender()
{
	// Update matrices for game objects
	m_pPlayer->CalculateWorldTransformMatrix();
	m_pNPCManager->CalculateWorldTransformMatrix();
	m_pEnemyManager->CalculateWorldTransformMatrix();
	m_pItemManager->CalculateWorldTransformMatrix();
	m_pProjectileManager->CalculateWorldTransformMatrix();
}

void VoxGame::BeginShaderRender()
{
	glShader* pShader = nullptr;

	if (m_pVoxSettings->m_shadows)
	{
		m_pRenderer->BeginGLSLShader(m_shadowShader);

		pShader = m_pRenderer->GetShader(m_shadowShader);
		GLuint shadowMapUniform = glGetUniformLocationARB(pShader->GetProgramObject(), "ShadowMap");
		m_pRenderer->PrepareShaderTexture(7, shadowMapUniform);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDepthTextureFromFrameBuffer(m_shadowFrameBuffer));
		glUniform1iARB(glGetUniformLocationARB(pShader->GetProgramObject(), "renderShadow"), m_pVoxSettings->m_shadows);
		glUniform1iARB(glGetUniformLocationARB(pShader->GetProgramObject(), "alwaysShadow"), false);
	}
	else
	{
		m_pRenderer->BeginGLSLShader(m_defaultShader);

		pShader = m_pRenderer->GetShader(m_defaultShader);
	}

	bool fogEnabled = (m_pFrontendManager->GetFrontendScreen() == FrontendScreen_MainMenu) ? false : m_pVoxSettings->m_fogRendering;
	glUniform1iARB(glGetUniformLocationARB(pShader->GetProgramObject(), "enableFog"), fogEnabled);
	float lfogEnd = m_pChunkManager->GetLoaderRadius() - Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE;
	float lfogStart = lfogEnd - Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE*2.0f;
	GLfloat fogColor[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 1.0f);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	glFogf(GL_FOG_START, lfogStart);
	glFogf(GL_FOG_END, lfogEnd);
	glEnable(GL_FOG);
}

void VoxGame::EndShaderRender()
{
	glDisable(GL_FOG);

	if (m_pVoxSettings->m_shadows)
	{
		m_pRenderer->EndGLSLShader(m_shadowShader);
	}
	else
	{
		m_pRenderer->EndGLSLShader(m_defaultShader);
	}
}

void VoxGame::Render()
{
	if (m_pVoxWindow->GetMinimized())
	{
		// Don't call any render functions if minimized
		return;
	}

	// Begin rendering
	m_pRenderer->BeginScene(true, true, true);

		// Shadow rendering to the shadow frame buffer
		if (m_pVoxSettings->m_shadows)
		{
			RenderShadows();
		}

		// ---------------------------------------
		// Render 3d
		// ---------------------------------------
		m_pRenderer->PushMatrix();
			// Set the default projection mode
			m_pRenderer->SetProjectionMode(PM_PERSPECTIVE, m_defaultViewport);

			// Enable vector normalization, for scaling and lighting
			m_pRenderer->EnableVectorNormalize();

			// Set back culling as default
			m_pRenderer->SetCullMode(CM_BACK);

			// Set default depth test
			m_pRenderer->EnableDepthTest(DT_LESS);

			// Set the lookat camera
			m_pGameCamera->Look();

			// Enable the lights
			m_pRenderer->PushMatrix();
				m_pRenderer->EnableLight(m_defaultLight, 0);
			m_pRenderer->PopMatrix();

			// Multisampling MSAA
			if (m_multiSampling)
			{
				m_pRenderer->EnableMultiSampling();
			}
			else
			{
				m_pRenderer->DisableMultiSampling();
			}

			// Water reflections
			if (m_pVoxSettings->m_waterRendering && m_pChunkManager->IsUnderWater(m_pGameCamera->GetPosition()) == false && m_gameMode != GameMode_FrontEnd)
			{
				RenderWaterReflections();
			}

			// SSAO frame buffer rendering start
			if (m_deferredRendering)
			{
				m_pRenderer->StartRenderingToFrameBuffer(m_SSAOFrameBuffer);
			}

			m_pRenderer->SetClearColour(0.0f, 0.0f, 0.0f, 1.0f);
			m_pRenderer->ClearScene(true, true, true);

			// Render the lights (DEBUG)
			m_pRenderer->PushMatrix();
				m_pRenderer->SetCullMode(CM_BACK);
				m_pRenderer->SetRenderMode(RM_SOLID);
				m_pRenderer->RenderLight(m_defaultLight);
			m_pRenderer->PopMatrix();

			// Render the skybox
			RenderSkybox();

			BeginShaderRender();
			{
				// Render the chunks
				m_pChunkManager->Render(false);
			}
			EndShaderRender();

			// NPC sub selection - For character creation screen
			m_pNPCManager->RenderSubSelectionNormalNPCs();

			// Render items outline and silhouette before the world/chunks
			m_pItemManager->Render(true, false, false, false);
			m_pItemManager->Render(false, false, true, false);

			// NPCs (only non outline and hover)
			m_pNPCManager->RenderOutlineNPCs();
			m_pNPCManager->Render(false, false, true, false, false, false);

			// Enemies outline
			m_pEnemyManager->RenderOutlineEnemies();
			m_pEnemyManager->Render(false, false, true, false);

			BeginShaderRender();
			{
				// Scenery
				m_pSceneryManager->Render(false, false, false, false, false);

				// Projectiles
				m_pProjectileManager->Render();

				// Items
				m_pItemManager->Render(false, false, false, false);

				// NPCs
				m_pNPCManager->ResetNumRenderNPCs();
				m_pNPCManager->Render(false, false, false, false, true, false);

				// Enemies
				m_pEnemyManager->Render(false, false, false, false);

				// NPCs (only outline and hover)
				m_pNPCManager->Render(false, false, false, true, false, false);
				m_pNPCManager->RenderSubSelectionOverlayNPCs();
			}
			EndShaderRender();

			// Player selection block
			m_pPlayer->RenderSelectionBlock();

			// Render the block particles
			m_pBlockParticleManager->Render(false);

			// Render the instanced objects
			if(m_instanceRender)
			{
				m_pInstanceManager->Render();
			}

			// Frontend
			m_pFrontendManager->Render();

			BeginShaderRender();
			{

				if (m_gameMode != GameMode_FrontEnd)
				{
					// Render the player
					if (m_cameraMode != CameraMode_FirstPerson)
					{
						m_pPlayer->Render();
					}
				}
			}
			EndShaderRender();

			// Render the transparency items above the water render, so that they appear properly under water
			if (m_pVoxSettings->m_waterRendering && m_gameMode != GameMode_FrontEnd)
			{
				if (m_cameraMode != CameraMode_FirstPerson)
				{
					m_pPlayer->RenderFace();
				}
				m_pNPCManager->RenderFaces();
				m_pEnemyManager->RenderFaces();

				if (m_cameraMode != CameraMode_FirstPerson)
				{
					m_pPlayer->RenderWeaponTrails();
				}
				m_pNPCManager->RenderWeaponTrails();
				m_pEnemyManager->RenderWeaponTrails();

				// Render water
				RenderWater();
			}

			// Debug rendering
			if(m_debugRender)
			{
				m_pLightingManager->DebugRender();

				m_pBlockParticleManager->RenderDebug();

				if (m_gameMode != GameMode_FrontEnd)
				{
					m_pPlayer->RenderDebug();
				}

				m_pNPCManager->RenderDebug();

				m_pEnemyManager->RenderDebug();

				m_pSceneryManager->RenderDebug();

				m_pItemManager->RenderDebug();

				m_pChunkManager->RenderDebug();

				m_pProjectileManager->RenderDebug();

				m_pBiomeManager->RenderDebug();

				if (m_gameMode == GameMode_FrontEnd)
				{
					m_pFrontendManager->RenderDebug();
				}
			}

			// Render player first person viewport
			if (m_gameMode != GameMode_FrontEnd)
			{
				if (m_cameraMode == CameraMode_FirstPerson)
				{
					RenderFirstPersonViewport();
				}
			}

			// SSAO frame buffer rendering stop
			if (m_deferredRendering)
			{
				m_pRenderer->StopRenderingToFrameBuffer(m_SSAOFrameBuffer);
			}
		m_pRenderer->PopMatrix();

		// Render the deferred lighting pass
		if (m_dynamicLighting)
		{
			RenderDeferredLighting();
		}

		// Render other viewports
		// Paperdoll for CharacterGUI
		RenderPaperdollViewport();
		// Portrait for HUD
		if (m_pVoxSettings->m_renderGUI)
		{
			RenderPortraitViewport();
		}

		// ---------------------------------------
		// Render transparency
		// ---------------------------------------
		RenderTransparency();

		// Render the SSAO texture
		if (m_deferredRendering)
		{
			RenderSSAOTexture();

			if (m_multiSampling && m_fxaaShader != -1)
			{
				RenderFXAATexture();
			}

			if(m_blur || m_pChunkManager->IsUnderWater(m_pGameCamera->GetPosition()))
			{
				RenderFirstPassFullScreen();
				RenderSecondPassFullScreen();
			}
		}

		// ---------------------------------------
		// Render 2d
		// ---------------------------------------
		m_pRenderer->PushMatrix();
			// Disable multisampling for 2d gui and text
			m_pRenderer->DisableMultiSampling();

			// Crosshair
			if (m_cameraMode == CameraMode_FirstPerson && m_bPaused == false)
			{
				if (m_pPlayer->IsDead() == false)
				{
					RenderCrosshair();
				}
			}

			// Text effects
			m_pTextEffectsManager->Render();

			// Cinematic mode (letter box)
			RenderCinematicLetterBox();

			// Render the HUD
			if (m_pVoxSettings->m_renderGUI)
			{
				RenderHUD();
			}
		m_pRenderer->PopMatrix();

		// Render other deferred rendering pipelines
		// Paperdoll SSAO for CharacterGUI
		RenderDeferredRenderingPaperDoll();
		// Portrait SSAO for HUD
		if (m_pVoxSettings->m_renderGUI)
		{
			RenderDeferredRenderingPortrait();
		}

		// Render the chunks 2d (debug text information)
		if (m_debugRender)
		{
			//m_pChunkManager->Render2D(m_pGameCamera, m_defaultViewport, m_defaultFont);
		}

		// Frontend 2D
		if(m_gameMode == GameMode_FrontEnd)
		{			
			m_pRenderer->PushMatrix();
				m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);
				m_pRenderer->SetCullMode(CM_BACK);

				m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

				m_pFrontendManager->Render2D();
			m_pRenderer->PopMatrix();
		}

		// Render the GUI
		RenderGUI();

		// Custom cursor
		if (m_pVoxSettings->m_customCursors && m_bCustomCursorOn)
		{
			if (m_gameMode != GameMode_FrontEnd || m_pFrontendManager->GetFrontendScreen() != FrontendScreen_Intro)
			{
				RenderCustomCursor();
			}
		}

		// Render debug information and text
		RenderDebugInformation();

		// Update the NPC screen positions for select character screen
		m_pNPCManager->UpdateScreenCoordinates2d(m_pGameCamera);

	// End rendering
	m_pRenderer->EndScene();


	// Pass render call to the window class, allow to swap buffers
	m_pVoxWindow->Render();
}

void VoxGame::RenderSkybox()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->BeginGLSLShader(m_cubeMapShader);

		glShader* pShader = m_pRenderer->GetShader(m_cubeMapShader);
		unsigned int cubemapTexture1 = glGetUniformLocationARB(pShader->GetProgramObject(), "cubemap1");
		m_pRenderer->PrepareShaderTexture(0, cubemapTexture1);
		m_pRenderer->BindCubeTexture(m_pSkybox->GetCubeMapTexture1());

		//unsigned int cubemapTexture2 = glGetUniformLocationARB(pShader->GetProgramObject(), "cubemap2");
		//m_pRenderer->PrepareShaderTexture(1, cubemapTexture2);
		//m_pRenderer->BindCubeTexture(m_pSkybox->GetCubeMapTexture2());

		pShader->setUniform1f("skyboxRatio", 0.0f);

		m_pRenderer->TranslateWorldMatrix(m_pPlayer->GetCenter().x, m_pPlayer->GetCenter().y, m_pPlayer->GetCenter().z);
		m_pSkybox->Render();
				
		m_pRenderer->EmptyCubeTextureIndex(0);
		//m_pRenderer->EmptyCubeTextureIndex(1);

		m_pRenderer->EmptyTextureIndex(2);
		m_pRenderer->EmptyTextureIndex(1);
		m_pRenderer->EmptyTextureIndex(0);

		m_pRenderer->EndGLSLShader(m_cubeMapShader);
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderShadows()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->StartRenderingToFrameBuffer(m_shadowFrameBuffer);
		m_pRenderer->SetColourMask(false, false, false, false);

		float loaderRadius = m_pChunkManager->GetLoaderRadius();
		m_pRenderer->SetupOrthographicProjection(-loaderRadius, loaderRadius, -loaderRadius, loaderRadius, 0.01f, 1000.0f);
		vec3 lightPos = m_defaultLightPosition + m_pPlayer->GetCenter(); // Make sure our light is always offset from the player
		m_pRenderer->SetLookAtCamera(vec3(lightPos.x, lightPos.y, lightPos.z), m_pPlayer->GetCenter(), vec3(0.0f, 1.0f, 0.0f));

		m_pRenderer->PushMatrix();
			m_pRenderer->SetCullMode(CM_FRONT);

			// Render the chunks
			m_pChunkManager->Render(true);

			if (m_gameMode != GameMode_FrontEnd)
			{
				// Render the player
				m_pPlayer->Render();
			}

			// NPCs
			m_pNPCManager->Render(false, false, false, false, false, true);
			m_pNPCManager->RenderSubSelectionNPCs();

			// Enemies
			m_pEnemyManager->Render(false, true, false, true);

			// Projectiles
			m_pProjectileManager->Render();

			// Scenery
			m_pSceneryManager->Render(false, false, true, false, false);

			// Items
			m_pItemManager->Render(false, false, false, true);

			// Render the block particles
			m_pBlockParticleManager->Render(false);

			// Render the instanced objects
			if(m_instanceRender)
			{
				m_pInstanceManager->Render();
			}

			m_pRenderer->SetTextureMatrix();
			m_pRenderer->SetCullMode(CM_BACK);
		m_pRenderer->PopMatrix();

		m_pRenderer->SetColourMask(true, true, true, true);
		m_pRenderer->StopRenderingToFrameBuffer(m_shadowFrameBuffer);
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderWaterReflections()
{
	m_pRenderer->StartRenderingToFrameBuffer(m_waterReflectionFrameBuffer);

	if (m_pChunkManager->IsUnderWater(m_pGameCamera->GetPosition()) == false)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->SetClearColour(0.0f, 0.0f, 0.0f, 1.0f);
			m_pRenderer->ClearScene(true, false, false);
			m_pRenderer->TranslateWorldMatrix(0.0f, (m_pChunkManager->GetWaterHeight()*2.0f), 0.0f);
			m_pRenderer->ScaleWorldMatrix(1.0f, -1.0f, 1.0f);
			m_pRenderer->SetCullMode(CM_FRONT);

			m_pRenderer->EnableClipPlane(0, 0.0f, 1.0f, 0.0f, -m_pChunkManager->GetWaterHeight());

			// Render the chunks
			m_pChunkManager->Render(true);

			// Player
			if (m_gameMode != GameMode_FrontEnd)
			{
				m_pPlayer->Render();
				m_pPlayer->RenderFace();
				m_pPlayer->RenderWeaponTrails();
			}

			//// NPCs
			//m_pNPCManager->Render(false, false, false, false, false, false);
			//m_pNPCManager->RenderFaces();

			//// Enemies
			//m_pEnemyManager->Render(false, true, false, false);
			//m_pEnemyManager->RenderFaces();

			//// Projectiles
			//m_pProjectileManager->Render();

			//// Scenery
			//m_pSceneryManager->Render(false, false, false, false, false);

			//// Items
			//m_pItemManager->Render(false, false, false, false);

			//// Render the instanced objects
			//if (m_instanceRender)
			//{
			//	m_pInstanceManager->Render();
			//}

			m_pRenderer->SetCullMode(CM_BACK);
			m_pRenderer->DisableClipPlane(0);
		m_pRenderer->PopMatrix();
	}
	m_pRenderer->StopRenderingToFrameBuffer(m_waterReflectionFrameBuffer);
}

void VoxGame::RenderWater()
{
	m_pRenderer->PushMatrix();

	m_pRenderer->BeginGLSLShader(m_waterShader);

		glShader* pShader = pShader = m_pRenderer->GetShader(m_waterShader);
		unsigned int reflectionTexture = glGetUniformLocationARB(pShader->GetProgramObject(), "reflectionTexture");
		unsigned int cubemapTexture = glGetUniformLocationARB(pShader->GetProgramObject(), "cubemap");
		if (m_pChunkManager->IsUnderWater(m_pGameCamera->GetPosition()) == false)
		{
			glActiveTextureARB(GL_TEXTURE1_ARB);
			m_pRenderer->BindRawTextureId(m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_waterReflectionFrameBuffer));
			glUniform1iARB(reflectionTexture, 1);
					
			glActiveTextureARB(GL_TEXTURE0_ARB);
			m_pRenderer->BindCubeTexture(m_pSkybox->GetCubeMapTexture1());
			glUniform1iARB(cubemapTexture, 0);
		}
		else
		{
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);

			glActiveTextureARB(GL_TEXTURE0_ARB);
			m_pRenderer->DisableCubeTexture();

			glActiveTextureARB(GL_TEXTURE1_ARB);
		}

		bool fogEnabled = (m_pFrontendManager->GetFrontendScreen() == FrontendScreen_MainMenu) ? false : m_pVoxSettings->m_fogRendering;
		glUniform1iARB(glGetUniformLocationARB(pShader->GetProgramObject(), "enableFog"), fogEnabled);
		float lfogEnd = m_pChunkManager->GetLoaderRadius() - Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE;
		float lfogStart = lfogEnd - Chunk::CHUNK_SIZE*Chunk::BLOCK_RENDER_SIZE*2.0f;
		GLfloat fogColor[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_DENSITY, 1.0f);
		glHint(GL_FOG_HINT, GL_DONT_CARE);
		glFogf(GL_FOG_START, lfogStart);
		glFogf(GL_FOG_END, lfogEnd);
		glEnable(GL_FOG);

		//pShader->setUniform1f("time", m_elapsedWaterTime);

		m_pRenderer->TranslateWorldMatrix(m_pPlayer->GetCenter().x, 0.0f, m_pPlayer->GetCenter().z);
		m_pChunkManager->RenderWater();

		glDisable(GL_FOG);

		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTextureARB(GL_TEXTURE0_ARB);
		m_pRenderer->DisableCubeTexture();
		m_pRenderer->EndGLSLShader(m_waterShader);
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderDeferredLighting()
{
	// Render deferred lighting to light frame buffer
	m_pRenderer->PushMatrix();
		m_pRenderer->StartRenderingToFrameBuffer(m_lightingFrameBuffer);

		m_pRenderer->SetFrontFaceDirection(FrontFaceDirection_CW);
		m_pRenderer->EnableTransparency(BF_ONE, BF_ONE);
		m_pRenderer->DisableDepthTest();

		// Set the default projection mode
		m_pRenderer->SetProjectionMode(PM_PERSPECTIVE, m_defaultViewport);

		// Set the lookat camera
		m_pGameCamera->Look();

		m_pRenderer->PushMatrix();
			m_pRenderer->BeginGLSLShader(m_lightingShader);

			glShader* pLightShader = m_pRenderer->GetShader(m_lightingShader);
			unsigned NormalsID = glGetUniformLocationARB(pLightShader->GetProgramObject(), "normals");
			unsigned PositionssID = glGetUniformLocationARB(pLightShader->GetProgramObject(), "positions");
			unsigned DepthsID = glGetUniformLocationARB(pLightShader->GetProgramObject(), "depths");

			m_pRenderer->PrepareShaderTexture(0, NormalsID);
			m_pRenderer->BindRawTextureId(m_pRenderer->GetNormalTextureFromFrameBuffer(m_SSAOFrameBuffer));

			m_pRenderer->PrepareShaderTexture(1, PositionssID);
			m_pRenderer->BindRawTextureId(m_pRenderer->GetPositionTextureFromFrameBuffer(m_SSAOFrameBuffer));

			m_pRenderer->PrepareShaderTexture(2, DepthsID);
			m_pRenderer->BindRawTextureId(m_pRenderer->GetDepthTextureFromFrameBuffer(m_SSAOFrameBuffer));

			pLightShader->setUniform1i("screenWidth", m_windowWidth);
			pLightShader->setUniform1i("screenHeight", m_windowHeight);
			pLightShader->setUniform1f("nearZ", 0.01f);
			pLightShader->setUniform1f("farZ", 1000.0f);

			for (int i = 0; i < m_pLightingManager->GetNumLights(); i++)
			{
				DynamicLight* lpLight = m_pLightingManager->GetLight(i);
				float lightRadius = lpLight->m_radius;

				vec3 cameraPos = vec3(m_pGameCamera->GetPosition().x, m_pGameCamera->GetPosition().y, m_pGameCamera->GetPosition().z);
				float length = glm::distance(cameraPos, lpLight->m_position);
				if (length < lightRadius + 0.5f) // Small change to account for differences in circle render (with slices) and circle radius
				{
					m_pRenderer->SetCullMode(CM_BACK);
				}
				else
				{
					m_pRenderer->SetCullMode(CM_FRONT);
				}

				pLightShader->setUniform1f("radius", lightRadius);
				pLightShader->setUniform1f("diffuseScale", lpLight->m_diffuseScale);

				float r = lpLight->m_colour.GetRed();
				float g = lpLight->m_colour.GetGreen();
				float b = lpLight->m_colour.GetBlue();
				float a = lpLight->m_colour.GetAlpha();
				pLightShader->setUniform4f("diffuseLightColor", r, g, b, a);

				m_pRenderer->PushMatrix();
					m_pRenderer->SetRenderMode(RM_SOLID);
					m_pRenderer->TranslateWorldMatrix(lpLight->m_position.x, lpLight->m_position.y + 0.5f, lpLight->m_position.z);
					m_pRenderer->DrawSphere(lightRadius, 30, 30);
				m_pRenderer->PopMatrix();
			}

			m_pRenderer->EmptyTextureIndex(2);
			m_pRenderer->EmptyTextureIndex(1);
			m_pRenderer->EmptyTextureIndex(0);

			m_pRenderer->EndGLSLShader(m_lightingShader);

		m_pRenderer->PopMatrix();

		m_pRenderer->SetFrontFaceDirection(FrontFaceDirection_CCW);
		m_pRenderer->DisableTransparency();
		m_pRenderer->SetCullMode(CM_BACK);
		m_pRenderer->EnableDepthTest(DT_LESS);

		m_pRenderer->StopRenderingToFrameBuffer(m_lightingFrameBuffer);

	m_pRenderer->PopMatrix();
}

void VoxGame::RenderTransparency()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_PERSPECTIVE, m_defaultViewport);
		m_pRenderer->SetCullMode(CM_BACK);

		// Set the lookat camera
		m_pGameCamera->Look();

		if (m_deferredRendering)
		{
			m_pRenderer->StartRenderingToFrameBuffer(m_transparencyFrameBuffer);
		}

		if (m_gameMode != GameMode_FrontEnd)
		{
			// Render the player's face
			if (m_cameraMode != CameraMode_FirstPerson)
			{
				m_pPlayer->RenderFace();

				// Render the player's weapon trails
				m_pPlayer->RenderWeaponTrails();
			}
		}

		// NPC Faces
		m_pNPCManager->RenderFaces();
		m_pNPCManager->RenderWeaponTrails();

		// Enemy Faces
		m_pEnemyManager->RenderFaces();
		m_pEnemyManager->RenderWeaponTrails();

		// Projectile trails
		m_pProjectileManager->RenderWeaponTrails();

		if (m_deferredRendering)
		{
			m_pRenderer->StopRenderingToFrameBuffer(m_transparencyFrameBuffer);
		}
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderSSAOTexture()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);
		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		if (m_multiSampling)
		{
			m_pRenderer->StartRenderingToFrameBuffer(m_FXAAFrameBuffer);
		}
		else if (m_blur || m_pChunkManager->IsUnderWater(m_pGameCamera->GetPosition()))
		{
			m_pRenderer->StartRenderingToFrameBuffer(m_firstPassFullscreenBuffer);
		}

		// SSAO shader
		m_pRenderer->BeginGLSLShader(m_SSAOShader);
		glShader* pShader = m_pRenderer->GetShader(m_SSAOShader);

		unsigned int textureId0 = glGetUniformLocationARB(pShader->GetProgramObject(), "bgl_DepthTexture");
		m_pRenderer->PrepareShaderTexture(0, textureId0);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDepthTextureFromFrameBuffer(m_SSAOFrameBuffer));

		unsigned int textureId1 = glGetUniformLocationARB(pShader->GetProgramObject(), "bgl_RenderedTexture");
		m_pRenderer->PrepareShaderTexture(1, textureId1);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_SSAOFrameBuffer));

		unsigned int textureId2 = glGetUniformLocationARB(pShader->GetProgramObject(), "light");
		m_pRenderer->PrepareShaderTexture(2, textureId2);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_lightingFrameBuffer));

		unsigned int textureId3 = glGetUniformLocationARB(pShader->GetProgramObject(), "bgl_TransparentTexture");
		m_pRenderer->PrepareShaderTexture(3, textureId3);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_transparencyFrameBuffer));

		unsigned int textureId4 = glGetUniformLocationARB(pShader->GetProgramObject(), "bgl_TransparentDepthTexture");
		m_pRenderer->PrepareShaderTexture(4, textureId4);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDepthTextureFromFrameBuffer(m_transparencyFrameBuffer));

		pShader->setUniform1i("screenWidth", m_windowWidth);
		pShader->setUniform1i("screenHeight", m_windowHeight);
		pShader->setUniform1f("nearZ", 0.01f);
		pShader->setUniform1f("farZ", 1000.0f);

		pShader->setUniform1f("samplingMultiplier", 1.5f);

		pShader->setUniform1i("lighting_enabled", m_dynamicLighting);
		pShader->setUniform1i("ssao_enabled", m_ssao);

		m_pRenderer->SetRenderMode(RM_TEXTURED);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 0.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 1.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, (float)m_windowHeight, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, (float)m_windowHeight, 1.0f);
		m_pRenderer->DisableImmediateMode();

		m_pRenderer->EmptyTextureIndex(4);
		m_pRenderer->EmptyTextureIndex(3);
		m_pRenderer->EmptyTextureIndex(2);
		m_pRenderer->EmptyTextureIndex(1);
		m_pRenderer->EmptyTextureIndex(0);

		m_pRenderer->EndGLSLShader(m_SSAOShader);

		if (m_multiSampling)
		{
			m_pRenderer->StopRenderingToFrameBuffer(m_FXAAFrameBuffer);
		}
		else if (m_blur || m_pChunkManager->IsUnderWater(m_pGameCamera->GetPosition()))
		{
			m_pRenderer->StopRenderingToFrameBuffer(m_firstPassFullscreenBuffer);
		}
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderFXAATexture()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);
		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		if (m_blur || m_pChunkManager->IsUnderWater(m_pGameCamera->GetPosition()))
		{
			m_pRenderer->StartRenderingToFrameBuffer(m_firstPassFullscreenBuffer);
		}

		m_pRenderer->BeginGLSLShader(m_fxaaShader);
		glShader* pShader = m_pRenderer->GetShader(m_fxaaShader);

		pShader->setUniform1i("screenWidth", m_windowWidth);
		pShader->setUniform1i("screenHeight", m_windowHeight);

		unsigned int textureId0 = glGetUniformLocationARB(pShader->GetProgramObject(), "texture");
		m_pRenderer->PrepareShaderTexture(0, textureId0);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_FXAAFrameBuffer));

		m_pRenderer->SetRenderMode(RM_TEXTURED);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 0.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 1.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, (float)m_windowHeight, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, (float)m_windowHeight, 1.0f);
		m_pRenderer->DisableImmediateMode();

		m_pRenderer->EmptyTextureIndex(0);

		m_pRenderer->EndGLSLShader(m_fxaaShader);

		if (m_blur || m_pChunkManager->IsUnderWater(m_pGameCamera->GetPosition()))
		{
			m_pRenderer->StopRenderingToFrameBuffer(m_firstPassFullscreenBuffer);
		}
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderFirstPassFullScreen()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);
		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		// Blur first pass (Horizontal)
		m_pRenderer->StartRenderingToFrameBuffer(m_secondPassFullscreenBuffer);
		m_pRenderer->BeginGLSLShader(m_blurHorizontalShader);
		glShader* pShader = m_pRenderer->GetShader(m_blurHorizontalShader);

		unsigned int textureId0 = glGetUniformLocationARB(pShader->GetProgramObject(), "texture");
		m_pRenderer->PrepareShaderTexture(0, textureId0);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_firstPassFullscreenBuffer));

		float blurSize = 0.0015f;

		// Override any blur amount if we have global blur set
		if (m_globalBlurAmount > 0.0f)
		{
			blurSize = m_globalBlurAmount;
		}
		
		pShader->setUniform1f("blurSize", blurSize);

		m_pRenderer->SetRenderMode(RM_TEXTURED);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 0.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 1.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, (float)m_windowHeight, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, (float)m_windowHeight, 1.0f);
		m_pRenderer->DisableImmediateMode();

		m_pRenderer->EmptyTextureIndex(0);

		m_pRenderer->EndGLSLShader(m_blurHorizontalShader);
		m_pRenderer->StopRenderingToFrameBuffer(m_secondPassFullscreenBuffer);
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderSecondPassFullScreen()
{
	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);
		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		// Blur second pass (Vertical)
		m_pRenderer->BeginGLSLShader(m_blurVerticalShader);
		glShader* pShader = m_pRenderer->GetShader(m_blurVerticalShader);

		unsigned int textureId0 = glGetUniformLocationARB(pShader->GetProgramObject(), "texture");
		m_pRenderer->PrepareShaderTexture(0, textureId0);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_secondPassFullscreenBuffer));

		float blurSize = 0.0015f;

		bool applyBlueTint = false;
		if (m_pChunkManager->IsUnderWater(m_pGameCamera->GetPosition()))
		{
			blurSize = 0.0015f;
			applyBlueTint = true;
		}

		// Override any blur amount if we have global blur set
		if (m_globalBlurAmount > 0.0f)
		{
			blurSize = m_globalBlurAmount;
		}
		
		pShader->setUniform1f("blurSize", blurSize);

		glUniform1iARB(glGetUniformLocationARB(pShader->GetProgramObject(), "applyBlueTint"), applyBlueTint);

		m_pRenderer->SetRenderMode(RM_TEXTURED);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 0.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 1.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, (float)m_windowHeight, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, (float)m_windowHeight, 1.0f);
		m_pRenderer->DisableImmediateMode();

		m_pRenderer->EmptyTextureIndex(0);

		m_pRenderer->EndGLSLShader(m_blurVerticalShader);
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderGUI()
{
	m_pRenderer->EmptyTextureIndex(0);

	// Render the GUI
	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);
		m_pRenderer->SetCullMode(CM_BACK);

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		m_pGUI->Render();
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderHUD()
{
	// Render the HUD
	if (m_gameMode == GameMode_Game)
	{
		m_pHUD->Render();
	}
}

void VoxGame::RenderCinematicLetterBox()
{
	float letterboxHeight = 100.0f * m_letterBoxRatio;

	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		m_pRenderer->SetRenderMode(RM_SOLID);

		m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateColourAlpha(0.0f, 0.0f, 0.0f, 1.0f);
			// Bottom
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 1.5f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, 0.0f, 1.5f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, letterboxHeight, 1.5f);
			m_pRenderer->ImmediateVertex(0.0f, letterboxHeight, 1.5f);
			// Top
			m_pRenderer->ImmediateVertex(0.0f, (float)m_windowHeight - letterboxHeight, 1.5f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, (float)m_windowHeight - letterboxHeight, 1.5f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, (float)m_windowHeight, 1.5f);
			m_pRenderer->ImmediateVertex(0.0f, (float)m_windowHeight, 1.5f);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderCrosshair()
{
	if (m_pVoxSettings->m_renderCrosshair == false)
	{
		// If render crosshair is turned off in the options menu
		return;
	}

	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		m_pRenderer->TranslateWorldMatrix(m_windowWidth*0.5f, m_windowHeight*0.5f, 0.0f);

		float crosshairSize = 1.0f;
		float crosshairBorder = 2.0f;
		float crosshairLength = 4.0f;

		m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
			m_pRenderer->ImmediateVertex(-crosshairSize, -crosshairSize, 3.0f);
			m_pRenderer->ImmediateVertex(crosshairSize, -crosshairSize, 3.0f);
			m_pRenderer->ImmediateVertex(crosshairSize, crosshairSize, 3.0f);
			m_pRenderer->ImmediateVertex(-crosshairSize, crosshairSize, 3.0f);

			m_pRenderer->ImmediateColourAlpha(0.0f, 0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(-crosshairBorder, -crosshairBorder, 3.0f);
			m_pRenderer->ImmediateVertex(crosshairBorder, -crosshairBorder, 3.0f);
			m_pRenderer->ImmediateVertex(crosshairBorder, crosshairBorder, 3.0f);
			m_pRenderer->ImmediateVertex(-crosshairBorder, crosshairBorder, 3.0f);
		m_pRenderer->DisableImmediateMode();

	m_pRenderer->PopMatrix();
}

void VoxGame::RenderCustomCursor()
{
	m_pRenderer->PushMatrix();
		//glLoadIdentity();
		m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_pRenderer->SetRenderMode(RM_TEXTURED);
		m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);

		float cursorSize = 28.0f;
		float xMouse = (float)m_pVoxWindow->GetCursorX();
		float yMouse = m_windowHeight - (float)m_pVoxWindow->GetCursorY() - cursorSize;

		if(m_bPressedCursorDown)
		{
			m_pRenderer->BindTexture(m_customCursorClickedBuffer);
		}
		else
		{
			m_pRenderer->BindTexture(m_customCursorNormalBuffer);
		}		

		m_pRenderer->TranslateWorldMatrix(xMouse, yMouse, 240.0f);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);

			m_pRenderer->ImmediateTextureCoordinate(0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 3.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 1.0f);
			m_pRenderer->ImmediateVertex(cursorSize, 0.0f, 3.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 0.0f);
			m_pRenderer->ImmediateVertex(cursorSize, cursorSize, 3.0f);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, cursorSize, 3.0f);
		m_pRenderer->DisableImmediateMode();

		m_pRenderer->DisableTransparency();

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderPaperdollViewport()
{
	if(m_pCharacterGUI->IsLoaded())
	{
		m_pRenderer->StartRenderingToFrameBuffer(m_paperdollBuffer);

		m_pRenderer->PushMatrix();
			glLoadIdentity();

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT);

			m_pRenderer->SetProjectionMode(PM_PERSPECTIVE, m_paperdollViewport);

			m_pRenderer->SetLookAtCamera(vec3(0.0f, 1.3f, 2.85f), vec3(0.0f, 1.3f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

			// Render the player
			m_pRenderer->PushMatrix();
				m_pRenderer->StartMeshRender();

				if(m_modelWireframe == false)
				{
					m_pRenderer->BeginGLSLShader(m_paperdollShader);
				}

				m_pRenderer->RotateWorldMatrix(0.0f, m_paperdollRenderRotation, 0.0f);

				m_pPlayer->RenderPaperdoll();

				if(m_modelWireframe == false)
				{
					m_pRenderer->EndGLSLShader(m_paperdollShader);
				}

				m_pRenderer->EndMeshRender();
				
				glActiveTextureARB(GL_TEXTURE0_ARB);
				glDisable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);

				// Render the block particles
				m_pBlockParticleManager->Render(true);

				if(m_modelWireframe == false)
				{
					m_pRenderer->BeginGLSLShader(m_textureShader);
				}
				
				m_pPlayer->RenderPaperdollFace();

				if(m_modelWireframe == false)
				{
					m_pRenderer->EndGLSLShader(m_textureShader);
				}
			m_pRenderer->PopMatrix();
		m_pRenderer->PopMatrix();

		m_pRenderer->StopRenderingToFrameBuffer(m_paperdollBuffer);
	}
}

void VoxGame::RenderPortraitViewport()
{
	if(m_pHUD->IsLoaded())
	{
		m_pRenderer->StartRenderingToFrameBuffer(m_portraitBuffer);

		m_pRenderer->PushMatrix();
			glLoadIdentity();

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glClearDepth(1.0);
			glClear(GL_DEPTH_BUFFER_BIT);

			m_pRenderer->SetProjectionMode(PM_PERSPECTIVE, m_portraitViewport);

			m_pRenderer->SetLookAtCamera(vec3(0.0f, 2.05f, 1.5f), vec3(0.0f, 2.05f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

			// Render the player
			m_pRenderer->PushMatrix();
				m_pRenderer->StartMeshRender();

				if(m_modelWireframe == false)
				{
					m_pRenderer->BeginGLSLShader(m_paperdollShader);
				}

				m_pRenderer->RotateWorldMatrix(0.0f, -30.0f, 0.0f);

				m_pPlayer->RenderPortrait();

				if(m_modelWireframe == false)
				{
					m_pRenderer->EndGLSLShader(m_paperdollShader);
				}

				m_pRenderer->EndMeshRender();

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glDisable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);

				if(m_modelWireframe == false)
				{
					m_pRenderer->BeginGLSLShader(m_textureShader);
				}

				m_pPlayer->RenderPortraitFace();

				if(m_modelWireframe == false)
				{
					m_pRenderer->EndGLSLShader(m_textureShader);
				}
			m_pRenderer->PopMatrix();
		m_pRenderer->PopMatrix();

		m_pRenderer->StopRenderingToFrameBuffer(m_portraitBuffer);
	}
}

void VoxGame::RenderFirstPersonViewport()
{
	m_pRenderer->PushMatrix();
		glLoadIdentity();

		m_pRenderer->SetProjectionMode(PM_PERSPECTIVE, m_firstpersonViewport);

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 1.3f, -0.5f), vec3(0.0f, 1.3f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		// Render the player first person mode - only hands and weapons
		m_pRenderer->PushMatrix();
			m_pRenderer->StartMeshRender();

			BeginShaderRender();
			{
				m_pPlayer->RenderFirstPerson();
			}
			EndShaderRender();

			m_pRenderer->EndMeshRender();

		m_pRenderer->PopMatrix();
	m_pRenderer->PopMatrix();
}

void VoxGame::RenderDeferredRenderingPaperDoll()
{
	m_pRenderer->StartRenderingToFrameBuffer(m_paperdollSSAOTextureBuffer);

	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_2D, m_paperdollViewport);

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		m_pRenderer->BeginGLSLShader(m_SSAOShader);
		glShader* pShader = m_pRenderer->GetShader(m_SSAOShader);

		unsigned int textureId = glGetUniformLocationARB(pShader->GetProgramObject(), "bgl_DepthTexture");
		glActiveTextureARB(GL_TEXTURE0_ARB);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDepthTextureFromFrameBuffer(m_paperdollBuffer));
		glUniform1iARB(textureId, 0);

		unsigned int textureId2 = glGetUniformLocationARB(pShader->GetProgramObject(), "bgl_RenderedTexture");
		glActiveTextureARB(GL_TEXTURE1_ARB);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_paperdollBuffer));
		glUniform1iARB(textureId2, 1);

		pShader->setUniform1i("screenWidth", m_windowWidth);
		pShader->setUniform1i("screenHeight", m_windowHeight);
		pShader->setUniform1f("nearZ", 0.01f);
		pShader->setUniform1f("farZ", 1000.0f);

		pShader->setUniform1f("samplingMultiplier", 0.5f);
		pShader->setUniform1i("lighting_enabled", 0);

		m_pRenderer->SetRenderMode(RM_TEXTURED);
			m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 2.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 0.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, 0.0f, 2.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 1.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, (float)m_windowHeight, 2.0f);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, (float)m_windowHeight, 2.0f);
		m_pRenderer->DisableImmediateMode();

		glActiveTextureARB(GL_TEXTURE3_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTextureARB(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_pRenderer->EndGLSLShader(m_SSAOShader);
	m_pRenderer->PopMatrix();

	m_pRenderer->StopRenderingToFrameBuffer(m_paperdollSSAOTextureBuffer);
}

void VoxGame::RenderDeferredRenderingPortrait()
{
	m_pRenderer->StartRenderingToFrameBuffer(m_portraitSSAOTextureBuffer);

	m_pRenderer->PushMatrix();
		m_pRenderer->SetProjectionMode(PM_2D, m_portraitViewport);

		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		m_pRenderer->BeginGLSLShader(m_SSAOShader);
		glShader* pShader = m_pRenderer->GetShader(m_SSAOShader);

		unsigned int textureId = glGetUniformLocationARB(pShader->GetProgramObject(), "bgl_DepthTexture");
		glActiveTextureARB(GL_TEXTURE0_ARB);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDepthTextureFromFrameBuffer(m_portraitBuffer));
		glUniform1iARB(textureId, 0);

		unsigned int textureId2 = glGetUniformLocationARB(pShader->GetProgramObject(), "bgl_RenderedTexture");
		glActiveTextureARB(GL_TEXTURE1_ARB);
		m_pRenderer->BindRawTextureId(m_pRenderer->GetDiffuseTextureFromFrameBuffer(m_portraitBuffer));
		glUniform1iARB(textureId2, 1);

		pShader->setUniform1i("screenWidth", m_windowWidth);
		pShader->setUniform1i("screenHeight", m_windowHeight);
		pShader->setUniform1f("nearZ", 0.01f);
		pShader->setUniform1f("farZ", 1000.0f);

		pShader->setUniform1f("samplingMultiplier", 0.5f);
		pShader->setUniform1i("lighting_enabled", 0);

		m_pRenderer->SetRenderMode(RM_TEXTURED);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 2.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 0.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, 0.0f, 2.0f);
			m_pRenderer->ImmediateTextureCoordinate(1.0f, 1.0f);
			m_pRenderer->ImmediateVertex((float)m_windowWidth, (float)m_windowHeight, 2.0f);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 1.0f);
			m_pRenderer->ImmediateVertex(0.0f, (float)m_windowHeight, 2.0f);
		m_pRenderer->DisableImmediateMode();

		glActiveTextureARB(GL_TEXTURE3_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTextureARB(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTextureARB(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_pRenderer->EndGLSLShader(m_SSAOShader);
	m_pRenderer->PopMatrix();

	m_pRenderer->StopRenderingToFrameBuffer(m_portraitSSAOTextureBuffer);
}

void VoxGame::RenderDebugInformation()
{
	char lCameraBuff[256];
	sprintf(lCameraBuff, "Pos(%.2f, %.2f, %.2f), Facing(%.2f, %.2f, %.2f) = %.2f, Up(%.2f, %.2f, %.2f) = %.2f, Right(%.2f, %.2f, %.2f) = %.2f, View(%.2f, %.2f, %.2f), Zoom=%.2f",
		m_pGameCamera->GetPosition().x, m_pGameCamera->GetPosition().y, m_pGameCamera->GetPosition().z,
		m_pGameCamera->GetFacing().x, m_pGameCamera->GetFacing().y, m_pGameCamera->GetFacing().z, length(m_pGameCamera->GetFacing()),
		m_pGameCamera->GetUp().x, m_pGameCamera->GetUp().y, m_pGameCamera->GetUp().z, length(m_pGameCamera->GetUp()),
		m_pGameCamera->GetRight().x, m_pGameCamera->GetRight().y, m_pGameCamera->GetRight().z, length(m_pGameCamera->GetRight()),
		m_pGameCamera->GetView().x, m_pGameCamera->GetView().y, m_pGameCamera->GetView().z,
		m_pGameCamera->GetZoomAmount());

	char lDrawingBuff[256];
	sprintf(lDrawingBuff, "Vertices: %i, Faces: %i", m_pRenderer->GetNumRenderedVertices(), m_pRenderer->GetNumRenderedFaces());
	char lChunksBuff[256];
	sprintf(lChunksBuff, "Chunks: %i, Render: %i", m_pChunkManager->GetNumChunksLoaded(), m_pChunkManager->GetNumChunksRender());
	char lParticlesBuff[256];
	sprintf(lParticlesBuff, "Particles: %i, Render: %i, Emitters: %i, Effects: %i", m_pBlockParticleManager->GetNumBlockParticles(), m_pBlockParticleManager->GetNumRenderableParticles(false), m_pBlockParticleManager->GetNumBlockParticleEmitters(), m_pBlockParticleManager->GetNumBlockParticleEffects());
	char lItemsBuff[256];
	sprintf(lItemsBuff, "Items: %i, Render: %i", m_pItemManager->GetNumItems(), m_pItemManager->GetNumRenderItems());
	char lNPCBuff[256];
	sprintf(lNPCBuff, "NPCs: %i, Render: %i", m_pNPCManager->GetNumNPCs(), m_pNPCManager->GetNumRenderNPCs());
	char lEnemiesBuff[256];
	sprintf(lEnemiesBuff, "Enemies: %i, Render: %i", m_pEnemyManager->GetNumEnemies(), m_pEnemyManager->GetNumRenderEnemies());
	char lProjectilesBuff[256];
	sprintf(lProjectilesBuff, "Projectiles: %i, Render: %i", m_pProjectileManager->GetNumProjectiles(), m_pProjectileManager->GetNumRenderProjectiles());
	char lInstancesBuff[256];
	sprintf(lInstancesBuff,  "Instance Parents: %i, Instance Objects: %i, Instance Render: %i", m_pInstanceManager->GetNumInstanceParents(), m_pInstanceManager->GetTotalNumInstanceObjects(), m_pInstanceManager->GetTotalNumInstanceRenderObjects());

	char lFPSBuff[128];
	float fpsWidthOffset = 65.0f;
	if (m_debugRender)
	{
		sprintf(lFPSBuff, "Delta: %.4f  FPS: %.0f", m_deltaTime, m_fps);
		fpsWidthOffset = 135.0f;
	}
	else
	{
		sprintf(lFPSBuff, "FPS: %.0f", m_fps);
	}

	char lBuildInfo[128];
#if defined(_DEBUG) || defined(NDEBUG)
	sprintf(lBuildInfo, "DEV %s", m_pVoxSettings->m_version.c_str());
#else
	sprintf(lBuildInfo, "RELEASE %s", m_pVoxSettings->m_version.c_str());
#endif //defined(_DEBUG) || defined(NDEBUG)

	int l_nTextHeight = m_pRenderer->GetFreeTypeTextHeight(m_defaultFont, "a");

	m_pRenderer->PushMatrix();
		m_pRenderer->EmptyTextureIndex(0);

		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->SetProjectionMode(PM_2D, m_defaultViewport);
		m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		if (m_debugRender)
		{
			m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, m_windowHeight - (l_nTextHeight * 1) - 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lCameraBuff);
			m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, m_windowHeight - (l_nTextHeight*2) - 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lDrawingBuff);
			m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, m_windowHeight - (l_nTextHeight * 3) - 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lChunksBuff);
			m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, m_windowHeight - (l_nTextHeight * 4) - 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lParticlesBuff);
			m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, m_windowHeight - (l_nTextHeight * 5) - 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lItemsBuff);
			m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, m_windowHeight - (l_nTextHeight * 6) - 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lNPCBuff);
			m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, m_windowHeight - (l_nTextHeight * 7) - 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lEnemiesBuff);
			m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, m_windowHeight - (l_nTextHeight * 8) - 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lProjectilesBuff);
			m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, m_windowHeight - (l_nTextHeight * 9) - 10.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lInstancesBuff);
		}

		if (STEAM_BUILD == false)
		{
			m_pRenderer->RenderFreeTypeText(m_defaultFont, m_windowWidth - fpsWidthOffset, 15.0f, 1.0f, Colour(1.0f, 1.0f, 1.0f), 1.0f, lFPSBuff);
		}
		m_pRenderer->RenderFreeTypeText(m_defaultFont, 15.0f, 15.0f, 1.0f, Colour(0.75f, 0.75f, 0.75f), 1.0f, lBuildInfo);

	m_pRenderer->PopMatrix();
}
