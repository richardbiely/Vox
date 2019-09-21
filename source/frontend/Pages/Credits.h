// ******************************************************************************
// Filename:    Credits.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   A front-end page that displays the credits for the game.
//
// Revision History:
//   Initial Revision - 12/10/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "../FrontendPage.h"

#include "../../NPC/NPCManager.h"

class CreditTextLine
{
public:
	string m_text;
	float m_screenWidthRatio;
	float m_screenHeightDifference;
	unsigned int m_fontId;
	unsigned int m_outlineFontId;
};

typedef std::vector<CreditTextLine*> CreditTextLineList;

class Credits : public FrontendPage
{
public:
	/* Public methods */
	Credits(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, int windowWidth, int windowHeight);
	~Credits();

	void Reset() override;

	void CreateCreditTextLinesForPage(int pageIndex);
	void ClearCreditTextLines();

	void CreateCreditsNPCForPage(int pageIndex);
	void ClearCreditsNPC();

	void SetWindowDimensions(int windowWidth, int windowHeight) override;

	// Skinning the GUI
	void SkinGUI() override;
	void UnSkinGUI() override;

	// Loading
	void Load() override;
	void Unload() override;

	void GotoNextCreditPage();

	void Update(float dt) override;

	void Render() override;
	void Render2D() override;

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
	int m_numCreditPages;
	int m_currentCreditPage;

	NPC* m_pCreditsNPC;

	float m_clickToNextTimer;

	CreditTextLineList m_vpCreditLineList;
};
