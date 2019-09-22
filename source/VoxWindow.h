// ******************************************************************************
// Filename:    VoxWindow.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   The Vox window class is an interface and wrapper around the glfw windows
//   library and adds common functionality to bind this to the Vox application.
//   The window class handles most 'window' functionality that isnt directly
//   part of the game, stuff like window resizing, fullscreen, cursors, etc.
// 
// Revision History:
//   Initial Revision - 27/10/15
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;

#include <glfw/glfw3.h>

class VoxGame;
class VoxSettings;


typedef struct Joystick
{
	bool m_present;
	char* m_name;
	float* m_axes;
	unsigned char* m_buttons;
	int m_axisCount;
	int m_buttonCount;
} Joystick;

class VoxWindow
{
public:
	/* Public methods */
	VoxWindow(VoxGame* pVoxGame, VoxSettings* pVoxSettings);
	~VoxWindow();

	void Create();
	void Destroy();
	void Update(float dt);
	void Render();

	void InitializeWindowContext(GLFWwindow* window);

	// Windows dimensions
	int GetWindowWidth();
	int GetWindowHeight();
	void ResizeWindow(int width, int height);

	// Minimized
	bool GetMinimized();

	// Cursor
	int GetCursorX();
	int GetCursorY();
	void SetCursorPosition(int x, int y);
	void TurnCursorOff(bool forceOff);
	void TurnCursorOn(bool resetCursorPosition, bool forceOn);
	bool IsCursorOn();

	// Joysticks
	void UpdateJoySticks();
	bool IsJoyStickConnected(int joyStickNum);
	float GetJoystickAxisValue(int joyStickNum, int axisIndex);
	bool GetJoystickButton(int joyStickNum, int axisIndex);
	float GetJoystickAnalogDeadZone();

	// Fullscreen
	void ToggleFullScreen(bool fullscreen);

	// Events
	void PollEvents();

protected:
	/* Protected methods */

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	VoxGame* m_pVoxGame;
	VoxSettings* m_pVoxSettings;

	// The window object pointer
	GLFWwindow* m_pWindow;

	// Window dimensions
	int m_windowWidth;
	int m_windowHeight;
	int m_oldWindowWidth;
	int m_oldWindowHeight;

	// Minimized flag
	bool m_minimized;

	// Cursor position
	int m_cursorX;
	int m_cursorY;
	int m_cursorOldX;
	int m_cursorOldY;

	// Joysticks
	Joystick m_joysticks[GLFW_JOYSTICK_LAST - GLFW_JOYSTICK_1 + 1];
	int m_joystickCount;
	float m_joystickAnalogDeadZone;
};
