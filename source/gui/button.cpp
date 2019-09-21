// ******************************************************************************
//
// Filename:	button.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A simple button class that defines standard
//   button behaviour.
//
// Revision History:
//   Initial Revision - 28/05/07
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "button.h"
#include "directdrawrectangle.h"
#include "multitextureicon.h"
#include "guiwindow.h"
#include "icon.h"

Button::Button(Renderer* pRenderer, unsigned int GUIFont, const std::string &label)
  : AbstractButton(pRenderer, GUIFont, label)
{
	SetDefaultIcons(pRenderer);

	m_label.SetLocation(0, 0);
	Add(&m_label);

	m_label.SetDepth(3.0f);

	m_pressedOffsetX = 0;
	m_pressedOffsetY = 0;
	m_offsetApplied = false;

	m_bChangeLabelText = false;
}

Button::Button(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, Colour fontColour, Colour outlineColour)
  : AbstractButton(pRenderer, GUIFont, label)
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
	Add(&m_label);

	m_label.SetDepth(3.0f);

	m_pressedOffsetX = 0;
	m_pressedOffsetY = 0;
	m_offsetApplied = false;

	m_bChangeLabelText = false;
}

Button::Button(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, const std::string &defaultTexture, const std::string &selectedTexture, const std::string &hoverTexture, const std::string &disabledTexture)
  : AbstractButton(pRenderer, GUIFont, label)
{
	m_pDefaultIcon = new Icon(pRenderer, defaultTexture);
	m_pHoverIcon = new Icon(pRenderer, hoverTexture);
	m_pSelectedIcon = new Icon(pRenderer, selectedTexture);
	m_pDisabledIcon = new Icon(pRenderer, disabledTexture);

	Add(m_pDefaultIcon);
	Add(m_pSelectedIcon);
	Add(m_pHoverIcon);
	Add(m_pDisabledIcon);

	m_label.SetLocation(0, 0);
	Add(&m_label);

	m_label.SetDepth(3.0f);

	m_pressedOffsetX = 0;
	m_pressedOffsetY = 0;
	m_offsetApplied = false;

	m_bChangeLabelText = false;
}

Button::~Button()
{
	Remove(m_pDefaultIcon);
	Remove(m_pSelectedIcon);
	Remove(m_pHoverIcon);
	Remove(m_pDisabledIcon);

	delete m_pDefaultIcon;
	delete m_pSelectedIcon;
	delete m_pHoverIcon;
	delete m_pDisabledIcon;

	for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
	{
		Remove(m_vpAddedComponentList[i]);

		delete m_vpAddedComponentList[i];
		m_vpAddedComponentList[i] = 0;
	}
}

void Button::AddText(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, Colour colour, int xOffset, int yOffset, bool outline, Colour outlineColour)
{
	Label* lpNewLabel = new Label(pRenderer, GUIFont, label, colour);
	lpNewLabel->SetLocation(xOffset, yOffset);
	lpNewLabel->SetDepth(lpNewLabel->GetDepth()+2.0f);

	if(outline)
	{
		lpNewLabel->SetOutline(true);
		lpNewLabel->SetOutlineColour(outlineColour);
		lpNewLabel->SetOutlineFont(OutlineGUIFont);
	}

	m_vpAddedComponentList.push_back(lpNewLabel);

	Add(lpNewLabel);
}

void Button::AddIcon(Renderer* pRenderer, const std::string &fileName, int texWidth, int texHeight, int width, int height, int xOffset, int yOffset, float depth)
{
	// texWidth and texHeight should always be a power of 2.

	Icon* lpNewIcon = new Icon(pRenderer, fileName, texWidth, texHeight);
	lpNewIcon->SetDimensions(xOffset, yOffset, width, height);
	lpNewIcon->SetDepth(depth);

	m_vpAddedComponentList.push_back(lpNewIcon);

	Add(lpNewIcon);
}

