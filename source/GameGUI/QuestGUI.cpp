// ******************************************************************************
// Filename:    QuestGUI.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 13/03/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "QuestGUI.h"

#include "../frontend/FrontendManager.h"
#include "../Player/Player.h"
#include "../Quests/Quest.h"
#include "../Quests/QuestJournal.h"
#include "../utils/FileUtils.h"
#include "../utils/Random.h"
#include "../utils/Interpolator.h"
#include "../VoxGame.h"


QuestGUI::QuestGUI(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, ChunkManager* pChunkManager, Player* pPlayer, InventoryManager* pInventoryManager, int windowWidth, int windowHeight)
{
	m_pRenderer = pRenderer;
	m_pGUI = pGUI;
	m_pFrontendManager = pFrontendManager;
	m_pChunkManager = pChunkManager;
	m_pPlayer = pPlayer;
	m_pInventoryManager = pInventoryManager;

	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	// Quest Window
	m_pQuestLogWindow = new GUIWindow(m_pRenderer, m_pFrontendManager->GetFrontendFont_Medium(), "Quest Log");
	m_pQuestLogWindow->AllowMoving(true);
	m_pQuestLogWindow->AllowClosing(false);
	m_pQuestLogWindow->AllowMinimizing(false);
	m_pQuestLogWindow->AllowScrolling(false);
	m_pQuestLogWindow->SetRenderTitleBar(true);
	m_pQuestLogWindow->SetRenderWindowBackground(true);
	m_pQuestLogWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
	m_pQuestLogWindow->Hide();

	m_pTitleBarIcon = new Icon(m_pRenderer, "", 44, 44);
	m_pTitleBarIcon->SetDepth(4.0f);

	m_pQuestLogWindowBackgroundIcon = new Icon(m_pRenderer, "", 292, 364);
	m_pQuestLogWindowBackgroundIcon->SetDepth(1.0f);

	m_pTitleBarBackgroundIcon = new Icon(m_pRenderer, "", 133, 35);
	m_pTitleBarBackgroundIcon->SetDepth(1.0f);

	m_pCloseExitButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pCloseExitButton->SetLabelOffset(0, 5);
	m_pCloseExitButton->SetCallBackFunction(_CloseExitPressed);
	m_pCloseExitButton->SetCallBackData(this);

	char titleText[] = "";
	m_pQuestTitleHeaderLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_40(), titleText, Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pQuestTitleHeaderLabel->SetOutline(true);
	m_pQuestTitleHeaderLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pQuestTitleHeaderLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_40_Outline());
	m_pQuestTitleHeaderLabel->SetDepth(4.0f);

	char questLongText[] = "";
	m_pQuestTextLabel = new FormattedLabel(m_pRenderer, m_pFrontendManager->GetFrontendFont_22(), m_pFrontendManager->GetFrontendFont_22(), m_pFrontendManager->GetFrontendFont_22(), questLongText);
	m_pQuestTextLabel->SetOutline(true);
	m_pQuestTextLabel->SetColour(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	m_pQuestTextLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pQuestTextLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_22_Outline());
	m_pQuestTextLabel->SetDepth(3.0f);
	m_pQuestTextLabel->SetWordWrap(true);

	m_pTabOpenCloseButton_Icon_Right = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Right->SetDepth(1.0f);

	m_pTabOpenCloseButton_Icon_Right_Hover = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Right_Hover->SetDepth(1.0f);

	m_pTabOpenCloseButton_Icon_Right_Pressed = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Right_Pressed->SetDepth(1.0f);

	m_pTabOpenCloseButton_Icon_Left = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Left->SetDepth(1.0f);

	m_pTabOpenCloseButton_Icon_Left_Hover = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Left_Hover->SetDepth(1.0f);

	m_pTabOpenCloseButton_Icon_Left_Pressed = new Icon(m_pRenderer, "", 12, 80);
	m_pTabOpenCloseButton_Icon_Left_Pressed->SetDepth(1.0f);

	m_pQuestsTab_Icon_Right = new Icon(m_pRenderer, "", 164, 276);
	m_pQuestsTab_Icon_Right->SetDepth(0.5f);
	m_pQuestsTab_Icon_Left = new Icon(m_pRenderer, "", 164, 276);
	m_pQuestsTab_Icon_Left->SetDepth(0.5f);

	m_pTabOpenCloseButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_30(), m_pFrontendManager->GetFrontendFont_30_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pTabOpenCloseButton->SetDefaultIcon(m_pTabOpenCloseButton_Icon_Right);
	m_pTabOpenCloseButton->SetHoverIcon(m_pTabOpenCloseButton_Icon_Right_Hover);
	m_pTabOpenCloseButton->SetSelectedIcon(m_pTabOpenCloseButton_Icon_Right_Pressed);
	m_pTabOpenCloseButton->SetDisabledIcon(m_pTabOpenCloseButton_Icon_Right);
	m_pTabOpenCloseButton->SetLabelOffset(0, 5);
	m_pTabOpenCloseButton->SetCallBackFunction(_OpenCloseTabPressed);
	m_pTabOpenCloseButton->SetCallBackData(this);

	m_pQuestListScrollbar = new ScrollBar(m_pRenderer);
	m_pQuestListScrollbar->SetScrollDirection(EScrollBarDirection_Vertical);
	m_pQuestListScrollbar->SetScrollSize(0.25f);
	m_pQuestListScrollbar->SetScrollPosition(1.0f);
	m_pQuestListScrollbar->SetDepth(4.0f);
	m_pQuestListScrollbar->SetScissorEnabled(true);

	char currentQuestsText[] = "Current Quests";
	m_pCurrentQuestsHeaderLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_22(), currentQuestsText, Colour(0.89f, 0.90f, 0.1f, 1.0f));
	m_pCurrentQuestsHeaderLabel->SetOutline(true);
	m_pCurrentQuestsHeaderLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pCurrentQuestsHeaderLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_22_Outline());
	m_pCurrentQuestsHeaderLabel->SetDepth(4.0f);

	char completedQuestsText[] = "Completed Quests";
	m_pCompletedQuestsHeaderLabel = new Label(m_pRenderer, m_pFrontendManager->GetFrontendFont_22(), completedQuestsText, Colour(0.89f, 0.90f, 0.1f, 1.0f));
	m_pCompletedQuestsHeaderLabel->SetOutline(true);
	m_pCompletedQuestsHeaderLabel->SetOutlineColour(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	m_pCompletedQuestsHeaderLabel->SetOutlineFont(m_pFrontendManager->GetFrontendFont_22_Outline());
	m_pCompletedQuestsHeaderLabel->SetDepth(4.0f);

	m_pAlphaButton = new Icon(m_pRenderer, "", 4, 4);
	m_pAlphaButton->SetDepth(0.5f);

	m_pQuestLogWindow->SetBackgroundIcon(m_pQuestLogWindowBackgroundIcon);
	m_pQuestLogWindow->SetTitlebarBackgroundIcon(m_pTitleBarBackgroundIcon);
	m_pQuestLogWindow->AddComponent(m_pCloseExitButton);
	m_pQuestLogWindow->AddComponent(m_pTitleBarIcon);
	m_pQuestLogWindow->AddComponent(m_pQuestTitleHeaderLabel);
	m_pQuestLogWindow->AddComponent(m_pQuestTextLabel);
	m_pQuestLogWindow->AddComponent(m_pQuestsTab_Icon_Right);
	m_pQuestLogWindow->AddComponent(m_pQuestsTab_Icon_Left);
	m_pQuestLogWindow->AddComponent(m_pTabOpenCloseButton);

	SetWindowDimensions(m_windowWidth, m_windowHeight);

	m_questTabsOpen = false;
	m_questTabLeftSide = false;
	m_addedQuestListScrollbar = false;

	// Load delay
	m_loadDelay = false;
	m_loadDelayTime = 0.0f;

	m_loaded = false;
}

