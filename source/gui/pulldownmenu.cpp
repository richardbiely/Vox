// ******************************************************************************
//
// Filename:	pulldown.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A pulldown menu GUI component.
//
// Revision History:
//   Initial Revision - 18/05/09
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "pulldownmenu.h"
#include "directdrawrectangle.h"
#include "guiwindow.h"


PulldownMenu::PulldownMenu(Renderer* pRenderer, unsigned int GUIFont, const std::string &MenuTitle)
  : Container(pRenderer),
    mTextColour(0.0f, 0.0f, 0.0f),
    mMenuTitle(pRenderer, GUIFont, MenuTitle, Colour(0.0f, 0.0f, 0.0f)),
		mMenuName(pRenderer, GUIFont, MenuTitle, Colour(0.0f, 0.0f, 0.0f)),
    mPulldownMenu(pRenderer, GUIFont, MenuTitle, false),
		mScrollBar(pRenderer),
		m_bSelected(false),
		m_bHover(false),
		m_bDisabled(false),
		mGUIFont(GUIFont),
    m_pPulldownIcon(0),
		m_pPulldownSelectedIcon(0),
		m_pPulldownHoverIcon(0),
		m_pPulldownDisabledIcon(0),
		m_pMenuBackgroundDefault(0),
		m_pMenuBackgroundHover(0),
		m_pMenuBackgroundSelected(0),
		m_pMenuBackgroundDisabled(0),
		m_pTitleBackgroundDefault(0),
		m_pTitleBackgroundHover(0),
		m_pTitleBackgroundSelected(0),
    m_pTitleBackgroundDisabled(0)
{
	SetDefaultIcons(pRenderer);

	mScrollBar.SetScrollDirection(EScrollBarDirection_Vertical);
	mScrollBar.SetScrollPosition(0.75f);

	mPulldownMenu.SetPullDownMenuParent(this);

	Add(&mPulldownMenu);

	Add(&mScrollBar);

	mMenuTitle.SetLocation(0, 0);
	Add(&mMenuTitle);

	m_renderMenuHeader = false;
	mMenuName.SetLocation(0, 0);
	Add(&mMenuName);

	mPulldownMenu.SetMenuItemSpacer(2);

	//mPulldownMenu.SetAutoSizeMenuWidth(false);

	mGUIFont = GUIFont;

	m_outline = false;

	mNumItems = 0;
	mSelectedIndex = -1;
	mMaxDisplayItems = 0;

	m_bDisplayScrollbar = true;

	mpSelectedMenuItem = nullptr;

	m_MenuItemChangedCallback = nullptr;
	m_pMenuItemChangedCallbackData = nullptr;
}

PulldownMenu::PulldownMenu(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &MenuTitle, Colour fontColour, Colour outlineColour)
  : Container(pRenderer),
	mTextColour(0.0f, 0.0f, 0.0f),
	mMenuTitle(pRenderer, GUIFont, MenuTitle, Colour(0.0f, 0.0f, 0.0f)),
	mMenuName(pRenderer, GUIFont, MenuTitle, Colour(0.0f, 0.0f, 0.0f)),
	mPulldownMenu(pRenderer, GUIFont, MenuTitle, false),
	mScrollBar(pRenderer),
	m_bSelected(false),
	m_bHover(false),
	m_bDisabled(false),
	mGUIFont(GUIFont),
	m_pPulldownIcon(0),
	m_pPulldownSelectedIcon(0),
	m_pPulldownHoverIcon(0),
	m_pPulldownDisabledIcon(0),
	m_pMenuBackgroundDefault(0),
	m_pMenuBackgroundHover(0),
	m_pMenuBackgroundSelected(0),
	m_pMenuBackgroundDisabled(0),
	m_pTitleBackgroundDefault(0),
	m_pTitleBackgroundHover(0),
	m_pTitleBackgroundSelected(0),
	m_pTitleBackgroundDisabled(0)
{
	SetDefaultIcons(pRenderer);

	mScrollBar.SetScrollDirection(EScrollBarDirection_Vertical);
	mScrollBar.SetScrollPosition(0.75f);

	mPulldownMenu.SetPullDownMenuParent(this);

	Add(&mPulldownMenu);

	Add(&mScrollBar);

	mMenuTitle.SetLocation(0, 0);
	mMenuTitle.SetColour(fontColour);
	mMenuTitle.SetOutline(true);
	mMenuTitle.SetOutlineColour(outlineColour);
	mMenuTitle.SetOutlineFont(OutlineGUIFont);
	Add(&mMenuTitle);

	m_renderMenuHeader = false;
	mMenuName.SetLocation(0, 0);
	mMenuName.SetColour(fontColour);
	mMenuName.SetOutline(true);
	mMenuName.SetOutlineColour(outlineColour);
	mMenuName.SetOutlineFont(OutlineGUIFont);
	Add(&mMenuName);

	//mPulldownMenu.SetAutoSizeMenuWidth(false);

	mGUIFont = GUIFont;
	mOutlineGUIFont = OutlineGUIFont;

	m_outline = true;
	m_outlineColour = outlineColour;
	mTextColour = fontColour;

	mNumItems = 0;
	mSelectedIndex = -1;
	mMaxDisplayItems = 0;

	m_bDisplayScrollbar = true;

	mpSelectedMenuItem = nullptr;

	m_MenuItemChangedCallback = nullptr;
	m_pMenuItemChangedCallbackData = nullptr;
}

