// ******************************************************************************
// Filename:    QuestGUI.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 13/03/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "../blocks/ChunkManager.h"
#include "../Player/Player.h"
#include "../Inventory/InventoryManager.h"
#include "../gui/guiwindow.h"
#include "../gui/formattedlabel.h"
#include "../gui/scrollbar.h"
#include "../gui/openglgui.h"

class GameWindow;
class FrontendManager;
class QuestJournal;
class Quest;
class QuestGUI;

class QuestButton
{
public:
	Button* m_pButton;
	Quest* m_pQuest;
	QuestGUI* m_pQuestGUI;
};


class QuestGUI
{
public:
	/* Public methods */
	QuestGUI(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, ChunkManager* pChunkManager, Player* pPlayer, InventoryManager* pInventoryManager, int windowWidth, int windowHeight);
	~QuestGUI();

	void SetQuestJournal(QuestJournal* pQuestJournal);

	void SetWindowDimensions(int windowWidth, int windowHeight);

	// Skinning the GUI
	void SkinGUI() const;
	void UnSkinGUI();

	// Loading
	void Load(bool loadDelay = false, float loadDelayTime = 0.0f);
	void Unload();
	bool IsLoadDelayed();
	bool IsLoaded();

	void CreateQuestButtons();
	void DeleteQuestButtons();

	void RefreshQuestButtons();

	void SetQuestData(string questTitle, string questText);

	// Updating
	void Update(float dt);
	
	// Rendering
	void Render();

protected:
	/* Protected methods */
	static void _CloseExitPressed(void *pData);
	void CloseExitPressed();

	static void _OpenCloseTabPressed(void *pData);
	void OpenCloseTabPressed();

	static void _QuestButtonPressed(void *pData);
	void QuestButtonPressed(QuestButton* pQuestButtonData);

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	Renderer* m_pRenderer;
	OpenGLGUI* m_pGUI;
	GameWindow* m_pGameWindow;
	FrontendManager* m_pFrontendManager;
	ChunkManager* m_pChunkManager;
	Player* m_pPlayer;
	InventoryManager* m_pInventoryManager;
	QuestJournal* m_pQuestJournal;

	int m_windowWidth;
	int m_windowHeight;

	bool m_loaded;

	// Load delay
	bool m_loadDelay;
	float m_loadDelayTime;

	int m_questWindowWidth;
	int m_questWindowHeight;
	int m_titlebarWidth;
	int m_titlebarHeight;
	int m_questTabOpenButtonWidth;
	int m_questTabOpenButtonHeight;
	int m_questTabWidth;
	int m_questTabHeight;
	bool m_questTabsOpen;
	bool m_questTabLeftSide;
	bool m_addedQuestListScrollbar;

	// GUI components
	GUIWindow* m_pQuestLogWindow;
	Icon* m_pTitleBarIcon;
	Icon* m_pTitleBarBackgroundIcon;
	Icon* m_pQuestLogWindowBackgroundIcon;
	Button* m_pCloseExitButton;
	Label* m_pQuestTitleHeaderLabel;
	FormattedLabel* m_pQuestTextLabel;
	Icon* m_pTabOpenCloseButton_Icon_Right;
	Icon* m_pTabOpenCloseButton_Icon_Right_Hover;
	Icon* m_pTabOpenCloseButton_Icon_Right_Pressed;
	Icon* m_pTabOpenCloseButton_Icon_Left;
	Icon* m_pTabOpenCloseButton_Icon_Left_Hover;
	Icon* m_pTabOpenCloseButton_Icon_Left_Pressed;
	Button* m_pTabOpenCloseButton;
	Icon* m_pQuestsTab_Icon_Right;
	Icon* m_pQuestsTab_Icon_Left;
	ScrollBar* m_pQuestListScrollbar;
	Label* m_pCurrentQuestsHeaderLabel;
	Label* m_pCompletedQuestsHeaderLabel;
	Icon* m_pAlphaButton;
	vector<QuestButton*> m_vpCurrentQuestButtons;
	vector<QuestButton*> m_vpCompletedQuestButtons;
};
