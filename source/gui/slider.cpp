// ******************************************************************************
//
// Filename:	slider.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A slider GUI component to allow quick selection of range values.
//
// Revision History:
//   Initial Revision - 21/03/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "slider.h"
#include "directdrawrectangle.h"
#include "multitextureicon.h"
#include "guiwindow.h"


Slider::Slider(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, float min, float max, float defaultValue, bool addIncrementButtons, float incrementAmount)
  : Container(pRenderer),
    m_value(pRenderer, GUIFont, "", Colour(0.0f, 0.0f, 0.0f)),
	m_name(pRenderer, GUIFont, label, Colour(0.0f, 0.0f, 0.0f)),
    m_fontID(GUIFont),
	m_minValue(min),
	m_maxValue(max),
	m_defaultValue(defaultValue),
	m_eSliderDirection(ESliderDirection_Horizontal)
{
	m_pScrollerDefault = new DirectDrawRectangle(pRenderer);
	m_pScrollerHover = new DirectDrawRectangle(pRenderer);
	m_pScrollerDragging = new DirectDrawRectangle(pRenderer);
	m_pScrollerDisabled = new DirectDrawRectangle(pRenderer);
	m_pScrollBackbarDefault = new DirectDrawRectangle(pRenderer);
	m_pScrollBackbarPressed = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollerDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollerHover;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollerDragging;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollerDisabled;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollBackbarDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollBackbarPressed;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.5f, 0.5f, 0.5f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.5f, 0.5f, 0.5f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.5f, 0.5f, 0.5f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.5f, 0.5f, 0.5f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.05f, 0.05f, 0.05f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.05f, 0.05f, 0.05f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.05f, 0.05f, 0.05f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.05f, 0.05f, 0.05f, 1.0f));

	m_pPlusButton = new Button(pRenderer, GUIFont, "+");
	m_pPlusButton->SetCallBackFunction(_PlusPressed);
	m_pPlusButton->SetCallBackData(this);
	m_pPlusButton->SetLabelOffset(0, 0);
	m_pMinusButton = new Button(pRenderer, GUIFont, "-");
	m_pMinusButton->SetCallBackFunction(_MinusPressed);
	m_pMinusButton->SetCallBackData(this);
	m_pMinusButton->SetLabelOffset(1, 2);

	m_pPlusButton->SetDepth(2.0f);
	m_pMinusButton->SetDepth(2.0f);

	m_currentValue = defaultValue;

	m_bShowValue = true;
	m_ValuePrecision = 2;

	m_bDragging = false;
	m_bAllowDragging = true;
	m_bHover = false;
	m_bDragginOutside = false;
	m_bFirstOutsideDrag = false;
	m_bAllowDragginOutside = false;
	m_bPressedBar = false;
	m_bAllowReleasingOnBar = true;
	m_bOverBar = false;

    m_bRenderBackgroundBar = true;

	m_bDynamicColourBackground = false;

	m_lastX = 0;
	m_lastY = 0;

	m_disabled = false;

	Add(m_pScrollerDefault);
	Add(m_pScrollerHover);
	Add(m_pScrollerDragging);
	Add(m_pScrollerDisabled);
	Add(m_pScrollBackbarDefault);
	Add(m_pScrollBackbarPressed);

	m_labelOffset_X = 0;
	m_labelOffset_Y = 0;
	m_valueOffset_X = 0;
	m_valueOffset_Y = 0;
	m_incrementButtonsOffset_X = 0;
	m_incrementButtonsOffset_Y = 0;

	m_incrementButtons = addIncrementButtons;
	m_incrementAmount = incrementAmount;

	if(addIncrementButtons)
	{
		Add(m_pPlusButton);
		Add(m_pMinusButton);
	}

	Add(&m_value);
	Add(&m_name);

    m_ValueChangedCallback = nullptr;
    m_pValueChangedCallbackData = nullptr;

	ValueChanged();
}