PulldownMenu::PulldownMenu(Renderer* pRenderer, unsigned int GUIFont, const std::string &MenuTitle,
						   const std::string &defaultPulldownTexture, const std::string &selectedPulldownTexture, const std::string &hoverPulldownTexture, const std::string &disabledPulldownTexture,
						   const std::string &defaultTitleTexture, const std::string &selectedTitleTexture, const std::string &hoverTitleTexture, const std::string &disabledTitleTexture,
						   const std::string &defaultMenuTexture, const std::string &selectedMenuTexture, const std::string &hoverMenuTexture, const std::string &disabledMenuTexture)
  : Container(pRenderer),
    mTextColour(0.0f, 0.0f, 0.0f),
    mMenuTitle(pRenderer, GUIFont, MenuTitle, Colour(0.0f, 0.0f, 0.0f)),
	mMenuName(pRenderer, GUIFont, MenuTitle, Colour(0.0f, 0.0f, 0.0f)),
    mPulldownMenu(pRenderer, GUIFont, MenuTitle, false),
	mScrollBar(pRenderer),
	m_bSelected(false),
	m_bHover(false),
	m_bDisabled(false),
	mGUIFont(GUIFont),
	m_pPulldownIcon(0),
	m_pPulldownSelectedIcon(0),
	m_pPulldownHoverIcon(0),
	m_pPulldownDisabledIcon(0),
	m_pMenuBackgroundDefault(0),
	m_pMenuBackgroundHover(0),
	m_pMenuBackgroundSelected(0),
	m_pMenuBackgroundDisabled(0),
	m_pTitleBackgroundDefault(0),
	m_pTitleBackgroundHover(0),
	m_pTitleBackgroundSelected(0),
	m_pTitleBackgroundDisabled(0)
{
	m_pPulldownIcon = new Icon(pRenderer, defaultPulldownTexture);
	m_pPulldownSelectedIcon = new Icon(pRenderer, selectedPulldownTexture);
	m_pPulldownHoverIcon = new Icon(pRenderer, hoverPulldownTexture);
	m_pPulldownDisabledIcon = new Icon(pRenderer, disabledPulldownTexture);

	m_pMenuBackgroundDefault = new Icon(pRenderer, defaultMenuTexture);
	m_pMenuBackgroundHover = new Icon(pRenderer, hoverMenuTexture);
	m_pMenuBackgroundSelected = new Icon(pRenderer, selectedMenuTexture);
	m_pMenuBackgroundDisabled = new Icon(pRenderer, disabledMenuTexture);

	m_pTitleBackgroundDefault = new Icon(pRenderer, defaultTitleTexture);
	m_pTitleBackgroundHover = new Icon(pRenderer, hoverTitleTexture);
	m_pTitleBackgroundSelected = new Icon(pRenderer, selectedTitleTexture);
	m_pTitleBackgroundDisabled = new Icon(pRenderer, disabledTitleTexture);

	mScrollBar.SetScrollDirection(EScrollBarDirection_Vertical);
	mScrollBar.SetScrollPosition(0.75f);

	Add(m_pPulldownIcon);
	Add(m_pPulldownSelectedIcon);
	Add(m_pPulldownHoverIcon);
	Add(m_pPulldownDisabledIcon);
	Add(m_pMenuBackgroundDefault);
	Add(m_pMenuBackgroundHover);
	Add(m_pMenuBackgroundSelected);
	Add(m_pMenuBackgroundDisabled);
	Add(m_pTitleBackgroundDefault);
	Add(m_pTitleBackgroundHover);
	Add(m_pTitleBackgroundSelected);
	Add(m_pTitleBackgroundDisabled);
	
	mPulldownMenu.SetPullDownMenuParent(this);

	Add(&mPulldownMenu);

	Add(&mScrollBar);

	mMenuTitle.SetLocation(0, 0);
	Add(&mMenuTitle);

	m_renderMenuHeader = false;
	mMenuName.SetLocation(0, 0);
	Add(&mMenuName);

	//mPulldownMenu.SetAutoSizeMenuWidth(false);

	mGUIFont = GUIFont;

	m_outline = false;

	mNumItems = 0;
	mSelectedIndex = -1;
	mMaxDisplayItems = 0;

	m_bDisplayScrollbar = true;

	mpSelectedMenuItem = nullptr;

	m_MenuItemChangedCallback = nullptr;
	m_pMenuItemChangedCallbackData = nullptr;
}

