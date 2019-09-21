// ******************************************************************************
// Filename:    VoxCamera.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 27/10/15
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "VoxGame.h"

// Camera controls
void VoxGame::UpdateCamera(float dt)
{
	if (m_pPlayer->IsDead() == false)
	{
		if (m_bPaused == false)
		{
			if (m_cameraMode == CameraMode_MouseRotate)
			{
				UpdateCameraAutoCamera(dt, false);
			}
			else if (m_cameraMode == CameraMode_AutoCamera)
			{
				UpdateCameraAutoCamera(dt, true);
			}
			else if (m_cameraMode == CameraMode_FirstPerson)
			{
				UpdateCameraFirstPerson(dt);
			}
			else if (m_cameraMode == CameraMode_NPCDialog)
			{
				UpdateCameraNPCDialog(dt);
			}
			else if (m_cameraMode == CameraMode_EnemyTarget)
			{
				UpdateCameraEnemyTarget(dt);
			}			
		}

		if (m_gameMode == GameMode_Game && m_cameraMode != CameraMode_Debug)
		{
			UpdateCameraModeSwitching();
		}
	}
}

void VoxGame::UpdateCameraModeSwitching()
{
	if (m_cameraDistance < 1.5f)
	{
		if (m_cameraMode != CameraMode_FirstPerson)
		{
			SavePreviousCameraMode();
			SetCameraMode(CameraMode_FirstPerson);
			m_pPlayer->SetFirstPersonMode(); // Enter first person mode
			m_pGameCamera->SetZoomAmount(1.5f);
		}
	}
	else
	{
		if (m_cameraMode == CameraMode_FirstPerson)
		{
			if (m_previousCameraMode == CameraMode_MouseRotate)
			{
				m_pMouseRotateCameraOptionBox->SetToggled(true);
			}
			else if (m_previousCameraMode == CameraMode_AutoCamera)
			{
				m_pAutoCameraOptionBox->SetToggled(true);
			}

			m_cameraDistance = 1.5f;
			m_maxCameraDistance = 1.5f;

			m_pPlayer->SetThirdPersonMode(); // Enter third person mode

			RestorePreviousCameraMode();
			InitializeCameraRotation();
		}
	}
}

void VoxGame::InitializeCameraRotation()
{
	m_currentX = m_pVoxWindow->GetCursorX();
	m_currentY = m_pVoxWindow->GetCursorY();

	vec3 ratios = normalize(vec3(2.5f, 1.0f, 0.0f));

	m_cameraBehindPlayerPosition = m_pPlayer->GetCenter() + Player::PLAYER_CENTER_OFFSET;
	m_cameraBehindPlayerPosition += m_pPlayer->GetForwardVector() * -(m_cameraDistance*ratios.x);
	m_cameraBehindPlayerPosition += m_pPlayer->GetUpVector() * (m_cameraDistance*ratios.y);

	// Only set the position, since the player will be controlling the rotation of the camera
	m_pGameCamera->SetFakePosition(m_cameraBehindPlayerPosition);
}

