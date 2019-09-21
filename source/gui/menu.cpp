// ******************************************************************************
//
// Filename:	menu.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A menu is a container of menu items.
//
// Revision History:
//   Initial Revision - 05/05/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "menu.h"
#include "menubar.h"
#include "directdrawrectangle.h"
#include "guiwindow.h"
#include "menuitem.h"
#include "icon.h"

Menu::Menu(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, bool lShowTopLevelMenu)
  : AbstractButton(pRenderer, GUIFont, label),
    mbShowTopLevelMenu(lShowTopLevelMenu)
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

	if(mbShowTopLevelMenu)
	{
		Container::Add(m_pDefaultIcon);
		Container::Add(m_pSelectedIcon);
		Container::Add(m_pHoverIcon);
		Container::Add(m_pDisabledIcon);

		m_label.SetLocation(0, 0);
		Container::Add(&m_label);
	}

	mbMenuOpen = false;

	mbAutosizeMenuBase = true;

	mBiggestWidth = 0;
	mCurrentMenuheight = 0;
	mMenuItemSpacer = 0;

	m_pPullDownMenuParent = nullptr;
}


Menu::Menu(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, bool lShowTopLevelMenu, const std::string &defaultTexture, const std::string &selectedTexture, const std::string &hoverTexture, const std::string &disabledTexture)
  : AbstractButton(pRenderer, GUIFont, label),
    mbShowTopLevelMenu(lShowTopLevelMenu)
{
	m_pDefaultIcon = new Icon(pRenderer, defaultTexture);
	m_pSelectedIcon = new Icon(pRenderer, selectedTexture);
	m_pHoverIcon = new Icon(pRenderer, hoverTexture);
	m_pDisabledIcon = new Icon(pRenderer, disabledTexture);

	if(mbShowTopLevelMenu)
	{
		Container::Add(m_pDefaultIcon);
		Container::Add(m_pSelectedIcon);
		Container::Add(m_pHoverIcon);
		Container::Add(m_pDisabledIcon);

		m_label.SetLocation(0, 0);
		Container::Add(&m_label);
	}

	mbMenuOpen = false;

	mbAutosizeMenuBase = true;

	mBiggestWidth = 0;
	mCurrentMenuheight = 0;
	mMenuItemSpacer = 0;

	m_pPullDownMenuParent = nullptr;
}

Menu::~Menu()
{
	delete m_pDefaultIcon;
	delete m_pSelectedIcon;
	delete m_pHoverIcon;
	delete m_pDisabledIcon;
}

void Menu::ResetMenu()
{
	mbMenuOpen = false;

	mBiggestWidth = GetDimensions().m_width;
	mCurrentMenuheight = 0;
	mMenuItemSpacer = 2;
}

void Menu::AddEventListeners()
{
	if(mbShowTopLevelMenu)
	{
		AddMouseListener(this);
		AddFocusListener(this);
	}

	// Also add all children's event listeners
	Container::AddEventListeners();
}

void Menu::RemoveEventListeners()
{
	if(mbShowTopLevelMenu)
	{
		RemoveMouseListener(this);
		RemoveFocusListener(this);
	}

	// Also remove all children's event listeners
	Container::RemoveEventListeners();

	// Remove hover and select properties, since we have made this component un-selectable by the mouse
	SetHover(false);
	SetSelected(false);
	
	SetFocus(false);
}

void Menu::SetPullDownMenuParent(PulldownMenu* pPullDownMenuParent)
{
	m_pPullDownMenuParent = pPullDownMenuParent;
}

PulldownMenu* Menu::GetPullDownMenuParent()
{
	return m_pPullDownMenuParent;
}

