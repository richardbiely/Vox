// ******************************************************************************
// Filename:    MainMenu.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   A front-end page for the main menu of the front-end, this is the main
//   entry point of the menu system used for front-end flow, after the intro.
//
// Revision History:
//   Initial Revision - 12/10/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "../FrontendPage.h"
#include "../../gui/icon.h"
#include "../../gui/label.h"
#include "../../gui/button.h"


class MainMenu : public FrontendPage
{
public:
	/* Public methods */
	MainMenu(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontPageManager, int windowWidth, int windowHeight);
	~MainMenu();

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
	static void _SinglePlayerPressed(void *apData);
	void SinglePlayerPressed();

	static void _MultiPlayerPressed(void *apData);
	void MultiPlayerPressed();

	static void _OptionsPressed(void *apData);
	void OptionsPressed();

	static void _ModsPressed(void *apData);
	void ModsPressed();

	static void _CreditsPressed(void *apData);
	void CreditsPressed();

	static void _QuitPressed(void *apData);
	void QuitPressed();

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	// Vox logo texture
	int m_logoWidth;
	int m_logoHeight;

	unsigned int m_logoTexture;

	float m_blurAmount;

	Label* m_pCreditLabel;

	Button* m_pSinglePlayerButton;
	Button* m_pMultiPlayerButton;
	Button* m_pOptionsButton;
	Button* m_pModsButton;
	Button* m_pCreditsButton;
	Button* m_pQuitButton;

	float m_textFadeInTime;
	float m_textFadeInTimer;
	float m_titleFadeInTime;
	float m_titleFadeInTimer;

	float m_textAlpha;
	float m_titleAlpha;
};
