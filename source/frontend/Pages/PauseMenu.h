// ******************************************************************************
// Filename:    PauseMenu.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   A front-end page to show a pause menu from within the game. Allows the
//   player access to the options/mod menu and also functionality to quit back
//   to the front-end (main menu).
//
// Revision History:
//   Initial Revision - 12/10/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "../FrontendPage.h"
#include "../../gui/button.h"

class Icon;

class PauseMenu : public FrontendPage
{
public:
	/* Public methods */
	PauseMenu(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, int windowWidth, int windowHeight);
	~PauseMenu();

	void Reset() override;

	void SetWindowDimensions(int width, int height) override;

	void SkinGUI() override;
	void UnSkinGUI() override;

	void Load() override;
	void Unload() override;

	void Update(float dt) override;

	void Render() override;
	void Render2D() override;

protected:
	/* Protected methods */
	static void _ResumePressed(void *apData);
	void ResumePressed();

	static void _OptionsPressed(void *apData);
	void OptionsPressed();

	static void _ModsPressed(void *apData);
	void ModsPressed();

	static void _QuitPressed(void *apData);
	void QuiPressed();

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	Button* m_pResumeButton;
	Button* m_pOptionsButton;
	Button* m_pModsButton;
	Button* m_pQuitButton;

	Icon* m_pBackgroundIcon;
};