QuestGUI::~QuestGUI()
{
	delete m_pQuestLogWindow;
	
	DeleteQuestButtons();

	delete m_pTitleBarIcon;
	delete m_pTitleBarBackgroundIcon;
	delete m_pQuestLogWindowBackgroundIcon;
	delete m_pCloseExitButton;
	delete m_pQuestTitleHeaderLabel;
	delete m_pQuestTextLabel;
	delete m_pTabOpenCloseButton_Icon_Left;
	delete m_pTabOpenCloseButton_Icon_Left_Hover;
	delete m_pTabOpenCloseButton_Icon_Left_Pressed;
	delete m_pTabOpenCloseButton_Icon_Right;
	delete m_pTabOpenCloseButton_Icon_Right_Hover;
	delete m_pTabOpenCloseButton_Icon_Right_Pressed;
	delete m_pTabOpenCloseButton;
	delete m_pQuestsTab_Icon_Right;
	delete m_pQuestsTab_Icon_Left;
	delete m_pCurrentQuestsHeaderLabel;
	delete m_pCompletedQuestsHeaderLabel;
	delete m_pQuestListScrollbar;
	delete m_pAlphaButton;
}

void QuestGUI::SetQuestJournal(QuestJournal* pQuestJournal)
{
	m_pQuestJournal = pQuestJournal;
}