PulldownMenu::~PulldownMenu()
{
	// Delete the menu items that we have dynamically created
	while (!m_vpMenuItemList.empty())
	{
		MenuItem* lpItem = m_vpMenuItemList.back();
		delete lpItem;
		m_vpMenuItemList.pop_back();
	}

	// Delete the pulldown icons
	delete m_pPulldownIcon;
	delete m_pPulldownSelectedIcon;
	delete m_pPulldownHoverIcon;
	delete m_pPulldownDisabledIcon;

	delete m_pMenuBackgroundDefault;
	delete m_pMenuBackgroundHover;
	delete m_pMenuBackgroundSelected;
	delete m_pMenuBackgroundDisabled;

	delete m_pTitleBackgroundDefault;
	delete m_pTitleBackgroundHover;
	delete m_pTitleBackgroundSelected;
	delete m_pTitleBackgroundDisabled;
}

void PulldownMenu::RemoveIcon(RenderRectangle *pRemoveIcon)
{
	if (pRemoveIcon)
	{
		Remove(pRemoveIcon);
		delete pRemoveIcon;
		pRemoveIcon = nullptr;
	}
}

void PulldownMenu::SetDefaultIcons(Renderer* pRenderer)
{
	RemoveIcon(m_pPulldownIcon);
	RemoveIcon(m_pPulldownHoverIcon);
	RemoveIcon(m_pPulldownSelectedIcon);
	RemoveIcon(m_pPulldownDisabledIcon);

	RemoveIcon(m_pMenuBackgroundDefault);
	RemoveIcon(m_pMenuBackgroundHover);
	RemoveIcon(m_pMenuBackgroundSelected);
	RemoveIcon(m_pMenuBackgroundDisabled);

	RemoveIcon(m_pTitleBackgroundDefault);
	RemoveIcon(m_pTitleBackgroundHover);
	RemoveIcon(m_pTitleBackgroundSelected);
	RemoveIcon(m_pTitleBackgroundDisabled);

	m_pPulldownIcon = new DirectDrawRectangle(pRenderer);
	m_pPulldownHoverIcon = new DirectDrawRectangle(pRenderer);
	m_pPulldownSelectedIcon = new DirectDrawRectangle(pRenderer);
	m_pPulldownDisabledIcon = new DirectDrawRectangle(pRenderer);

	m_pMenuBackgroundDefault = new DirectDrawRectangle(pRenderer);
	m_pMenuBackgroundHover = new DirectDrawRectangle(pRenderer);
	m_pMenuBackgroundSelected = new DirectDrawRectangle(pRenderer);
	m_pMenuBackgroundDisabled = new DirectDrawRectangle(pRenderer);

	m_pTitleBackgroundDefault = new DirectDrawRectangle(pRenderer);
	m_pTitleBackgroundHover = new DirectDrawRectangle(pRenderer);
	m_pTitleBackgroundSelected = new DirectDrawRectangle(pRenderer);
	m_pTitleBackgroundDisabled = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pPulldownIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pPulldownHoverIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pPulldownSelectedIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pPulldownDisabledIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pMenuBackgroundDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pMenuBackgroundHover;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pMenuBackgroundSelected;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pMenuBackgroundDisabled;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pTitleBackgroundDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pTitleBackgroundHover;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pTitleBackgroundSelected;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pTitleBackgroundDisabled;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	Add(m_pPulldownIcon);
	Add(m_pPulldownSelectedIcon);
	Add(m_pPulldownHoverIcon);
	Add(m_pPulldownDisabledIcon);
	Add(m_pMenuBackgroundDefault);
	Add(m_pMenuBackgroundHover);
	Add(m_pMenuBackgroundSelected);
	Add(m_pMenuBackgroundDisabled);
	Add(m_pTitleBackgroundDefault);
	Add(m_pTitleBackgroundHover);
	Add(m_pTitleBackgroundSelected);
	Add(m_pTitleBackgroundDisabled);

	// Set the dimensions of the icons
	Dimensions d = GetDimensions();
	m_pPulldownIcon->SetDimensions(d.m_width - d.m_height, 0, d.m_height, d.m_height);
	m_pPulldownSelectedIcon->SetDimensions(d.m_width - d.m_height, 0, d.m_height, d.m_height);
	m_pPulldownHoverIcon->SetDimensions(d.m_width - d.m_height, 0, d.m_height, d.m_height);
	m_pPulldownDisabledIcon->SetDimensions(d.m_width - d.m_height, 0, d.m_height, d.m_height);

	// Also set the dimensions of the title background icons
	m_pTitleBackgroundDefault->SetDimensions(0, 0, d.m_width - d.m_height, d.m_height);
	m_pTitleBackgroundHover->SetDimensions(0, 0, d.m_width - d.m_height, d.m_height);
	m_pTitleBackgroundSelected->SetDimensions(0, 0, d.m_width - d.m_height, d.m_height);
	m_pTitleBackgroundDisabled->SetDimensions(0, 0, d.m_width - d.m_height, d.m_height);

	// Also set the dimensions of the title label
	mMenuTitle.SetLocation(3, 3 /*mPulldownMenu.GetMenuItemSpacer(), mPulldownMenu.GetMenuItemSpacer()*/);

	// Also set the dimensions of the header label
	int lHeaderWidth = m_pRenderer->GetFreeTypeTextWidth(mGUIFont, "%s", mMenuName.GetText().c_str());
	mMenuName.SetLocation(-5 - lHeaderWidth, 3);

	Dimensions d_Scrollbar = mScrollBar.GetDimensions();
	m_pMenuBackgroundDefault->SetDimensions(0, -d_Scrollbar.m_height, d.m_width - d.m_height, d_Scrollbar.m_height);
	m_pMenuBackgroundDisabled->SetDimensions(0, -d_Scrollbar.m_height, d.m_width - d.m_height, d_Scrollbar.m_height);

	mPulldownMenu.SetDepth(GetDepth()+1.0f);
	mMenuTitle.SetDepth(GetDepth() + 1.0f);

	// Also set the default icons of the scrollbar
	mScrollBar.SetDefaultIcons(pRenderer);
}

