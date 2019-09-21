// ******************************************************************************
//
// Filename:	menuitem.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A menu item component that represents a clickable item in a menu.
//
// Revision History:
//   Initial Revision - 19/04/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "menuitem.h"
#include "menu.h"
#include "focusmanager.h"
#include "directdrawrectangle.h"
#include "pulldownmenu.h"
#include "guiwindow.h"


MenuItem::MenuItem(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, Colour lLabelColour)
  : AbstractButton(pRenderer, GUIFont, label)
{
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

	m_label.SetLocation(0, 0);
	m_label.SetColour(lLabelColour);
	Add(&m_label);

	m_pDefaultIcon->SetDepth(3.0f);
	m_pSelectedIcon->SetDepth(3.0f);
	m_pHoverIcon->SetDepth(3.0f);
	m_pDisabledIcon->SetDepth(3.0f);

	// Start out disabled, since most menus are not open to start with...
	SetEnabled(false);

	m_bOnlyShowSelectedItems = false;
}

MenuItem::MenuItem(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, Colour lLabelColour, Colour outlineColour)
  : AbstractButton(pRenderer, GUIFont, label)
{
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

	m_label.SetLocation(0, 0);
	m_label.SetColour(lLabelColour);
	m_label.SetOutline(true);
	m_label.SetOutlineColour(outlineColour);
	m_label.SetOutlineFont(OutlineGUIFont);
	Add(&m_label);

	m_pDefaultIcon->SetDepth(3.0f);
	m_pSelectedIcon->SetDepth(3.0f);
	m_pHoverIcon->SetDepth(3.0f);
	m_pDisabledIcon->SetDepth(3.0f);

	// Start out disabled, since most menus are not open to start with...
	SetEnabled(false);

	m_bOnlyShowSelectedItems = false;
}

MenuItem::MenuItem(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, Colour lLabelColour, const std::string &defaultTexture, const std::string &selectedTexture, const std::string &hoverTexture, const std::string &disabledTexture)
  : AbstractButton(pRenderer, GUIFont, label)
{
	m_pDefaultIcon = new Icon(pRenderer, defaultTexture);
	m_pSelectedIcon = new Icon(pRenderer, selectedTexture);
	m_pHoverIcon = new Icon(pRenderer, hoverTexture);
	m_pDisabledIcon = new Icon(pRenderer, disabledTexture);

	Add(m_pDefaultIcon);
	Add(m_pSelectedIcon);
	Add(m_pHoverIcon);
	Add(m_pDisabledIcon);

	m_label.SetLocation(0, 0);
	m_label.SetColour(lLabelColour);
	Add(&m_label);

	m_pDefaultIcon->SetDepth(3.0f);
	m_pSelectedIcon->SetDepth(3.0f);
	m_pHoverIcon->SetDepth(3.0f);
	m_pDisabledIcon->SetDepth(3.0f);

	// Start out disabled, since most menus are not open to start with...
	SetEnabled(false);

	m_bOnlyShowSelectedItems = false;
}

MenuItem::~MenuItem()
{
	delete m_pDefaultIcon;
	delete m_pSelectedIcon;
	delete m_pHoverIcon;
	delete m_pDisabledIcon;
}

void MenuItem::AddEventListeners()
{
	AddMouseListener(this);
	AddFocusListener(this);
}

void MenuItem::RemoveEventListeners()
{
	RemoveMouseListener(this);
	RemoveFocusListener(this);

	// Remove hover and select properties, since we have made this component un-selectable by the mouse
	SetHover(false);
	SetSelected(false);

	SetFocus(false);
}