void VoxGame::UpdateCameraAutoCamera(float dt, bool updateCameraPosition)
{
	if (updateCameraPosition)
	{
		float movementModifierNormal = 10.0f;
		float movementModifierMoving = 0.125f;
		float movementModifierChangeAmount = 2.0f;
		if (m_keyboardMovement || m_gamepadMovement)
		{
			m_autoCameraMovingModifier = movementModifierMoving;
		}
		else
		{
			if (m_autoCameraMovingModifier < movementModifierNormal)
			{
				m_autoCameraMovingModifier += movementModifierChangeAmount * dt;
			}
			else
			{
				m_autoCameraMovingModifier = movementModifierNormal;
			}
		}

		vec3 ratios = normalize(vec3(2.5f, 1.0f, 0.0f));
		float catchupSpeed = 1.0f * (1.0f - (m_cameraDistance / 20.0f)) * m_autoCameraMovingModifier;
		vec3 cameraFacing = m_pGameCamera->GetFacing();
		cameraFacing.y = 0.0f;

		m_cameraBehindPlayerPosition = m_pPlayer->GetCenter() + Player::PLAYER_CENTER_OFFSET;
		m_cameraBehindPlayerPosition += m_pPlayer->GetForwardVector() * -(m_cameraDistance*ratios.x);
		m_cameraBehindPlayerPosition += m_pPlayer->GetUpVector() * (m_cameraDistance*ratios.y);

		m_cameraPosition_AutoModeCached = m_cameraBehindPlayerPosition;

		// Update the target vectors based on the cached and calculated values
		{
			vec3 newPos = m_cameraPosition_AutoModeCached;
			vec3 toPos = newPos - m_cameraPosition_AutoMode;
			m_cameraPosition_AutoMode += toPos * (catchupSpeed * 2.0f) * dt;
		}

		// Position
		vec3 posDiff = m_cameraPosition_AutoMode - m_pGameCamera->GetFakePosition();
		vec3 newPos = vec3(m_pGameCamera->GetFakePosition().x, m_pGameCamera->GetFakePosition().y + ((posDiff.y * catchupSpeed) * dt), m_pGameCamera->GetFakePosition().z);


		vec3 cameraLookAt = m_pPlayer->GetCenter() + Player::PLAYER_CENTER_OFFSET;
		vec3 targetFacing = normalize(cameraLookAt - m_cameraPosition_AutoMode);
		targetFacing.y = 0.0f;
		targetFacing = normalize(targetFacing);
		vec3 camFacing = m_pGameCamera->GetFacing();
		camFacing.y = 0.0f;
		camFacing = normalize(camFacing);
		vec3 crossResult = cross(targetFacing, camFacing);
		float dotResult = dot(targetFacing, camFacing);
		float rotationDegrees = RadToDeg(acos(dotResult));
		if(rotationDegrees > 1.0f)
		{
			if (crossResult.y > 0.0f)
			{
				rotationDegrees = -rotationDegrees;
			}
			float changeAmount = rotationDegrees * 1.0f * m_autoCameraMovingModifier;
			m_pGameCamera->RotateAroundPointY(changeAmount * dt, true);
		}
	}

	// Forward
	vec3 cameraLookAt = m_pPlayer->GetCenter() + Player::PLAYER_CENTER_OFFSET;
	vec3 cameraForward = normalize(cameraLookAt - m_pGameCamera->GetFakePosition());
	m_pGameCamera->SetFacing(cameraForward);
	
	// Right
	vec3 cameraRight = normalize(cross(cameraForward, m_pPlayer->GetUpVector()));
	m_pGameCamera->SetRight(cameraRight);

	// Up
	vec3 cameraUp = normalize(cross(cameraRight, cameraForward));
	m_pGameCamera->SetUp(cameraUp);
}

void VoxGame::UpdateCameraFirstPerson(float dt)
{
	m_pGameCamera->SetFakePosition(m_pPlayer->GetCenter() + Player::PLAYER_CENTER_OFFSET);
	m_pPlayer->SetForwardVector(m_pGameCamera->GetFacing());
}

void VoxGame::UpdateCameraNPCDialog(float dt)
{
	// Position
	vec3 diff = m_targetCameraPosition_NPCDialog - m_pGameCamera->GetPosition();
	m_pGameCamera->SetFakePosition(m_pGameCamera->GetPosition() + ((diff * 5.0f) * dt));

	// Forward
	diff = m_targetCameraView_NPCDialog - m_pGameCamera->GetView();
	vec3 cameraLookAt = m_pGameCamera->GetView() + ((diff * 5.0f) * dt);
	vec3 cameraForward = normalize(cameraLookAt - m_pGameCamera->GetFakePosition());
	m_pGameCamera->SetFacing(cameraForward);

	// Right
	vec3 cameraRight = normalize(cross(cameraForward, m_pPlayer->GetUpVector()));
	m_pGameCamera->SetRight(cameraRight);

	// Up
	vec3 cameraUp = normalize(cross(cameraRight, cameraForward));
	m_pGameCamera->SetUp(cameraUp);
}