void PulldownMenu::SetPulldownIconDefault(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pPulldownIcon)
	{
		// If we already own a default icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pPulldownIcon);

		delete m_pPulldownIcon;
		m_pPulldownIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pPulldownIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pPulldownIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pPulldownIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pPulldownIcon);

	Dimensions dimensions = GetDimensions();
	int lWidth = mPulldownMenu.GetDimensions().m_width;
	m_pPulldownIcon->SetDimensions(lWidth, 0, dimensions.m_height, dimensions.m_height);
}

void PulldownMenu::SetPulldownIconHover(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pPulldownHoverIcon)
	{
		// If we already own a default icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pPulldownHoverIcon);

		delete m_pPulldownHoverIcon;
		m_pPulldownHoverIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pPulldownHoverIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pPulldownHoverIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pPulldownHoverIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pPulldownHoverIcon);

	Dimensions dimensions = GetDimensions();
	int lWidth = mPulldownMenu.GetDimensions().m_width;
	m_pPulldownHoverIcon->SetDimensions(lWidth, 0, dimensions.m_height, dimensions.m_height);
}

void PulldownMenu::SetPulldownIconPressed(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pPulldownSelectedIcon)
	{
		// If we already own a default icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pPulldownSelectedIcon);

		delete m_pPulldownSelectedIcon;
		m_pPulldownSelectedIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pPulldownSelectedIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pPulldownSelectedIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pPulldownSelectedIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pPulldownSelectedIcon);

	Dimensions dimensions = GetDimensions();
	int lWidth = mPulldownMenu.GetDimensions().m_width;
	m_pPulldownSelectedIcon->SetDimensions(lWidth, 0, dimensions.m_height, dimensions.m_height);
}

void PulldownMenu::SetPulldownIconDisabled(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pPulldownDisabledIcon)
	{
		// If we already own a default icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pPulldownDisabledIcon);

		delete m_pPulldownDisabledIcon;
		m_pPulldownDisabledIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pPulldownDisabledIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pPulldownDisabledIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pPulldownDisabledIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pPulldownDisabledIcon);

	Dimensions dimensions = GetDimensions();
	int lWidth = mPulldownMenu.GetDimensions().m_width;
	m_pPulldownDisabledIcon->SetDimensions(lWidth, 0, dimensions.m_height, dimensions.m_height);
}

unsigned int PulldownMenu::GetGUIFont()
{
	return mGUIFont;
}

void PulldownMenu::SetDimensions(int x, int y, int width, int height)
{
	// Set the dimensions of the actual menu
	mPulldownMenu.SetDimensions(0, 0, width, height);

	width += mPulldownMenu.GetMenuItemSpacer() * 2;
	height += mPulldownMenu.GetMenuItemSpacer() * 2;

	Component::SetDimensions(x, y, width + height, height);

	// Also set the dimensions of the pulldown icons
	m_pPulldownIcon->SetDimensions(width, 0, height, height);
	m_pPulldownSelectedIcon->SetDimensions(width, 0, height, height);
	m_pPulldownHoverIcon->SetDimensions(width, 0, height, height);
	m_pPulldownDisabledIcon->SetDimensions(width, 0, height, height);

	// Also set the dimensions of the title background icons
	m_pTitleBackgroundDefault->SetDimensions(0, 0, width, height);
	m_pTitleBackgroundHover->SetDimensions(0, 0, width, height);
	m_pTitleBackgroundSelected->SetDimensions(0, 0, width, height);
	m_pTitleBackgroundDisabled->SetDimensions(0, 0, width, height);

	// Also set the dimensions of the menu background icons

	// Also set the dimensions of the title label
	mMenuTitle.SetLocation(3, 3 /*mPulldownMenu.GetMenuItemSpacer(), mPulldownMenu.GetMenuItemSpacer()*/);

	// Also set the dimensions of the header label
	int lHeaderWidth = m_pRenderer->GetFreeTypeTextWidth(mGUIFont, "%s", mMenuName.GetText().c_str());
	mMenuName.SetLocation(-5 - lHeaderWidth, 3);
}

void PulldownMenu::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void PulldownMenu::AddEventListeners()
{
	AddMouseListener(this);
	AddFocusListener(this);

	// Also add all children's event listeners
	Container::AddEventListeners();

	//mScrollBar.AddEventListeners();
}