void MenuItem::SetDimensions(int x, int y, int width, int height)
{
	int lMenuItemSpacer = 0;
	
	Menu* lpMenuParent = (Menu *)GetParent();
	if(lpMenuParent)
	{
		lMenuItemSpacer = lpMenuParent->GetMenuItemSpacer();
	}

	//width += lMenuItemSpacer * 2;
	height += lMenuItemSpacer * 2;

	Component::SetDimensions(x, y, width, height);

	// Also set the dimensions of the icons
	m_pDefaultIcon->SetDimensions(0, 0, width, height);
	m_pSelectedIcon->SetDimensions(0, 0, width, height);
	m_pHoverIcon->SetDimensions(0, 0, width, height);
	m_pDisabledIcon->SetDimensions(0, 0, width, height);

	// Also set the dimensions of the label
	m_label.SetLocation(3, 3 /*lMenuItemSpacer, lMenuItemSpacer*/);
}

void MenuItem::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void MenuItem::SetWidth(int width)
{
	//Menu* lpMenuParent = (Menu *)GetParent();
	//if(lpMenuParent)
	//{
	//	width += lpMenuParent->GetMenuItemSpacer() * 2;
	//}

	Component::SetWidth(width);

	// Also set the width of the icons
	m_pDefaultIcon->SetWidth(width);
	m_pSelectedIcon->SetWidth(width);
	m_pHoverIcon->SetWidth(width);
	m_pDisabledIcon->SetWidth(width);
}

void MenuItem::SetHeight(int height)
{
	Menu* lpMenuParent = (Menu *)GetParent();
	if(lpMenuParent)
	{
		height += lpMenuParent->GetMenuItemSpacer() * 2;
	}

	Component::SetHeight(height);

	// Also set the height of the icons
	m_pDefaultIcon->SetHeight(height);
	m_pSelectedIcon->SetHeight(height);
	m_pHoverIcon->SetHeight(height);
	m_pDisabledIcon->SetHeight(height);
}

void MenuItem::SetOnlyShowSelectedIcons(bool lShow)
{
	m_bOnlyShowSelectedItems = lShow;

	if(m_bOnlyShowSelectedItems)
	{
		m_pSelectedIcon->SetVisible(false);
		m_pHoverIcon->SetVisible(false);
		m_pDisabledIcon->SetVisible(false);
		m_pDefaultIcon->SetVisible(false);
	}
}

bool MenuItem::IsOnlyShowSelectedIcons() const
{
	return m_bOnlyShowSelectedItems;
}

EComponentType MenuItem::GetComponentType() const
{
	return EComponentType_MenuItem;
}

bool MenuItem::IsParentMenuOpen()
{
	Menu* lpParentMenu = (Menu* )GetParent();

	// Check to see if our parent menu is currently open
	if(lpParentMenu != nullptr)
	{
		if(lpParentMenu->IsMenuOpen())
		{
			return true;
		}
	}

	return false;
}

void MenuItem::MenuItemSelectedFromPulldowmMenu()
{
	Menu* lpParentMenu = (Menu* )GetParent();

	if(lpParentMenu && lpParentMenu->GetComponentType() == EComponentType_Menu)
	{
		PulldownMenu* lpParentPulldownMenu = (PulldownMenu*)lpParentMenu->GetParent();

		if(lpParentPulldownMenu && lpParentPulldownMenu->GetComponentType() == EComponentType_PulldownMenu)
		{
			// Store this as the selected menu item
			lpParentPulldownMenu->SetSelectedMenuItem(this);

			lpParentPulldownMenu->SetTitleString(GetLabel().GetText());

			// Pulldown menu item changed callback
			lpParentPulldownMenu->MenuItemChanged();
		}
	}
}