void QuestGUI::SetWindowDimensions(int windowWidth, int windowHeight)
{
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;

	m_questWindowWidth = 292;
	m_questWindowHeight = 364;
	m_titlebarWidth = 153;
	m_titlebarHeight = 35;
	m_questTabOpenButtonWidth = 12;
	m_questTabOpenButtonHeight = 80;
	m_questTabWidth = 164;
	m_questTabHeight = 276;

	m_pQuestLogWindow->SetDimensions(50, 250, m_questWindowWidth, m_questWindowHeight);
	m_pQuestLogWindow->SetTitleBarDimensions(0, 0, m_titlebarWidth, m_titlebarHeight);
	m_pQuestLogWindow->SetTitleOffset(50, 5);
	m_pQuestLogWindow->SetApplicationDimensions(m_windowWidth, m_windowHeight);
	m_pQuestLogWindow->SetApplicationBorder(25, 15, 10, 10);

	m_pTitleBarIcon->SetDimensions(0, m_questWindowHeight, 44, 44);

	m_pCloseExitButton->SetDimensions(m_questWindowWidth-32, m_questWindowHeight, 32, 32);

	m_pQuestTitleHeaderLabel->SetLocation(20, 315);
	m_pQuestTextLabel->SetDimensions(20, 150, 250, 160);

	m_pTabOpenCloseButton->SetDimensions(-m_questTabOpenButtonWidth, (m_questWindowHeight/2)-(m_questTabOpenButtonHeight/2), m_questTabOpenButtonWidth, m_questTabOpenButtonHeight);
	m_pQuestsTab_Icon_Left->SetDimensions(-m_questTabWidth, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);
	m_pQuestsTab_Icon_Right->SetDimensions(0, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);

	// Scrollbar
	int scrollbarWidth = 16;
	int scrollbarHeight = m_questTabHeight-12;
	m_pQuestListScrollbar->SetDimensions(m_questWindowWidth+m_questTabWidth-scrollbarWidth-4, (m_questWindowHeight/2)-(m_questTabHeight/2)+8, scrollbarWidth, scrollbarHeight);

	// Quest headers
	int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCurrentQuestsHeaderLabel->GetText().c_str());
	int textHeight = m_pRenderer->GetFreeTypeTextHeight(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCurrentQuestsHeaderLabel->GetText().c_str());
	m_pCurrentQuestsHeaderLabel->SetLocation(m_questWindowWidth+8, (m_questWindowHeight/2)+(m_questTabHeight/2)-textHeight-4);

	textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCompletedQuestsHeaderLabel->GetText().c_str());
	int numActiveQuests = (int)m_vpCurrentQuestButtons.size() + 1;
	m_pCompletedQuestsHeaderLabel->SetLocation(m_questWindowWidth+8, (m_questWindowHeight/2)+(m_questTabHeight/2)-textHeight-(numActiveQuests*textHeight)-4);
}

// Skinning the GUI
void QuestGUI::SkinGUI() const
{
	string themeName = VoxGame::GetInstance()->GetModsManager()->GetHUDTextureTheme();

	string iconName = "media/textures/gui/" + themeName + "/QuestGUI/quest_title_icon.tga";
	m_pTitleBarIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/QuestGUI/quest_window_background.tga";
	m_pQuestLogWindowBackgroundIcon->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/QuestGUI/titlebar_background.tga";
	m_pTitleBarBackgroundIcon->SetIcon(iconName);

	m_pQuestLogWindow->SetBackgroundIcon(m_pQuestLogWindowBackgroundIcon);
	m_pQuestLogWindow->SetTitlebarBackgroundIcon(m_pTitleBarBackgroundIcon);

	Point location = m_pQuestLogWindow->GetLocation();
	m_pQuestLogWindow->SetDimensions(location.m_x, location.m_y, m_questWindowWidth, m_questWindowHeight);
	m_pQuestLogWindow->SetTitleBarDimensions(0, 0, m_titlebarWidth, m_titlebarHeight);


	iconName = "media/textures/gui/" + themeName + "/QuestGUI/tab_openclose_right.tga";
	m_pTabOpenCloseButton_Icon_Right->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/QuestGUI/tab_openclose_right_hover.tga";
	m_pTabOpenCloseButton_Icon_Right_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/QuestGUI/tab_openclose_right_pressed.tga";
	m_pTabOpenCloseButton_Icon_Right_Pressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/QuestGUI/tab_openclose_left.tga";
	m_pTabOpenCloseButton_Icon_Left->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/QuestGUI/tab_openclose_left_hover.tga";
	m_pTabOpenCloseButton_Icon_Left_Hover->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/QuestGUI/tab_openclose_left_pressed.tga";
	m_pTabOpenCloseButton_Icon_Left_Pressed->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/QuestGUI/quests_tab_right.tga";
	m_pQuestsTab_Icon_Right->SetIcon(iconName);
	iconName = "media/textures/gui/" + themeName + "/QuestGUI/quests_tab_left.tga";
	m_pQuestsTab_Icon_Left->SetIcon(iconName);

	iconName = "media/textures/gui/" + themeName + "/Common/alpha_button.tga";
	m_pAlphaButton->SetIcon(iconName);

	m_pFrontendManager->SetScrollbarIcons(m_pQuestListScrollbar);

	m_pCloseExitButton->SetDefaultIcon(m_pFrontendManager->GetCloseExitButtonIcon());
	m_pCloseExitButton->SetHoverIcon(m_pFrontendManager->GetCloseExitButtonIcon_Hover());
	m_pCloseExitButton->SetSelectedIcon(m_pFrontendManager->GetCloseExitButtonIcon_Pressed());
	m_pCloseExitButton->SetDisabledIcon(m_pFrontendManager->GetCloseExitButtonIcon());
}