Slider::Slider(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, float min, float max, float defaultValue, Colour fontColour, Colour outlineColour, bool addIncrementButtons, float incrementAmount)
  : Container(pRenderer),
    m_value(pRenderer, GUIFont, "", Colour(0.0f, 0.0f, 0.0f)),
    m_name(pRenderer, GUIFont, label, Colour(0.0f, 0.0f, 0.0f)),
	m_fontID(GUIFont),
	m_minValue(min),
	m_maxValue(max),
	m_defaultValue(defaultValue),
	m_eSliderDirection(ESliderDirection_Horizontal)
{
	m_pScrollerDefault = new DirectDrawRectangle(pRenderer);
	m_pScrollerHover = new DirectDrawRectangle(pRenderer);
	m_pScrollerDragging = new DirectDrawRectangle(pRenderer);
	m_pScrollerDisabled = new DirectDrawRectangle(pRenderer);
	m_pScrollBackbarDefault = new DirectDrawRectangle(pRenderer);
	m_pScrollBackbarPressed = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollerDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollerHover;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollerDragging;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollerDisabled;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollBackbarDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollBackbarPressed;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.5f, 0.5f, 0.5f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.5f, 0.5f, 0.5f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.5f, 0.5f, 0.5f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.5f, 0.5f, 0.5f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.05f, 0.05f, 0.05f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.05f, 0.05f, 0.05f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.05f, 0.05f, 0.05f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.05f, 0.05f, 0.05f, 1.0f));

	m_pPlusButton = new Button(pRenderer, GUIFont, OutlineGUIFont, "+", fontColour, outlineColour);
	m_pPlusButton->SetCallBackFunction(_PlusPressed);
	m_pPlusButton->SetCallBackData(this);
	m_pPlusButton->SetLabelOffset(0, 0);
	m_pMinusButton = new Button(pRenderer, GUIFont, OutlineGUIFont, "-", fontColour, outlineColour);
	m_pMinusButton->SetCallBackFunction(_MinusPressed);
	m_pMinusButton->SetCallBackData(this);
	m_pMinusButton->SetLabelOffset(1, 2);

	m_pPlusButton->SetDepth(2.0f);
	m_pMinusButton->SetDepth(2.0f);

	m_currentValue = defaultValue;

	m_bShowValue = true;
	m_ValuePrecision = 2;

	m_bDragging = false;
	m_bAllowDragging = true;
	m_bHover = false;
	m_bDragginOutside = false;
	m_bFirstOutsideDrag = false;
	m_bAllowDragginOutside = false;
	m_bPressedBar = false;
	m_bAllowReleasingOnBar = true;
	m_bOverBar = false;

	m_bRenderBackgroundBar = true;

	m_bDynamicColourBackground = false;

	m_lastX = 0;
	m_lastY = 0;

	m_disabled = false;

	Add(m_pScrollerDefault);
	Add(m_pScrollerHover);
	Add(m_pScrollerDragging);
	Add(m_pScrollerDisabled);
	Add(m_pScrollBackbarDefault);
	Add(m_pScrollBackbarPressed);

	m_labelOffset_X = 0;
	m_labelOffset_Y = 0;
	m_valueOffset_X = 0;
	m_valueOffset_Y = 0;
	m_incrementButtonsOffset_X = 0;
	m_incrementButtonsOffset_Y = 0;

	m_incrementButtons = addIncrementButtons;
	m_incrementAmount = incrementAmount;

	if(addIncrementButtons)
	{
		Add(m_pPlusButton);
		Add(m_pMinusButton);
	}

	m_value.SetLocation(0, 0);
	m_value.SetColour(fontColour);
	m_value.SetOutline(true);
	m_value.SetOutlineColour(outlineColour);
	m_value.SetOutlineFont(OutlineGUIFont);
	Add(&m_value);

	m_name.SetLocation(0, 0);
	m_name.SetColour(fontColour);
	m_name.SetOutline(true);
	m_name.SetOutlineColour(outlineColour);
	m_name.SetOutlineFont(OutlineGUIFont);
	Add(&m_name);

	m_ValueChangedCallback = nullptr;
	m_pValueChangedCallbackData = nullptr;

	ValueChanged();
}

Slider::~Slider()
{
	delete m_pScrollerDefault;
	delete m_pScrollerHover;
	delete m_pScrollerDragging;
	delete m_pScrollerDisabled;
	delete m_pScrollBackbarDefault;
	delete m_pScrollBackbarPressed;

	delete m_pPlusButton;
	delete m_pMinusButton;
}

void Slider::AddEventListeners()
{
	Container::AddEventListeners();

	AddMouseListener(this);
}

void Slider::RemoveEventListeners()
{
	Container::RemoveEventListeners();

	RemoveMouseListener(this);
}

void Slider::SetScrollerDefaultIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollerDefault)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollerDefault);

		delete m_pScrollerDefault;
		m_pScrollerDefault = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollerDefault = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollerDefault = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollerDefault);
	
	ValueChanged(true);
}

void Slider::SetScrollerHoverIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollerHover)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollerHover);

		delete m_pScrollerHover;
		m_pScrollerHover = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollerHover = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollerHover = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollerHover);

	ValueChanged(true);
}

void Slider::SetScrollerDraggingIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollerDragging)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollerDragging);

		delete m_pScrollerDragging;
		m_pScrollerDragging = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollerDragging = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollerDragging = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollerDragging);

	ValueChanged(true);
}

void Slider::SetScrollerDisabledIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollerDisabled)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollerDisabled);

		delete m_pScrollerDisabled;
		m_pScrollerDisabled = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollerDisabled = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollerDisabled = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollerDisabled);

	ValueChanged(true);
}

void Slider::SetScrollBackbarDefault(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollBackbarDefault)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollBackbarDefault);

		delete m_pScrollBackbarDefault;
		m_pScrollBackbarDefault = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollBackbarDefault = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollBackbarDefault = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollBackbarDefault);

	ValueChanged(true);
}

void Slider::SetScrollBackbarPressed(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollBackbarPressed)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollBackbarPressed);

		delete m_pScrollBackbarPressed;
		m_pScrollBackbarPressed = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollBackbarPressed = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollBackbarPressed = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollBackbarPressed);

	ValueChanged(true);
}

