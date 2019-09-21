// ******************************************************************************
//
// Filename:	scrollbar.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A scrollbar GUI component to allow scrolling horizontal or vertically. Can
//   be used for a variety of purposes, but mainly for scrolling large windows.
//
// Note:
//   When being used in vertical mode, the left arrow is on top and the right
//   arrow is on the bottom.
//
// Revision History:
//   Initial Revision - 29/07/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "container.h"
#include "mouselistener.h"
#include "icon.h"
#include "../utils/CountdownTimer.h"


enum EScrollBarDirection
{
	EScrollBarDirection_Horizontal = 0,
	EScrollBarDirection_Vertical,
};


class ScrollBar : public Container, public MouseListener/*, public FocusListener*/
{
public:
	/* Public methods */
	ScrollBar(Renderer* pRenderer);
	ScrollBar(Renderer* pRenderer, const std::string &leftDefaultTexture, const std::string &leftPressedTexture, const std::string &leftHoverTexture, const std::string &leftDisabledTexture,
										 const std::string &rightDefaultTexture, const std::string &rightPressedTexture, const std::string &rightHoverTexture, const std::string &rightDisabledTexture,
										 const std::string &scrollDefaultTexture, const std::string &scrollPressedTexture, const std::string &scrollHoverTexture, const std::string &scrollDisabledTexture,
										 const std::string &BackbarTexture);

	virtual ~ScrollBar();

	void AddEventListeners() override;
	void RemoveEventListeners() override;

	void RemoveIcon(RenderRectangle *pRemoveIcon);
	void SetDefaultIcons(Renderer* pRenderer);
	void SetLeftArrowDefaultIcon(RenderRectangle *icon);
	void SetLeftArrowHoverIcon(RenderRectangle *icon);
	void SetLeftArrowSelectedIcon(RenderRectangle *icon);
	void SetLeftArrowDisabledIcon(RenderRectangle *icon);
	void SetRightArrowDefaultIcon(RenderRectangle *icon);
	void SetRightArrowHoverIcon(RenderRectangle *icon);
	void SetRightArrowSelectedIcon(RenderRectangle *icon);
	void SetRightArrowDisabledIcon(RenderRectangle *icon);
	void SetScrollbarDefaultIcon(RenderRectangle *icon);
	void SetScrollbarHoverIcon(RenderRectangle *icon);
	void SetScrollbarSelectedIcon(RenderRectangle *icon);
	void SetScrollbarDisabledIcon(RenderRectangle *icon);
	void SetBackbarIcon(RenderRectangle *icon);

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	void SetArrowDimensions(int width, int height);

	void AddScrollAreaItem(Component* component);
	void ClearScrollAreaItems();

	void SetScrollArea(int x, int y, int width, int height);
	Dimensions GetScrollArea();

	void SetScrollPosition(float lPosition);
	float GetScrollPosition() const;

	void SetScrollSize(float lSize);
	float GetScrollSize();

	float GetScrollRatio(); // Returns a 0.0f -> 1.0f value based on where the scroll bar is currently positioned

	void UpdateScrollbarDimension();

	void ClampScrollBarPosition();

	void SetScissorEnabled(bool scissor);

	EScrollBarDirection GetScrollDirection();
	void SetScrollDirection(EScrollBarDirection lDirection);

	void SetSelectedRight(bool val);	// Bottom when in vertical mode
	void SetHoverRight(bool val);		// Bottom when in vertical mode
	void SetSelectedLeft(bool val);		// Top when in vertical mode
	void SetHoverLeft(bool val);		// Top when in vertical mode
	void SetDisabled(bool val);

	bool IsSelectedRight() const;
	bool IsHoverRight() const;
	bool IsSelectedLeft() const;
	bool IsHoverLeft() const;
	bool IsDisabled() const;

	EComponentType GetComponentType() const override;

	void SetValueChangedCallback(FunctionCallback lFunction);
	void SetValueChangedCallBackData(void *lpData);

	void SetAudio(bool set);
	void SetAudioVolume(float volume);

protected:
	/* Protected methods */
	void MouseEntered(const MouseEvent& lEvent) override;
	void MouseExited(const MouseEvent& lEvent) override;
	void MousePressed(const MouseEvent& lEvent) override;
	void MouseReleased(const MouseEvent& lEvent) override;
	void MouseReleasedOutside(const MouseEvent& lEvent);
	void MouseClicked(const MouseEvent& lEvent) override;
	void MouseDragged(const MouseEvent& lEvent);
	void MouseDraggedOutside(const MouseEvent& lEvent);
	void MouseMotion(const MouseEvent& lEvent);

	/*
	void FocusLost(const FocusEvent& lEvent);
	void FocusGained(const FocusEvent& lEvent);
	*/

	virtual void OnMouseEnter();
	virtual void OnMouseExit();
	virtual void OnMousePressed();
	virtual void OnMouseReleased();
	virtual void OnMouseClicked();
	virtual void OnMouseMotion();

	virtual void OnValueChanged();

	void DrawChildren() override;
	void DrawSelf() override;

private:
	/* Private methods */
	static void _UpdateArrowButtons(void *lpData);
	void UpdateArrowButtons();

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	RenderRectangle	*m_pLeftArrowDefault;		// Top when in vertical mode
	RenderRectangle	*m_pLeftArrowHover;
	RenderRectangle	*m_pLeftArrowPressed;
	RenderRectangle	*m_pLeftArrowDisabled;

	RenderRectangle	*m_pRightArrowDefault;		// Bottom when in vertical mode
	RenderRectangle	*m_pRightArrowHover;
	RenderRectangle	*m_pRightArrowPressed;
	RenderRectangle	*m_pRightArrowDisabled;

	RenderRectangle	*m_pScrollbarDefault;
	RenderRectangle	*m_pScrollbarHover;
	RenderRectangle	*m_pScrollbarPressed;
	RenderRectangle	*m_pScrollbarDisabled;

	RenderRectangle	*m_pBackbar;

	bool	m_bSelectedRight;
	bool	m_bHoverRight;
	bool	m_bSelectedLeft;
	bool	m_bHoverLeft;
	bool	m_bDisabled;

	bool	m_bHoverScrollBar;
	bool	m_bDraggingScrollBar;
	bool	m_bFirstOutsideDrag;

	float	m_scrollPosition;
	float	m_ScrollSize;

	int m_lastX;
	int m_lastY;

	bool m_scissorEnabled;

	Dimensions m_scrollArea;

	ComponentList m_vpScrollAreaComponentList;

	CountdownTimer* m_pArrowButtonUpdate;

	EScrollBarDirection m_eScrollDirection;

	// Function callback for when this scrollbar value is changed
	FunctionCallback m_ValueChangedCallback;
	void *m_pValueChangedCallbackData;
};
