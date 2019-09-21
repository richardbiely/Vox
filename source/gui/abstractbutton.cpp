// ******************************************************************************
//
// Filename:	abstractbutton.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   An abstract button class used as a base
//   class for all button related components.
//
// Revision History:
//   Initial Revision - 28/05/07
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "abstractbutton.h"


AbstractButton::AbstractButton(Renderer* pRenderer, unsigned int GUIFont, const std::string &label)
  : Container(pRenderer),
    m_label(pRenderer, GUIFont, label, Colour(1.0f, 1.0f, 1.0f)),
    m_pDefaultIcon(0),
    m_pSelectedIcon(0),
	m_pHoverIcon(0),
	m_pDisabledIcon(0),
	m_bSelected(false),
	m_bHover(false),
	m_bDisabled(false)
{
	m_bDisplayLabel = true;

	m_Callback = nullptr;
	m_pCallbackData = nullptr;

	m_Callback_Pressed = nullptr;
	m_pCallbackData_Pressed = nullptr;

	m_Callback_Released = nullptr;
	m_pCallbackData_Released = nullptr;

	m_EnterCallback = nullptr;
	m_pEnterCallbackData = nullptr;

	m_ExitCallback = nullptr;
	m_pExitCallbackData = nullptr;

	m_labelOffsetX = 0;
	m_labelOffsetY = 0;
}

AbstractButton::~AbstractButton()
{
}

void AbstractButton::SetLabel(const string &label)
{
	m_label.SetText(label);
	
	// Set the label location to be the center of the button
	int buttonLabelHeight = m_pRenderer->GetFreeTypeTextHeight(m_label.GetFontID(), "%s", m_label.GetText().c_str());
	int buttonLabelWidth = m_pRenderer->GetFreeTypeTextWidth(m_label.GetFontID(), "%s", m_label.GetText().c_str());
	int m_center_width = (int)((m_dimensions.m_width * 0.5f ) - (buttonLabelWidth * 0.5f));
	int m_center_height = (int)((m_dimensions.m_height * 0.5f) - (buttonLabelHeight * 0.5f));
	m_label.SetLocation(m_center_width + m_labelOffsetX, m_center_height + m_labelOffsetY);
}


void AbstractButton::SetDisplayLabel(bool display)
{
	m_bDisplayLabel = display;
}

void AbstractButton::RemoveIcon(RenderRectangle *pRemoveIcon)
{
	if (pRemoveIcon)
	{
		Remove(pRemoveIcon);
		delete pRemoveIcon;
		pRemoveIcon = nullptr;
	}
}

void AbstractButton::RemoveIcon(const std::string &fileName)
{
	bool removed = false;
	for (int i = 0; i < GetNumberOfChildren() && removed == false; i++)
	{
		if (GetChild(i)->GetComponentType() == EComponentType_Icon)
		{
			Icon* pIcon = (Icon*)GetChild(i);
			if (pIcon->GetFileName() == fileName)
			{
				Remove(pIcon);

				removed = true;
			}
		}
	}
}

void AbstractButton::SetDefaultIcons(Renderer* pRenderer)
{
	RemoveIcon(m_pDefaultIcon);
	RemoveIcon(m_pHoverIcon);
	RemoveIcon(m_pSelectedIcon);
	RemoveIcon(m_pDisabledIcon);

	m_pDefaultIcon = new DirectDrawRectangle(pRenderer);
	m_pHoverIcon = new DirectDrawRectangle(pRenderer);
	m_pSelectedIcon = new DirectDrawRectangle(pRenderer);
	m_pDisabledIcon = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pDefaultIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pHoverIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pSelectedIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pDisabledIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	Add(m_pDefaultIcon);
	Add(m_pSelectedIcon);
	Add(m_pHoverIcon);
	Add(m_pDisabledIcon);

	// Set the dimensions of the icons
	Dimensions d = GetDimensions();
	m_pDefaultIcon->SetDimensions(0, 0, d.m_width, d.m_height);
	m_pSelectedIcon->SetDimensions(0, 0, d.m_width, d.m_height);
	m_pHoverIcon->SetDimensions(0, 0, d.m_width, d.m_height);
	m_pDisabledIcon->SetDimensions(0, 0, d.m_width, d.m_height);
}

