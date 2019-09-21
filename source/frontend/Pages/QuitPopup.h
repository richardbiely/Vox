// ******************************************************************************
// Filename:    QuitPopup.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   A front-end page to show a confirm/cancel popup when the player goes to
//   close the Vox application while in game. This happens for all types of
//   close requests that the application receives.
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


class QuitPopup : public FrontendPage
{
public:
	/* Public methods */
	QuitPopup(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, int windowWidth, int windowHeight);
	~QuitPopup();

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
	static void _ConfirmPressed(void *apData);
	void ConfirmPressed();

	static void _CancelPressed(void *apData);
	void CancelPressed();

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	int m_backgroundWidth;
	int m_backgroundHeight;

	Label* m_pQuitText;

	Icon* m_pBackgroundIcon;

	Button* m_pCancelButton;
	Button* m_pConfirmButton;
};