void VoxGame::UpdateCameraEnemyTarget(float dt)
{
	// Target camera position
	vec3 TargetCameraPosition = m_pPlayer->GetCenter();
	TargetCameraPosition += m_pPlayer->GetRightVector() * (-2.0f * m_targetCameraXAxisAmount);
	TargetCameraPosition += m_pPlayer->GetForwardVector() * (-1.0f * m_targetCameraForwardRatio);
	TargetCameraPosition += m_pPlayer->GetUpVector() * (0.75f * m_targetCameraYRatio);

	// Target camera view
	vec3 toTarget = m_pPlayer->GetTargetEnemy()->GetProjectileHitboxCenter() - m_pPlayer->GetCenter();
	vec3 TargetCameraView = m_pPlayer->GetCenter() + toTarget*0.65f;

	float toTargetDistance = length(toTarget);
	float ratio = (toTargetDistance - 5.0f) / 15.0f;
	if (ratio < 0.0f)
	{
		ratio = 0.0f;
	}
	if (ratio > 1.0f)
	{
		ratio = 1.0f;
	}

	float lUpVectorRatioTarget = 0.0f;
	if (toTarget.y > 2.0f && toTarget.y > toTargetDistance*0.25f)
	{
		lUpVectorRatioTarget = -0.1f * ratio;
	}
	else
	{
		lUpVectorRatioTarget = 2.0f + (1.5f * (1.0f - ratio));
	}

	m_targetCameraYRatio = lUpVectorRatioTarget;

	// Lag on the y Ratio updating
	float yRatioDiff = fabs(lUpVectorRatioTarget - m_targetCameraYRatio);
	if (m_targetCameraYRatio > lUpVectorRatioTarget)
	{
		m_targetCameraYRatio -= (yRatioDiff * 3.0f) * dt;
	}
	else if (m_targetCameraYRatio < lUpVectorRatioTarget)
	{
		m_targetCameraYRatio += (yRatioDiff * 3.0f) * dt;
	}

	// Lag the forward vector ratio
	float lForwardVectorRatioTarget = (2.0f + (1.5f * (1.0f - ratio))) * 0.75f;
	float forwardRatioDiff = fabs(lForwardVectorRatioTarget - m_targetCameraForwardRatio);
	if (m_targetCameraForwardRatio > lForwardVectorRatioTarget)
	{
		m_targetCameraForwardRatio -= (forwardRatioDiff * 3.0f) * dt;
	}
	else if (m_targetCameraForwardRatio < lForwardVectorRatioTarget)
	{
		m_targetCameraForwardRatio += (forwardRatioDiff * 3.0f) * dt;
	}

	// Lag on the X amount
	float XAxisDiff = fabs(m_targetCameraXAxisAmount_Target - m_targetCameraXAxisAmount);
	if (m_targetCameraXAxisAmount > m_targetCameraXAxisAmount_Target)
	{
		m_targetCameraXAxisAmount -= (XAxisDiff * 3.0f) * dt;
	}
	else if (m_targetCameraXAxisAmount < m_targetCameraXAxisAmount_Target)
	{
		m_targetCameraXAxisAmount += (XAxisDiff * 3.0f) * dt;
	}


	vec3 targetPosition = TargetCameraPosition;
	vec3 targetView = TargetCameraView;

	vec3 diffPos = targetPosition - m_pGameCamera->GetPosition();
	vec3 diffView = targetView - m_pGameCamera->GetView();

	// Position
	m_pGameCamera->SetFakePosition(m_pGameCamera->GetPosition() + ((diffPos * 3.0f) * dt));

	// Forward
	vec3 cameraLookAt = m_pGameCamera->GetView() + ((diffView * 3.0f) * dt);
	vec3 cameraForward = normalize(cameraLookAt - m_pGameCamera->GetFakePosition());
	m_pGameCamera->SetFacing(cameraForward);

	// Right
	vec3 cameraRight = normalize(cross(cameraForward, m_pPlayer->GetUpVector()));
	m_pGameCamera->SetRight(cameraRight);

	// Up
	vec3 cameraUp = normalize(cross(cameraRight, cameraForward));
	m_pGameCamera->SetUp(cameraUp);
}

