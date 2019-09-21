// ******************************************************************************
//
// Filename:	optionbox.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   An option box (radio button) GUI widget
//
// Revision History:
//   Initial Revision - 02/02/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "optionbox.h"
#include "optioncontroller.h"
#include "directdrawrectangle.h"
#include "multitextureicon.h"
#include "guiwindow.h"


OptionBox::OptionBox(Renderer* pRenderer, unsigned int GUIFont, const std::string &label)
  : AbstractButton(pRenderer, GUIFont, label),
	m_pToggledIcon(0),
	m_pToggledSelectedIcon(0),
	m_pToggledHoverIcon(0),
	m_pToggledDisabledIcon(0)
{
	SetDefaultIcons(pRenderer);

	m_label.SetLocation(0, 0);
	m_label.SetDepth(3.0f);
	Add(&m_label);

	m_bToggled = false;

	m_pressedOffsetX = 0;
	m_pressedOffsetY = 0;
	m_offsetApplied = false;

	m_bChangeLabelText = false;
}

OptionBox::OptionBox(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, Colour fontColour, Colour outlineColour)
  : AbstractButton(pRenderer, GUIFont, label),
	m_pToggledIcon(0),
	m_pToggledSelectedIcon(0),
	m_pToggledHoverIcon(0),
	m_pToggledDisabledIcon(0)
{
	SetDefaultIcons(pRenderer);

	m_normalLabelColour = fontColour;
	m_hoverLabelColour = fontColour;
	m_PressedLabelColour = fontColour;

	m_label.SetLocation(0, 0);
	m_label.SetColour(fontColour);
	m_label.SetOutline(true);
	m_label.SetOutlineColour(outlineColour);
	m_label.SetOutlineFont(OutlineGUIFont);
	m_label.SetDepth(3.0f);
	Add(&m_label);

	m_bToggled = false;

	m_pressedOffsetX = 0;
	m_pressedOffsetY = 0;
	m_offsetApplied = false;

	m_bChangeLabelText = false;
}

OptionBox::OptionBox(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, const std::string &defaultTexture, const std::string &selectedTexture, const std::string &hoverTexture, const std::string &disabledTexture,
																							  const std::string &toggledTexture, const std::string &toggledSelectedTexture, const std::string &toggledHoverTexture, const std::string &toggledDisabledTexture)
  : AbstractButton(pRenderer, GUIFont, label)
{
	m_pDefaultIcon = new Icon(pRenderer, defaultTexture);
	m_pSelectedIcon = new Icon(pRenderer, selectedTexture);
	m_pHoverIcon = new Icon(pRenderer, hoverTexture);
	m_pDisabledIcon = new Icon(pRenderer, disabledTexture);
	m_pToggledIcon = new Icon(pRenderer, toggledTexture);
	m_pToggledSelectedIcon = new Icon(pRenderer, toggledSelectedTexture);
	m_pToggledHoverIcon = new Icon(pRenderer, toggledHoverTexture);
	m_pToggledDisabledIcon = new Icon(pRenderer, toggledDisabledTexture);

	m_label.SetLocation(0, 0);
	Add(&m_label);

	m_bToggled = false;

	Add(m_pDefaultIcon);
	Add(m_pSelectedIcon);
	Add(m_pHoverIcon);
	Add(m_pDisabledIcon);
	Add(m_pToggledIcon);
	Add(m_pToggledSelectedIcon);
	Add(m_pToggledHoverIcon);
	Add(m_pToggledDisabledIcon);

	m_bChangeLabelText = false;
}

OptionBox::~OptionBox()
{
	delete m_pDefaultIcon;
	delete m_pSelectedIcon;
	delete m_pHoverIcon;
	delete m_pDisabledIcon;
	delete m_pToggledIcon;
	delete m_pToggledSelectedIcon;
	delete m_pToggledHoverIcon;
	delete m_pToggledDisabledIcon;
}