void Slider::SetScrollBackbarIncrementIconDefault(RenderRectangle *icon)
{
	m_pPlusButton->SetDefaultIcon(icon);
}

void Slider::SetScrollBackbarIncrementIconHover(RenderRectangle *icon)
{
	m_pPlusButton->SetHoverIcon(icon);
}

void Slider::SetScrollBackbarIncrementIconPressed(RenderRectangle *icon)
{
	m_pPlusButton->SetSelectedIcon(icon);
}

void Slider::SetScrollBackbarIncrementIconDisabled(RenderRectangle *icon)
{
	m_pPlusButton->SetDisabledIcon(icon);
}

void Slider::SetScrollBackbarDecrementIconDefault(RenderRectangle *icon)
{
	m_pMinusButton->SetDefaultIcon(icon);
}

void Slider::SetScrollBackbarDecrementIconHover(RenderRectangle *icon)
{
	m_pMinusButton->SetHoverIcon(icon);
}

void Slider::SetScrollBackbarDecrementIconPressed(RenderRectangle *icon)
{
	m_pMinusButton->SetSelectedIcon(icon);
}

void Slider::SetScrollBackbarDecrementIconDisabled(RenderRectangle *icon)
{
	m_pMinusButton->SetDisabledIcon(icon);
}

void Slider::SetDimensions(int x, int y, int width, int height)
{
	int lLabelSpace = 4;

	Component::SetDimensions(x, y, width, height);

	// Set the position on the value label to be next to the slider horizontally but centered vertically
	int valueLabelHeight = m_pRenderer->GetFreeTypeTextHeight(m_fontID, "%s", m_value.GetText().c_str());
	int l_center_height = (height / 2) - (valueLabelHeight / 2);
	m_value.SetLocation(width + lLabelSpace + m_valueOffset_X, l_center_height + m_valueOffset_Y);

	int nameSpace = 5;
	int nameLabelWidth = m_pRenderer->GetFreeTypeTextWidth(m_fontID, "%s", m_name.GetText().c_str());
	int nameLabelHeight = m_pRenderer->GetFreeTypeTextHeight(m_fontID, "%s", m_name.GetText().c_str());
	l_center_height = (height / 2) - (nameLabelHeight / 2);
	m_name.SetLocation(-nameLabelWidth - nameSpace + m_valueOffset_X, l_center_height + m_valueOffset_Y);

	if(m_eSliderDirection == ESliderDirection_Horizontal)
	{
		int half_height = height / 2;
		m_pPlusButton->SetDimensions(width + lLabelSpace + 35 + m_incrementButtonsOffset_X, half_height + m_incrementButtonsOffset_Y, half_height, half_height);
		m_pMinusButton->SetDimensions(width + lLabelSpace + 35 + m_incrementButtonsOffset_X, 0 + m_incrementButtonsOffset_Y, half_height, half_height);
	}
	else
	{
		// TODO: Plus and Minus increment buttons when a vertical slider
	}
}

void Slider::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void Slider::SetShowValue(bool lShow)
{
	m_bShowValue = lShow;
}

void Slider::SetValuePrecision(int lPresision)
{
	m_ValuePrecision = lPresision;

	UpdateTextString();
}

void Slider::SetValuePositionOffset(int lOffsetX, int lOffsetY)
{
	m_valueOffset_X = lOffsetX;
	m_valueOffset_Y = lOffsetY;
}

void Slider::SetLabelPositionOffset(int lOffsetX, int lOffsetY)
{
	m_labelOffset_X = lOffsetX;
	m_labelOffset_Y = lOffsetY;
}

void Slider::SetIncrementButtonsOffset(int lOffsetX, int lOffsetY)
{
	m_incrementButtonsOffset_X = lOffsetX;
	m_incrementButtonsOffset_Y = lOffsetY;

	int lLabelSpace = 4;
	int width = GetDimensions().m_width;
	int height = GetDimensions().m_height;

	if(m_eSliderDirection == ESliderDirection_Horizontal)
	{
		int half_height = height / 2;
		m_pPlusButton->SetDimensions(width + lLabelSpace + 35 + m_incrementButtonsOffset_X, half_height + m_incrementButtonsOffset_Y, half_height, half_height);
		m_pMinusButton->SetDimensions(width + lLabelSpace + 35 + m_incrementButtonsOffset_X, 0 + m_incrementButtonsOffset_Y, half_height, half_height);
	}
	else
	{
		// TODO: Plus and Minus increment buttons when a vertical slider
	}
}

void Slider::UpdateTextString()
{
	// Update the text string
	char lValue[16];
	
	switch(m_ValuePrecision)
	{
		case 0: sprintf(lValue, "%.0f", m_currentValue); break;
		case 1: sprintf(lValue, "%.1f", m_currentValue); break;
		case 2: sprintf(lValue, "%.2f", m_currentValue); break;
		case 3: sprintf(lValue, "%.3f", m_currentValue); break;
		default: sprintf(lValue, "%.2f", m_currentValue); break;
	};
	
	m_value.SetText(lValue);
}