void Button::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);

	// Also set the dimensions of the icons
	m_pDefaultIcon->SetDimensions(0, 0, width, height);
	m_pSelectedIcon->SetDimensions(0, 0, width, height);
	m_pHoverIcon->SetDimensions(0, 0, width, height);
	m_pDisabledIcon->SetDimensions(0, 0, width, height);

	// Set the label location to be the center of the button
	int buttonLabelHeight = m_pRenderer->GetFreeTypeTextHeight(m_label.GetFontID(), "%s", m_label.GetText().c_str());
	int buttonLabelWidth = m_pRenderer->GetFreeTypeTextWidth(m_label.GetFontID(), "%s", m_label.GetText().c_str());
	int m_center_width = (int)((width * 0.5f ) - (buttonLabelWidth * 0.5f));
	int m_center_height = (int)((height * 0.5f) - (buttonLabelHeight * 0.5f));
	m_label.SetLocation(m_center_width + m_labelOffsetX, m_center_height + m_labelOffsetY);
	m_offsetApplied = false;
}

void Button::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void Button::SetLabelOffset(int x, int y)
{
	m_labelOffsetX = x;
	m_labelOffsetY = y;

	m_label.SetLocation(m_label.GetLocation().m_x + m_labelOffsetX, m_label.GetLocation().m_y + m_labelOffsetY);
}

void Button::SetPressedOffset(int x, int y)
{
	m_pressedOffsetX = x;
	m_pressedOffsetY = y;
}

void Button::SetNormalLabelColour(Colour normal)
{
	m_normalLabelColour = normal;
}

void Button::SetHoverLabelColour(Colour hover)
{
	m_hoverLabelColour = hover;

	m_bChangeLabelText = true;
}

void Button::SetPressedLabelColour(Colour pressed)
{
	m_PressedLabelColour = pressed;

	m_bChangeLabelText = true;
}

void Button::SetLabelColour(Colour colour)
{
	m_label.SetColour(colour);

	for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
	{
		if(m_vpAddedComponentList[i]->GetComponentType() == EComponentType_Label)
		{
			((Label*)m_vpAddedComponentList[i])->SetColour(colour);
		}
	}
}

void Button::SetLabelOutlineColour(Colour colour)
{
	m_label.SetOutlineColour(colour);

	for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
	{
		if(m_vpAddedComponentList[i]->GetComponentType() == EComponentType_Label)
		{
			((Label*)m_vpAddedComponentList[i])->SetOutlineColour(colour);
		}
	}
}

void Button::AddEventListeners()
{
	AddMouseListener(this);
}

void Button::RemoveEventListeners()
{
	RemoveMouseListener(this);

	// Remove hover and select properties, since we have made this component un-selectable by the mouse
	SetHover(false);
	SetSelected(false);
}

EComponentType Button::GetComponentType() const
{
	return EComponentType_Button;
}

void Button::MouseEntered(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetHover(true);

	OnMouseEnter();

	// Call the callback function
	if(m_EnterCallback)
	{
		m_EnterCallback(m_pEnterCallbackData);
	}

	if(m_bChangeLabelText)
	{
		for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
		{
			if(m_vpAddedComponentList[i]->GetComponentType() == EComponentType_Label)
			{
				((Label*)m_vpAddedComponentList[i])->SetColour(m_hoverLabelColour);
			}
		}

		m_label.SetColour(m_hoverLabelColour);
	}
}

void Button::MouseExited(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	if(IsSelected() && m_Callback_Released)
	{
		// If we have pressed the button, but not released when we move outside of the button
		m_Callback_Released(m_pCallbackData_Released);
	}

	SetHover(false);

	// Call the callback function
	if(m_ExitCallback)
	{
		m_ExitCallback(m_pExitCallbackData);
	}

	// If we are selected when we exit, reposition back the offset
	if(IsSelected() && m_offsetApplied)
	{
		for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
		{
			m_vpAddedComponentList[i]->SetLocation(m_vpAddedComponentList[i]->GetLocation().m_x - m_pressedOffsetX, m_vpAddedComponentList[i]->GetLocation().m_y - m_pressedOffsetY);
		}

		m_label.SetLocation(m_label.GetLocation().m_x - m_pressedOffsetX, m_label.GetLocation().m_y - m_pressedOffsetY);

		m_offsetApplied = false;
	}

	if(m_bChangeLabelText)
	{
		for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
		{
			if(m_vpAddedComponentList[i]->GetComponentType() == EComponentType_Label)
			{
				((Label*)m_vpAddedComponentList[i])->SetColour(m_normalLabelColour);
			}
		}
		m_label.SetColour(m_normalLabelColour);
	}

	// Also removed the button selection if we exit it's dimensions
	SetSelected(false);

	OnMouseExit();
}