void PulldownMenu::RemoveEventListeners()
{
	RemoveMouseListener(this);
	RemoveFocusListener(this);

	// Also remove all children's event listeners
	Container::RemoveEventListeners();

	//mScrollBar.RemoveEventListeners();

	// Remove hover and select properties, since we have made this component un-selectable by the mouse
	SetHover(false);
	SetSelected(false);
}

void PulldownMenu::AddPulldownItem(const std::string &label)
{
	MenuItem* lpNewItem = nullptr;
	
	if(m_outline)
	{
		lpNewItem = new MenuItem(m_pRenderer, mGUIFont, mOutlineGUIFont, label, mTextColour, m_outlineColour);
	}
	else
	{
		lpNewItem = new MenuItem(m_pRenderer, mGUIFont, label, mTextColour);
	}
	

	// Set the new item dimensions
	int lNewItemWidth = m_pRenderer->GetFreeTypeTextWidth(mGUIFont, "%s", label.c_str());
	int lNewItemHeight = m_pRenderer->GetFreeTypeTextHeight(mGUIFont, "%s", label.c_str());
	lpNewItem->SetWidth(lNewItemWidth);
	lpNewItem->SetHeight(lNewItemHeight);

	// Set the new item to have a function callback that occurs when we select this menu item
	lpNewItem->SetCallBackFunction(_MenuItemSelected);
	lpNewItem->SetCallBackData(lpNewItem);
	lpNewItem->SetOnlyShowSelectedIcons(true); // Don't use the menu items OWN background icons
	lpNewItem->SetDepth(GetDepth()-1.0f);

	// TODO : Get this working with depth!!
	//lpNewItem->SetHoverIcon(m_pMenuBackgroundHover);
	//lpNewItem->SetSelectedIcon(m_pMenuBackgroundSelected);

	mPulldownMenu.Add(lpNewItem, m_bDisplayScrollbar);

	// Increase our num items counter
	mNumItems++;

	// Reset the position of the pull down icons, since we might have increase the width of the initial menu
	int lWidth = mPulldownMenu.GetDimensions().m_width;
	int lHeight = mPulldownMenu.GetDimensions().m_height;
	m_pPulldownIcon->SetDimensions(lWidth, 0, lHeight, lHeight);
	m_pPulldownSelectedIcon->SetDimensions(lWidth, 0, lHeight, lHeight);
	m_pPulldownHoverIcon->SetDimensions(lWidth, 0, lHeight, lHeight);
	m_pPulldownDisabledIcon->SetDimensions(lWidth, 0, lHeight, lHeight);

	// Reset the dimensions of the title background icons
	m_pTitleBackgroundDefault->SetDimensions(0, 0, lWidth, lHeight);
	m_pTitleBackgroundHover->SetDimensions(0, 0, lWidth, lHeight);
	m_pTitleBackgroundSelected->SetDimensions(0, 0, lWidth, lHeight);
	m_pTitleBackgroundDisabled->SetDimensions(0, 0, lWidth, lHeight);

	// Reset the initial component dimensions
	Component::SetDimensions(GetDimensions().m_x, GetDimensions().m_y, lWidth + lHeight, lHeight);

	// Also set the dimensions of the scrollbar, since adding new items increases the height of the pulldown menu
	int lPullDownHeight = mPulldownMenu.GetCurrentMenuheight();
	int lPullDownHeightModifiedToMaxDisplay = mMaxDisplayItems * (lNewItemHeight + (mPulldownMenu.GetMenuItemSpacer() * 2));
	int lHeightToUseForScrollbar = lPullDownHeight;
	if(mNumItems > mMaxDisplayItems)
	{
		lHeightToUseForScrollbar = lPullDownHeightModifiedToMaxDisplay;
	}
	mScrollBar.SetDimensions(lWidth, -lHeightToUseForScrollbar, lHeight, lHeightToUseForScrollbar);
	
	// Reset the dimensions of the menu background icons
	int lMenuItemWidth = lWidth;
	if(!m_bDisplayScrollbar)
	{
		lMenuItemWidth +=lHeight;
	}
	m_pMenuBackgroundDefault->SetDimensions(0, -lHeightToUseForScrollbar, lMenuItemWidth, lHeightToUseForScrollbar);
	// NOT NEEDED : This is the size of a menu item... m_pMenuBackgroundHover->SetDimensions(0, -lHeightToUseForScrollbar, lMenuItemWidth, lHeightToUseForScrollbar);
	// NOT NEEDED : This is the size of a menu item... m_pMenuBackgroundSelected->SetDimensions(0, -lHeightToUseForScrollbar, lMenuItemWidth, lHeightToUseForScrollbar);
	m_pMenuBackgroundDisabled->SetDimensions(0, -lHeightToUseForScrollbar, lMenuItemWidth, lHeightToUseForScrollbar);

	UpdateScrollbar();

	// Check a vector list of all the added items, so we can delete them later, since they are created dynamically
	m_vpMenuItemList.push_back(lpNewItem);
}

int PulldownMenu::GetNumItems() const
{
	return mNumItems;
}

