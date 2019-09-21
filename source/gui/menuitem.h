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

#pragma once

#include "abstractbutton.h"


class MenuItem : public AbstractButton, public MouseListener, public FocusListener
{
public:
	/* Public methods */
	MenuItem(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, Colour lLabelColour);
	MenuItem(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, Colour lLabelColour, Colour outlineColour);
	MenuItem(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, Colour lLabelColour, const std::string &defaultTexture, const std::string &selectedTexture, const std::string &hoverTexture, const std::string &disabledTexture);

	~MenuItem();

	void AddEventListeners() override;
	void RemoveEventListeners() override;

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;
	void SetWidth(int width);
	void SetHeight(int height);

	void SetOnlyShowSelectedIcons(bool lShow);
	bool IsOnlyShowSelectedIcons() const;

	EComponentType GetComponentType() const override;

	bool IsParentMenuOpen();

	void MenuItemSelectedFromPulldowmMenu();

protected:
	/* Protected methods */
	void MouseEntered(const MouseEvent& lEvent) override;
	void MouseExited(const MouseEvent& lEvent) override;
	void MousePressed(const MouseEvent& lEvent) override;
	void MouseReleased(const MouseEvent& lEvent) override;
	void MouseReleasedOutside(const MouseEvent& lEvent);
	void MouseClicked(const MouseEvent& lEvent) override;

	void MenuItemPressed();

	void FocusLost(const FocusEvent& lEvent);
	void FocusGained(const FocusEvent& lEvent);

	virtual void OnMouseEnter();
	virtual void OnMouseExit();
	virtual void OnMousePressed();
	virtual void OnMouseReleased();
	virtual void OnMouseClicked();

	void DrawSelf() override;

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	bool m_bOnlyShowSelectedItems;
};
