// ******************************************************************************
//
// Filename:	pulldown.h
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

#pragma once

#include "container.h"
#include "renderrectangle.h"
#include "menu.h"
#include "menuitem.h"
#include "scrollbar.h"


typedef std::vector<MenuItem*> MenuItemList;


class PulldownMenu : public Container, public MouseListener, public FocusListener
{
public:
	/* Public methods */
	PulldownMenu(Renderer* pRenderer, unsigned int GUIFont, const std::string &MenuTitle);
	PulldownMenu(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &MenuTitle, Colour fontColour, Colour outlineColour);

	PulldownMenu(Renderer* pRenderer, unsigned int GUIFont, const std::string &MenuTitle,
				 const std::string &defaultPulldownTexture, const std::string &selectedPulldownTexture, const std::string &hoverPulldownTexture, const std::string &disabledPulldowTexture,
				 const std::string &defaultTitleTexture, const std::string &selectedTitleTexture, const std::string &hoverTitleTexture, const std::string &disabledTitleTexture,
				 const std::string &defaultMenuTexture, const std::string &selectedMenuTexture, const std::string &hoverMenuTexture, const std::string &disabledMenuTexture);

	~PulldownMenu();

	void RemoveIcon(RenderRectangle *pRemoveIcon);
	void SetDefaultIcons(Renderer* pRenderer);
	void SetPulldownIconDefault(RenderRectangle *icon);
	void SetPulldownIconHover(RenderRectangle *icon);
	void SetPulldownIconPressed(RenderRectangle *icon);
	void SetPulldownIconDisabled(RenderRectangle *icon);

	unsigned int GetGUIFont();

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	void AddEventListeners() override;
	void RemoveEventListeners() override;

	void AddPulldownItem(const std::string &label);
	int GetNumItems() const;
	int GetSelectedItem() const;

	void RemoveAllPullDownMenuItems();

	void ResetPullDownMenu();

	void SetSelectedItem(const std::string &label);
	void SetSelectedMenuItem(MenuItem* lpMenuItem);
	MenuItem* GetSelectedMenuItem() const;

	void SetMaxNumItemsDisplayed(int lMaxDisplay);
	int GetMaxNumItemsDisplayed() const;

	void UpdateScrollbar();

	void SetTitleString(const std::string &label);

	void SetRenderHeader(bool render);

	void SetSelected(bool val);
	void SetHover(bool val);
	void SetDisabled(bool val);

	bool IsSelected() const;
	bool IsHover() const;
	bool IsDisabled() const;

	void SetDisplayScrollBar(bool lShow);

	void OpenMenu();
	void CloseMenu();
	bool IsMenuOpen();

	bool ChildHasFocus();
	bool MenuChildHasFocus();

	EComponentType GetComponentType() const override;

	ScrollBar* GetPulldownScrollBar();

	void MenuItemChanged();

	void SetMenuItemChangedCallBackFunction(FunctionCallback lFunction);
	void SetMenuItemChangedCallBackData(void *lpData);

protected:
	/* Protected methods */
	void MouseEntered(const MouseEvent& lEvent) override;
	void MouseExited(const MouseEvent& lEvent) override;
	void MousePressed(const MouseEvent& lEvent) override;
	void MouseReleased(const MouseEvent& lEvent) override;
	void MouseReleasedOutside(const MouseEvent& lEvent);
	void MouseClicked(const MouseEvent& lEvent) override;

	void FocusLost(const FocusEvent& lEvent);
	void FocusGained(const FocusEvent& lEvent);

	virtual void OnMouseEnter();
	virtual void OnMouseExit();
	virtual void OnMousePressed();
	virtual void OnMouseReleased();
	virtual void OnMouseClicked();

	void DrawChildren() override;
	void DrawSelf() override;

private:
	/* Private methods */
	static void _MenuItemSelected(void *lpData);

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	RenderRectangle *m_pPulldownIcon;
	RenderRectangle *m_pPulldownSelectedIcon;
	RenderRectangle *m_pPulldownHoverIcon;
	RenderRectangle *m_pPulldownDisabledIcon;

	RenderRectangle *m_pMenuBackgroundDefault;
	RenderRectangle *m_pMenuBackgroundHover;
	RenderRectangle *m_pMenuBackgroundSelected;
	RenderRectangle *m_pMenuBackgroundDisabled;

	RenderRectangle *m_pTitleBackgroundDefault;
	RenderRectangle *m_pTitleBackgroundHover;
	RenderRectangle *m_pTitleBackgroundSelected;
	RenderRectangle *m_pTitleBackgroundDisabled;

	unsigned int mGUIFont;
	unsigned int mOutlineGUIFont;

	bool m_outline;
	Colour m_outlineColour;

	Label mMenuName;

	bool m_renderMenuHeader;

	Label mMenuTitle;
	Colour mTextColour;

	Menu mPulldownMenu;

	ScrollBar mScrollBar;

	MenuItemList m_vpMenuItemList;

	bool	m_bSelected;
	bool	m_bHover;
	bool	m_bDisabled;

	bool	m_bDisplayScrollbar;

	int mNumItems;
	int mSelectedIndex;
	int mMaxDisplayItems;

	MenuItem* mpSelectedMenuItem;

	// Function callback for when this slider has a value changed event
	FunctionCallback m_MenuItemChangedCallback;
	void *m_pMenuItemChangedCallbackData;
};