void MenuItem::MouseEntered(const MouseEvent& lEvent)
{
	if(!IsParentMenuOpen())
	{
		return;
	}

	Menu* lpParentMenu = (Menu* )GetParent();

	// Make sure that we are inside the bounds of the parent menu
	if(lpParentMenu->GetPullDownMenuParent() != nullptr)
	{
		int lTextHeight = m_pRenderer->GetFreeTypeTextHeight(lpParentMenu->GetPullDownMenuParent()->GetGUIFont(), "%s", lpParentMenu->GetMenuTitle().c_str());
		int lMenuHeight = lTextHeight + (lpParentMenu->GetMenuItemSpacer() * 2);
		int lFullMenuDisplayHeight = lpParentMenu->GetPullDownMenuParent()->GetMaxNumItemsDisplayed() * lMenuHeight;

		Point location = lpParentMenu->GetPullDownMenuParent()->GetLocation();
		for(Component* parent = lpParentMenu->GetPullDownMenuParent()->GetParent(); parent != 0;)
		{
			Point parentLocation = parent->GetLocation();

			location.m_x += parentLocation.m_x;
			location.m_y += parentLocation.m_y;

			parent = parent->GetParent();
		}
		int lMenuX = location.m_x;
		int lMenuY = location.m_y - lFullMenuDisplayHeight;
		int lMenuWidth = lpParentMenu->GetBiggestWidth()+ (lpParentMenu->GetMenuItemSpacer() * 2);

		if(lEvent.GetX() > lMenuX && lEvent.GetX() <= lMenuX+lMenuWidth && lEvent.GetY() > lMenuY && lEvent.GetY() <= lMenuY+lFullMenuDisplayHeight)
		{
			SetHover(true);

			OnMouseEnter();
		}
	}
}

void MenuItem::MouseExited(const MouseEvent& lEvent)
{
	if(!IsParentMenuOpen())
	{
		return;
	}

	SetHover(false);

	// Also removed the button selection if we exit it's dimensions
	SetSelected(false);

	OnMouseExit();
}

void MenuItem::MousePressed(const MouseEvent& lEvent)
{
	if(!IsParentMenuOpen())
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
}

void MenuItem::MouseReleased(const MouseEvent& lEvent)
{
	if(!IsParentMenuOpen())
	{
		return;
	}

	Menu* lpParentMenu = (Menu* )GetParent();
	
	// Make sure that we are inside the bounds of the parent menu
	if(lpParentMenu->GetPullDownMenuParent() != nullptr)
	{
		int lTextHeight = m_pRenderer->GetFreeTypeTextHeight(lpParentMenu->GetPullDownMenuParent()->GetGUIFont(), "%s", lpParentMenu->GetMenuTitle().c_str());
		int lMenuHeight = lTextHeight + (lpParentMenu->GetMenuItemSpacer() * 2);
		int lFullMenuDisplayHeight = lpParentMenu->GetPullDownMenuParent()->GetMaxNumItemsDisplayed() * lMenuHeight;

		Point location = lpParentMenu->GetPullDownMenuParent()->GetLocation();
		for(Component* parent = lpParentMenu->GetPullDownMenuParent()->GetParent(); parent != 0;)
		{
			Point parentLocation = parent->GetLocation();

			location.m_x += parentLocation.m_x;
			location.m_y += parentLocation.m_y;

			parent = parent->GetParent();
		}
		int lMenuX = location.m_x;
		int lMenuY = location.m_y - lFullMenuDisplayHeight;
		int lMenuWidth = lpParentMenu->GetBiggestWidth()+ (lpParentMenu->GetMenuItemSpacer() * 2);

		if(lEvent.GetX() > lMenuX && lEvent.GetX() <= lMenuX+lMenuWidth && lEvent.GetY() > lMenuY && lEvent.GetY() <= lMenuY+lFullMenuDisplayHeight)
		{
			// Close the menu, since we have clicked this menu item
			lpParentMenu->CloseMenu();

			SetHover(false);
			SetSelected(false);

			// Signal that we have pressed this menu item
			MenuItemPressed();
		}
	}


	FocusManager::GetInstance()->SetFocusOwner(0);

	OnMouseReleased();
}

void MenuItem::MouseReleasedOutside(const MouseEvent& lEvent)
{
	FocusManager::GetInstance()->SetFocusOwner(0);

	Menu* lpParentMenu = (Menu* )GetParent();
	lpParentMenu->CloseMenu();
}