void Slider::SetSliderParams(int barHeight, int sliderWidth, ESliderDirection lDirection)
{
	m_barHeight = barHeight;
	m_sliderWidth = sliderWidth;
	m_eSliderDirection = lDirection;

	ValueChanged();
}

float Slider::GetMinValue()
{
	return m_minValue;
}

float Slider::GetMaxValue()
{
	return m_maxValue;
}

float Slider::GetDefaultValue()
{
	return m_defaultValue;
}

float Slider::GetCurrentValue()
{
	return m_currentValue;
}

void Slider::SetMinValue(float minValue)
{
	m_minValue = minValue;
}

void Slider::SetMaxValue(float maxValue)
{
	m_maxValue = maxValue;
}

void Slider::SetDefaultValue(float defaultValue)
{
	m_defaultValue = defaultValue;
}

void Slider::SetCurrentValue(float currentValue, bool supressCallback)
{
	m_currentValue = currentValue;

	ValueChanged(supressCallback);
}

void Slider::SetRenderBackgroundBar(bool render)
{
    m_bRenderBackgroundBar = render;
}

void Slider::SetDynamicBackgroundColour(Colour top_left, Colour bottom_left, Colour top_right, Colour bottom_right)
{
	m_bDynamicColourBackground = true;
	m_backgroundColour_topLeft = top_left;
	m_backgroundColour_topRight = top_right;
	m_backgroundColour_BottomLeft = bottom_left;
	m_backgroundColour_BottomRight = bottom_right;
}

ESliderDirection Slider::GetSliderDirection()
{
	return m_eSliderDirection;
}

void Slider::SetSliderDirection(ESliderDirection lDirection)
{
	m_eSliderDirection = lDirection;
}

void Slider::SetDisabled(bool val)
{
	m_disabled = val;

	SetEnabled(m_disabled == false);

	m_pPlusButton->SetDisabled(val);
	m_pMinusButton->SetDisabled(val);
}

EComponentType Slider::GetComponentType() const
{
	return EComponentType_Slider;
}

void Slider::SetVaueChangedCallBackFunction(FunctionCallback lFunction)
{
    m_ValueChangedCallback = lFunction;
}

void Slider::SetVaueChangedCallBackData(void *lpData)
{
    m_pValueChangedCallbackData = lpData;
}

void Slider::MouseEntered(const MouseEvent& lEvent)
{
	m_bDragginOutside = false;
	m_bFirstOutsideDrag = false;

	OnMouseEnter();
}

void Slider::MouseExited(const MouseEvent& lEvent)
{
	m_bHover = false;
	m_bOverBar = false;
	m_bDragginOutside = true;

	OnMouseExit();
}

void Slider::MousePressed(const MouseEvent& lEvent)
{
	int lSliderX;
	int lSliderY;
	int lSliderWidth;
	int lSliderHeight;

	int lBarX;
	int lBarY;
	int lBarWidth;
	int lBarHeight;

	if(m_eSliderDirection == ESliderDirection_Horizontal)
	{
		lSliderX = GetLocationOnScreen().m_x + (int)((m_dimensions.m_width - m_sliderWidth) * ((m_currentValue - m_minValue) / (m_maxValue - m_minValue)));
		lSliderY = GetLocationOnScreen().m_y;
		lSliderWidth = m_sliderWidth;
		lSliderHeight = m_dimensions.m_height;

		lBarX = GetLocationOnScreen().m_x + (m_sliderWidth / 2);
		lBarY = GetLocationOnScreen().m_y + (m_dimensions.m_height / 2) - (m_barHeight / 2);
		lBarWidth = m_dimensions.m_width - m_sliderWidth;
		lBarHeight = m_barHeight;
	}
	else //if(m_eSliderDirection == ESliderDirection_Vertical)
	{
		lSliderX = GetLocationOnScreen().m_x;
		lSliderY = GetLocationOnScreen().m_y + (int)((m_dimensions.m_height - m_sliderWidth) * ((m_currentValue - m_minValue) / (m_maxValue - m_minValue)));
		lSliderWidth = m_dimensions.m_width;
		lSliderHeight = m_sliderWidth;

		lBarX = GetLocationOnScreen().m_x + (m_dimensions.m_width / 2) - (m_barHeight / 2);
		lBarY = GetLocationOnScreen().m_y + (m_sliderWidth / 2);
		lBarWidth = m_barHeight;
		lBarHeight = m_dimensions.m_height - m_sliderWidth;
	}

	int mouseX = lEvent.GetX();
	int mouseY = lEvent.GetY();

	// Check to see if we have clicked the slider to start dragging
	if((mouseX >= lSliderX) && (mouseX < lSliderX + lSliderWidth) && (mouseY >= lSliderY) && (mouseY < lSliderY + lSliderHeight))
	{
		if(m_bAllowDragging)
		{
			m_bDragging = true;
			m_bAllowDragginOutside = true;
		}
	}
	else
	{
		// Check to see if we have clicked on the bar to 'zoom' to a location
		if((mouseX >= lBarX) && (mouseX < lBarX + lBarWidth) && (mouseY >= lBarY) && (mouseY < lBarY + lBarHeight))
		{
			if(m_bAllowReleasingOnBar && !m_bDragging)
			{
				m_bPressedBar = true;
			}
		}
		else
		{
			// Don't allow releasing on the bar since we didnt first click on the bar, have to wait for release now
			m_bAllowReleasingOnBar = false;
		}

		// Don't allow dragging, or dragging outside, since our first press wasnt on the slider, have to wait for release now
		if(!m_bDragging)
		{
			m_bAllowDragging = false;
			m_bAllowDragginOutside = false;
		}
	}

	// If our parent is a GUIWindow, then makew this window have focus in the GUI, used to make it's depth the highest
	if(GetParent() != nullptr && GetParent()->GetComponentType() == EComponentType_GUIWindow)
	{
		GUIWindow* lpParentWindow = (GUIWindow *)GetParent();
		lpParentWindow->SetFocusWindow();
	}

	OnMousePressed();
}

