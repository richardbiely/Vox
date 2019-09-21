// ******************************************************************************
// Filename:    Credits.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 12/10/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "Credits.h"
#include "../FrontendManager.h"
#include "../../VoxGame.h"


Credits::Credits(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, int windowWidth, int windowHeight)
	: FrontendPage(pRenderer, pGUI, pFrontendManager, FrontendScreen_Credits, windowWidth, windowHeight)
{
	SetWindowDimensions(m_windowWidth, m_windowHeight);

	m_pCreditsNPC = nullptr;

	m_clickToNextTimer = 0.0f;
}

Credits::~Credits()
{
	ClearCreditTextLines();
	ClearCreditsNPC();
}

void Credits::Reset()
{
}

void Credits::CreateCreditTextLinesForPage(int pageIndex)
{
	if(pageIndex == 0)
	{
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "Steven Ball";
			pNewCreditsLine->m_screenWidthRatio = 0.75f;
			pNewCreditsLine->m_screenHeightDifference = 315.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_80();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_80_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "@AlwaysGeeky";
			pNewCreditsLine->m_screenWidthRatio = 0.75f;
			pNewCreditsLine->m_screenHeightDifference = 365.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_50();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_50_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "---------------------------";
			pNewCreditsLine->m_screenWidthRatio = 0.75f;
			pNewCreditsLine->m_screenHeightDifference = 420.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_50();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_50_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "Concept and Development";
			pNewCreditsLine->m_screenWidthRatio = 0.75f;
			pNewCreditsLine->m_screenHeightDifference = 475.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_40();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_40_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "Programming, Design, Artwork";
			pNewCreditsLine->m_screenWidthRatio = 0.75f;
			pNewCreditsLine->m_screenHeightDifference = 520.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_40();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_40_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
	}
	else if(pageIndex == 1)
	{
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "Zach Soares";
			pNewCreditsLine->m_screenWidthRatio = 0.25f;
			pNewCreditsLine->m_screenHeightDifference = 315.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_80();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_80_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "@y2bcrazy";
			pNewCreditsLine->m_screenWidthRatio = 0.25f;
			pNewCreditsLine->m_screenHeightDifference = 365.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_50();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_50_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "---------------------------";
			pNewCreditsLine->m_screenWidthRatio = 0.25f;
			pNewCreditsLine->m_screenHeightDifference = 420.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_50();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_50_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "Voxel Artwork and Design";
			pNewCreditsLine->m_screenWidthRatio = 0.25f;
			pNewCreditsLine->m_screenHeightDifference = 475.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_40();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_40_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
	}
	else if(pageIndex == 2)
	{
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "Denver Poteet";
			pNewCreditsLine->m_screenWidthRatio = 0.75f;
			pNewCreditsLine->m_screenHeightDifference = 315.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_80();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_80_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "@DenverSkyline";
			pNewCreditsLine->m_screenWidthRatio = 0.75f;
			pNewCreditsLine->m_screenHeightDifference = 365.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_50();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_50_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "---------------------------";
			pNewCreditsLine->m_screenWidthRatio = 0.75f;
			pNewCreditsLine->m_screenHeightDifference = 420.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_50();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_50_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "2D Artwork";
			pNewCreditsLine->m_screenWidthRatio = 0.75f;
			pNewCreditsLine->m_screenHeightDifference = 475.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_40();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_40_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
	}
	else if(pageIndex == 3)
	{
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "Kevin Balke";
			pNewCreditsLine->m_screenWidthRatio = 0.25f;
			pNewCreditsLine->m_screenHeightDifference = 315.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_80();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_80_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "@V_axys";
			pNewCreditsLine->m_screenWidthRatio = 0.25f;
			pNewCreditsLine->m_screenHeightDifference = 365.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_50();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_50_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "---------------------------";
			pNewCreditsLine->m_screenWidthRatio = 0.25f;
			pNewCreditsLine->m_screenHeightDifference = 420.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_50();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_50_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
		{
			CreditTextLine* pNewCreditsLine = new CreditTextLine();
			pNewCreditsLine->m_text = "Music Composition and Sound Effects";
			pNewCreditsLine->m_screenWidthRatio = 0.25f;
			pNewCreditsLine->m_screenHeightDifference = 475.0f;
			pNewCreditsLine->m_fontId = m_pFrontendManager->GetFrontendFont_40();
			pNewCreditsLine->m_outlineFontId = m_pFrontendManager->GetFrontendFont_40_Outline();

			m_vpCreditLineList.push_back(pNewCreditsLine);
		}
	}
}

void Credits::ClearCreditTextLines()
{
	for(unsigned int i = 0; i < (int)m_vpCreditLineList.size(); i++)
	{
		delete m_vpCreditLineList[i];
		m_vpCreditLineList[i] = 0;
	}
	m_vpCreditLineList.clear();
}

