// ******************************************************************************
// Filename:    CreateCharacter.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   A front-end page for creating a new character and customizing how the
//   character looks, by selecting the different sub-sections of a character.
//
// Revision History:
//   Initial Revision - 12/10/14
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "../FrontendPage.h"
#include "../../NPC/NPCManager.h"
#include "../../gui/icon.h"
#include "../../gui/label.h"
#include "../../gui/button.h"
#include "../../gui/formattedlabel.h"
#include "../../gui/slider.h"
#include "../../gui/guiwindow.h"
#include "../../gui/textbox.h"
#include "../../gui/openglgui.h"

class CreateCharacter;

enum PresetSection
{
	PresetSection_None = 0,
	PresetSection_Head,
	PresetSection_Body,
	PresetSection_Legs,
	PresetSection_RightShoulder,
	PresetSection_LeftShoulder,
	PresetSection_RightHand,
	PresetSection_LeftHand,
	PresetSection_RightFoot,
	PresetSection_LeftFoot,
};

class PresetButtonData
{
public:
	CreateCharacter* m_pCreateCharacter;
	PresetSection m_presetSection;
	string m_presetFilename;
	bool m_customPreset;
};

class DeleteButtonData
{
public:
	CreateCharacter* m_pCreateCharacter;
	Button* m_pDeleteButton;
	Button* m_pPresetButtonConnectedTo;
	PresetSection m_presetSection;
	string m_presetFilename;
};

class CreateCharacter : public FrontendPage
{
public:
	/* Public methods */
	CreateCharacter(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontendManager, int windowWidth, int windowHeight);
	~CreateCharacter();

	void Reset();

	void SetWindowDimensions(int windowWidth, int windowHeight);

	// Skinning the GUI
	void SkinGUI();
	void UnSkinGUI();

	// Loading
	void Load();
	void Unload();

	bool LoadDefaultSettings(string defaultFile, PresetSection m_section);

	void LoadSelectionCharacters();

	void SetHoverNPC(NPC* pHoverNPC);
	void SetSelectedNPC(NPC* pSelectedNPC);

	void CreatePresetButtons(PresetSection presetSection, bool showWindows);
	void DeletePresetButtons();

	void ShowTooltip(int x, int y);
	void HideTooltip();

	void SavedNewPresetModel(string modelToLoadToCharacter);
	void UpdateCustomCounter(int incrementValue);
	void ClosedVoxelEditor(bool showDefaultsWindow);

	void UpdatePresetsPulldown();
	void UpdateEyeTexturePulldown();
	void Update(float dt);

	void Render();
	void Render2D();

protected:
	/* Protected methods */
	static void _SelectPressed(void *apData);
	void SelectPressed();

	static void _CreatePressed(void *apData);
	void CreatePressed();

	static void _BackPressed(void *apData);
	void BackPressed();

	static void _EyesTexturesPulldownChanged(void *pData);
	void EyesTexturesPulldownChanged();

	static void _PresetsPulldownChanged(void *pData);
	void PresetsPulldownChanged();
	
	static void _ArrowLeftPressed(void *pData);
	void ArrowLeftPressed();

	static void _ArrowLeftReleased(void *pData);
	void ArrowLeftReleased();

	static void _ArrowRightPressed(void *pData);
	void ArrowRightPressed();

	static void _ArrowRightReleased(void *pData);
	void ArrowRightReleased();

	static void _CreateCustomPressed(void *pData);
	void CreateCustomPressed();

	static void _PresetButtonPressed(void *apData);
	void PresetButtonPressed(PresetButtonData* presetButtonData);

	static void _DeleteButtonPressed(void *apData);
	void DeleteButtonPressed(DeleteButtonData* deleteButtonData);

	static void _SaveDefaultsPressed(void *apData);
	void SaveDefaultsPressed() const;
	
private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	NPCList m_vpCharacterLineUp;

	// Hover and selection NPC pointers
	NPC* m_pHoverCreatioNPC;
	NPC* m_pSelectedNPC;
	NPC* m_pCustomCreationNPC;

	// Text alpha animation
	float m_classTextFadeInAlpha;
	float m_classTextFadeInFadeInTime;
	float m_classTextFadeInFadeInTimer;

	vec3 m_selectionNPCOriginPoint;