void OptionBox::AddEventListeners()
{
	AddMouseListener(this);
}

void OptionBox::RemoveEventListeners()
{
	RemoveMouseListener(this);

	// Remove hover and select properties, since we have made this component un-selectable by the mouse
	SetHover(false);
	SetSelected(false);
}

void OptionBox::RemoveIcon(RenderRectangle *pRemoveIcon)
{
	if (pRemoveIcon)
	{
		Remove(pRemoveIcon);
		delete pRemoveIcon;
		pRemoveIcon = nullptr;
	}
}

void OptionBox::SetDefaultIcons(Renderer* pRenderer)
{
	RemoveIcon(m_pDefaultIcon);
	RemoveIcon(m_pHoverIcon);
	RemoveIcon(m_pSelectedIcon);
	RemoveIcon(m_pDisabledIcon);
	RemoveIcon(m_pToggledIcon);
	RemoveIcon(m_pToggledSelectedIcon);
	RemoveIcon(m_pToggledHoverIcon);
	RemoveIcon(m_pToggledDisabledIcon);

	m_pDefaultIcon = new DirectDrawRectangle(pRenderer);
	m_pHoverIcon = new DirectDrawRectangle(pRenderer);
	m_pSelectedIcon = new DirectDrawRectangle(pRenderer);
	m_pDisabledIcon = new DirectDrawRectangle(pRenderer);
	m_pToggledIcon = new DirectDrawRectangle(pRenderer);
	m_pToggledSelectedIcon = new DirectDrawRectangle(pRenderer);
	m_pToggledHoverIcon = new DirectDrawRectangle(pRenderer);
	m_pToggledDisabledIcon = new DirectDrawRectangle(pRenderer);

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

	lpDirectDrawRect = (DirectDrawRectangle *)m_pToggledIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(1.0f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(1.0f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(1.0f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(1.0f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pToggledSelectedIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pToggledHoverIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pToggledDisabledIcon;
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
	Add(m_pToggledIcon);
	Add(m_pToggledSelectedIcon);
	Add(m_pToggledHoverIcon);
	Add(m_pToggledDisabledIcon);

	// Set the dimensions of the icons
	Dimensions d = GetDimensions();
	m_pDefaultIcon->SetDimensions(0, 0, d.m_height, d.m_height);
	m_pSelectedIcon->SetDimensions(0, 0, d.m_height, d.m_height);
	m_pHoverIcon->SetDimensions(0, 0, d.m_height, d.m_height);
	m_pDisabledIcon->SetDimensions(0, 0, d.m_height, d.m_height);
	m_pToggledIcon->SetDimensions(0, 0, d.m_height, d.m_height);
	m_pToggledSelectedIcon->SetDimensions(0, 0, d.m_height, d.m_height);
	m_pToggledHoverIcon->SetDimensions(0, 0, d.m_height, d.m_height);
	m_pToggledDisabledIcon->SetDimensions(0, 0, d.m_height, d.m_height);
}

void OptionBox::SetToggledIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pToggledIcon)
	{
		// If we already own a toggled icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pToggledIcon);

		delete m_pToggledIcon;
		m_pToggledIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pToggledIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pToggledIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pToggledIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pToggledIcon);
}

void OptionBox::SetToggledSelectedIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pToggledSelectedIcon)
	{
		// If we already own a toggled selected icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pToggledSelectedIcon);

		delete m_pToggledSelectedIcon;
		m_pToggledSelectedIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pToggledSelectedIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pToggledSelectedIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pToggledSelectedIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pToggledSelectedIcon);
}

void OptionBox::SetToggledHoverIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pToggledHoverIcon)
	{
		// If we already own a toggled hover icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pToggledHoverIcon);

		delete m_pToggledHoverIcon;
		m_pToggledHoverIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pToggledHoverIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pToggledHoverIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pToggledHoverIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pToggledHoverIcon);
}