void Slider::MouseReleased(const MouseEvent& lEvent)
{
	if(!m_bDragging && m_bPressedBar)
	{
		int lSliderWidth;
		int lSliderHeight;

		int lBarX;
		int lBarY;
		int lBarWidth;
		int lBarHeight;

		if(m_eSliderDirection == ESliderDirection_Horizontal)
		{
			lBarX = GetLocationOnScreen().m_x + (m_sliderWidth / 2);
			lBarY = GetLocationOnScreen().m_y + (m_dimensions.m_height / 2) - (m_barHeight / 2);
			lBarWidth = m_dimensions.m_width - m_sliderWidth;
			lBarHeight = m_barHeight;

			lSliderWidth = m_sliderWidth;
			lSliderHeight = m_dimensions.m_height;
		}
		else //if(m_eSliderDirection == ESliderDirection_Vertical)
		{
			lBarX = GetLocationOnScreen().m_x + (m_dimensions.m_width / 2) - (m_barHeight / 2);
			lBarY = GetLocationOnScreen().m_y + (m_sliderWidth / 2);
			lBarWidth = m_barHeight;
			lBarHeight = m_dimensions.m_height - m_sliderWidth;

			lSliderWidth = m_dimensions.m_width;
			lSliderHeight = m_sliderWidth;
		}

		int mouseX = lEvent.GetX();
		int mouseY = lEvent.GetY();

		// Store the value before we change it, to see if we have actually changed the value
		float lValueBefore = m_currentValue;

		if((mouseX >= lBarX) && (mouseX < lBarX + lBarWidth) && (mouseY >= lBarY) && (mouseY < lBarY + lBarHeight))
		{
			// Check to see if we have clicked on the bar to 'zoom' to a location

			if(m_eSliderDirection == ESliderDirection_Horizontal)
			{
				int lRelativeX = mouseX - GetLocationOnScreen().m_x;
				float ratio = (float)lRelativeX / (float)lBarWidth;
				m_currentValue = m_minValue + ((m_maxValue - m_minValue) * ratio);
			}
			else //if(m_eSliderDirection == ESliderDirection_Vertical)
			{
				int lRelativeY = mouseY - GetLocationOnScreen().m_y;
				float ratio = (float)lRelativeY / (float)lBarWidth;
				m_currentValue = m_minValue + ((m_maxValue - m_minValue) * ratio);
			}
		}

		if(m_currentValue != lValueBefore)
		{
			ValueChanged();
		}

		m_bPressedBar = false;
	}

	m_bDragging = false;
	m_bPressedBar = false;

	// Allow dragging again now, since we have released the button, we can now check for dragging again on pressing
	m_bAllowDragging = true;

	// Allow releasing on the bar now, we can now check for this again on pressing
	m_bAllowReleasingOnBar = true;

	OnMouseReleased();
}

void Slider::MouseReleasedOutside(const MouseEvent& lEvent)
{
	m_bDragging = false;
	m_bPressedBar = false;

	// Allow releasing on the bar now, we can now check for this again on pressing
	m_bAllowReleasingOnBar = true;
}

void Slider::MouseClicked(const MouseEvent& lEvent)
{
	OnMouseClicked();
}

void Slider::MouseDragged(const MouseEvent& lEvent)
{
	if(m_bDragging)
	{
		int l_ChangeX = lEvent.GetX();
		int l_ChangeY = lEvent.GetY();

		if(l_ChangeX == 0 && l_ChangeY == 0)
			return;

		float lValueChange;
		if(m_eSliderDirection == ESliderDirection_Horizontal)
		{
			lValueChange = static_cast<float>(l_ChangeX) * ((m_maxValue - m_minValue) / (static_cast<float>(m_dimensions.m_width) - static_cast<float>(m_sliderWidth)));
		}
		else //if(m_eSliderDirection == ESliderDirection_Vertical)
		{
			lValueChange = static_cast<float>(l_ChangeY) * ((m_maxValue - m_minValue) / (static_cast<float>(m_dimensions.m_height) - static_cast<float>(m_sliderWidth)));
		}

		// Store the value before we change it, to see if we have actually changed the value
		float lValueBefore = m_currentValue;

		m_currentValue += lValueChange;

		if(m_currentValue > m_maxValue)
		{
			m_currentValue = m_maxValue;
		}
		if(m_currentValue < m_minValue)
		{
			m_currentValue = m_minValue;
		}

		if(m_currentValue != lValueBefore)
		{
			ValueChanged();
		}
	}
}

