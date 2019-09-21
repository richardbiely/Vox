// ******************************************************************************
// Filename:    SelectCharacter.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   A front-end page to allow the player to select which character they want
//   to play as, this page shows a line up of characters (NPCs) that the player
//   can select from. Also includes chracter customization and items equipped.
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
#include "../../gui/formattedlabel.h"

#include "../../NPC/NPCManager.h"


class SelectCharacter : public FrontendPage
{
public:
	/* Public methods */
	SelectCharacter(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontPageManager, int windowWidth, int windowHeight);
	~SelectCharacter();

	void Reset() override;

	void SetWindowDimensions(int width, int height) override;

	void SkinGUI() override;
	void UnSkinGUI() override;

	void Load() override;
	void Unload() override;

	void LoadSelectionCharacters();

	int GetNumCreatedCharacters();

	void SetHoverNPC(NPC* pHoverNPC);
	void SetSelectedNPC(NPC* pSelectedNPC);

	void ShowTooltip(int x, int y);
	void HideTooltip();

	void OpenPopup(string popupTitle, string popupText);
	void ClosePopup();

	void Update(float dt) override;

	void Render() override;
	void Render2D() override;

protected:
	/* Protected methods */
	static void _SelectCharacterPressed(void *apData);
	void SelectCharacterPressed();

	static void _DeleteCharacterPressed(void *apData);
	void DeleteCharacterPressed();

	static void _CreateCharacterPressed(void *apData);
	void CreateCharacterPressed();

	static void _BackPressed(void *apData);
	void BackPressed();

	static void _PopupConfirmPressed(void *apData);
	void PopupConfirmPressed();

	static void _PopupCancelPressed(void *apData);
	void PopupCancelPressed();

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	NPCList m_vpCharacterLineUp;

	bool m_addedCreateButton;
	bool m_updateCreateButton;

	// Hover and selection NPC pointers
	NPC* m_pHoverSelectionNPC;
	NPC* m_pSelectedNPC;

	// Text alpha animation
	float m_nameTextFadeInAlpha;
	float m_nameTextFadeInFadeInTime;
	float m_nameTextFadeInFadeInTimer;

	// Tooltip
	bool m_toolTipVisible;
	int m_tooltipWidth;
	int m_tooltipHeight;
	int m_tooltipDescBorder;
	Icon* m_pTooltipBackground;
	Label* m_pTooltipNameLabel;
	FormattedLabel* m_pTooltipDescriptionLabel;

	// Popup
	int m_popupWidth;
	int m_popupHeight;
	int m_popupBorderSpacer;
	int m_popupTitleSpacer;
	int m_popupIconSize;
	int m_popupIconSpacer;
	Label* m_popupTitle;
	FormattedLabel* m_popupText;
	Button* m_pPopupConfirmButton;
	Button* m_pPopupCancelButton;
	Icon* m_pPopupBackgroundIcon;

	Button* m_pSelectCharacterButton;
	Button* m_pDeleteCharacterButton;
	Button* m_pCreateCharacterButton;
	Button* m_pBackButton;
};