void Credits::CreateCreditsNPCForPage(int pageIndex)
{
	if(pageIndex == 0)
	{
		m_pCreditsNPC = VoxGame::GetInstance()->GetNPCManager()->CreateNPC("CreditsCharacter", "Human", "Steve", vec3(24.0f, 0.51f, 4.5f), 0.08f, false, false);
		m_pCreditsNPC->SetCreditsNPC(true);
		//m_pCreditsNPC->LookAtPoint(vec3(0.0f, 8.0f, 1.0f));
	}
	if(pageIndex == 1)
	{
		m_pCreditsNPC = VoxGame::GetInstance()->GetNPCManager()->CreateNPC("CreditsCharacter", "Human", "Mummy", vec3(26.0f, 0.51f, 4.5f), 0.08f, false, false);
		m_pCreditsNPC->SetCreditsNPC(true);
		//m_pCreditsNPC->LookAtPoint(vec3(0.0f, 8.0f, 1.0f));
		m_pCreditsNPC->SetAnimationSpeed(0.5f, true, AnimationSections_FullBody);
		m_pCreditsNPC->GetVoxelCharacter()->PlayAnimation(AnimationSections_FullBody, false, AnimationSections_FullBody, "MummyWalk");
	}
	if(pageIndex == 2)
	{
		m_pCreditsNPC = VoxGame::GetInstance()->GetNPCManager()->CreateNPC("CreditsCharacter", "Human", "AshElemental", vec3(24.0f, 0.51f, 4.5f), 0.08f, false, false);
		m_pCreditsNPC->SetCreditsNPC(true);
		//m_pCreditsNPC->LookAtPoint(vec3(0.0f, 8.0f, 1.0f));
	}
	if(pageIndex == 3)
	{
		m_pCreditsNPC = VoxGame::GetInstance()->GetNPCManager()->CreateNPC("CreditsCharacter", "Human", "Ranger", vec3(26.0f, 0.51f, 4.5f), 0.08f, false, false);
		m_pCreditsNPC->SetCreditsNPC(true);
		//m_pCreditsNPC->LookAtPoint(vec3(0.0f, 8.0f, 1.0f));
	}
}

void Credits::ClearCreditsNPC()
{
	if(m_pCreditsNPC != nullptr)
	{
		VoxGame::GetInstance()->GetNPCManager()->DeleteNPC(m_pCreditsNPC->GetName());
		m_pCreditsNPC = nullptr;
	}
}

void Credits::SetWindowDimensions(int windowWidth, int windowHeight)
{
	FrontendPage::SetWindowDimensions(windowWidth, windowHeight);
}

// Skinning the GUI
void Credits::SkinGUI()
{
	string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();
}

void Credits::UnSkinGUI()
{
}

// Loading
void Credits::Load()
{
	m_currentCreditPage = 0;
	m_numCreditPages = 4;

	SetTitleAndSubtitle("CREDITS", "See who made this wonderful game.");

	m_pCreditsNPC = nullptr;

	GotoNextCreditPage();
}

void Credits::Unload()
{
	RemoveTitlesAndSubTitle();
	ClearCreditTextLines();
	ClearCreditsNPC();
}

void Credits::GotoNextCreditPage()
{
	if(m_clickToNextTimer > 0.0f)
	{
		// Don't allow clicking through the credits too quickly!
		return;
	}

	if(m_currentCreditPage == m_numCreditPages)
	{
		m_pFrontendManager->SetFrontendScreen(FrontendScreen_MainMenu);
	}
	else
	{
		ClearCreditTextLines();
		CreateCreditTextLinesForPage(m_currentCreditPage);
		ClearCreditsNPC();
		CreateCreditsNPCForPage(m_currentCreditPage);

		m_clickToNextTimer = 1.5f;

		m_currentCreditPage++;
	}
}

void Credits::Update(float dt)
{
	FrontendPage::Update(dt);

	m_cameraPosition = vec3(25.0f, 2.25f, 8.0f);
	m_cameraView = vec3(25.0f, 1.5f, 0.0f);

	if(m_clickToNextTimer > 0.0f)
	{
		m_clickToNextTimer -= dt;
	}
}

void Credits::Render()
{
	FrontendPage::Render();
}

void Credits::Render2D()
{
	FrontendPage::Render2D();

	for(unsigned int i = 0; i < (int)m_vpCreditLineList.size(); i++)
	{
		CreditTextLine* pCreditsLine = m_vpCreditLineList[i];

		float xText = m_windowWidth * pCreditsLine->m_screenWidthRatio;
		float yText = m_windowHeight - pCreditsLine->m_screenHeightDifference;

		char displayText[256];
		strcpy(displayText, pCreditsLine->m_text.c_str());

		unsigned int fontId = pCreditsLine->m_fontId;
		unsigned int outlineFontId = pCreditsLine->m_outlineFontId;

		int lTextWidth = m_pRenderer->GetFreeTypeTextWidth(fontId, "%s", displayText);
		xText -= (int)(lTextWidth*0.5f);

		m_pRenderer->PushMatrix();
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
			m_pRenderer->SetProjectionMode(PM_2D, VoxGame::GetInstance()->GetDefaultViewport());
			m_pRenderer->SetLookAtCamera(vec3(0.0f, 0.0f, 250.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
			m_pRenderer->RenderFreeTypeText(fontId, xText, yText, 1.0f, Colour(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, displayText);
			m_pRenderer->RenderFreeTypeText(outlineFontId, xText, yText, 1.0f, Colour(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, displayText);
			m_pRenderer->DisableTransparency();
		m_pRenderer->PopMatrix();
	}
}