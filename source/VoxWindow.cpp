// ******************************************************************************
// Filename:    VoxWindow.cpp
// Project:     Vox
// Author:      Steven Ball
// 
// Revision History:
//   Initial Revision - 27/10/15
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "glew/include/GL/glew.h"

#ifdef _WIN32
#include <windows.h>
#endif //_WIN32
#include <GL/gl.h>
#include <GL/glu.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define strdup(x) _strdup(x)
#endif

#pragma comment (lib, "opengl32")
#pragma comment (lib, "glu32")

#include "VoxWindow.h"
#include "VoxGame.h"
#include "VoxSettings.h"

// Callback functionality
void WindowResizeCallback(GLFWwindow* window, int width, int height);
void WindowCloseCallback(GLFWwindow* window);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void CharacterCallback(GLFWwindow* window, unsigned int keyCode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MouseScrollCallback(GLFWwindow* window, double x, double y);


VoxWindow::VoxWindow(VoxGame* pVoxGame, VoxSettings* pVoxSettings)
{
	m_pVoxGame = pVoxGame;
	m_pVoxSettings = pVoxSettings;

	/* Minimized flag */
	m_minimized = false;

	/* Set default cursor positions */
	m_cursorX = 0;
	m_cursorY = 0;
	m_cursorOldX = 0;
	m_cursorOldY = 0;

	/* Default joystick params */
	m_joystickCount = 0;
	m_joystickAnalogDeadZone = 0.20f;

	/* Default windows dimensions */
	m_windowWidth = m_pVoxSettings->m_windowWidth;
	m_windowHeight = m_pVoxSettings->m_windowHeight;
	m_oldWindowWidth = m_windowWidth;
	m_oldWindowHeight = m_windowHeight;
}

VoxWindow::~VoxWindow()
{
}

void VoxWindow::Create()
{
	/* Initialize the window library */
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	/* Initialize any rendering params */
	int samples = 8;
	glfwWindowHint(GLFW_SAMPLES, samples);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	glGetIntegerv(GL_SAMPLES_ARB, &samples);

	/* Initialize the joysticks object */
	memset(m_joysticks, 0, sizeof(m_joysticks));

	/* Create a windowed mode window and it's OpenGL context */
	m_pWindow = glfwCreateWindow(m_windowWidth, m_windowHeight, "Vox", nullptr, nullptr);
	if (!m_pWindow)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	/* Initialize this window object */
	InitializeWindowContext(m_pWindow);

	if (m_pVoxSettings->m_fullscreen)
	{
		ToggleFullScreen(true);
	}
}

void VoxWindow::Destroy()
{
	glfwTerminate();
}

void VoxWindow::Update(float dt)
{
	// Updae the cursor positions
	double x;
	double y;
	glfwGetCursorPos(m_pWindow, &x, &y);

	m_cursorX = (int)floor(x);
	m_cursorY = (int)floor(y);
}

void VoxWindow::Render()
{
	/* Swap front and back buffers */
	glfwSwapBuffers(m_pWindow);
}

void VoxWindow::InitializeWindowContext(GLFWwindow* window)
{
	/* Window resize callback */
	glfwSetWindowSizeCallback(window, WindowResizeCallback);

	/* Window close message callback */
	glfwSetWindowCloseCallback(window, WindowCloseCallback);

	/* Input callbacks */
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCharCallback(window, CharacterCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetScrollCallback(window, MouseScrollCallback);

	/* Center on screen */
	const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwGetWindowSize(window, &m_windowWidth, &m_windowHeight);
	glfwSetWindowPos(window, (vidmode->width - m_windowWidth) / 2, (vidmode->height - m_windowHeight) / 2);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSwapInterval(m_pVoxSettings->m_vsync);

	/* Force resize */
	WindowResizeCallback(window, m_windowWidth, m_windowHeight);

	/* Show the window */
	glfwShowWindow(window);
}

// Windows dimensions
int VoxWindow::GetWindowWidth()
{
	return m_windowWidth;
}

int VoxWindow::GetWindowHeight()
{
	return m_windowHeight;
}

void VoxWindow::ResizeWindow(int width, int height)
{
	m_minimized = (width == 0 || height == 0);

	m_windowWidth = width;
	m_windowHeight = height;
}

bool VoxWindow::GetMinimized()
{
	return m_minimized;
}

// Cursor position
int VoxWindow::GetCursorX()
{
	return m_cursorX;
}

int VoxWindow::GetCursorY()
{
	return m_cursorY;
}

void VoxWindow::SetCursorPosition(int x, int y)
{
	glfwSetCursorPos(m_pWindow, x, y);
}

void VoxWindow::TurnCursorOff(bool forceOff)
{
	if (IsCursorOn() == true)
	{
		if (m_pVoxSettings->m_customCursors == false || forceOff)
		{
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}

	m_cursorOldX = m_cursorX;
	m_cursorOldY = m_cursorY;

	// Signal to the GUI that we have turned off the cursor, reset buttons states, cursor pos, etc
	m_pVoxGame->GUITurnOffCursor();
}

void VoxWindow::TurnCursorOn(bool resetCursorPosition, bool forceOn)
{
	if (IsCursorOn() == false)
	{
		if (m_pVoxSettings->m_customCursors == false || forceOn)
		{
			glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	if (resetCursorPosition)
	{
		SetCursorPosition(m_cursorOldX, m_cursorOldY);
	}
}

bool VoxWindow::IsCursorOn()
{
	return glfwGetInputMode(m_pWindow, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

// Joysticks
void VoxWindow::UpdateJoySticks()
{
	for (int i = 0; i < sizeof(m_joysticks) / sizeof(Joystick); i++)
	{
		Joystick* j = m_joysticks + i;

		if (glfwJoystickPresent(GLFW_JOYSTICK_1 + i))
		{
			const float* axes;
			const unsigned char* buttons;
			int axis_count, button_count;

			free(j->m_name);
			j->m_name = strdup(glfwGetJoystickName(GLFW_JOYSTICK_1 + i));

			axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1 + i, &axis_count);
			if (axis_count != j->m_axisCount)
			{
				j->m_axisCount = axis_count;
				j->m_axes = (float*)realloc(j->m_axes, j->m_axisCount * sizeof(float));
			}

			memcpy(j->m_axes, axes, axis_count * sizeof(float));

			buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1 + i, &button_count);
			if (button_count != j->m_buttonCount)
			{
				j->m_buttonCount = button_count;
				j->m_buttons = (unsigned char*)realloc(j->m_buttons, j->m_buttonCount);
			}

			memcpy(j->m_buttons, buttons, button_count * sizeof(unsigned char));

			if (!j->m_present)
			{
				printf("\nFound joystick %i named \'%s\' with %i axes, %i buttons\n",
					i + 1, j->m_name, j->m_axisCount, j->m_buttonCount);

				m_joystickCount++;
			}

			j->m_present = GL_TRUE;
		}
		else
		{
			if (j->m_present)
			{
				printf("\nLost joystick %i named \'%s\'\n", i + 1, j->m_name);

				free(j->m_name);
				free(j->m_axes);
				free(j->m_buttons);
				memset(j, 0, sizeof(Joystick));

				m_joystickCount--;
			}
		}
	}
}

bool VoxWindow::IsJoyStickConnected(int joyStickNum)
{
	Joystick* j = m_joysticks + joyStickNum;

	return j->m_present;
}

float VoxWindow::GetJoystickAxisValue(int joyStickNum, int axisIndex)
{
	Joystick* j = m_joysticks + joyStickNum;

	if (j->m_present)
	{
		return j->m_axes[axisIndex];
	}
	else 
	{
		return 0.0f;
	}
}

bool VoxWindow::GetJoystickButton(int joyStickNum, int axisIndex)
{
	Joystick* j = m_joysticks + joyStickNum;

	if(j->m_present)
	{
		return (j->m_buttons[axisIndex] != 0);
	}
	else
	{
		return false;
	}
}

float VoxWindow::GetJoystickAnalogDeadZone()
{
	return m_joystickAnalogDeadZone;
}

// Fullscreen
void VoxWindow::ToggleFullScreen(bool fullscreen)
{
	if (fullscreen)
	{
		const GLFWvidmode* vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

		m_oldWindowWidth = m_windowWidth;
		m_oldWindowHeight = m_windowHeight;

		m_windowWidth = vidMode->width;
		m_windowHeight = vidMode->height;
	}
	else
	{
		m_windowWidth = m_oldWindowWidth;
		m_windowHeight = m_oldWindowHeight;
	}

	// Create new window
	GLFWwindow* newWindow = glfwCreateWindow(m_windowWidth, m_windowHeight, "Vox", fullscreen ? glfwGetPrimaryMonitor() : nullptr, m_pWindow);

	/* Initialize this new window object */
	InitializeWindowContext(newWindow);

	// Destroy the existing window pointer and assign new one, since we are context switching
	glfwDestroyWindow(m_pWindow);
	m_pWindow = newWindow;
}

// Events
void VoxWindow::PollEvents()
{
	/* Poll for and process events */
	glfwPollEvents();
}

// Callbacks
void WindowResizeCallback(GLFWwindow* window, int width, int height)
{
	VoxGame::GetInstance()->ResizeWindow(width, height);
}

void WindowCloseCallback(GLFWwindow* window)
{
	VoxGame::GetInstance()->CloseWindow();
}