void Menu::Add(MenuItem* lpMenuItem, bool lScrollBarPresent /*= true*/)
{
	Container::Add(lpMenuItem);

	int lNewMenuItemWidth = lpMenuItem->GetDimensions().m_width;
	int lNewMenuItemHeight = lpMenuItem->GetDimensions().m_height;

	// PULLDOWN MENU HACK : Bit of a hack, but if the scrollbar is not present on pulldown menus, then take up this space with the menu item
	if(!lScrollBarPresent)
	{
		lNewMenuItemWidth += (lNewMenuItemHeight + mMenuItemSpacer * 2);
	}

	// Check if this new menu item has a bigger width than our current biggest width
	if(lNewMenuItemWidth > mBiggestWidth)
	{
		mBiggestWidth = lNewMenuItemWidth;

		if(mbAutosizeMenuBase)
		{
			if(!lScrollBarPresent)
			{
				SetWidth(mBiggestWidth - (lNewMenuItemHeight + mMenuItemSpacer * 2));
			}
			else
			{
				SetWidth(mBiggestWidth);
			}
		}

		// Notify all our children menu items of the NEW biggest width
		ComponentList::const_iterator iterator;

		for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
		{
			if((*iterator)->GetComponentType() == EComponentType_MenuItem)
			{
				(*iterator)->SetWidth(mBiggestWidth);
			}
		}
	}

	// Set the location of the new menu item
	lpMenuItem->SetDimensions(0, -mCurrentMenuheight, mBiggestWidth-1, lNewMenuItemHeight);

	mCurrentMenuheight += lNewMenuItemHeight + mMenuItemSpacer * 2;
}

MenuItem* Menu::FindItem(const std::string &label)
{
	// Go through all our children and ensure that 
	ComponentList::const_iterator iterator;

	for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
	{
		if((*iterator)->GetComponentType() == EComponentType_MenuItem)
		{
			MenuItem* lpMenuItem = (MenuItem *)(*iterator);
			if(strcmp(lpMenuItem->GetLabel().GetText().c_str(), label.c_str()) == 0)
			{
				return lpMenuItem;
			}
		}
	}

	return nullptr;
}

void Menu::SetAutoSizeMenuWidth(bool lAutoSize)
{
	mbAutosizeMenuBase = lAutoSize;
}

void Menu::SetDimensions(int x, int y, int width, int height)
{
	if(width < mBiggestWidth)
	{
		width = mBiggestWidth;
	}
	else
	{
		mBiggestWidth = width;
	}

	//width += mMenuItemSpacer * 2;
	height += mMenuItemSpacer * 2;
	if(mbShowTopLevelMenu)
	{
		mCurrentMenuheight += height;
	}

	Component::SetDimensions(x, y, width, height);

	// Also set the dimensions of the icons
	m_pDefaultIcon->SetDimensions(0, 0, width, height);
	m_pSelectedIcon->SetDimensions(0, 0, width, height);
	m_pHoverIcon->SetDimensions(0, 0, width, height);
	m_pDisabledIcon->SetDimensions(0, 0, width, height);

	// Also set the dimensions of the label
	m_label.SetLocation(mMenuItemSpacer, mMenuItemSpacer);
}

void Menu::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void Menu::SetWidth(int width)
{
	//width += mMenuItemSpacer * 2;

	Component::SetWidth(width);

	// Also set the width of the icons
	m_pDefaultIcon->SetWidth(width);
	m_pSelectedIcon->SetWidth(width);
	m_pHoverIcon->SetWidth(width);
	m_pDisabledIcon->SetWidth(width);
}

void Menu::SetHeight(int height)
{
	height += mMenuItemSpacer * 2;
	if(mbShowTopLevelMenu)
	{
		mCurrentMenuheight += height;
	}

	Component::SetHeight(height);

	// Also set the height of the icons
	m_pDefaultIcon->SetHeight(height);
	m_pSelectedIcon->SetHeight(height);
	m_pHoverIcon->SetHeight(height);
	m_pDisabledIcon->SetHeight(height);
}

void Menu::ResetToTextWidth()
{
	int lWidth = m_pRenderer->GetFreeTypeTextWidth(m_label.GetFontID(), "%s", m_label.GetText().c_str());
	
	SetWidth(lWidth);
}