void Slider::MouseDraggedOutside(const MouseEvent& lEvent)
{
	if(!m_bAllowDragginOutside)
	{
		// If we are not allowing dragging outside, just early out
		return;
	}

	if(!m_bFirstOutsideDrag)
	{
		m_lastX = lEvent.GetX();
		m_lastY = lEvent.GetY();
		m_bFirstOutsideDrag = true;
	}

	if(m_bDragginOutside)
	{
		int lNewX = lEvent.GetX();
		int lNewY = lEvent.GetY();

		int lMinX = GetLocationOnScreen().m_x;
		int lMinY = GetLocationOnScreen().m_y;
		int lMaxX = lMinX + m_dimensions.m_width;
		int lMaxY = lMinY + m_dimensions.m_height;

		int l_ChangeX = lNewX - m_lastX;
		int l_ChangeY = lNewY - m_lastY;

		if(l_ChangeX == 0 && l_ChangeY == 0)
			return;

		// Store the value before we change it, to see if we have actually changed the value
		float lValueBefore = m_currentValue;

		float lValueChange;
		if(m_eSliderDirection == ESliderDirection_Horizontal)
		{
			lValueChange = static_cast<float>(l_ChangeX) * ((m_maxValue - m_minValue) / (static_cast<float>(m_dimensions.m_width) - static_cast<float>(m_sliderWidth)));

			if( (lValueChange > 0) && (lNewX > lMinX))
			{
				m_currentValue += lValueChange;
			}
			else if((lValueChange < 0) && (lNewX < lMaxX))
			{
				m_currentValue += lValueChange;
			}
		}
		else //if(m_eSliderDirection == ESliderDirection_Vertical)
		{
			lValueChange = static_cast<float>(l_ChangeY) * ((m_maxValue - m_minValue) / (static_cast<float>(m_dimensions.m_height) - static_cast<float>(m_sliderWidth)));

			if( (lValueChange > 0) && (lNewY > lMinY))
			{
				m_currentValue += lValueChange;
			}
			else if((lValueChange < 0) && (lNewY < lMaxY))
			{
				m_currentValue += lValueChange;
			}
		}


		if(m_currentValue > m_maxValue)
		{
			m_currentValue = m_maxValue;
		}
		if(m_currentValue < m_minValue)
		{
			m_currentValue = m_minValue;
		}

		if(m_currentValue != lValueBefore)
		{
			ValueChanged();
		}

		m_lastX = lNewX;
		m_lastY = lNewY;
	}
}

void Slider::MouseMotion(const MouseEvent& lEvent)
{
	int lSliderX;
	int lSliderY;
	int lSliderWidth;
	int lSliderHeight;

	int lBarX;
	int lBarY;
	int lBarWidth;
	int lBarHeight;

	if(m_eSliderDirection == ESliderDirection_Horizontal)
	{
		lSliderX = GetLocationOnScreen().m_x + (int)((m_dimensions.m_width - m_sliderWidth) * ((m_currentValue - m_minValue) / (m_maxValue - m_minValue)));
		lSliderY = GetLocationOnScreen().m_y;
		lSliderWidth = m_sliderWidth;
		lSliderHeight = m_dimensions.m_height;

		lBarX = GetLocationOnScreen().m_x + (m_sliderWidth / 2);
		lBarY = GetLocationOnScreen().m_y + (m_dimensions.m_height / 2) - (m_barHeight / 2);
		lBarWidth = m_dimensions.m_width - m_sliderWidth;
		lBarHeight = m_barHeight;
	}
	else //if(m_eSliderDirection == ESliderDirection_Vertical)
	{
		lSliderX = GetLocationOnScreen().m_x;
		lSliderY = GetLocationOnScreen().m_y + (int)((m_dimensions.m_height - m_sliderWidth) * ((m_currentValue - m_minValue) / (m_maxValue - m_minValue)));
		lSliderWidth = m_dimensions.m_width;
		lSliderHeight = m_sliderWidth;

		lBarX = GetLocationOnScreen().m_x + (m_dimensions.m_width / 2) - (m_barHeight / 2);
		lBarY = GetLocationOnScreen().m_y + (m_sliderWidth / 2);
		lBarWidth = m_barHeight;
		lBarHeight = m_dimensions.m_height - m_sliderWidth;
	}

	int mouseX = lEvent.GetX();
	int mouseY = lEvent.GetY();

	// Check to see if we are over the slider
	if((mouseX > lSliderX) && (mouseX < lSliderX + lSliderWidth) && (mouseY > lSliderY) && (mouseY < lSliderY + lSliderHeight))
	{
		if(!m_bPressedBar)
		{
			m_bHover = true;
		}
	}
	else
	{
		m_bHover = false;
	}

	// Check to see if we are over the bar
	if((mouseX >= lBarX) && (mouseX < lBarX + lBarWidth) && (mouseY >= lBarY) && (mouseY < lBarY + lBarHeight))
	{
		m_bOverBar = true;
	}
	else
	{
		m_bOverBar = false;
	}
}