void OptionBox::SetToggledDisabledIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pToggledDisabledIcon)
	{
		// If we already own a toggled disabled icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pToggledDisabledIcon);

		delete m_pToggledDisabledIcon;
		m_pToggledDisabledIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pToggledDisabledIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pToggledDisabledIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pToggledDisabledIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pToggledDisabledIcon);
}

void OptionBox::SetDimensions(int x, int y, int width, int height)
{
	int lIconLabelSpace = 4;

	// Also need to include the optionbox label as part of the mouse area (and the space between icon and text)
	int labelWidth = 0;
	if(m_bDisplayLabel)
	{
		labelWidth = lIconLabelSpace + m_pRenderer->GetFreeTypeTextWidth(m_label.GetFontID(), "%s", m_label.GetText().c_str());
	}

	Component::SetDimensions(x, y, width + labelWidth, height);

	// Also set the dimensions of the icons
	m_pDefaultIcon->SetDimensions(0, 0, width, height);
	m_pSelectedIcon->SetDimensions(0, 0, width, height);
	m_pHoverIcon->SetDimensions(0, 0, width, height);
	m_pDisabledIcon->SetDimensions(0, 0, width, height);
	m_pToggledIcon->SetDimensions(0, 0, width, height);
	m_pToggledSelectedIcon->SetDimensions(0, 0, width, height);
	m_pToggledHoverIcon->SetDimensions(0, 0, width, height);
	m_pToggledDisabledIcon->SetDimensions(0, 0, width, height);

	// Set the position on the label to be next to the icons horizontally but centered vertically
	int buttonLabelHeight = m_pRenderer->GetFreeTypeTextHeight(m_label.GetFontID(), "%s", m_label.GetText().c_str());
	int m_center_height = (int)(((float)height / 2.0f) - ((float)buttonLabelHeight / 2.0f));
	m_label.SetLocation(width + lIconLabelSpace, m_center_height);

	m_offsetApplied = false;
}

void OptionBox::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void OptionBox::SetLabelOffset(int x, int y)
{
	m_labelOffsetX = x;
	m_labelOffsetY = y;

	m_label.SetLocation(m_label.GetLocation().m_x + m_labelOffsetX, m_label.GetLocation().m_y + m_labelOffsetY);
}

void OptionBox::SetLabelPosition(int x, int y)
{
	m_label.SetLocation(x, y);
}

void OptionBox::SetPressedOffset(int x, int y)
{
	m_pressedOffsetX = x;
	m_pressedOffsetY = y;
}

void OptionBox::SetLabelColour(Colour color)
{
	m_label.SetColour(color);
}

void OptionBox::SetNormalLabelColour(Colour normal)
{
	m_normalLabelColour = normal;

	m_bChangeLabelText = true;
}

void OptionBox::SetHoverLabelColour(Colour hover)
{
	m_hoverLabelColour = hover;

	m_bChangeLabelText = true;
}

void OptionBox::SetPressedLabelColour(Colour pressed)
{
	m_PressedLabelColour = pressed;

	m_bChangeLabelText = true;
}

bool OptionBox::GetToggled() const
{
	return m_bToggled;
}

void OptionBox::SetToggled(bool toggle)
{
	if(toggle)
	{
		OptionController* l_pParentController = (OptionController*)GetParent();

		l_pParentController->ResetAllOptions();
		l_pParentController->SetActiveOption(this);
	}

	m_bToggled = toggle;
}

EComponentType OptionBox::GetComponentType() const
{
	return EComponentType_OptionBox;
}

void OptionBox::MouseEntered(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetHover(true);

	OnMouseEnter();

	if(m_bChangeLabelText)
	{
		m_label.SetColour(m_hoverLabelColour);
	}
}