void QuestGUI::UnSkinGUI()
{

}

// Loading
void QuestGUI::Load(bool loadDelay, float loadDelayTime)
{
	m_loadDelay = loadDelay;
	m_loadDelayTime = loadDelayTime;

	m_pQuestLogWindow->DepthSortComponentChildren();
	if (m_loadDelay == false)
	{
		m_pGUI->AddWindow(m_pQuestLogWindow);
		m_pQuestLogWindow->Show();
	}

	CreateQuestButtons();

	m_questTabsOpen = false;
	//m_addedQuestListScrollbar = false;
	Update(0.01f);

	m_loaded = true;
}

void QuestGUI::Unload()
{
	m_loadDelay = false;
	m_loadDelayTime = 0.0f;

	m_pGUI->RemoveWindow(m_pQuestLogWindow);

	DeleteQuestButtons();

	m_loaded = false;

	if (VoxGame::GetInstance()->IsGUIWindowStillDisplayed() == false)
	{
		VoxGame::GetInstance()->TurnCursorOff(false);
		if (VoxGame::GetInstance()->ShouldRestorePreviousCameraMode())
		{
			VoxGame::GetInstance()->RestorePreviousCameraMode();
			VoxGame::GetInstance()->InitializeCameraRotation();
		}
	}
}

bool QuestGUI::IsLoadDelayed()
{
	return (m_loadDelay == true && m_loadDelayTime > 0.0f);
}

bool QuestGUI::IsLoaded()
{
	return m_loaded;
}

void QuestGUI::CreateQuestButtons()
{
	int numCurrentQuests = m_pQuestJournal->GetNumCurrentQuests();
	for(int i = 0; i < numCurrentQuests; i++)
	{
		Quest* pQuest = m_pQuestJournal->GetCurrentQuest(i);

		Button* pNewButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), m_pFrontendManager->GetFrontendFont_20_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
		pNewButton->AddText(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), m_pFrontendManager->GetFrontendFont_20_Outline(), pQuest->GetName().c_str(), Colour(1.0f, 1.0f, 1.0f, 1.0f), 2, 2, true, Colour(0.0f, 0.0f, 0.0f, 1.0f));
	
		pNewButton->SetDefaultIcon(m_pAlphaButton);
		pNewButton->SetHoverIcon(m_pAlphaButton);
		pNewButton->SetSelectedIcon(m_pAlphaButton);
		pNewButton->SetDisabledIcon(m_pAlphaButton);
		pNewButton->SetPressedOffset(0, -4);
		pNewButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
		pNewButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
		pNewButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
		pNewButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());

		int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_20(), "%s", pQuest->GetName().c_str());
		int textHeight = m_pRenderer->GetFreeTypeTextHeight(m_pFrontendManager->GetFrontendFont_20(), "%s", pQuest->GetName().c_str());
		pNewButton->SetDimensions(0, 0, m_questTabWidth-32, textHeight);

		QuestButton* pQuestButton = new QuestButton();
		pQuestButton->m_pButton = pNewButton;
		pQuestButton->m_pQuest = pQuest;
		pQuestButton->m_pQuestGUI = this;

		pNewButton->SetCallBackFunction(_QuestButtonPressed);
		pNewButton->SetCallBackData(pQuestButton);

		m_vpCurrentQuestButtons.push_back(pQuestButton);
	}

	int numCompletedQuests = m_pQuestJournal->GetNumCompletedQuests();
	for(int i = 0; i < numCompletedQuests; i++)
	{
		Quest* pQuest = m_pQuestJournal->GetCompletedQuest(i);

		Button* pNewButton = new Button(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), m_pFrontendManager->GetFrontendFont_20_Outline(), "", Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f));
		pNewButton->AddText(m_pRenderer, m_pFrontendManager->GetFrontendFont_20(), m_pFrontendManager->GetFrontendFont_20_Outline(), pQuest->GetName().c_str(), Colour(0.5f, 0.5f, 0.5f, 1.0f), 2, 2, true, Colour(0.0f, 0.0f, 0.0f, 1.0f));
		
		pNewButton->SetDefaultIcon(m_pAlphaButton);
		pNewButton->SetHoverIcon(m_pAlphaButton);
		pNewButton->SetSelectedIcon(m_pAlphaButton);
		pNewButton->SetDisabledIcon(m_pAlphaButton);
		pNewButton->SetPressedOffset(0, -4);
		pNewButton->SetLabelColour(m_pFrontendManager->GetNormalFontColour());
		pNewButton->SetNormalLabelColour(m_pFrontendManager->GetNormalFontColour());
		pNewButton->SetHoverLabelColour(m_pFrontendManager->GetHoverFontColour());
		pNewButton->SetPressedLabelColour(m_pFrontendManager->GetPressedFontColour());

		int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_20(), "%s", pQuest->GetName().c_str());
		int textHeight = m_pRenderer->GetFreeTypeTextHeight(m_pFrontendManager->GetFrontendFont_20(), "%s", pQuest->GetName().c_str());
		pNewButton->SetDimensions(0, 0, m_questTabWidth-32, textHeight);

		QuestButton* pQuestButton = new QuestButton();
		pQuestButton->m_pButton = pNewButton;
		pQuestButton->m_pQuest = pQuest;
		pQuestButton->m_pQuestGUI = this;

		pNewButton->SetCallBackFunction(_QuestButtonPressed);
		pNewButton->SetCallBackData(pQuestButton);

		m_vpCompletedQuestButtons.push_back(pQuestButton);
	}
}