	// Tooltip
	bool m_toolTipVisible;
	int m_tooltipWidth;
	int m_tooltipHeight;
	int m_tooltipDescBorder;
	Icon* m_pTooltipBackground;
	Label* m_pTooltipNameLabel;
	FormattedLabel* m_pTooltipDescriptionLabel;

	bool m_creatingCustom;

	PresetSection m_selectedPresetSection;

	bool shouldUpdatePresetWindowAfterDelete;

	Button* m_pNextButton;
	Button* m_pCreateButton;
	Button* m_pBackButton;

	int m_presetsWindowWidth;
	int m_presetsWindowHeight;
	int m_customizeWindowWidth;
	int m_customizeWindowHeight;
	int m_scaleWindowWidth;
	int m_scaleWindowHeight;
	int m_defaultsWindowWidth;
	int m_defaultsWindowHeight;
	int m_titlebarWidth;
	int m_titlebarHeight;
	int m_presetButtonWidth;
	int m_presetButtonHeight;
	int m_presetButtonSpacer;

	// Customize window
	GUIWindow* m_pCustomizeWindow;
	Icon* m_pCustomizeTitleBarIcon;
	Icon* m_pCustomizeTitleBarBackgroundIcon;
	Icon* m_pCustomizeWindowBackgroundIcon;
	TextBox* m_pNameTextBox;
	PulldownMenu* m_pEyesTexturesPulldown;
	Slider* m_pCharacterSizeSlider;
	PulldownMenu* m_pPresetsPulldown;

	// Preset window
	GUIWindow* m_pPresetsWindow;
	Icon* m_pPresetTitleBarIcon;
	Icon* m_pPresetTitleBarBackgroundIcon;
	Icon* m_pPresetWindowBackgroundIcon;
	Button* m_pCreateCustomPresetButton;
	ScrollBar* m_pPresetScrollbar;

	// Scale window
	GUIWindow* m_pScaleWindow;
	Icon* m_pScaleTitleBarIcon;
	Icon* m_pScaleTitleBarBackgroundIcon;
	Icon* m_pScaleWindowBackgroundIcon;
	Slider* m_pPresetScaleSlider;
	Slider* m_pEyesSizeSlider;
	Slider* m_pMouthSizeSlider;

	// Defaults window
	GUIWindow* m_pDefaultsWindow;
	Icon* m_pDefaultsTitleBarIcon;
	Icon* m_pDefaultsTitleBarBackgroundIcon;
	Icon* m_pDefaultsWindowBackgroundIcon;
	Slider* m_pXOffsetSlider;
	Slider* m_pYOffsetSlider;
	Slider* m_pZOffsetSlider;
	Button* m_pSaveDefaultsButton;

	bool m_leftArrowPressed;
	bool m_rightArrowPressed;

	Button* m_pArrowLeftButton;
	Button* m_pArrowRightButton;

	std::vector<Button*> m_vpPresetButtons;
	std::vector<Button*> m_vpDeleteButtons;

	Icon* m_pPresetButtonBorderDefault;
	Icon* m_pPresetButtonBorderHover;
	Icon* m_pPresetButtonBorderPressed;
	Icon* m_pPresetButtonBorderDisabled;
	Icon* m_pCustomButtonBorderDefault;
	Icon* m_pCustomButtonBorderHover;
	Icon* m_pCustomButtonBorderPressed;
	Icon* m_pCustomButtonBorderDisabled;
	Icon* m_pDeleteCustomButtonBorderDefault;
	Icon* m_pDeleteCustomButtonBorderHover;
	Icon* m_pDeleteCustomButtonBorderPressed;
	Icon* m_pDeleteCustomButtonBorderDisabled;
	Icon* m_pAddNewButtonBorderDefault;
	Icon* m_pAddNewButtonBorderHover;
	Icon* m_pAddNewButtonBorderPressed;
	Icon* m_pAddNewButtonBorderDisabled;

	std::vector<PresetButtonData*> m_vpPresetButtonDataItems;
	std::vector<DeleteButtonData*> m_vpDeleteButtonDataItems;

	string m_modelNameForVoxelEditor;
	string m_newModelNameForVoxelEditor;
	bool m_shouldAllowOverwriteButton;

	float m_defaultEyeWidth;
	float m_defaultMouthWidth;
	vec3 m_defaultEyeOffset;
	vec3 m_defaultMouthOffset;
};
