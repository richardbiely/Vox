// ******************************************************************************
// Filename:    ModMenu.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 12/10/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "../FrontendPage.h"
#include "../../Inventory/InventoryManager.h"
#include "../../gui/guiwindow.h"
#include "../../gui/optioncontroller.h"
#include "../../gui/scrollbar.h"
#include "../../gui/draggablerenderrectangle.h"
#include "../../gui/formattedlabel.h"
#include "../../gui/openglgui.h"

class ModMenu;

class ModButtonData
{
public:
	ModMenu* m_pModMenu;
	Button* m_pModButton;
	string m_modName;
	string m_modDescription;
	string m_modAuthor;
	bool m_toggled;
	bool m_allowToggleOff;
	bool m_allowMultipleSelection;
	bool m_gameplayButton;
	bool m_graphicsButton;
	bool m_soundButton;
	bool m_HUDButton;
	bool m_miscButton;
};

class ModMenu : public FrontendPage
{
public:
	/* Public methods */
	ModMenu(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontPageManager, int windowWidth, int windowHeight);
	~ModMenu();

	void Reset() override;

	void ClearModButtonData();

	void SetWindowDimensions(int windowWidth, int windowHeight) override;

	// Pre-tick the already loaded mods and set them as toggled
	void SelectLoadedMods();

	// Are we returning to the main menu, or are we in the game?
	void SetReturnToMainMenu(bool mainMenu);

	// Skinning
	void SkinGUI() override;
	void UnSkinGUI() override;

	// Load unload
	void Load() override;
	void Unload() override;

	void CreateGameplayModButtons();
	void RemoveGameplayModButtons();

	void CreateGraphicsModButtons();
	void RemoveGraphicsModButtons();

	void CreateSoundModButtons();
	void RemoveSoundModButtons();

	void CreateHUDModButtons();
	void RemoveHUDModButtons();

	void CreateMiscModButtons();
	void RemoveMiscModButtons();

	// Tooltips
	void UpdateToolTipAppear(float dt);
	void ShowTooltip(ModButtonData* pModButtonData);
	void HideTooltip();

	// Update
	void Update(float dt) override;

	// Render
	void Render() override;
	void Render2D() override;

protected:
	/* Protected methods */
	static void _CloseExitPressed(void *apData);
	void CloseExitPressed();

	static void _GameplayTabPressed(void *pData);
	void GameplayTabPressed();

	static void _GraphicsTabPressed(void *pData);
	void GraphicsTabPressed();

	static void _SoundTabPressed(void *pData);
	void SoundTabPressed();

	static void _GUITabPressed(void *pData);
	void GUITabPressed();

	static void _MiscTabPressed(void *pData);
	void MiscTabPressed();

	static void _ModButtonPressed(void *apData);
	void ModButtonPressed(ModButtonData* pModButtonData);

	static void _ModButtonEntered(void *apData);
	void ModButtonEntered(ModButtonData* pModButtonData);

	static void _ModButtonExited(void *apData);
	void ModButtonExited(ModButtonData* pModButtonData);

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	bool m_loaded;

	int m_modWindowWidth;
	int m_modWindowHeight;
	int m_titlebarWidth;
	int m_titlebarHeight;

	// Are we in the front-end or in the game
	bool m_returnToMainMenu;

	int m_modButtonWidth;
	int m_modButtonheight;
	int m_modButtonSpace;

	// Mod window
	GUIWindow* m_pModWindow;
	Icon* m_pTitleBarIcon;
	Icon* m_pTitleBarBackgroundIcon;
	Icon* m_pModWindowBackgroundIcon;
	Button* m_pCloseExitButton;

	// Tabs
	OptionController* m_pModsModeController;
	OptionBox* m_pGameplayMode;
	OptionBox* m_pGraphicsMode;
	OptionBox* m_pSoundMode;
	OptionBox* m_pHUDMode;
	OptionBox* m_pMiscMode;

	ScrollBar* m_pModsScrollbar;
	std::vector<Button*> m_vpGameplayModButtons;
	std::vector<Button*> m_vpGraphicsModButtons;
	std::vector<Button*> m_vpSoundModButtons;
	std::vector<Button*> m_vpHUDModButtons;
	std::vector<Button*> m_vpMiscModButtons;

	std::vector<ModButtonData*> m_vpModButtonData;

	// Tooltip
	Icon* m_pTooltipBackground_Common;
	Icon* m_pTooltipBackground_Uncommon;
	Icon* m_pTooltipBackground_Magical;
	Icon* m_pTooltipBackground_Rare;
	Icon* m_pTooltipBackground_Epic;
	Label* m_pTooltipNameLabel;
	FormattedLabel* m_pTooltipDescriptionLabel;
	Label* m_pTooltipSlotLabel;
	Label* m_pTooltipQualityLabel;
	bool m_toolTipVisible;
	int m_tooltipWidth;
	int m_tooltipHeight;
	int m_tooltipDescBorder;
	ItemQuality m_tooltipQuality;
	float m_tooltipAppearDelayTimer;
	bool m_toolTipComponentsAdded;
};
