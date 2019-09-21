// ******************************************************************************
// Filename:    VoxControls.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 27/10/15
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "VoxGame.h"

// Controls
void VoxGame::UpdateControls(float dt)
{
	if (m_gamepadMovement == false)
	{
		UpdateKeyboardControls(dt);
		UpdateMouseControls(dt);
	}

	if (m_keyboardMovement == false)
	{
		if (m_pVoxWindow->IsJoyStickConnected(0))
		{
			UpdateGamePadControls(dt);
		}
	}
}

void VoxGame::UpdateKeyboardControls(float dt)
{
	GameMode gameMode = GetGameMode();

	if (gameMode == GameMode_Debug || m_cameraMode == CameraMode_Debug)
	{
		// Keyboard camera movements
		if (m_bKeyboardForward)
		{
			m_pGameCamera->Fly(20.0f * dt, true);
		}
		if (m_bKeyboardBackward)
		{
			m_pGameCamera->Fly(-20.0f * dt, true);
		}
		if (m_bKeyboardStrafeLeft)
		{
			m_pGameCamera->Strafe(-20.0f * dt, true);
		}
		if (m_bKeyboardStrafeRight)
		{
			m_pGameCamera->Strafe(20.0f * dt, true);
		}
		if (m_bKeyboardUp)
		{
			m_pGameCamera->Levitate(20.0f * dt, true);
		}
		if (m_bKeyboardDown)
		{
			m_pGameCamera->Levitate(-20.0f * dt, true);
		}
	}
	else if (gameMode == GameMode_Game)
	{
		// Pause menu
		if (m_bKeyboardMenu)
		{
			m_bKeyboardMenu = false;

			if (IsGUIWindowStillDisplayed())
			{
				CloseAllGUIWindows();

				if (IsGUIWindowStillDisplayed() == false)
				{
					TurnCursorOff(false);
				}
			}
			else
			{
				if (m_pFrontendManager->GetFrontendScreen() == FrontendScreen_None)
				{
					SetPauseMenu();
				}
				else if (m_pFrontendManager->GetFrontendScreen() == FrontendScreen_PauseMenu)
				{
					UnsetPauseMenu();
				}
				else if (m_pFrontendManager->GetFrontendScreen() == FrontendScreen_OptionsMenu)
				{
					m_pFrontendManager->SetFrontendScreen(FrontendScreen_PauseMenu);
				}
				else if (m_pFrontendManager->GetFrontendScreen() == FrontendScreen_ModMenu)
				{
					m_pFrontendManager->SetFrontendScreen(FrontendScreen_PauseMenu);
				}
			}

			// Cancel quit popup menu with escape button
			if (m_pFrontendManager->GetFrontendScreen() == FrontendScreen_QuitPopup)
			{
				VoxGame::GetInstance()->CancelQuitPopup();
			}
		}

		if (m_pPlayer->IsDead() == false)
		{
			if (m_bPaused == false)  // If we are paused, don't allow gameplay control movements
			{
				if (IsGUIWindowStillDisplayed() == true)
				{
					// Reset movement speed since we have a GUI window open
					m_movementSpeed = 0.0f;
				}
				else
				{
					// Jumping
					if (m_bKeyboardSpace)
					{
						m_pPlayer->Jump();
					}

					// Attacking
					if (m_bAttackPressed_Mouse && m_bCanDoAttack_Mouse)
					{
						// Check interactions
						bool interaction = CheckInteractions();
						if (interaction == false)
						{
							m_pPlayer->PressAttack();
						}

						m_bCanDoAttack_Mouse = false;
					}
					if (m_bAttackReleased_Mouse)
					{
						m_pPlayer->ReleaseAttack();
						m_bAttackReleased_Mouse = false;
					}

					// Player movements
					bool resetMovementVector = false;
					if (m_bKeyboardForward == false && m_bKeyboardBackward == false && m_bKeyboardStrafeLeft == false && m_bKeyboardStrafeRight == false)
					{
						// Reduce the movement speed (drag)
						m_movementSpeed -= m_maxMovementSpeed / (m_movementDragTime / dt);

						if (m_movementSpeed <= 0.0f)
						{
							m_movementSpeed = 0.0f;
							m_keyboardMovement = false;
							m_pPlayer->StopMoving();
						}
					}
					else
					{
						m_keyboardMovement = true;

						// Increase the movement speed since we are pressing a movement key
						m_movementSpeed += m_maxMovementSpeed / (m_movementIncreaseTime / dt);

						// Don't allow faster than max movement
						if (m_movementSpeed > m_maxMovementSpeed)
						{
							m_movementSpeed = m_maxMovementSpeed;
						}
					}

					float targetModeMovementRatio = 0.65f;

					// Forwards, backwards, strafe, left, right directional movement
					if (m_bKeyboardForward)
					{
						if (resetMovementVector == false)
						{
							m_movementDirection = vec3(0.0f, 0.0f, 0.0f);
							resetMovementVector = true;
						}

						if (m_pPlayer->GetTargetEnemy() == nullptr)
						{
							vec3 cameraRight = m_pGameCamera->GetRight();
							vec3 playerUp = m_pPlayer->GetUpVector();
							vec3 moveDirection = normalize(cross(cameraRight, playerUp));
							m_movementDirection -= moveDirection;
						}
						else
						{
							m_pPlayer->Move(m_movementSpeed * targetModeMovementRatio * dt);
						}
					}

					if (m_bKeyboardBackward)
					{
						if (resetMovementVector == false)
						{
							m_movementDirection = vec3(0.0f, 0.0f, 0.0f);
							resetMovementVector = true;
						}

						if (m_pPlayer->GetTargetEnemy() == nullptr)
						{
							vec3 cameraRight = m_pGameCamera->GetRight();
							vec3 playerUp = m_pPlayer->GetUpVector();
							vec3 moveDirection = normalize(cross(cameraRight, playerUp));
							m_movementDirection += moveDirection;
						}
						else
						{
							m_pPlayer->Move(-m_movementSpeed * targetModeMovementRatio * dt);
						}
					}

					if (m_bKeyboardStrafeLeft)
					{
						if (resetMovementVector == false)
						{
							m_movementDirection = vec3(0.0f, 0.0f, 0.0f);
							resetMovementVector = true;
						}

						if (m_pPlayer->GetTargetEnemy() == nullptr)
						{
							vec3 cameraRight = m_pGameCamera->GetRight();
							vec3 moveDirection = -cameraRight;
							m_movementDirection += moveDirection;
						}
						else
						{
							m_targetCameraXAxisAmount_Target += (0.75f * dt);
							if (m_targetCameraXAxisAmount_Target > 1.0f)
							{
								m_targetCameraXAxisAmount_Target = 1.0f;
							}

							m_pPlayer->Strafe(m_movementSpeed * targetModeMovementRatio * dt);
						}
					}

					if (m_bKeyboardStrafeRight)
					{
						if (resetMovementVector == false)
						{
							m_movementDirection = vec3(0.0f, 0.0f, 0.0f);
							resetMovementVector = true;
						}

						if (m_pPlayer->GetTargetEnemy() == nullptr)
						{
							vec3 cameraRight = m_pGameCamera->GetRight();
							vec3 moveDirection = -cameraRight;
							m_movementDirection -= moveDirection;
						}
						else
						{
							m_targetCameraXAxisAmount_Target -= (0.75f * dt);
							if (m_targetCameraXAxisAmount_Target < -1.0f)
							{
								m_targetCameraXAxisAmount_Target = -1.0f;
							}

							m_pPlayer->Strafe(-m_movementSpeed * targetModeMovementRatio * dt);
						}
					}

					if (length(m_movementDirection) > 0.001f && m_movementSpeed > m_movementStopThreshold)
					{
						bool shouldChangePlayerFacing = (m_cameraMode != CameraMode_FirstPerson);

						m_movementDirection = normalize(m_movementDirection);
						vec3 amountMoved = m_pPlayer->MoveAbsolute(m_movementDirection, m_movementSpeed * dt, shouldChangePlayerFacing);
						m_pGameCamera->SetFakePosition(m_pGameCamera->GetFakePosition() + amountMoved);
					}
				}
			}
		}
	}
}