void Slider::OnMouseEnter()
{
	/* Nothing */
}

void Slider::OnMouseExit()
{
	/* Nothing */
}

void Slider::OnMousePressed()
{
	/* Nothing */
}

void Slider::OnMouseReleased()
{
	/* Nothing */
}

void Slider::OnMouseClicked()
{
	/* Nothing */
}

void Slider::OnValueChanged()
{
	/* Nothing */
}

void Slider::ValueChanged(bool supressCallback)
{
	// Update the text string
	UpdateTextString();

	// Update the slider dimensions
	int lSliderX;
	int lSliderY;
	int lSliderWidth;
	int lSliderHeight;

	int lBarX;
	int lBarY;
	int lBarWidth;
	int lBarHeight;

	if(m_eSliderDirection == ESliderDirection_Horizontal)
	{
		lSliderX = (int)((m_dimensions.m_width - m_sliderWidth) * ((m_currentValue - m_minValue) / (m_maxValue - m_minValue)));
		lSliderY = 0;
		lSliderWidth = m_sliderWidth;
		lSliderHeight = m_dimensions.m_height;

		lBarX = (m_sliderWidth / 2);
		lBarY = (m_dimensions.m_height / 2) - (m_barHeight / 2);
		lBarWidth = m_dimensions.m_width - m_sliderWidth;
		lBarHeight = m_barHeight;
	}
	else //m_eSliderDirection == ESliderDirection_Vertical
	{
		lSliderX = 0;
		lSliderY = (int)((m_dimensions.m_height - m_sliderWidth) * ((m_currentValue - m_minValue) / (m_maxValue - m_minValue)));
		lSliderWidth = m_dimensions.m_width;
		lSliderHeight = m_sliderWidth;

		lBarX = (m_dimensions.m_width / 2) - (m_barHeight / 2);
		lBarY = (m_sliderWidth / 2);
		lBarWidth = m_barHeight;
		lBarHeight = m_dimensions.m_height - m_sliderWidth;
	}

	m_pScrollerDefault->SetDimensions(lSliderX, lSliderY, lSliderWidth, lSliderHeight);
	m_pScrollerHover->SetDimensions(lSliderX, lSliderY, lSliderWidth, lSliderHeight);
	m_pScrollerDragging->SetDimensions(lSliderX, lSliderY, lSliderWidth, lSliderHeight);
	m_pScrollerDisabled->SetDimensions(lSliderX, lSliderY, lSliderWidth, lSliderHeight);
	m_pScrollBackbarDefault->SetDimensions(lBarX, lBarY, lBarWidth, lBarHeight);
	m_pScrollBackbarPressed->SetDimensions(lBarX, lBarY, lBarWidth, lBarHeight);

	OnValueChanged();

    // Call the callback function callback if this class is just a simple button
    if(supressCallback == false && m_ValueChangedCallback)
    {
        m_ValueChangedCallback(m_pValueChangedCallbackData);
    }
}

void Slider::_PlusPressed(void *lpData)
{
	Slider *_this = (Slider*)lpData;

	if(_this)
	{
		_this->PlusPressed();
	}
}

void Slider::PlusPressed()
{
	m_currentValue += m_incrementAmount;

	if(m_currentValue > m_maxValue)
	{
		m_currentValue = m_maxValue;
	}

	ValueChanged();
}

void Slider::_MinusPressed(void *lpData)
{
	Slider *_this = (Slider*)lpData;

	if(_this)
	{
		_this->MinusPressed();
	}
}

void Slider::MinusPressed()
{
	m_currentValue -= m_incrementAmount;

	if(m_currentValue < m_minValue)
	{
		m_currentValue = m_minValue;
	}

	ValueChanged();
}