void Button::MousePressed(const MouseEvent& lEvent)
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

	// Colour change
	if(m_bChangeLabelText)
	{
		for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
		{
			if(m_vpAddedComponentList[i]->GetComponentType() == EComponentType_Label)
			{
				((Label*)m_vpAddedComponentList[i])->SetColour(m_PressedLabelColour);
			}
		}

		m_label.SetColour(m_PressedLabelColour);
	}

	// Position change
	if(m_offsetApplied == false)
	{
		for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
		{
			m_vpAddedComponentList[i]->SetLocation(m_vpAddedComponentList[i]->GetLocation().m_x + m_pressedOffsetX, m_vpAddedComponentList[i]->GetLocation().m_y + m_pressedOffsetY);
		}

		m_label.SetLocation(m_label.GetLocation().m_x + m_pressedOffsetX, m_label.GetLocation().m_y + m_pressedOffsetY);

		m_offsetApplied = true;
	}
	
	if(m_Callback_Pressed)
	{
		m_Callback_Pressed(m_pCallbackData_Pressed);
	}
}

void Button::MouseReleased(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetSelected(false);

	OnMouseReleased();

	if(m_offsetApplied)
	{
		for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
		{
			if(m_bChangeLabelText)
			{
				if(m_vpAddedComponentList[i]->GetComponentType() == EComponentType_Label)
				{
					if(IsHover())
					{
						((Label*)m_vpAddedComponentList[i])->SetColour(m_hoverLabelColour);
					}
					else
					{
						((Label*)m_vpAddedComponentList[i])->SetColour(m_normalLabelColour);
					}
				
				}
			}

			m_vpAddedComponentList[i]->SetLocation(m_vpAddedComponentList[i]->GetLocation().m_x - m_pressedOffsetX, m_vpAddedComponentList[i]->GetLocation().m_y - m_pressedOffsetY);
		}
		m_label.SetLocation(m_label.GetLocation().m_x - m_pressedOffsetX, m_label.GetLocation().m_y - m_pressedOffsetY);
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

		m_offsetApplied = false;
	}


	if(m_Callback_Released)
	{
		m_Callback_Released(m_pCallbackData_Released);
	}
}

void Button::MouseClicked(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	// Call the function if this class has a derived sub-class
	OnMouseClicked();

	if(m_bChangeLabelText)
	{
		if(IsHover())
		{
			for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
			{
				if(m_vpAddedComponentList[i]->GetComponentType() == EComponentType_Label)
				{
					((Label*)m_vpAddedComponentList[i])->SetColour(m_hoverLabelColour);
				}
			}
			m_label.SetColour(m_hoverLabelColour);
		}
		else
		{
			for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
			{
				if(m_vpAddedComponentList[i]->GetComponentType() == EComponentType_Label)
				{
					((Label*)m_vpAddedComponentList[i])->SetColour(m_normalLabelColour);
				}
			}
			m_label.SetColour(m_normalLabelColour);
		}
	}

	// Call the callback function callback if this class is just a simple button
	if(m_Callback)
	{
		m_Callback(m_pCallbackData);
	}
}

void Button::OnMouseEnter()
{
	/* Nothing */
}

void Button::OnMouseExit()
{
	/* Nothing */
}

void Button::OnMousePressed()
{
	/* Nothing */
}

void Button::OnMouseReleased()
{
	/* Nothing */
}

void Button::OnMouseClicked()
{
	/* Nothing */
}

void Button::DrawSelf()
{
	// Reset visibility
	m_pSelectedIcon->SetVisible(false);
	m_pHoverIcon->SetVisible(false);
	m_pDisabledIcon->SetVisible(false);
	m_pDefaultIcon->SetVisible(false);

	if(IsSelected())
	{
		m_pSelectedIcon->SetVisible(true);
	}
	else if(IsHover())
	{
		m_pHoverIcon->SetVisible(true);
	}
	else if(IsDisabled())
	{
		m_pDisabledIcon->SetVisible(true);
	}
	else
	{
		m_pDefaultIcon->SetVisible(true);
	}

	// Set the label visibility
	m_label.SetVisible(m_bDisplayLabel);
}