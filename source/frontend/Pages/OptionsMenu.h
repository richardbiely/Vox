// ******************************************************************************
// Filename:    OptionsMenu.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 12/10/14
//
// Copyright (c) 2005-2011, Steven Ball
// ******************************************************************************

#pragma once

#include "../FrontendPage.h"
#include "../../gui/guiwindow.h"
#include "../../gui/optioncontroller.h"
#include "../../gui/slider.h"
#include "../../gui/checkbox.h"


class OptionsMenu : public FrontendPage
{
public:
	/* Public methods */
	OptionsMenu(Renderer* pRenderer, OpenGLGUI* pGUI, FrontendManager* pFrontPageManager, int windowWidth, int windowHeight);
	~OptionsMenu();

	void Reset() override;

	void SetWindowDimensions(int windowWidth, int windowHeight) override;

	// Are we returning to the main menu, or are we in the game?
	void SetReturnToMainMenu(bool mainMenu);

	// Skinning
	void SkinGUI() override;
	void UnSkinGUI() override;

	// Loading and saving options to file
	void LoadOptions();
	void SaveOptions();

	// Load unload
	void Load() override;
	void Unload() override;

	// Disable options
	void DisableShadowOption();

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

	static void _InterfaceTabPressed(void *pData);
	void InterfaceTabPressed();

	static void _ControlsTabPressed(void *pData);
	void ControlsTabPressed();

	static void _ToggleFullScreenPressed(void *pData);
	void ToggleFullScreenPressed();

	static void _CustomCursorsTogglePressed(void *pData);
	void CustomCursorsTogglePressed();

	static void _RenderHUDTogglePressed(void *pData);
	void RenderHUDTogglePressed();

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	bool m_loaded;

	// Are we in the front-end or in the game
	bool m_returnToMainMenu;

	int m_optionsWindowWidth;
	int m_optionsWindowHeight;
	int m_titlebarWidth;
	int m_titlebarHeight;

	bool m_initiallyUsingCustomCursors;

	// Options window
	GUIWindow* m_pOptionsWindow;
	Icon* m_pTitleBarIcon;
	Icon* m_pTitleBarBackgroundIcon;
	Icon* m_pOptionsWindowBackgroundIcon;
	Button* m_pCloseExitButton;

	// Tabs
	OptionController* m_pOptionsModeController;
	OptionBox* m_pGameplayMode;
	OptionBox* m_pGraphicsMode;
	OptionBox* m_pSoundMode;
	OptionBox* m_pControlsMode;
	OptionBox* m_pInterfaceMode;

	// Gameplay
	CheckBox* m_pInvertedMouseMode;
	OptionBox* m_pNormalCamOption;
	OptionBox* m_pFollowCamOption;
	OptionController* m_pCameraSelector;
	Slider* m_pMouseSensativitySlider;
	Slider* m_pGamepadSensativitySlider;
	Icon* m_pSliderBackgroundIcon_MouseSensativity;
	Icon* m_pSliderBackgroundIcon_GamepadSensativity;

	// Graphics
	CheckBox* m_pShadowsCheckBox;
	CheckBox* m_pFogRenderCheckBox;
	CheckBox* m_pWaterRenderCheckBox;
	Button* m_pFullscreenButton;

	// Sound
	CheckBox* m_pSoundEffects;
	Slider* m_pSoundEffectsVolume;
	CheckBox* m_pMusic;
	Slider* m_pMusicVolume;
	Icon* m_pSliderBackgroundIcon_Sound;
	Icon* m_pSliderBackgroundIcon_Music;

	// Interface
	CheckBox* m_pRenderGUI;
	CheckBox* m_pRenderCrosshair;
	CheckBox* m_pHUDAnimations;
	CheckBox* m_pCustomCursors;
	CheckBox* m_pEquipHoverIcon;
	CheckBox* m_pToolTipAppearDelay;
	CheckBox* m_pConfirmDeleteItem;
	Button* m_pResetGUIToDefault;

	// Controls

	// Lists of components for each tab
	ComponentList m_vpGameplayComponents;
	ComponentList m_vpGraphicsComponents;
	ComponentList m_vpSoundComponents;
	ComponentList m_vpControlsComponents;
	ComponentList m_vpInterfaceComponents;
};