void Slider::DrawSelf()
{
	//Container::DrawSelf();

	/*
	int l_depth = static_cast<int>(GetDepth());

	m_pRenderer->PushMatrix();
		m_pRenderer->SetRenderMode(RM_SOLID);
		// Draw slider
		m_pRenderer->EnableImmediateMode(IM_QUADS);
		if(m_bDragging)
		{
			m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if(m_bHover)
		{
			m_pRenderer->ImmediateColourAlpha(1.0f, 0.0f, 1.0f, 1.0f);
		}
		else
		{
			m_pRenderer->ImmediateColourAlpha(0.0f, 0.0f, 0.0f, 1.0f);
		}
		m_pRenderer->ImmediateVertex(lSliderX, lSliderY, l_depth);
		m_pRenderer->ImmediateVertex(lSliderX + lSliderWidth, lSliderY, l_depth);
		m_pRenderer->ImmediateVertex(lSliderX + lSliderWidth, lSliderY + lSliderHeight, l_depth);
		m_pRenderer->ImmediateVertex(lSliderX, lSliderY + lSliderHeight, l_depth);
		m_pRenderer->DisableImmediateMode();


		// Draw bar
		m_pRenderer->EnableImmediateMode(IM_QUADS);
		Colour lStartColour = mStartColour;
		Colour lEndColour = mEndColour;
		if(m_bOverBar && m_bPressedBar)
		{
			lStartColour.SetRed(lStartColour.GetRed() * 0.75f);
			lStartColour.SetGreen(lStartColour.GetGreen() * 0.75f);
			lStartColour.SetBlue(lStartColour.GetBlue() * 0.75f);

			lEndColour.SetRed(lEndColour.GetRed() * 0.75f);
			lEndColour.SetGreen(lEndColour.GetGreen() * 0.75f);
			lEndColour.SetBlue(lEndColour.GetBlue() * 0.75f);
		}
		m_pRenderer->ImmediateColourAlpha(lStartColour.GetRed(), lStartColour.GetGreen(), lStartColour.GetBlue(), lStartColour.GetAlpha());
		m_pRenderer->ImmediateVertex(lBarX, lBarY, l_depth);
		m_pRenderer->ImmediateColourAlpha(lEndColour.GetRed(), lEndColour.GetGreen(), lEndColour.GetBlue(), lEndColour.GetAlpha());
		m_pRenderer->ImmediateVertex(lBarX + lBarWidth, lBarY, l_depth);
		m_pRenderer->ImmediateVertex(lBarX + lBarWidth, lBarY + lBarHeight, l_depth);
		m_pRenderer->ImmediateColourAlpha(lStartColour.GetRed(), lStartColour.GetGreen(), lStartColour.GetBlue(), lStartColour.GetAlpha());
		m_pRenderer->ImmediateVertex(lBarX, lBarY + lBarHeight, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
	*/

	m_pScrollerDefault->SetVisible(false);
	m_pScrollerHover->SetVisible(false);
	m_pScrollerDragging->SetVisible(false);
	m_pScrollBackbarDefault->SetVisible(false);
	m_pScrollBackbarPressed->SetVisible(false);
	m_pScrollerDisabled->SetVisible(false);

	if(m_disabled)
	{
		m_pScrollerDisabled->SetVisible(true);
	}
	else if(m_bDragging)
	{
		m_pScrollerDragging->SetVisible(true);
	}
	else if(m_bHover)
	{
		m_pScrollerHover->SetVisible(true);
	}
	else
	{
		m_pScrollerDefault->SetVisible(true);
	}

    if(m_bRenderBackgroundBar)
    {
        if(m_bOverBar && m_bPressedBar)
        {
            m_pScrollBackbarPressed->SetVisible(true);
        }
        else
        {
            m_pScrollBackbarDefault->SetVisible(true);
        }
    }
	else if(m_bDynamicColourBackground)
	{
		int lBarX;
		int lBarY;
		int lBarWidth;
		int lBarHeight;
		int l_depth = static_cast<int>(GetDepth()) + 1;

		if(m_eSliderDirection == ESliderDirection_Horizontal)
		{
			lBarX = 0;
			lBarY = 0;
			lBarWidth = m_dimensions.m_width;
			lBarHeight = m_dimensions.m_height;
		}
		else //m_eSliderDirection == ESliderDirection_Vertical
		{
			lBarX = 0;
			lBarY = 0;
			lBarWidth = m_dimensions.m_width;
			lBarHeight = m_dimensions.m_height;
		}

		m_pRenderer->PushMatrix();
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->EnableImmediateMode(IM_QUADS);
				m_pRenderer->ImmediateColourAlpha(m_backgroundColour_BottomLeft.GetRed(), m_backgroundColour_BottomLeft.GetGreen(), m_backgroundColour_BottomLeft.GetBlue(), m_backgroundColour_BottomLeft.GetAlpha());
				m_pRenderer->ImmediateVertex(lBarX, lBarY, l_depth);
				m_pRenderer->ImmediateColourAlpha(m_backgroundColour_BottomRight.GetRed(), m_backgroundColour_BottomRight.GetGreen(), m_backgroundColour_BottomRight.GetBlue(), m_backgroundColour_BottomRight.GetAlpha());
				m_pRenderer->ImmediateVertex(lBarX + lBarWidth, lBarY, l_depth);
				m_pRenderer->ImmediateColourAlpha(m_backgroundColour_topRight.GetRed(), m_backgroundColour_topRight.GetGreen(), m_backgroundColour_topRight.GetBlue(), m_backgroundColour_topRight.GetAlpha());
				m_pRenderer->ImmediateVertex(lBarX + lBarWidth, lBarY + lBarHeight, l_depth);
				m_pRenderer->ImmediateColourAlpha(m_backgroundColour_topLeft.GetRed(), m_backgroundColour_topLeft.GetGreen(), m_backgroundColour_topLeft.GetBlue(), m_backgroundColour_topLeft.GetAlpha());
			m_pRenderer->ImmediateVertex(lBarX, lBarY + lBarHeight, l_depth);
			m_pRenderer->DisableImmediateMode();
		m_pRenderer->PopMatrix();
	}

	// Set the value label visibility
	m_value.SetVisible(m_bShowValue);

	m_name.SetVisible(true);
}