void Menu::SetMenuItemSpacer(int spacer)
{
	mMenuItemSpacer = spacer;
}

int Menu::GetMenuItemSpacer() const
{
	return mMenuItemSpacer;
}

int Menu::GetBiggestWidth() const
{
	return mBiggestWidth;
}

int Menu::GetCurrentMenuheight() const
{
	return mCurrentMenuheight;
}

const std::string Menu::GetMenuTitle() const
{
	return m_label.GetText();
}

void Menu::OpenMenu()
{
	mbMenuOpen = true;

	// Enable all our menu items when we open the menu, so that they can get focus
	ComponentList::const_iterator iterator;

	for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
	{
		if((*iterator)->GetComponentType() == EComponentType_MenuItem)
		{
			(*iterator)->SetEnabled(true);
		}
	}
}

void Menu::CloseMenu()
{
	mbMenuOpen = false;

	// Disable all our menu items when we close the menu, so that they cannot get focus
	ComponentList::const_iterator iterator;

	for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
	{
		if((*iterator)->GetComponentType() == EComponentType_MenuItem)
		{
			(*iterator)->SetEnabled(false);
		}
	}
}

bool Menu::IsMenuOpen()
{
	return mbMenuOpen;
}
bool Menu::ChildHasFocus()
{
	// Check to see if one of our children gained focus
	ComponentList::const_iterator iterator;

	for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
	{
		if((*iterator)->GetComponentType() == EComponentType_MenuItem)
		{
			if((*iterator)->HasFocus())
			{
				return true;
			}
		}
	}

	return false;
}

EComponentType Menu::GetComponentType() const
{
	return EComponentType_Menu;
}

void Menu::MouseEntered(const MouseEvent& lEvent)
{
	SetHover(true);

	OnMouseEnter();
}

void Menu::MouseExited(const MouseEvent& lEvent)
{
	SetHover(false);

	// Also removed the button selection if we exit it's dimensions
	SetSelected(false);

	OnMouseExit();
}

void Menu::MousePressed(const MouseEvent& lEvent)
{
	if(mbMenuOpen)
	{
		MenuBar* lpMenuBar = (MenuBar *)GetParent();

		if(lpMenuBar != nullptr)
		{
			lpMenuBar->CloseAllMenus();
		}
	}
	else
	{
		MenuBar* lpMenuBar = (MenuBar *)GetParent();

		if(lpMenuBar != nullptr)
		{
			lpMenuBar->CloseAllMenus();
		}		

		OpenMenu();
	}

	// If our parent is a GUIWindow, then makew this window have focus in the GUI, used to make it's depth the highest
	if(GetParent() != nullptr && GetParent()->GetComponentType() == EComponentType_GUIWindow)
	{
		GUIWindow* lpParentWindow = (GUIWindow *)GetParent();
		lpParentWindow->SetFocusWindow();
	}

	OnMousePressed();
}

void Menu::MouseReleased(const MouseEvent& lEvent)
{
	SetSelected(false);

	OnMouseReleased();
}

void Menu::MouseReleasedOutside(const MouseEvent& lEvent)
{
	CloseMenu();
}

void Menu::MouseClicked(const MouseEvent& lEvent)
{
	OnMouseClicked();
}

void Menu::FocusLost(const FocusEvent& lEvent)
{
	if(ChildHasFocus())
	{

	}
	else
	{
		CloseMenu();
	}
}

void Menu::FocusGained(const FocusEvent& lEvent)
{

}

void Menu::OnMouseEnter()
{
	/* Nothing */
}

void Menu::OnMouseExit()
{
	/* Nothing */
}

void Menu::OnMousePressed()
{
	/* Nothing */
}

void Menu::OnMouseReleased()
{
	/* Nothing */
}

void Menu::OnMouseClicked()
{
	/* Nothing */
}

void Menu::DrawSelf()
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
	m_label.SetVisible(true);
}