void MenuItem::MouseClicked(const MouseEvent& lEvent)
{
	if(!IsParentMenuOpen())
	{
		return;
	}

	SetHover(false);
	SetSelected(false);

	// Close the menu, since we have clicked this menu item
	Menu* lpParentMenu = (Menu* )GetParent();
	lpParentMenu->CloseMenu();

	// Make sure that we are inside the bounds of the parent menu
	if(lpParentMenu->GetPullDownMenuParent() != nullptr)
	{
		int lTextHeight = m_pRenderer->GetFreeTypeTextHeight(lpParentMenu->GetPullDownMenuParent()->GetGUIFont(), "%s", lpParentMenu->GetMenuTitle().c_str());
		int lMenuHeight = lTextHeight + (lpParentMenu->GetMenuItemSpacer() * 2);
		int lFullMenuDisplayHeight = lpParentMenu->GetPullDownMenuParent()->GetMaxNumItemsDisplayed() * lMenuHeight;

		Point location = lpParentMenu->GetPullDownMenuParent()->GetLocation();
		for(Component* parent = lpParentMenu->GetPullDownMenuParent()->GetParent(); parent != 0;)
		{
			Point parentLocation = parent->GetLocation();

			location.m_x += parentLocation.m_x;
			location.m_y += parentLocation.m_y;

			parent = parent->GetParent();
		}
		int lMenuX = location.m_x;
		int lMenuY = location.m_y - lFullMenuDisplayHeight;
		int lMenuWidth = lpParentMenu->GetBiggestWidth()+ (lpParentMenu->GetMenuItemSpacer() * 2);

		if(lEvent.GetX() > lMenuX && lEvent.GetX() <= lMenuX+lMenuWidth && lEvent.GetY() > lMenuY && lEvent.GetY() <= lMenuY+lFullMenuDisplayHeight)
		{
			// Signal that we have pressed this menu item
			MenuItemPressed();
		}
	}

	FocusManager::GetInstance()->SetFocusOwner(0);

	OnMouseClicked();
}

void MenuItem::MenuItemPressed()
{
	// Call the callback function callback if this class is just a simple button
	if(m_Callback)
	{
		m_Callback(m_pCallbackData);
	}
}

void MenuItem::FocusLost(const FocusEvent& lEvent)
{
	if(!IsParentMenuOpen())
	{
		return;
	}

	Menu* lpParentMenu = (Menu* )GetParent();
	lpParentMenu->CloseMenu();
}

void MenuItem::FocusGained(const FocusEvent& lEvent)
{
	if(!IsParentMenuOpen())
	{
		FocusManager::GetInstance()->SetFocusOwner(0);

		return;
	}
}

void MenuItem::OnMouseEnter()
{
	/* Nothing */
}

void MenuItem::OnMouseExit()
{
	/* Nothing */
}

void MenuItem::OnMousePressed()
{
	/* Nothing */
}

void MenuItem::OnMouseReleased()
{
	/* Nothing */
}

void MenuItem::OnMouseClicked()
{
	/* Nothing */
}

void MenuItem::DrawSelf()
{
	// Reset visibility
	m_pSelectedIcon->SetVisible(false);
	m_pHoverIcon->SetVisible(false);
	m_pDisabledIcon->SetVisible(false);
	m_pDefaultIcon->SetVisible(false);

	if(!IsParentMenuOpen())
	{
		m_label.SetVisible(false);

		return;
	}

	if(m_bOnlyShowSelectedItems)
	{
		if(IsSelected())
		{
			m_pSelectedIcon->SetVisible(true);
		}
		else if(IsHover())
		{
			m_pHoverIcon->SetVisible(true);
		}
	}
	else
	{
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
	}

	// Set the label visibility
	m_label.SetVisible(true);
}