int PulldownMenu::GetSelectedItem() const
{
	return mSelectedIndex;
}

void PulldownMenu::RemoveAllPullDownMenuItems()
{
	mPulldownMenu.RemoveAll();
	mPulldownMenu.ResetMenu();

	while (!m_vpMenuItemList.empty())
	{
		MenuItem* lpItem = m_vpMenuItemList.back();

		delete lpItem;
		m_vpMenuItemList.pop_back();
	}

	mNumItems = 0;
	mSelectedIndex = -1;

	m_bDisplayScrollbar = true;

	mpSelectedMenuItem = nullptr;
}

void PulldownMenu::ResetPullDownMenu()
{
	mSelectedIndex = -1;
	mpSelectedMenuItem = nullptr;

	mMenuTitle.SetText(mMenuName.GetText());

	// Reset the dimensions of the scrollbar - since this won't get updated until we add a new menu item later
	mScrollBar.SetDimensions(0, 0, 0, 0);
	m_pMenuBackgroundDefault->SetDimensions(0, 0, 0, 0);
	m_pMenuBackgroundDisabled->SetDimensions(0, 0, 0, 0);
}

void PulldownMenu::SetSelectedItem(const std::string &label)
{
	mpSelectedMenuItem = mPulldownMenu.FindItem(label);

	if(mpSelectedMenuItem != nullptr)
	{
		SetTitleString(mpSelectedMenuItem->GetLabel().GetText());
	}
}

void PulldownMenu::SetSelectedMenuItem(MenuItem* lpMenuItem)
{
	mpSelectedMenuItem = lpMenuItem;
}

MenuItem* PulldownMenu::GetSelectedMenuItem() const
{
	return mpSelectedMenuItem;
}

void PulldownMenu::SetMaxNumItemsDisplayed(int lMaxDisplay)
{
	mMaxDisplayItems = lMaxDisplay;
}

int PulldownMenu::GetMaxNumItemsDisplayed() const
{
	return mMaxDisplayItems;
}

void PulldownMenu::UpdateScrollbar()
{
	if(mNumItems > mMaxDisplayItems)
	{
		float lScrollRatio = (float)mMaxDisplayItems / (float)mNumItems;
		mScrollBar.SetScrollSize(lScrollRatio);
		mScrollBar.SetScrollPosition(1.0f);

		mScrollBar.SetDisabled(false);
	}
	else
	{
		mScrollBar.SetDisabled(true);
	}
}

void PulldownMenu::SetTitleString(const std::string &label)
{
	mMenuTitle.SetText(label);
}

void PulldownMenu::SetRenderHeader(bool render)
{
	m_renderMenuHeader = render;
}

void PulldownMenu::SetSelected(bool val)
{
	m_bSelected = val;
	mPulldownMenu.SetSelected(val);
}

void PulldownMenu::SetHover(bool val)
{
	m_bHover = val;
	mPulldownMenu.SetHover(val);
}

void PulldownMenu::SetDisabled(bool val)
{
	m_bDisabled = val;
	mPulldownMenu.SetDisabled(val);

	if(m_bDisabled)
	{
		SetSelected(false);
		SetHover(false);
	}
}

bool PulldownMenu::IsSelected() const
{
	return m_bSelected;
}

bool PulldownMenu::IsHover() const
{
	return m_bHover;
}

bool PulldownMenu::IsDisabled() const
{
	return m_bDisabled;
}

void PulldownMenu::SetDisplayScrollBar(bool lShow)
{
	m_bDisplayScrollbar = lShow;

	if(m_bDisplayScrollbar)
	{
		Add(&mScrollBar);

		//mScrollBar.SetVisible(true);
		//mScrollBar.SetEnabled(true);
	}
	else
	{
		Remove(&mScrollBar);

		//mScrollBar.SetVisible(false);
		//mScrollBar.SetEnabled(false);
	}
}

void PulldownMenu::OpenMenu()
{
	mPulldownMenu.OpenMenu();
}

void PulldownMenu::CloseMenu()
{
	mPulldownMenu.CloseMenu();
}

bool PulldownMenu::IsMenuOpen()
{
	return mPulldownMenu.IsMenuOpen();
}

bool PulldownMenu::ChildHasFocus()
{
	if(mPulldownMenu.ChildHasFocus() || mScrollBar.HasFocus())
	{
		return true;
	}

	return false;
}

bool PulldownMenu::MenuChildHasFocus()
{
	if(mPulldownMenu.ChildHasFocus())
	{
		return true;
	}

	return false;
}

EComponentType PulldownMenu::GetComponentType() const
{
	return EComponentType_PulldownMenu;
}

ScrollBar* PulldownMenu::GetPulldownScrollBar()
{
	return &mScrollBar;
}

void PulldownMenu::MenuItemChanged()
{
	if(m_MenuItemChangedCallback)
	{
		m_MenuItemChangedCallback(m_pMenuItemChangedCallbackData);
	}
}

void PulldownMenu::SetMenuItemChangedCallBackFunction(FunctionCallback lFunction)
{
	m_MenuItemChangedCallback = lFunction;
}