void OptionBox::MouseExited(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetHover(false);

	// If we are selected when we exit, reposition back the offset
	if(IsSelected() && m_offsetApplied)
	{
		m_label.SetLocation(m_label.GetLocation().m_x - m_pressedOffsetX, m_label.GetLocation().m_y - m_pressedOffsetY);

		m_offsetApplied = false;
	}

	// Also removed the button selection if we exit it's dimensions
	SetSelected(false);

	OnMouseExit();

	if(m_bChangeLabelText)
	{
		m_label.SetColour(m_normalLabelColour);
	}
}

void OptionBox::MousePressed(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	// If our parent is a GUIWindow, then makew this window have focus in the GUI, used to make it's depth the highest
	if(GetParent() != nullptr && GetParent()->GetComponentType() == EComponentType_GUIWindow)
	{
		GUIWindow* lpParentWindow = (GUIWindow *)GetParent();
		lpParentWindow->SetFocusWindow();
	}

	SetSelected(true);

	OnMousePressed();

	if(m_offsetApplied == false)
	{
		m_label.SetLocation(m_label.GetLocation().m_x + m_pressedOffsetX, m_label.GetLocation().m_y + m_pressedOffsetY);
		m_offsetApplied = true;
	}

	if(m_bChangeLabelText)
	{
		m_label.SetColour(m_PressedLabelColour);
	}
}

void OptionBox::MouseReleased(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetSelected(false);

	OnMouseReleased();

	if(m_offsetApplied)
	{
		m_label.SetLocation(m_label.GetLocation().m_x - m_pressedOffsetX, m_label.GetLocation().m_y - m_pressedOffsetY);

		m_offsetApplied = false;
	}

	if(m_bChangeLabelText)
	{
		if(IsHover())
		{
			m_label.SetColour(m_hoverLabelColour);
		}
		else
		{
			m_label.SetColour(m_normalLabelColour);
		}
	}
}

void OptionBox::MouseClicked(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetToggled(true);

	m_bToggled = true;

	OnMouseClicked();

	// Call the callback function callback if this class is just a simple button
	if(m_Callback)
	{
		m_Callback(m_pCallbackData);
	}
}

void OptionBox::OnMouseEnter()
{
	/* Nothing */
}

void OptionBox::OnMouseExit()
{
	/* Nothing */
}

void OptionBox::OnMousePressed()
{
	/* Nothing */
}

void OptionBox::OnMouseReleased()
{
	/* Nothing */
}

void OptionBox::OnMouseClicked()
{
	/* Nothing */
}

void OptionBox::DrawSelf()
{
	//Container::DrawSelf();

	// Reset visibility
	m_pSelectedIcon->SetVisible(false);
	m_pHoverIcon->SetVisible(false);
	m_pDisabledIcon->SetVisible(false);
	m_pDefaultIcon->SetVisible(false);
	m_pToggledIcon->SetVisible(false);
	m_pToggledSelectedIcon->SetVisible(false);
	m_pToggledHoverIcon->SetVisible(false);
	m_pToggledDisabledIcon->SetVisible(false);

	if(IsSelected())
	{
		if(m_bToggled)
		{
			m_pToggledSelectedIcon->SetVisible(true);
		}
		else
		{
			m_pSelectedIcon->SetVisible(true);
		}
	}
	else if(IsHover())
	{
		if(m_bToggled)
		{
			m_pToggledHoverIcon->SetVisible(true);
		}
		else
		{
			m_pHoverIcon->SetVisible(true);
		}
	}
	else if(IsDisabled())
	{
		if(m_bToggled)
		{
			m_pToggledDisabledIcon->SetVisible(true);
		}
		else
		{
			m_pDisabledIcon->SetVisible(true);
		}
	}
	else
	{
		if(m_bToggled)
		{
			m_pToggledIcon->SetVisible(true);
		}
		else
		{
			m_pDefaultIcon->SetVisible(true);
		}		
	}

	// Set the label visibility
	m_label.SetVisible(m_bDisplayLabel);
}