void QuestGUI::DeleteQuestButtons()
{
	m_pQuestListScrollbar->ClearScrollAreaItems();

	for(unsigned int i = 0; i < m_vpCurrentQuestButtons.size(); i++)
	{
		delete m_vpCurrentQuestButtons[i]->m_pButton;

		delete m_vpCurrentQuestButtons[i];
		m_vpCurrentQuestButtons[i] = 0;
	}
	m_vpCurrentQuestButtons.clear();

	for(unsigned int i = 0; i < m_vpCompletedQuestButtons.size(); i++)
	{
		delete m_vpCompletedQuestButtons[i]->m_pButton;

		delete m_vpCompletedQuestButtons[i];
		m_vpCompletedQuestButtons[i] = 0;
	}
	m_vpCompletedQuestButtons.clear();
}

void QuestGUI::RefreshQuestButtons()
{
	DeleteQuestButtons();
	CreateQuestButtons();
	m_addedQuestListScrollbar = false;
}

void QuestGUI::SetQuestData(string questTitle, string questText)
{
	m_pQuestTitleHeaderLabel->SetText(questTitle.c_str());
	m_pQuestTextLabel->SetText(questText.c_str());
}

// Updating
void QuestGUI::Update(float dt)
{
	if (m_loadDelay == true)
	{
		if (m_loadDelayTime <= 0.0f)
		{
			m_loadDelay = false;
			m_pGUI->AddWindow(m_pQuestLogWindow);
			m_pQuestLogWindow->Show();
		}
		else
		{
			m_loadDelayTime -= dt;
		}
	}

	// Update the quest tab left and right sides
	if(m_questTabLeftSide)
	{
		m_pTabOpenCloseButton->SetDefaultIcon(m_pTabOpenCloseButton_Icon_Left);
		m_pTabOpenCloseButton->SetHoverIcon(m_pTabOpenCloseButton_Icon_Left_Hover);
		m_pTabOpenCloseButton->SetSelectedIcon(m_pTabOpenCloseButton_Icon_Left_Pressed);
		m_pTabOpenCloseButton->SetDisabledIcon(m_pTabOpenCloseButton_Icon_Left);

		if(m_questTabsOpen)
		{
			if(m_addedQuestListScrollbar == false)
			{
				m_pGUI->RemoveWindow(m_pQuestLogWindow);

				m_pQuestLogWindow->AddComponent(m_pQuestListScrollbar);
				m_pQuestListScrollbar->AddScrollAreaItem(m_pCurrentQuestsHeaderLabel);
				m_pQuestListScrollbar->AddScrollAreaItem(m_pCompletedQuestsHeaderLabel);

				for(int i = 0; i < m_vpCurrentQuestButtons.size(); i++)
				{
					m_pQuestListScrollbar->AddScrollAreaItem(m_vpCurrentQuestButtons[i]->m_pButton);
				}
				for(int i = 0; i < m_vpCompletedQuestButtons.size(); i++)
				{
					m_pQuestListScrollbar->AddScrollAreaItem(m_vpCompletedQuestButtons[i]->m_pButton);
				}

				m_addedQuestListScrollbar = true;

				m_pGUI->AddWindow(m_pQuestLogWindow);
				m_pQuestLogWindow->Show();
			}

			m_pTabOpenCloseButton->SetDimensions(-m_questTabWidth-m_questTabOpenButtonWidth, (m_questWindowHeight/2)-(m_questTabOpenButtonHeight/2), m_questTabOpenButtonWidth, m_questTabOpenButtonHeight);
			m_pQuestsTab_Icon_Left->SetDimensions(-m_questTabWidth, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);
			m_pQuestsTab_Icon_Right->SetDimensions(0, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);
		}
		else
		{
			if(m_addedQuestListScrollbar == true)
			{
				m_pQuestLogWindow->RemoveComponent(m_pQuestListScrollbar);
				m_pQuestLogWindow->RemoveComponent(m_pCurrentQuestsHeaderLabel);
				m_pQuestLogWindow->RemoveComponent(m_pCompletedQuestsHeaderLabel);

				for(int i = 0; i < m_vpCurrentQuestButtons.size(); i++)
				{
					m_pQuestLogWindow->RemoveComponent(m_vpCurrentQuestButtons[i]->m_pButton);
				}
				for(int i = 0; i < m_vpCompletedQuestButtons.size(); i++)
				{
					m_pQuestLogWindow->RemoveComponent(m_vpCompletedQuestButtons[i]->m_pButton);
				}

				m_addedQuestListScrollbar = false;
			}

			m_pTabOpenCloseButton->SetDimensions(-m_questTabOpenButtonWidth, (m_questWindowHeight/2)-(m_questTabOpenButtonHeight/2), m_questTabOpenButtonWidth, m_questTabOpenButtonHeight);
			m_pQuestsTab_Icon_Left->SetDimensions(0, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);
			m_pQuestsTab_Icon_Right->SetDimensions(0, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);
		}

		// Scrollbar
		int scrollbarWidth = 16;
		int scrollbarHeight = m_questTabHeight-12;
		m_pQuestListScrollbar->SetDimensions(-scrollbarWidth, (m_questWindowHeight/2)-(m_questTabHeight/2)+8, scrollbarWidth, scrollbarHeight);
		m_pQuestListScrollbar->SetScrollArea(-m_questTabWidth+20, 0, m_questTabWidth-20, scrollbarHeight);

		// Quest headers
		int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCurrentQuestsHeaderLabel->GetText().c_str());
		int textHeight = m_pRenderer->GetFreeTypeTextHeight(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCurrentQuestsHeaderLabel->GetText().c_str());
		m_pCurrentQuestsHeaderLabel->SetLocation(-(m_questTabWidth)+24, m_questTabHeight-textHeight-12);

		textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCompletedQuestsHeaderLabel->GetText().c_str());
		int numActiveQuests = (int)m_vpCurrentQuestButtons.size() + 1;
		m_pCompletedQuestsHeaderLabel->SetLocation(-(m_questTabWidth)+24, m_questTabHeight-textHeight-(numActiveQuests*textHeight)-12);

		// Quest buttons
		for(int i = 0; i < m_vpCurrentQuestButtons.size(); i++)
		{
			m_vpCurrentQuestButtons[i]->m_pButton->SetLocation(-(m_questTabWidth)+28, m_questTabHeight-textHeight-((i+1)*textHeight)-12);
		}
		for(int i = 0; i < m_vpCompletedQuestButtons.size(); i++)
		{
			m_vpCompletedQuestButtons[i]->m_pButton->SetLocation(-(m_questTabWidth)+28, m_questTabHeight-textHeight-((numActiveQuests+(i+1))*textHeight)-12);
		}
	}
	else
	{
		m_pTabOpenCloseButton->SetDefaultIcon(m_pTabOpenCloseButton_Icon_Right);
		m_pTabOpenCloseButton->SetHoverIcon(m_pTabOpenCloseButton_Icon_Right_Hover);
		m_pTabOpenCloseButton->SetSelectedIcon(m_pTabOpenCloseButton_Icon_Right_Pressed);
		m_pTabOpenCloseButton->SetDisabledIcon(m_pTabOpenCloseButton_Icon_Right);

		if(m_questTabsOpen)
		{
			if(m_addedQuestListScrollbar == false)
			{
				m_pGUI->RemoveWindow(m_pQuestLogWindow);

				m_pQuestLogWindow->AddComponent(m_pQuestListScrollbar);
				m_pQuestListScrollbar->AddScrollAreaItem(m_pCurrentQuestsHeaderLabel);
				m_pQuestListScrollbar->AddScrollAreaItem(m_pCompletedQuestsHeaderLabel);

				for(int i = 0; i < m_vpCurrentQuestButtons.size(); i++)
				{
					m_pQuestListScrollbar->AddScrollAreaItem(m_vpCurrentQuestButtons[i]->m_pButton);
				}
				for(int i = 0; i < m_vpCompletedQuestButtons.size(); i++)
				{
					m_pQuestListScrollbar->AddScrollAreaItem(m_vpCompletedQuestButtons[i]->m_pButton);
				}

				m_addedQuestListScrollbar = true;

				m_pGUI->AddWindow(m_pQuestLogWindow);
				m_pQuestLogWindow->Show();
			}

			m_pTabOpenCloseButton->SetDimensions(m_questWindowWidth+m_questTabWidth, (m_questWindowHeight/2)-(m_questTabOpenButtonHeight/2), m_questTabOpenButtonWidth, m_questTabOpenButtonHeight);
			m_pQuestsTab_Icon_Left->SetDimensions(0, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);
			m_pQuestsTab_Icon_Right->SetDimensions(m_questWindowWidth, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);
		}
		else
		{
			if(m_addedQuestListScrollbar == true)
			{
				m_pQuestLogWindow->RemoveComponent(m_pQuestListScrollbar);
				m_pQuestLogWindow->RemoveComponent(m_pCurrentQuestsHeaderLabel);
				m_pQuestLogWindow->RemoveComponent(m_pCompletedQuestsHeaderLabel);

				for(int i = 0; i < m_vpCurrentQuestButtons.size(); i++)
				{
					m_pQuestLogWindow->RemoveComponent(m_vpCurrentQuestButtons[i]->m_pButton);
				}
				for(int i = 0; i < m_vpCompletedQuestButtons.size(); i++)
				{
					m_pQuestLogWindow->RemoveComponent(m_vpCompletedQuestButtons[i]->m_pButton);
				}

				m_addedQuestListScrollbar = false;
			}

			m_pTabOpenCloseButton->SetDimensions(m_questWindowWidth, (m_questWindowHeight/2)-(m_questTabOpenButtonHeight/2), m_questTabOpenButtonWidth, m_questTabOpenButtonHeight);
			m_pQuestsTab_Icon_Left->SetDimensions(0, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);
			m_pQuestsTab_Icon_Right->SetDimensions(0, (m_questWindowHeight/2)-(m_questTabHeight/2), m_questTabWidth, m_questTabHeight);
		}

		// Scrollbar
		int scrollbarWidth = 16;
		int scrollbarHeight = m_questTabHeight-12;
		m_pQuestListScrollbar->SetDimensions(m_questWindowWidth+m_questTabWidth-scrollbarWidth-4, (m_questWindowHeight/2)-(m_questTabHeight/2)+8, scrollbarWidth, scrollbarHeight);
		m_pQuestListScrollbar->SetScrollArea(-m_questTabWidth+20, 0, m_questTabWidth-20, scrollbarHeight);

		// Quest headers
		int textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCurrentQuestsHeaderLabel->GetText().c_str());
		int textHeight = m_pRenderer->GetFreeTypeTextHeight(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCurrentQuestsHeaderLabel->GetText().c_str());
		m_pCurrentQuestsHeaderLabel->SetLocation(-(m_questTabWidth)+24, m_questTabHeight-textHeight-12);

		textWidth = m_pRenderer->GetFreeTypeTextWidth(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCompletedQuestsHeaderLabel->GetText().c_str());
		int numActiveQuests = (int)m_vpCurrentQuestButtons.size() + 1;
		m_pCompletedQuestsHeaderLabel->SetLocation(-(m_questTabWidth)+24, m_questTabHeight-textHeight-(numActiveQuests*textHeight)-12);

		// Quest buttons
		for(int i = 0; i < m_vpCurrentQuestButtons.size(); i++)
		{
			m_vpCurrentQuestButtons[i]->m_pButton->SetLocation(-(m_questTabWidth)+28, m_questTabHeight-textHeight-((i+1)*textHeight)-12);
		}
		for(int i = 0; i < m_vpCompletedQuestButtons.size(); i++)
		{
			m_vpCompletedQuestButtons[i]->m_pButton->SetLocation(-(m_questTabWidth)+28, m_questTabHeight-textHeight-((numActiveQuests+(i+1))*textHeight)-12);
		}
	}

	if(m_pQuestLogWindow->GetDimensions().m_x < 200)
	{
		m_questTabLeftSide = false;
	}
	else if(m_pQuestLogWindow->GetDimensions().m_x+m_pQuestLogWindow->GetDimensions().m_width > m_windowWidth - 200)
	{
		m_questTabLeftSide = true;
	}

	// Scrollbar
	int visibleSize = m_pQuestListScrollbar->GetScrollArea().m_height;
	int textHeight = m_pRenderer->GetFreeTypeTextHeight(m_pFrontendManager->GetFrontendFont_22(), "%s", m_pCurrentQuestsHeaderLabel->GetText().c_str());
	int neededHeight = (((int)m_vpCurrentQuestButtons.size()+(int)m_vpCompletedQuestButtons.size()+2) * textHeight);
	int heightDiff = neededHeight - visibleSize;

	if(heightDiff > 0)
	{
		m_pQuestListScrollbar->SetScrollSize((float)visibleSize / (float)neededHeight);
		m_pQuestListScrollbar->SetDisabled(false);

		float scrollRatio = m_pQuestListScrollbar->GetScrollRatio();
		int offsetButtonY = (int)(heightDiff * scrollRatio);

		// Quest headers
		int yPos = 4 + m_pQuestListScrollbar->GetScrollArea().m_y+m_pQuestListScrollbar->GetScrollArea().m_height - 2 - (textHeight);
		m_pCurrentQuestsHeaderLabel->SetLocation(-(m_questTabWidth)+24, yPos + offsetButtonY);
		int numActiveQuests = (int)m_vpCurrentQuestButtons.size() + 1;
		yPos = 4 + m_pQuestListScrollbar->GetScrollArea().m_y+m_pQuestListScrollbar->GetScrollArea().m_height - 2 - ((numActiveQuests+1)*textHeight);
		m_pCompletedQuestsHeaderLabel->SetLocation(-(m_questTabWidth)+24, yPos + offsetButtonY);

		// Quest buttons
		for(int i = 0; i < m_vpCurrentQuestButtons.size(); i++)
		{
			int yPos = 4 + m_pQuestListScrollbar->GetScrollArea().m_y+m_pQuestListScrollbar->GetScrollArea().m_height - 2 - ((i+2)*textHeight);
			m_vpCurrentQuestButtons[i]->m_pButton->SetLocation(-(m_questTabWidth)+28, yPos + offsetButtonY);
		}
		for(int i = 0; i < m_vpCompletedQuestButtons.size(); i++)
		{
			int yPos = 4 + m_pQuestListScrollbar->GetScrollArea().m_y+m_pQuestListScrollbar->GetScrollArea().m_height - 2 - ((numActiveQuests+(i+2))*textHeight);
			m_vpCompletedQuestButtons[i]->m_pButton->SetLocation(-(m_questTabWidth)+28, yPos + offsetButtonY);
		}

		//for(unsigned int i = 0; i < m_vpRecipeSlotItem_Filtered.size(); i++)
		//{
		//	int xPos = -m_craftingResultsScrollAreaWidth;
		//	int yPos = 4 + m_pQuestListScrollbar->GetScrollArea().m_y+m_pQuestListScrollbar->GetScrollArea().m_height - 4 - ((i+1)*36);
		//
		//	m_vpRecipeSlotItem_Filtered[i]->m_pResultsIcon->SetLocation(xPos, yPos + offsetButtonY);
		//}
	}
	else
	{
		m_pQuestListScrollbar->SetScrollSize(0.0f);
		m_pQuestListScrollbar->SetDisabled(true);
	}
}

// Rendering
void QuestGUI::Render()
{
	if(m_loaded == false)
	{
		return;
	}
}

void QuestGUI::_CloseExitPressed(void *pData)
{
	QuestGUI* lpQuestGUI = (QuestGUI*)pData;
	lpQuestGUI->CloseExitPressed();
}

void QuestGUI::CloseExitPressed()
{
	Unload();
}

void QuestGUI::_OpenCloseTabPressed(void *pData)
{
	QuestGUI* lpQuestGUI = (QuestGUI*)pData;
	lpQuestGUI->OpenCloseTabPressed();
}

void QuestGUI::OpenCloseTabPressed()
{
	m_questTabsOpen = !m_questTabsOpen;
}

void QuestGUI::_QuestButtonPressed(void *pData)
{
	QuestButton* lpQuestButton = (QuestButton*)pData;
	lpQuestButton->m_pQuestGUI->QuestButtonPressed(lpQuestButton);
}

void QuestGUI::QuestButtonPressed(QuestButton* pQuestButton)
{
	SetQuestData(pQuestButton->m_pQuest->GetName(), pQuestButton->m_pQuest->GetStartText());
}