void AbstractButton::SetDefaultIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pDefaultIcon)
	{
		// If we already own a default icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pDefaultIcon);

		delete m_pDefaultIcon;
		m_pDefaultIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pDefaultIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pDefaultIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pDefaultIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pDefaultIcon);
}

void AbstractButton::SetSelectedIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pSelectedIcon)
	{
		// If we already own a selected icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pSelectedIcon);

		delete m_pSelectedIcon;
		m_pSelectedIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then create a new copy of it
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pSelectedIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pSelectedIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pSelectedIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pSelectedIcon);
}

void AbstractButton::SetHoverIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pHoverIcon)
	{
		// If we already own a hover icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pHoverIcon);

		delete m_pHoverIcon;
		m_pHoverIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pHoverIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pHoverIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pHoverIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pHoverIcon);
}

void AbstractButton::SetDisabledIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pDisabledIcon)
	{
		// If we already own a disabled icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pDisabledIcon);

		delete m_pDisabledIcon;
		m_pDisabledIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pDisabledIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pDisabledIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pDisabledIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pDisabledIcon);
}

Label AbstractButton::GetLabel() const
{
	return m_label;
}

void AbstractButton::SetSelected(bool val)
{
	m_bSelected = val;

	// TEMP : Remove button state animations until animation switching can be worked on.
	/*
	if(m_bSelected)
	{
		SetAnimation(m_pSelectedAnimation);
	}
	else
	{
		if(m_bHover)
		{
			SetAnimation(m_pHoverAnimation);
		}
		else
		{
			SetAnimation(m_pDefaultAnimation);
		}
	}
	*/
}

void AbstractButton::SetHover(bool val)
{
	m_bHover = val;

	// TEMP : Remove button state animations until animation switching can be worked on.
	/*
	if(m_bHover)
	{
		SetAnimation(m_pHoverAnimation);
	}
	else
	{
		if(m_bSelected)
		{
			SetAnimation(m_pSelectedAnimation);
		}
		else
		{
			SetAnimation(m_pDefaultAnimation);
		}
	}
	*/
}

void AbstractButton::SetDisabled(bool val)
{
	m_bDisabled = val;

	if(m_bDisabled)
	{
		SetSelected(false);
		SetHover(false);

		// TEMP : Remove button state animations until animation switching can be worked on.
		//SetAnimation(m_pDisabledAnimation);
	}
}

bool AbstractButton::IsSelected() const
{
	return m_bSelected;
}

bool AbstractButton::IsHover() const
{
	return m_bHover;
}

bool AbstractButton::IsDisabled() const
{
	return m_bDisabled;
}

void AbstractButton::SetCallBackFunction(FunctionCallback lFunction)
{
	m_Callback = lFunction;
}

void AbstractButton::SetCallBackData(void *lpData)
{
	m_pCallbackData = lpData;
}

void AbstractButton::SetCallBackFunction_Pressed(FunctionCallback lFunction)
{
	m_Callback_Pressed = lFunction;
}

void AbstractButton::SetCallBackData_Pressed(void *lpData)
{
	m_pCallbackData_Pressed = lpData;
}

void AbstractButton::SetCallBackFunction_Released(FunctionCallback lFunction)
{
	m_Callback_Released = lFunction;
}

void AbstractButton::SetCallBackData_Released(void *lpData)
{
	m_pCallbackData_Released = lpData;
}


void AbstractButton::SetEnterCallBackFunction(FunctionCallback lFunction)
{
	m_EnterCallback = lFunction;
}

void AbstractButton::SetEnterCallBackData(void *lpData)
{
	m_pEnterCallbackData = lpData;
}

void AbstractButton::SetExitCallBackFunction(FunctionCallback lFunction)
{
	m_ExitCallback = lFunction;
}

void AbstractButton::SetExitCallBackData(void *lpData)
{
	m_pExitCallbackData = lpData;
}

EComponentType AbstractButton::GetComponentType() const
{
	return EComponentType_AbstractButton;
}