void VoxGame::UpdateMouseControls(float dt)
{
	GameMode gameMode = GetGameMode();
	if (gameMode == GameMode_Debug || m_cameraMode == CameraMode_Debug)
	{
		if (m_bCameraRotate)
		{
			MouseCameraRotate();
		}
	}
	else if (gameMode == GameMode_Game)
	{
		if (m_pPlayer->IsDead() == false)
		{
			if (m_bPaused == false)  // If we are paused, don't allow gameplay control movements
			{
				if (IsGUIWindowStillDisplayed() == false)
				{
					if (m_cameraMode == CameraMode_MouseRotate)
					{
						MouseCameraRotate();
					}
					else if (m_cameraMode == CameraMode_AutoCamera)
					{
						/* Do nothing, camera rotation is not controlled by player. */
					}
					else if (m_cameraMode == CameraMode_FirstPerson)
					{
						MouseCameraRotate();
					}
				}
			}
		}
	}
}

void VoxGame::UpdateGamePadControls(float dt)
{
	JoystickCameraZoom(dt);

	m_bJoystickJump = m_pVoxWindow->GetJoystickButton(0, 0);
	if (m_bAttackPressed_Joystick)
	{
		m_bCanDoAttack_Joystick = m_pVoxWindow->GetJoystickAxisValue(0, 2) >= -0.75f;
		m_bAttackReleased_Joystick = m_pVoxWindow->GetJoystickAxisValue(0, 2) >= -0.75f;
	}
	m_bAttackPressed_Joystick = m_pVoxWindow->GetJoystickAxisValue(0, 2) <= -0.75f;

	m_bTargetEnemyPressed_Joystick = false; // TODO : Need to get newer version of glfw to support different triggers for LT, RT
	m_bTargetEnemyReleased_Joystick = false; // TODO : Need to get newer version of glfw to support different triggers for LT, RT

	GameMode gameMode = GetGameMode();
	if (gameMode == GameMode_Debug || m_cameraMode == CameraMode_Debug)
	{
		JoystickCameraMove(dt);
		JoystickCameraRotate(dt);
	}
	else if (gameMode == GameMode_Game)
	{
		if (m_pPlayer->IsDead() == false)
		{
			if (m_bPaused == false)  // If we are paused, don't allow gameplay control movements
			{
				if (IsGUIWindowStillDisplayed() == true)
				{
					// Reset movement speed since we have a GUI window open
					m_movementSpeed = 0.0f;
				}
				else
				{
					if (m_cameraMode == CameraMode_MouseRotate)
					{
						JoystickCameraRotate(dt);
					}
					else if (m_cameraMode == CameraMode_AutoCamera)
					{
						/* Do nothing, camera rotation is not controlled by player. */
					}
					else if (m_cameraMode == CameraMode_FirstPerson)
					{
						JoystickCameraRotate(dt);
					}

					// Jumping
					if (m_bJoystickJump)
					{
						m_pPlayer->Jump();
					}

					// Enemy targetting
					if (m_cameraMode != CameraMode_FirstPerson)
					{
						if (m_bTargetEnemyPressed_Joystick)
						{
							if(m_pPlayer->GetTargetEnemy() == nullptr)
							{
								SetEnemyTarget();
							}
						}
						if (m_bTargetEnemyReleased_Joystick)
						{
							if (m_pPlayer->GetTargetEnemy() != nullptr)
							{
								ReleaseEnemyTarget();
							}
						}
					}

					// Attacking
					if (m_bAttackPressed_Joystick && m_bCanDoAttack_Joystick)
					{
						// Check interactions
						bool interaction = CheckInteractions();
						if (interaction == false)
						{
							m_pPlayer->PressAttack();
						}
						m_bCanDoAttack_Joystick = false;
					}
					if (m_bAttackReleased_Joystick)
					{
						m_pPlayer->ReleaseAttack();
						m_bAttackReleased_Joystick = false;
					}

					// Player movements
					float axisX = m_pVoxWindow->GetJoystickAxisValue(0, 0);
					float axisY = m_pVoxWindow->GetJoystickAxisValue(0, 1);

					// Dead zones
					if (fabs(axisX) < m_pVoxWindow->GetJoystickAnalogDeadZone())
					{
						axisX = 0.0f;
					}
					if (fabs(axisY) < m_pVoxWindow->GetJoystickAnalogDeadZone())
					{
						axisY = 0.0f;
					}

					if (fabs(axisX) <= 0.0f && fabs(axisY) <= 0.0f)
					{
						// Reduce the movement speed (drag)
						m_movementSpeed -= m_maxMovementSpeed / (m_movementDragTime / dt);

						if (m_movementSpeed <= 0.0f)
						{
							m_movementSpeed = 0.0f;
							m_gamepadMovement = false;
							m_pPlayer->StopMoving();
						}
					}
					else
					{
						m_gamepadMovement = true;

						// Increase the movement speed since we are pressing a movement key
						m_movementSpeed += m_maxMovementSpeed / (m_movementIncreaseTime / dt);

						// Don't allow faster than max movement
						if (m_movementSpeed > m_maxMovementSpeed)
						{
							m_movementSpeed = m_maxMovementSpeed;
						}
					}

					if (m_pPlayer->GetTargetEnemy() == nullptr)
					{
						vec3 cameraRight = m_pGameCamera->GetRight();
						vec3 playerUp = m_pPlayer->GetUpVector();
						vec3 moveDirection = normalize(cross(cameraRight, playerUp));
						m_movementDirection += moveDirection * axisY;
						m_movementDirection += cameraRight * axisX;

						if (length(m_movementDirection) > 0.001f && m_movementSpeed > m_movementStopThreshold)
						{
							bool shouldChangePlayerFacing = (m_cameraMode != CameraMode_FirstPerson);

							m_movementDirection = normalize(m_movementDirection);
							vec3 amountMoved = m_pPlayer->MoveAbsolute(m_movementDirection, m_movementSpeed * dt, shouldChangePlayerFacing);
							m_pGameCamera->SetFakePosition(m_pGameCamera->GetFakePosition() + amountMoved);
						}
					}
					else
					{
						m_targetCameraXAxisAmount_Target -= (axisX * dt);
						if (m_targetCameraXAxisAmount_Target > 1.0f)
						{
							m_targetCameraXAxisAmount_Target = 1.0f;
						}
						if (m_targetCameraXAxisAmount_Target < -1.0f)
						{
							m_targetCameraXAxisAmount_Target = -1.0f;
						}

						m_pPlayer->Move(-axisY * 10.0f * dt);  // TODO : Is this a good value for gamepad movement speed?
						m_pPlayer->Strafe(-axisX * 10.0f * dt);  // TODO : Is this a good value for gamepad movement speed?
					}
				}
			}
		}
	}
}