void PulldownMenu::SetMenuItemChangedCallBackData(void *lpData)
{
	m_pMenuItemChangedCallbackData = lpData;
}

void PulldownMenu::MouseEntered(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetHover(true);

	OnMouseEnter();
}

void PulldownMenu::MouseExited(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	PulldownMenu* pParent = mPulldownMenu.GetPullDownMenuParent();

	// Make sure that we are inside the bounds of the parent menu
	int lTextHeight = m_pRenderer->GetFreeTypeTextHeight(GetGUIFont(), "%s", mPulldownMenu.GetMenuTitle().c_str());
	int lMenuHeight = lTextHeight + (mPulldownMenu.GetMenuItemSpacer() * 2);
	int lFullMenuDisplayHeight = pParent->GetMaxNumItemsDisplayed() * lMenuHeight;
	int lMenuX = pParent->GetLocation().m_x;
	int lMenuY = pParent->GetLocation().m_y - lFullMenuDisplayHeight;
	int lMenuWidth = mPulldownMenu.GetBiggestWidth()+ (mPulldownMenu.GetMenuItemSpacer() * 2);

	//if(lEvent.GetX() > lMenuX && lEvent.GetX() <= lMenuX+lMenuWidth && lEvent.GetY() > lMenuY && lEvent.GetY() <= lMenuY+lFullMenuDisplayHeight)
	{
		SetHover(false);

		// Also removed the button selection if we exit it's dimensions
		SetSelected(false);

		OnMouseExit();
	}
}

void PulldownMenu::MousePressed(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	if(mPulldownMenu.IsMenuOpen())
	{
		SetSelected(false);

		mPulldownMenu.CloseMenu();
	}
	else
	{
		SetSelected(true);

		mPulldownMenu.OpenMenu();
	}

	// If our parent is a GUIWindow, then makew this window have focus in the GUI, used to make it's depth the highest
	if(GetParent() != nullptr && GetParent()->GetComponentType() == EComponentType_GUIWindow)
	{
		GUIWindow* lpParentWindow = (GUIWindow *)GetParent();
		lpParentWindow->SetFocusWindow();
	}

	OnMousePressed();
}

void PulldownMenu::MouseReleased(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetSelected(false);

	OnMouseReleased();
}


void PulldownMenu::MouseReleasedOutside(const MouseEvent& lEvent)
{
	// Make sure that we are inside the bounds of the parent menu
	int lTextHeight = m_pRenderer->GetFreeTypeTextHeight(GetGUIFont(), "%s", mPulldownMenu.GetMenuTitle().c_str());
	int lMenuHeight = lTextHeight + (mPulldownMenu.GetMenuItemSpacer() * 2);
	int lFullMenuDisplayHeight = mPulldownMenu.GetPullDownMenuParent()->GetMaxNumItemsDisplayed() * lMenuHeight;
	int lMenuX = mPulldownMenu.GetPullDownMenuParent()->GetLocation().m_x;
	int lMenuY = mPulldownMenu.GetPullDownMenuParent()->GetLocation().m_y - lFullMenuDisplayHeight;
	int lMenuWidth = mPulldownMenu.GetBiggestWidth()+ (mPulldownMenu.GetMenuItemSpacer() * 2);

	//mPulldownMenu.CloseMenu();
}

void PulldownMenu::MouseClicked(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	OnMouseClicked();
}

void PulldownMenu::FocusLost(const FocusEvent& lEvent)
{
	if(ChildHasFocus())
	{

	}
	else
	{
		//mPulldownMenu.CloseMenu();
	}
}

void PulldownMenu::FocusGained(const FocusEvent& lEvent)
{

}

void PulldownMenu::OnMouseEnter()
{
	/* Nothing */
}

void PulldownMenu::OnMouseExit()
{
	/* Nothing */
}

void PulldownMenu::OnMousePressed()
{
	/* Nothing */
}

void PulldownMenu::OnMouseReleased()
{
	/* Nothing */
}

void PulldownMenu::OnMouseClicked()
{
	/* Nothing */
}