void VoxGame::UpdateCameraClipping(float dt)
{
	vec3 cameraPosition = m_targetCameraPositionBeforeClipping;

	if (m_gameMode == GameMode_Game && m_cameraMode != CameraMode_Debug)
	{
		int numIterations = 0;
		int maxNumIterations = 100;
		vec3 basePos;
		vec3 testPos;
		vec3 cameraFacingUnit = normalize(m_pGameCamera->GetFacing());
		bool collides = true;
		int blockX, blockY, blockZ;
		vec3 blockPos;
		bool active = false;
		vec3 toPlayer = ((m_pPlayer->GetCenter() + Player::PLAYER_CENTER_OFFSET) - m_pGameCamera->GetFakePosition());
		float distance = length(toPlayer);
		float incrementAmount = distance / maxNumIterations;
		while (collides == true && numIterations < maxNumIterations)
		{
			collides = false;
			basePos = cameraPosition;

			vec3 playerRight = m_pPlayer->GetRightVector() * 0.25f;
			testPos = basePos + playerRight;
			Chunk* pChunk = nullptr;
			active = m_pChunkManager->GetBlockActiveFrom3DPosition(testPos.x, testPos.y, testPos.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);
			if (active)
			{
				collides = true;
			}

			playerRight = m_pPlayer->GetRightVector() * -0.25f;
			testPos = basePos + playerRight;
			pChunk = nullptr;
			active = m_pChunkManager->GetBlockActiveFrom3DPosition(testPos.x, testPos.y, testPos.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);
			if (active)
			{
				collides = true;
			}

			vec3 playerUp = m_pPlayer->GetUpVector() * -0.25f;
			testPos = basePos + playerUp;
			pChunk = nullptr;
			active = m_pChunkManager->GetBlockActiveFrom3DPosition(testPos.x, testPos.y, testPos.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);
			if (active)
			{
				collides = true;
			}

			playerUp = m_pPlayer->GetUpVector() * 0.25f;
			testPos = basePos + playerUp;
			pChunk = nullptr;
			active = m_pChunkManager->GetBlockActiveFrom3DPosition(testPos.x, testPos.y, testPos.z, &blockPos, &blockX, &blockY, &blockZ, &pChunk);
			if (active)
			{
				collides = true;
			}

			if (collides == true)
			{
				cameraPosition += m_pGameCamera->GetFacing() * incrementAmount;
			}

			numIterations++;
		}
	}

	m_cameraPositionAfterClipping = cameraPosition;
	m_pGameCamera->SetPosition(m_cameraPositionAfterClipping);
}

void VoxGame::UpdateCameraZoom(float dt)
{
	// Make sure we gradually move inwards/outwards
	if (m_cameraMode != CameraMode_FirstPerson)
	{
		float camDiff = fabs(m_cameraDistance - m_maxCameraDistance);
		float changeAmount = 0.0f;
		if (m_cameraDistance < m_maxCameraDistance)
		{
			changeAmount = camDiff * dt;
		}
		else if (m_cameraDistance >= m_maxCameraDistance)
		{
			changeAmount = -camDiff * dt;
		}

		m_cameraDistance += changeAmount;
		m_pGameCamera->Zoom(changeAmount, true);
	}
}

bool VoxGame::ShouldRestorePreviousCameraMode()
{
	return m_shouldRestorePreviousCameraMode;
}

void VoxGame::SavePreviousCameraMode()
{
	m_previousCameraMode = m_cameraMode;
}

void VoxGame::RestorePreviousCameraMode()
{
	SetCameraMode(m_previousCameraMode);
	m_shouldRestorePreviousCameraMode = false;
}