void PulldownMenu::DrawChildren()
{
	int lTextHeight = m_pRenderer->GetFreeTypeTextHeight(mGUIFont, "%s", mPulldownMenu.GetMenuTitle().c_str());
	int lMenuHeight = lTextHeight + (mPulldownMenu.GetMenuItemSpacer() * 2);
	int lFullMenuDisplayHeight = mMaxDisplayItems * lMenuHeight;
	//int lIconWidth = lMenuHeight;

	// Do a scissor test here
	Point location = GetLocation();
	for(Component* parent = GetParent(); parent != 0;)
	{
		Point parentLocation = parent->GetLocation();

		location.m_x += parentLocation.m_x;
		location.m_y += parentLocation.m_y;

		parent = parent->GetParent();
	}
	int lScissorX = location.m_x;
	int lScissorY = location.m_y - lFullMenuDisplayHeight;
	int lScissorWidth = mPulldownMenu.GetBiggestWidth()+ (mPulldownMenu.GetMenuItemSpacer() * 2);
	int lScissorHeight = lFullMenuDisplayHeight;
	int l_depth = static_cast<int>(GetDepth());

	/*
	// DEBUG : Draw the scissor region
	m_pRenderer->PushMatrix();
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
		m_pRenderer->EnableImmediateMode(IM_LINE_LOOP);
		m_pRenderer->ImmediateVertex((lScissorX - GetLocation().m_x),						(lScissorY - GetLocation().m_y),					l_depth);
		m_pRenderer->ImmediateVertex((lScissorX - GetLocation().m_x) + lScissorWidth,		(lScissorY - GetLocation().m_y),					l_depth);
		m_pRenderer->ImmediateVertex((lScissorX - GetLocation().m_x) + lScissorWidth + 1,	(lScissorY - GetLocation().m_y ) + lScissorHeight,	l_depth);
		m_pRenderer->ImmediateVertex((lScissorX - GetLocation().m_x),						(lScissorY - GetLocation().m_y ) + lScissorHeight,	l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
	*/

	bool lbNeedScissor = false;

	// Modify the menu position, depending on where the scrollbar is
	if(mNumItems > mMaxDisplayItems)
	{
		float missingAmount = 1.0f - mScrollBar.GetScrollSize();
		float scrollPositionRelative = (mScrollBar.GetScrollPosition() - (mScrollBar.GetScrollSize()*0.5f));
		float lModifiedScrollPosition = 1.0f - scrollPositionRelative / missingAmount;
		int lFullHeight = (mNumItems) * lMenuHeight; 
		int lSeenHeight = lFullHeight - (lFullMenuDisplayHeight);
		float lModifiedY = lModifiedScrollPosition * (float)lSeenHeight - lMenuHeight;
		mPulldownMenu.SetLocation(0, (int)lModifiedY);

		lbNeedScissor = true;
	}
	else
	{
		int lModifiedY = -lMenuHeight;
		mPulldownMenu.SetLocation(0, lModifiedY);
	}


	//Container::DrawChildren();

	// NOTE : We have to do this manually, since we only want to scissor test the menu child

	// This vector list is depth sorted, so we render in depth order, starting with the lowest depth first
	if(IsContainer() && HasChildren() && IsVisible())
	{
		// Draw all children
		ComponentList::const_iterator iterator;

		for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
		{
			if((*iterator)->IsVisible())
			{
				if(lbNeedScissor && (*iterator)->GetComponentType() == EComponentType_Menu)
				{
					m_pRenderer->EnableScissorTest(lScissorX, lScissorY, lScissorWidth, lScissorHeight);;
				}

				(*iterator)->Draw();

				if(lbNeedScissor && (*iterator)->GetComponentType() == EComponentType_Menu)
				{
					m_pRenderer->DisableScissorTest();
				}
			}
		}
	}
}

void PulldownMenu::DrawSelf()
{
	//Container::DrawSelf();

	mMenuName.SetVisible(m_renderMenuHeader);

	// Reset visibility
	m_pPulldownIcon->SetVisible(false);
	m_pPulldownHoverIcon->SetVisible(false);
	m_pPulldownSelectedIcon->SetVisible(false);
	m_pPulldownDisabledIcon->SetVisible(false);

	m_pTitleBackgroundDefault->SetVisible(false);
	m_pTitleBackgroundHover->SetVisible(false);
	m_pTitleBackgroundSelected->SetVisible(false);
	m_pTitleBackgroundDisabled->SetVisible(false);

	m_pMenuBackgroundDefault->SetVisible(false);
	m_pMenuBackgroundHover->SetVisible(false);
	m_pMenuBackgroundSelected->SetVisible(false);
	m_pMenuBackgroundDisabled->SetVisible(false);


	if(mPulldownMenu.IsMenuOpen())
	{
		mScrollBar.SetVisible(true);
		mScrollBar.SetEnabled(true);

		if(IsDisabled())
		{
			m_pMenuBackgroundDisabled->SetVisible(true);
		}
		else
		{
			m_pMenuBackgroundDefault->SetVisible(true);
		}
	}
	else
	{
		mScrollBar.SetVisible(false);
		mScrollBar.SetEnabled(false);
	}

	if(IsSelected())
	{
		m_pPulldownSelectedIcon->SetVisible(true);
		m_pTitleBackgroundSelected->SetVisible(true);
	}
	else if(IsHover())
	{
		m_pPulldownHoverIcon->SetVisible(true);
		m_pTitleBackgroundHover->SetVisible(true);
	}
	else if(IsDisabled())
	{
		m_pPulldownDisabledIcon->SetVisible(true);
		m_pTitleBackgroundDisabled->SetVisible(true);
	}
	else
	{
		m_pPulldownIcon->SetVisible(true);
		m_pTitleBackgroundDefault->SetVisible(true);
	}
}

void PulldownMenu::_MenuItemSelected(void *lpData)
{
	MenuItem *lpMenuItem = (MenuItem*)lpData;

	if(lpMenuItem)
	{		
		lpMenuItem->MenuItemSelectedFromPulldowmMenu();
	}
}