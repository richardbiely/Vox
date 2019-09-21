// ******************************************************************************
//
// Filename:	scrollbar.cpp
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

#include "scrollbar.h"
#include "../utils/TimeManager.h"
#include "directdrawrectangle.h"
#include "multitextureicon.h"
#include "pulldownmenu.h"
#include "focusmanager.h"
#include "guiwindow.h"


ScrollBar::ScrollBar(Renderer* pRenderer)
  : Container(pRenderer),
	m_pLeftArrowDefault(0),
    m_pLeftArrowHover(0),
	m_pLeftArrowPressed(0),
	m_pLeftArrowDisabled(0),
	m_pRightArrowDefault(0),
	m_pRightArrowHover(0),
	m_pRightArrowPressed(0),
	m_pRightArrowDisabled(0),
	m_pScrollbarDefault(0),
	m_pScrollbarHover(0),
	m_pScrollbarPressed(0),
	m_pScrollbarDisabled(0),
	m_pBackbar(0)
{
	SetDefaultIcons(pRenderer);

	m_pLeftArrowDefault->SetDepth(1.5f);
	m_pLeftArrowHover->SetDepth(1.5f);
	m_pLeftArrowPressed->SetDepth(1.5f);
	m_pLeftArrowDisabled->SetDepth(1.5f);
	m_pRightArrowDefault->SetDepth(1.5f);
	m_pRightArrowHover->SetDepth(1.5f);
	m_pRightArrowPressed->SetDepth(1.5f);
	m_pRightArrowDisabled->SetDepth(1.5f);

	m_pScrollbarDefault->SetDepth(2.0f);
	m_pScrollbarHover->SetDepth(2.0f);
	m_pScrollbarPressed->SetDepth(2.0f);
	m_pScrollbarDisabled->SetDepth(2.0f);

	m_pBackbar->SetDepth(2.0f);


	m_bSelectedLeft = false;
	m_bHoverLeft = false;
	m_bSelectedRight = false;
	m_bHoverRight = false;
	m_bDisabled = false;

	m_bHoverScrollBar = false;
	m_bDraggingScrollBar = false;
	m_bFirstOutsideDrag = false;

	m_scrollPosition = 0.5f;
	m_ScrollSize = 1.0f;

	m_lastX = 0;
	m_lastY = 0;

	m_scissorEnabled = true;

	m_pArrowButtonUpdate = new CountdownTimer();
	m_pArrowButtonUpdate->SetCountdownTime(0.01f);
	m_pArrowButtonUpdate->SetLooping(true);
	m_pArrowButtonUpdate->SetCallBackFunction(_UpdateArrowButtons);
	m_pArrowButtonUpdate->SetCallBackData(this);
	m_pArrowButtonUpdate->StartCountdown();

	m_eScrollDirection = EScrollBarDirection_Horizontal;

	m_ValueChangedCallback = nullptr;
	m_pValueChangedCallbackData = nullptr;
}

ScrollBar::ScrollBar(Renderer* pRenderer, const std::string &leftDefaultTexture, const std::string &leftPressedTexture, const std::string &leftHoverTexture, const std::string &leftDisabledTexture,
					 const std::string &rightDefaultTexture, const std::string &rightPressedTexture, const std::string &rightHoverTexture, const std::string &rightDisabledTexture,
					 const std::string &scrollDefaultTexture, const std::string &scrollPressedTexture, const std::string &scrollHoverTexture, const std::string &scrollDisabledTexture,
					 const std::string &BackbarTexture)
  : Container(pRenderer),
	m_pLeftArrowDefault(0),
	m_pLeftArrowHover(0),
	m_pLeftArrowPressed(0),
	m_pLeftArrowDisabled(0),
	m_pRightArrowDefault(0),
	m_pRightArrowHover(0),
	m_pRightArrowPressed(0),
	m_pRightArrowDisabled(0),
	m_pScrollbarDefault(0),
	m_pScrollbarHover(0),
	m_pScrollbarPressed(0),
	m_pScrollbarDisabled(0),
	m_pBackbar(0)
{
	m_pLeftArrowDefault = new Icon(pRenderer, leftDefaultTexture);
	m_pLeftArrowHover = new Icon(pRenderer, leftHoverTexture);
	m_pLeftArrowPressed = new Icon(pRenderer, leftPressedTexture);
	m_pLeftArrowDisabled = new Icon(pRenderer, leftDisabledTexture);
	m_pRightArrowDefault = new Icon(pRenderer, rightDefaultTexture);
	m_pRightArrowHover = new Icon(pRenderer, rightHoverTexture);
	m_pRightArrowPressed = new Icon(pRenderer, rightPressedTexture);
	m_pRightArrowDisabled = new Icon(pRenderer, rightDisabledTexture);

	m_pScrollbarDefault = new Icon(pRenderer, scrollDefaultTexture);
	m_pScrollbarHover = new Icon(pRenderer, scrollHoverTexture);
	m_pScrollbarPressed = new Icon(pRenderer, scrollPressedTexture);
	m_pScrollbarDisabled = new Icon(pRenderer, scrollDisabledTexture);

	m_pBackbar = new Icon(pRenderer, BackbarTexture);

	Add(m_pLeftArrowDefault);
	Add(m_pLeftArrowHover);
	Add(m_pLeftArrowPressed);
	Add(m_pLeftArrowDisabled);
	Add(m_pRightArrowDefault);
	Add(m_pRightArrowHover);
	Add(m_pRightArrowPressed);
	Add(m_pRightArrowDisabled);

	Add(m_pScrollbarDefault);
	Add(m_pScrollbarHover);
	Add(m_pScrollbarPressed);
	Add(m_pScrollbarDisabled);

	Add(m_pBackbar);

	m_pLeftArrowDefault->SetDepth(1.5f);
	m_pLeftArrowHover->SetDepth(1.5f);
	m_pLeftArrowPressed->SetDepth(1.5f);
	m_pLeftArrowDisabled->SetDepth(1.5f);
	m_pRightArrowDefault->SetDepth(1.5f);
	m_pRightArrowHover->SetDepth(1.5f);
	m_pRightArrowPressed->SetDepth(1.5f);
	m_pRightArrowDisabled->SetDepth(1.5f);

	m_pScrollbarDefault->SetDepth(2.0f);
	m_pScrollbarHover->SetDepth(2.0f);
	m_pScrollbarPressed->SetDepth(2.0f);
	m_pScrollbarDisabled->SetDepth(2.0f);

	m_pBackbar->SetDepth(2.0f);

	m_bSelectedLeft = false;
	m_bHoverLeft = false;
	m_bSelectedRight = false;
	m_bHoverRight = false;
	m_bDisabled = false;

	m_bHoverScrollBar = false;
	m_bDraggingScrollBar = false;
	m_bFirstOutsideDrag = false;

	m_scrollPosition = 0.5f;
	m_ScrollSize = 1.0f;

	m_lastX = 0;
	m_lastY = 0;

	m_scissorEnabled = true;

	m_pArrowButtonUpdate = new CountdownTimer();
	m_pArrowButtonUpdate->SetCountdownTime(0.01f);
	m_pArrowButtonUpdate->SetLooping(true);
	m_pArrowButtonUpdate->SetCallBackFunction(_UpdateArrowButtons);
	m_pArrowButtonUpdate->SetCallBackData(this);
	m_pArrowButtonUpdate->StartCountdown();

	m_eScrollDirection = EScrollBarDirection_Horizontal;

	m_ValueChangedCallback = nullptr;
	m_pValueChangedCallbackData = nullptr;
}

ScrollBar::~ScrollBar()
{
	TimeManager::GetInstance()->RemoveCountdownTimer(m_pArrowButtonUpdate);

	delete m_pLeftArrowDefault;
	delete m_pLeftArrowHover;
	delete m_pLeftArrowPressed;
	delete m_pLeftArrowDisabled;
	delete m_pRightArrowDefault;
	delete m_pRightArrowHover;
	delete m_pRightArrowPressed;
	delete m_pRightArrowDisabled;

	delete m_pScrollbarDefault;
	delete m_pScrollbarHover;
	delete m_pScrollbarPressed;
	delete m_pScrollbarDisabled;

	delete m_pBackbar;
}

void ScrollBar::AddEventListeners()
{
	Container::AddEventListeners();

	AddMouseListener(this);
	//AddFocusListener(this);
}

void ScrollBar::RemoveEventListeners()
{
	Container::RemoveEventListeners();

	RemoveMouseListener(this);
	//RemoveFocusListener(this);

	// Remove hover and select properties, since we have made this component un-selectable by the mouse
	SetHoverRight(false);
	SetSelectedRight(false);
	SetHoverLeft(false);
	SetSelectedLeft(false);
}

void ScrollBar::RemoveIcon(RenderRectangle *pRemoveIcon)
{
	if (pRemoveIcon)
	{
		Remove(pRemoveIcon);
		delete pRemoveIcon;
		pRemoveIcon = nullptr;
	}
}

void ScrollBar::SetDefaultIcons(Renderer* pRenderer)
{
	RemoveIcon(m_pLeftArrowDefault);
	RemoveIcon(m_pLeftArrowHover);
	RemoveIcon(m_pLeftArrowPressed);
	RemoveIcon(m_pLeftArrowDisabled);
	RemoveIcon(m_pRightArrowDefault);
	RemoveIcon(m_pRightArrowHover);
	RemoveIcon(m_pRightArrowPressed);
	RemoveIcon(m_pRightArrowDisabled);

	RemoveIcon(m_pScrollbarDefault);
	RemoveIcon(m_pScrollbarHover);
	RemoveIcon(m_pScrollbarPressed);
	RemoveIcon(m_pScrollbarDisabled);

	RemoveIcon(m_pBackbar);

	m_pLeftArrowDefault = new DirectDrawRectangle(pRenderer);
	m_pLeftArrowHover = new DirectDrawRectangle(pRenderer);
	m_pLeftArrowPressed = new DirectDrawRectangle(pRenderer);
	m_pLeftArrowDisabled = new DirectDrawRectangle(pRenderer);
	m_pRightArrowDefault = new DirectDrawRectangle(pRenderer);
	m_pRightArrowHover = new DirectDrawRectangle(pRenderer);
	m_pRightArrowPressed = new DirectDrawRectangle(pRenderer);
	m_pRightArrowDisabled = new DirectDrawRectangle(pRenderer);

	m_pScrollbarDefault = new DirectDrawRectangle(pRenderer);
	m_pScrollbarHover = new DirectDrawRectangle(pRenderer);
	m_pScrollbarPressed = new DirectDrawRectangle(pRenderer);
	m_pScrollbarDisabled = new DirectDrawRectangle(pRenderer);

	m_pBackbar = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pLeftArrowDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pLeftArrowHover;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pLeftArrowPressed;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pLeftArrowDisabled;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pRightArrowDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pRightArrowHover;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pRightArrowPressed;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pRightArrowDisabled;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollbarDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollbarHover;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollbarPressed;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.25f, 0.25f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pScrollbarDisabled;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pBackbar;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	Add(m_pLeftArrowDefault);
	Add(m_pLeftArrowHover);
	Add(m_pLeftArrowPressed);
	Add(m_pLeftArrowDisabled);
	Add(m_pRightArrowDefault);
	Add(m_pRightArrowHover);
	Add(m_pRightArrowPressed);
	Add(m_pRightArrowDisabled);

	Add(m_pScrollbarDefault);
	Add(m_pScrollbarHover);
	Add(m_pScrollbarPressed);
	Add(m_pScrollbarDisabled);

	Add(m_pBackbar);

	Dimensions d = GetDimensions();
	if (m_eScrollDirection == EScrollBarDirection_Horizontal)
	{
		// TEMP : For the time being, make all arrow graphics square...
		m_pLeftArrowDefault->SetDimensions(0, 0, d.m_height, d.m_height);
		m_pLeftArrowHover->SetDimensions(0, 0, d.m_height, d.m_height);
		m_pLeftArrowPressed->SetDimensions(0, 0, d.m_height, d.m_height);
		m_pLeftArrowDisabled->SetDimensions(0, 0, d.m_height, d.m_height);
		m_pRightArrowDefault->SetDimensions(d.m_width - d.m_height, 0, d.m_height, d.m_height);
		m_pRightArrowHover->SetDimensions(d.m_width - d.m_height, 0, d.m_height, d.m_height);
		m_pRightArrowPressed->SetDimensions(d.m_width - d.m_height, 0, d.m_height, d.m_height);
		m_pRightArrowDisabled->SetDimensions(d.m_width - d.m_height, 0, d.m_height, d.m_height);
	}
	else //m_eScrollDirection == EScrollBarDirection_Vertical
	{
		// TEMP : For the time being, make all arrow graphics square...
		m_pLeftArrowDefault->SetDimensions(0, d.m_height - d.m_width, d.m_width, d.m_width);
		m_pLeftArrowHover->SetDimensions(0, d.m_height - d.m_width, d.m_width, d.m_width);
		m_pLeftArrowPressed->SetDimensions(0, d.m_height - d.m_width, d.m_width, d.m_width);
		m_pLeftArrowDisabled->SetDimensions(0, d.m_height - d.m_width, d.m_width, d.m_width);
		m_pRightArrowDefault->SetDimensions(0, 0, d.m_width, d.m_width);
		m_pRightArrowHover->SetDimensions(0, 0, d.m_width, d.m_width);
		m_pRightArrowPressed->SetDimensions(0, 0, d.m_width, d.m_width);
		m_pRightArrowDisabled->SetDimensions(0, 0, d.m_width, d.m_width);
	}
}

void ScrollBar::SetLeftArrowDefaultIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pLeftArrowDefault)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pLeftArrowDefault);

		delete m_pLeftArrowDefault;
		m_pLeftArrowDefault = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pLeftArrowDefault = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pLeftArrowDefault = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pLeftArrowDefault);

	Dimensions dimensions = GetDimensions();
	SetDimensions(dimensions.m_x, dimensions.m_y, dimensions.m_width, dimensions.m_height);
}

void ScrollBar::SetLeftArrowHoverIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pLeftArrowHover)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pLeftArrowHover);

		delete m_pLeftArrowHover;
		m_pLeftArrowHover = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pLeftArrowHover = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pLeftArrowHover = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pLeftArrowHover);

	Dimensions dimensions = GetDimensions();
	SetDimensions(dimensions.m_x, dimensions.m_y, dimensions.m_width, dimensions.m_height);
}

void ScrollBar::SetLeftArrowSelectedIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pLeftArrowPressed)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pLeftArrowPressed);

		delete m_pLeftArrowPressed;
		m_pLeftArrowPressed = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pLeftArrowPressed = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pLeftArrowPressed = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pLeftArrowPressed);

	Dimensions dimensions = GetDimensions();
	SetDimensions(dimensions.m_x, dimensions.m_y, dimensions.m_width, dimensions.m_height);
}

void ScrollBar::SetLeftArrowDisabledIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pLeftArrowDisabled)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pLeftArrowDisabled);

		delete m_pLeftArrowDisabled;
		m_pLeftArrowDisabled = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pLeftArrowDisabled = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pLeftArrowDisabled = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pLeftArrowDisabled);

	Dimensions dimensions = GetDimensions();
	SetDimensions(dimensions.m_x, dimensions.m_y, dimensions.m_width, dimensions.m_height);
}

void ScrollBar::SetRightArrowDefaultIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pRightArrowDefault)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pRightArrowDefault);

		delete m_pRightArrowDefault;
		m_pRightArrowDefault = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pRightArrowDefault = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pRightArrowDefault = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pRightArrowDefault);

	Dimensions dimensions = GetDimensions();
	SetDimensions(dimensions.m_x, dimensions.m_y, dimensions.m_width, dimensions.m_height);
}

void ScrollBar::SetRightArrowHoverIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pRightArrowHover)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pRightArrowHover);

		delete m_pRightArrowHover;
		m_pRightArrowHover = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pRightArrowHover = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pRightArrowHover = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pRightArrowHover);

	Dimensions dimensions = GetDimensions();
	SetDimensions(dimensions.m_x, dimensions.m_y, dimensions.m_width, dimensions.m_height);
}

void ScrollBar::SetRightArrowSelectedIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pRightArrowPressed)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pRightArrowPressed);

		delete m_pRightArrowPressed;
		m_pRightArrowPressed = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pRightArrowPressed = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pRightArrowPressed = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pRightArrowPressed);

	Dimensions dimensions = GetDimensions();
	SetDimensions(dimensions.m_x, dimensions.m_y, dimensions.m_width, dimensions.m_height);
}

void ScrollBar::SetRightArrowDisabledIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pRightArrowDisabled)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pRightArrowDisabled);

		delete m_pRightArrowDisabled;
		m_pRightArrowDisabled = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pRightArrowDisabled = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pRightArrowDisabled = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pRightArrowDisabled);

	Dimensions dimensions = GetDimensions();
	SetDimensions(dimensions.m_x, dimensions.m_y, dimensions.m_width, dimensions.m_height);
}

void ScrollBar::SetScrollbarDefaultIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollbarDefault)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollbarDefault);

		delete m_pScrollbarDefault;
		m_pScrollbarDefault = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollbarDefault = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollbarDefault = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollbarDefault);

	Dimensions dimensions = GetDimensions();
	SetDimensions(dimensions.m_x, dimensions.m_y, dimensions.m_width, dimensions.m_height);
}

void ScrollBar::SetScrollbarHoverIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollbarHover)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollbarHover);

		delete m_pScrollbarHover;
		m_pScrollbarHover = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollbarHover = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollbarHover = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollbarHover);
}

void ScrollBar::SetScrollbarSelectedIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollbarPressed)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollbarPressed);

		delete m_pScrollbarPressed;
		m_pScrollbarPressed = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollbarPressed = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollbarPressed = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollbarPressed);
}

void ScrollBar::SetScrollbarDisabledIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pScrollbarDisabled)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pScrollbarDisabled);

		delete m_pScrollbarDisabled;
		m_pScrollbarDisabled = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pScrollbarDisabled = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pScrollbarDisabled = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pScrollbarDisabled);
}

void ScrollBar::SetBackbarIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pBackbar)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pBackbar);

		delete m_pBackbar;
		m_pBackbar = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pBackbar = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pBackbar = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pBackbar);
}

void ScrollBar::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);

	// Also set the dimensions of the icons
	if(m_eScrollDirection == EScrollBarDirection_Horizontal)
	{
		// TEMP : For the time being, make all arrow graphics square...
		m_pLeftArrowDefault->SetDimensions(0, 0, height, height);
		m_pLeftArrowHover->SetDimensions(0, 0, height, height);
		m_pLeftArrowPressed->SetDimensions(0, 0, height, height);
		m_pLeftArrowDisabled->SetDimensions(0, 0, height, height);
		m_pRightArrowDefault->SetDimensions(width-height, 0, height, height);
		m_pRightArrowHover->SetDimensions(width-height, 0, height, height);
		m_pRightArrowPressed->SetDimensions(width-height, 0, height, height);
		m_pRightArrowDisabled->SetDimensions(width-height, 0, height, height);
	}
	else //m_eScrollDirection == EScrollBarDirection_Vertical
	{
		// TEMP : For the time being, make all arrow graphics square...
		m_pLeftArrowDefault->SetDimensions(0, height-width, width, width);
		m_pLeftArrowHover->SetDimensions(0, height-width, width, width);
		m_pLeftArrowPressed->SetDimensions(0, height-width, width, width);
		m_pLeftArrowDisabled->SetDimensions(0, height-width, width, width);
		m_pRightArrowDefault->SetDimensions(0, 0, width, width);
		m_pRightArrowHover->SetDimensions(0, 0, width, width);
		m_pRightArrowPressed->SetDimensions(0, 0, width, width);
		m_pRightArrowDisabled->SetDimensions(0, 0, width, width);
	}
}

void ScrollBar::SetArrowDimensions(int width, int height)
{
	Dimensions lLeftDimensions = m_pLeftArrowDefault->GetDimensions();
	Dimensions lRightDimensions = m_pRightArrowDefault->GetDimensions();

	// TEMP : For the time being, make all arrow graphics square...
	m_pLeftArrowDefault->SetDimensions(lLeftDimensions.m_x, lLeftDimensions.m_y, width, height);
	m_pLeftArrowHover->SetDimensions(lLeftDimensions.m_x, lLeftDimensions.m_y, width, height);
	m_pLeftArrowPressed->SetDimensions(lLeftDimensions.m_x, lLeftDimensions.m_y, width, height);
	m_pLeftArrowDisabled->SetDimensions(lLeftDimensions.m_x, lLeftDimensions.m_y, width, height);
	m_pRightArrowDefault->SetDimensions(lRightDimensions.m_x, lRightDimensions.m_y, width, height);
	m_pRightArrowHover->SetDimensions(lRightDimensions.m_x, lRightDimensions.m_y, width, height);
	m_pRightArrowPressed->SetDimensions(lRightDimensions.m_x, lRightDimensions.m_y, width, height);
	m_pRightArrowDisabled->SetDimensions(lRightDimensions.m_x, lRightDimensions.m_y, width, height);
}

void ScrollBar::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void ScrollBar::AddScrollAreaItem(Component* component)
{
	component->SetParent(this);

	component->AddEventListeners();

	component->SetAudio(m_audio);
	component->SetAudioVolume(m_audioVolume);

	m_vpScrollAreaComponentList.push_back(component);
}

void ScrollBar::ClearScrollAreaItems()
{
	ComponentList::const_iterator iterator;
	for(iterator = m_vpScrollAreaComponentList.begin(); iterator != m_vpScrollAreaComponentList.end(); ++iterator)
	{
		Component* pComponent = (Component*)(*iterator);

		pComponent->RemoveEventListeners();
	}

	m_vpScrollAreaComponentList.clear();
}

void ScrollBar::SetScrollArea(int x, int y, int width, int height)
{
	m_scrollArea.m_x = x;
	m_scrollArea.m_y = y;
	m_scrollArea.m_width = width;
	m_scrollArea.m_height = height;
}

Dimensions ScrollBar::GetScrollArea()
{
	return m_scrollArea;
}

void ScrollBar::SetScrollPosition(float lPosition)
{
	m_scrollPosition = lPosition;

	if(m_scrollPosition < 0.0f)
	{
		m_scrollPosition = 0.0f;
	}

	if(m_scrollPosition > 1.0f)
	{
		m_scrollPosition = 1.0f;
	}
}

float ScrollBar::GetScrollPosition() const
{
	return m_scrollPosition;
}

void ScrollBar::SetScrollSize(float lSize)
{
	m_ScrollSize = lSize;

	if(m_ScrollSize < 0.0f)
	{
		m_ScrollSize = 0.0f;
	}

	if(m_ScrollSize > 1.0f)
	{
		m_ScrollSize = 1.0f;
	}

	ClampScrollBarPosition();

	UpdateScrollbarDimension();
}

float ScrollBar::GetScrollSize()
{
	return m_ScrollSize;
}

float ScrollBar::GetScrollRatio()
{
	float scrollBorder = m_ScrollSize * 0.5f;
	float scrollRatio = 1.0f - ((m_scrollPosition - scrollBorder) / (1.0f - m_ScrollSize));

	if(m_ScrollSize <= 0.0f)
		return 0.0f;

	if(m_ScrollSize >= 1.0f)
		return 1.0f;

	if(scrollRatio < 0.0f)
		scrollRatio = 0.0f;

	if(scrollRatio > 1.0f)
		scrollRatio = 1.0f;

	return scrollRatio;
}

void ScrollBar::UpdateScrollbarDimension()
{
	int l_containerWidth = m_dimensions.m_width;
	int l_containerHeight = m_dimensions.m_height;
	float l_depth = GetDepth();

	float l_outlineX1 = 0.5f;
	float l_outlineX2 = l_containerWidth + 0.5f;
	float l_outlineY1 = 0.5f;
	float l_outlineY2 = l_containerHeight + 0.5f;

	int l_scrollX1;
	int l_scrollX2;
	int l_scrollY1;
	int l_scrollY2;

	if(m_eScrollDirection == EScrollBarDirection_Horizontal)
	{
		int l_TotalScrollWidth = m_dimensions.m_width - (m_dimensions.m_height * 2);
		int l_ScrollbarWidth = static_cast<int>(l_TotalScrollWidth * m_ScrollSize);
		int l_ScrollbarPosition = static_cast<int>(l_TotalScrollWidth * m_scrollPosition);
		l_scrollX1 = m_dimensions.m_height + l_ScrollbarPosition - (l_ScrollbarWidth / 2);
		l_scrollX2 = m_dimensions.m_height + l_ScrollbarPosition + (l_ScrollbarWidth / 2);
		l_scrollY1 = 0;
		l_scrollY2 = m_dimensions.m_height;

		m_pScrollbarDefault->SetDimensions(l_scrollX1, l_scrollY1, l_ScrollbarWidth, m_dimensions.m_height);
		m_pScrollbarHover->SetDimensions(l_scrollX1, l_scrollY1, l_ScrollbarWidth, m_dimensions.m_height);
		m_pScrollbarPressed->SetDimensions(l_scrollX1, l_scrollY1, l_ScrollbarWidth, m_dimensions.m_height);
		m_pScrollbarDisabled->SetDimensions(l_scrollX1, l_scrollY1, l_ScrollbarWidth, m_dimensions.m_height);

		m_pBackbar->SetDimensions(m_dimensions.m_height, 0, l_TotalScrollWidth, m_dimensions.m_height);
	}
	else //m_eScrollDirection == EScrollBarDirection_Horizontal
	{
		int l_TotalScrollHeight = m_dimensions.m_height - (m_dimensions.m_width * 2);
		int l_ScrollbarHeight = static_cast<int>(l_TotalScrollHeight * m_ScrollSize);
		int l_ScrollbarPosition = static_cast<int>(l_TotalScrollHeight * m_scrollPosition);
		l_scrollX1 = 0;
		l_scrollX2 = m_dimensions.m_width;
		l_scrollY1 = m_dimensions.m_width + l_ScrollbarPosition - (l_ScrollbarHeight / 2);
		l_scrollY2 = m_dimensions.m_width + l_ScrollbarPosition + (l_ScrollbarHeight / 2);

		m_pScrollbarDefault->SetDimensions(l_scrollX1, l_scrollY1, m_dimensions.m_width, l_ScrollbarHeight);
		m_pScrollbarHover->SetDimensions(l_scrollX1, l_scrollY1, m_dimensions.m_width, l_ScrollbarHeight);
		m_pScrollbarPressed->SetDimensions(l_scrollX1, l_scrollY1, m_dimensions.m_width, l_ScrollbarHeight);
		m_pScrollbarDisabled->SetDimensions(l_scrollX1, l_scrollY1, m_dimensions.m_width, l_ScrollbarHeight);

		m_pBackbar->SetDimensions(0, m_dimensions.m_width, m_dimensions.m_width, l_TotalScrollHeight);
	}
}

void ScrollBar::ClampScrollBarPosition()
{
	float lMinPos = m_ScrollSize / 2;
	float lMaxPos = 1.0f - (m_ScrollSize / 2);

	if(m_scrollPosition < lMinPos)
	{
		m_scrollPosition = lMinPos;
	}

	if(m_scrollPosition > lMaxPos)
	{
		m_scrollPosition = lMaxPos;
	}
}

void ScrollBar::SetScissorEnabled(bool scissor)
{
	m_scissorEnabled = scissor;
}

EScrollBarDirection ScrollBar::GetScrollDirection()
{
	return m_eScrollDirection;
}

void ScrollBar::SetScrollDirection(EScrollBarDirection lDirection)
{
	m_eScrollDirection = lDirection;

	// Force a re-setup of the dimensions, since we have changed the orientation
	SetDimensions(m_dimensions);
}

void ScrollBar::SetSelectedRight(bool val)
{
	m_bSelectedRight = val;
}

void ScrollBar::SetHoverRight(bool val)
{
	m_bHoverRight = val;
}

void ScrollBar::SetSelectedLeft(bool val)
{
	m_bSelectedLeft = val;
}

void ScrollBar::SetHoverLeft(bool val)
{
	m_bHoverLeft = val;
}

void ScrollBar::SetDisabled(bool val)
{
	m_bDisabled = val;
}

bool ScrollBar::IsSelectedRight() const
{
	return m_bSelectedRight;
}

bool ScrollBar::IsHoverRight() const
{
	return m_bHoverRight;
}

bool ScrollBar::IsSelectedLeft() const
{
	return m_bSelectedLeft;
}

bool ScrollBar::IsHoverLeft() const
{
	return m_bHoverLeft;
}

bool ScrollBar::IsDisabled() const
{
	return m_bDisabled;
}

EComponentType ScrollBar::GetComponentType() const
{
	return EComponentType_ScrollBar;
}

void ScrollBar::SetValueChangedCallback(FunctionCallback lFunction)
{
	m_ValueChangedCallback = lFunction;
}

void ScrollBar::SetValueChangedCallBackData(void *lpData)
{
	m_pValueChangedCallbackData = lpData;
}

void ScrollBar::SetAudio(bool set)
{
	Container::SetAudio(set);

	ComponentList::const_iterator iterator;
	for(iterator = m_vpScrollAreaComponentList.begin(); iterator != m_vpScrollAreaComponentList.end(); ++iterator)
	{
		(*iterator)->SetAudio(set);
	}
}

void ScrollBar::SetAudioVolume(float volume)
{
	Container::SetAudioVolume(volume);

	ComponentList::const_iterator iterator;
	for(iterator = m_vpScrollAreaComponentList.begin(); iterator != m_vpScrollAreaComponentList.end(); ++iterator)
	{
		(*iterator)->SetAudioVolume(volume);
	}
}

void ScrollBar::MouseEntered(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	m_bFirstOutsideDrag = false;

	OnMouseEnter();
}

void ScrollBar::MouseExited(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetHoverRight(false);
	SetHoverLeft(false);

	SetSelectedRight(false);
	SetSelectedLeft(false);

	m_bHoverScrollBar = false;

	OnMouseExit();
}

void ScrollBar::MousePressed(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	int lLeftX1;
	int lLeftX2;
	int lLeftY1;
	int lLeftY2;

	int lRightX1;
	int lRightX2;
	int lRightY1;
	int lRightY2;

	int l_scrollX1;
	int l_scrollX2;
	int l_scrollY1;
	int l_scrollY2;

	if(m_eScrollDirection == EScrollBarDirection_Horizontal)
	{
		lLeftX1 = GetLocationOnScreen().m_x;
		lLeftX2 = GetLocationOnScreen().m_x + GetDimensions().m_height;
		lLeftY1 = GetLocationOnScreen().m_y;
		lLeftY2 = GetLocationOnScreen().m_y + GetDimensions().m_height;

		lRightX1 = GetLocationOnScreen().m_x + GetDimensions().m_width - GetDimensions().m_height;
		lRightX2 = GetLocationOnScreen().m_x + GetDimensions().m_width;
		lRightY1 = GetLocationOnScreen().m_y;
		lRightY2 = GetLocationOnScreen().m_y + GetDimensions().m_height;

		int l_TotalScrollWidth = m_dimensions.m_width - (m_dimensions.m_height * 2);
		int l_ScrollbarWidth = static_cast<int>(l_TotalScrollWidth * m_ScrollSize);
		int l_ScrollbarPosition = static_cast<int>(l_TotalScrollWidth * m_scrollPosition);
		l_scrollX1 = GetLocationOnScreen().m_x + m_dimensions.m_height + l_ScrollbarPosition - (l_ScrollbarWidth / 2);
		l_scrollX2 = GetLocationOnScreen().m_x + m_dimensions.m_height + l_ScrollbarPosition + (l_ScrollbarWidth / 2);
		l_scrollY1 = GetLocationOnScreen().m_y;
		l_scrollY2 = GetLocationOnScreen().m_y + m_dimensions.m_height;
	}
	else //m_eScrollDirection == EScrollBarDirection_Vertical
	{
		// Remember, this is really the top arrow
		lLeftX1 = GetLocationOnScreen().m_x;
		lLeftX2 = GetLocationOnScreen().m_x + GetDimensions().m_width;
		lLeftY1 = GetLocationOnScreen().m_y + GetDimensions().m_height - GetDimensions().m_width;
		lLeftY2 = GetLocationOnScreen().m_y + GetDimensions().m_height;

		// Remember, this is really the bottom arrow
		lRightX1 = GetLocationOnScreen().m_x;
		lRightX2 = GetLocationOnScreen().m_x + GetDimensions().m_width;
		lRightY1 = GetLocationOnScreen().m_y;
		lRightY2 = GetLocationOnScreen().m_y + GetDimensions().m_width;

		int l_TotalScrollHeight = m_dimensions.m_height - (m_dimensions.m_width * 2);
		int l_ScrollbarHeight = static_cast<int>(l_TotalScrollHeight * m_ScrollSize);
		int l_ScrollbarPosition = static_cast<int>(l_TotalScrollHeight * m_scrollPosition);
		l_scrollX1 = GetLocationOnScreen().m_x;
		l_scrollX2 = GetLocationOnScreen().m_x + m_dimensions.m_width;
		l_scrollY1 = GetLocationOnScreen().m_y + m_dimensions.m_width + l_ScrollbarPosition - (l_ScrollbarHeight / 2);
		l_scrollY2 = GetLocationOnScreen().m_y + m_dimensions.m_width + l_ScrollbarPosition + (l_ScrollbarHeight / 2);
	}

	// Check for arrow buttons
	if(lEvent.GetX() >= lLeftX1 && lEvent.GetX() < lLeftX2 && lEvent.GetY() >= lLeftY1 && lEvent.GetY() < lLeftY2)
	{
		SetSelectedLeft(true);
		UpdateArrowButtons();
	}
	if(lEvent.GetX() >= lRightX1 && lEvent.GetX() < lRightX2 && lEvent.GetY() >= lRightY1 && lEvent.GetY() < lRightY2)
	{
		SetSelectedRight(true);
		UpdateArrowButtons();
	}

	// Check for scrollbar
	if(lEvent.GetX() >= l_scrollX1 && lEvent.GetX() < l_scrollX2 && lEvent.GetY() >= l_scrollY1 && lEvent.GetY() < l_scrollY2)
	{
		m_bDraggingScrollBar = true;
	}
	else
	{
		m_bDraggingScrollBar = false;
	}

	ClampScrollBarPosition();

	// If our parent is a GUIWindow, then make this window have focus in the GUI, used to make it's depth the highest
	/*
	if(GetParent() != nullptr && GetParent()->GetComponentType() == EComponentType_GUIWindow)
	{
		GUIWindow* lpParentWindow = (GUIWindow *)GetParent();
		lpParentWindow->SetFocusWindow();
	}
	*/

	OnMousePressed();
}

void ScrollBar::MouseReleased(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	SetSelectedRight(false);
	SetSelectedLeft(false);

	m_bDraggingScrollBar = false;

	OnMouseReleased();
}

void ScrollBar::MouseReleasedOutside(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	m_bDraggingScrollBar = false;
}

void ScrollBar::MouseClicked(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	OnMouseClicked();
}

void ScrollBar::MouseDragged(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	if(m_bDraggingScrollBar && !m_bHoverRight && !m_bHoverLeft)
	{
		int l_ChangeX = lEvent.GetX();
		int l_ChangeY = lEvent.GetY();

		int lMinX = m_dimensions.m_x + m_dimensions.m_height;				// Used for horizontal scrollbars
		int lMaxX = lMinX + m_dimensions.m_width - m_dimensions.m_height*2; // used for horizontal scrollbars
		int lMinY = m_dimensions.m_y + m_dimensions.m_width;				// Used for vertical scrollbars
		int lMaxY = lMinY + m_dimensions.m_height - m_dimensions.m_width*2;	// used for vertical scrollbars

		if(l_ChangeX == 0 && l_ChangeY == 0)
			return;

		float lValueChange = 0.0f;
		if(m_ScrollSize < 1.0f)
		{
			if(m_eScrollDirection == EScrollBarDirection_Horizontal)
			{
				int l_TotalScrollWidth = m_dimensions.m_width - (m_dimensions.m_height * 2);
				int l_ScrollbarWidth = static_cast<int>(l_TotalScrollWidth * m_ScrollSize);

				lValueChange = static_cast<float>(l_ChangeX) * (1.0f - m_ScrollSize) / (static_cast<float>(l_TotalScrollWidth - l_ScrollbarWidth));
			}
			else //m_eScrollDirection == EScrollBarDirection_Vertical
			{
				int l_TotalScrollHeight = m_dimensions.m_height - (m_dimensions.m_width * 2);
				int l_ScrollbarHeight = static_cast<int>(l_TotalScrollHeight * m_ScrollSize);
				lValueChange = static_cast<float>(l_ChangeY) * (1.0f - m_ScrollSize) / (static_cast<float>(l_TotalScrollHeight - l_ScrollbarHeight));
			}
		}		

		// Store the value before we change it, to see if we have actually changed the value
		float lValueBefore = m_scrollPosition;

		m_scrollPosition += lValueChange;

		ClampScrollBarPosition();

		if(m_scrollPosition != lValueBefore)
		{
			OnValueChanged();

			if(m_ValueChangedCallback)
			{
				m_ValueChangedCallback(m_pValueChangedCallbackData);
			}
		}
	}
}

void ScrollBar::MouseDraggedOutside(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	if(!m_bFirstOutsideDrag)
	{
		m_lastX = lEvent.GetX();
		m_lastY = lEvent.GetY();
		m_bFirstOutsideDrag = true;
	}

	if(m_bDraggingScrollBar)
	{
		int lNewX = lEvent.GetX();
		int lNewY = lEvent.GetY();

		int lMinX = GetLocationOnScreen().m_x + m_dimensions.m_height;				// Used for horizontal scrollbars
		int lMaxX = lMinX + m_dimensions.m_width - m_dimensions.m_height*2; // used for horizontal scrollbars
		int lMinY = GetLocationOnScreen().m_y + m_dimensions.m_width;				// Used for vertical scrollbars
		int lMaxY = lMinY + m_dimensions.m_height - m_dimensions.m_width*2;	// used for vertical scrollbars

		int l_ChangeX = lNewX - m_lastX;
		int l_ChangeY = lNewY - m_lastY;

		if(l_ChangeX == 0 && l_ChangeY == 0)
			return;

		float lValueChange = 0.0f;
		if(m_ScrollSize < 1.0f)
		{
			if(m_eScrollDirection == EScrollBarDirection_Horizontal)
			{
				int l_TotalScrollWidth = m_dimensions.m_width - (m_dimensions.m_height * 2);
				int l_ScrollbarWidth = static_cast<int>(l_TotalScrollWidth * m_ScrollSize);

				lValueChange = static_cast<float>(l_ChangeX) * (1.0f - m_ScrollSize) / (static_cast<float>(l_TotalScrollWidth - l_ScrollbarWidth));
			}
			else //m_eScrollDirection == EScrollBarDirection_Vertical
			{
				int l_TotalScrollHeight = m_dimensions.m_height - (m_dimensions.m_width * 2);
				int l_ScrollbarHeight = static_cast<int>(l_TotalScrollHeight * m_ScrollSize);
				lValueChange = static_cast<float>(l_ChangeY) * (1.0f - m_ScrollSize) / (static_cast<float>(l_TotalScrollHeight - l_ScrollbarHeight));
			}
		}		

		// Store the value before we change it, to see if we have actually changed the value
		float lValueBefore = m_scrollPosition;

		if(m_eScrollDirection == EScrollBarDirection_Horizontal)
		{
			if( (lValueChange > 0) && (lNewX > lMinX))
			{
				m_scrollPosition += lValueChange;
			}
			else if((lValueChange < 0) && (lNewX < lMaxX))
			{
				m_scrollPosition += lValueChange;
			}
		}
		else //m_eScrollDirection == EScrollBarDirection_Vertical
		{
			if( (lValueChange > 0) && (lNewY > lMinY))
			{
				m_scrollPosition += lValueChange;
			}
			else if((lValueChange < 0) && (lNewY < lMaxY))
			{
				m_scrollPosition += lValueChange;
			}
		}

		ClampScrollBarPosition();

		if(m_scrollPosition != lValueBefore)
		{
			OnValueChanged();

			if(m_ValueChangedCallback)
			{
				m_ValueChangedCallback(m_pValueChangedCallbackData);
			}
		}

		m_lastX = lNewX;
		m_lastY = lNewY;
	}
}

void ScrollBar::MouseMotion(const MouseEvent& lEvent)
{
	if(IsDisabled())
	{
		return;
	}

	int lLeftX1;
	int lLeftX2;
	int lLeftY1;
	int lLeftY2;

	int lRightX1;
	int lRightX2;
	int lRightY1;
	int lRightY2;

	int l_scrollX1;
	int l_scrollX2;
	int l_scrollY1;
	int l_scrollY2;

	if(m_eScrollDirection == EScrollBarDirection_Horizontal)
	{
		lLeftX1 = GetLocationOnScreen().m_x;
		lLeftX2 = GetLocationOnScreen().m_x + GetDimensions().m_height;
		lLeftY1 = GetLocationOnScreen().m_y;
		lLeftY2 = GetLocationOnScreen().m_y + GetDimensions().m_height;

		lRightX1 = GetLocationOnScreen().m_x + GetDimensions().m_width - GetDimensions().m_height;
		lRightX2 = GetLocationOnScreen().m_x + GetDimensions().m_width;
		lRightY1 = GetLocationOnScreen().m_y;
		lRightY2 = GetLocationOnScreen().m_y + GetDimensions().m_height;

		int l_TotalScrollWidth = m_dimensions.m_width - (m_dimensions.m_height * 2);
		int l_ScrollbarWidth = static_cast<int>(l_TotalScrollWidth * m_ScrollSize);
		int l_ScrollbarPosition = static_cast<int>(l_TotalScrollWidth * m_scrollPosition);
		l_scrollX1 = GetLocationOnScreen().m_x + m_dimensions.m_height + l_ScrollbarPosition - (l_ScrollbarWidth / 2);
		l_scrollX2 = GetLocationOnScreen().m_x + m_dimensions.m_height + l_ScrollbarPosition + (l_ScrollbarWidth / 2);
		l_scrollY1 = GetLocationOnScreen().m_y;
		l_scrollY2 = GetLocationOnScreen().m_y + m_dimensions.m_height;
	}
	else //m_eScrollDirection == EScrollBarDirection_Vertical
	{
		// Remember, this is really the top arrow
		lLeftX1 = GetLocationOnScreen().m_x;
		lLeftX2 = GetLocationOnScreen().m_x + GetDimensions().m_width;
		lLeftY1 = GetLocationOnScreen().m_y + GetDimensions().m_height - GetDimensions().m_width;
		lLeftY2 = GetLocationOnScreen().m_y + GetDimensions().m_height;

		// Remember, this is really the bottom arrow
		lRightX1 = GetLocationOnScreen().m_x;
		lRightX2 = GetLocationOnScreen().m_x + GetDimensions().m_width;
		lRightY1 = GetLocationOnScreen().m_y;
		lRightY2 = GetLocationOnScreen().m_y + GetDimensions().m_width;

		int l_TotalScrollHeight = m_dimensions.m_height - (m_dimensions.m_width * 2);
		int l_ScrollbarHeight = static_cast<int>(l_TotalScrollHeight * m_ScrollSize);
		int l_ScrollbarPosition = static_cast<int>(l_TotalScrollHeight * m_scrollPosition);
		l_scrollX1 = GetLocationOnScreen().m_x;
		l_scrollX2 = GetLocationOnScreen().m_x + m_dimensions.m_width;
		l_scrollY1 = GetLocationOnScreen().m_y + m_dimensions.m_width + l_ScrollbarPosition - (l_ScrollbarHeight / 2);
		l_scrollY2 = GetLocationOnScreen().m_y + m_dimensions.m_width + l_ScrollbarPosition + (l_ScrollbarHeight / 2);
	}

	// Check for arrow buttons
	if(lEvent.GetX() >= lLeftX1 && lEvent.GetX() < lLeftX2 && lEvent.GetY() >= lLeftY1 && lEvent.GetY() < lLeftY2)
	{
		SetHoverLeft(true);
	}
	else
	{
		SetHoverLeft(false);
		SetSelectedLeft(false);
	}
	if(lEvent.GetX() >= lRightX1 && lEvent.GetX() < lRightX2 && lEvent.GetY() >= lRightY1 && lEvent.GetY() < lRightY2)
	{
		SetHoverRight(true);
	}
	else
	{
		SetHoverRight(false);
		SetSelectedRight(false);
	}

	// Check for scrollbar
	if(lEvent.GetX() >= l_scrollX1 && lEvent.GetX() < l_scrollX2 && lEvent.GetY() >= l_scrollY1 && lEvent.GetY() < l_scrollY2)
	{
		m_bHoverScrollBar = true;
	}
	else
	{
		m_bHoverScrollBar = false;
	}

	OnMouseMotion();
}

/*
void ScrollBar::FocusLost(const FocusEvent& lEvent)
{
	PulldownMenu* lpParentPulldownMenu = (PulldownMenu* )GetParent();

	if(lpParentPulldownMenu && lpParentPulldownMenu->GetComponentType() == EComponentType_PulldownMenu)
	{
		if(lpParentPulldownMenu->MenuChildHasFocus())
		{
		}
		else
		{
			lpParentPulldownMenu->CloseMenu();
		}		
	}
}

void ScrollBar::FocusGained(const FocusEvent& lEvent)
{
	PulldownMenu* lpParentPulldownMenu = (PulldownMenu* )GetParent();

	if(lpParentPulldownMenu && lpParentPulldownMenu->GetComponentType() == EComponentType_PulldownMenu)
	{
		if(!lpParentPulldownMenu->IsMenuOpen())
		{
			FocusManager::GetInstance()->SetFocusOwner(0);

			return;
		}
	}
}
*/
void ScrollBar::OnMouseEnter()
{
	/* Nothing */
}

void ScrollBar::OnMouseExit()
{
	/* Nothing */
}

void ScrollBar::OnMousePressed()
{
	/* Nothing */
}

void ScrollBar::OnMouseReleased()
{
	/* Nothing */
}

void ScrollBar::OnMouseClicked()
{
	/* Nothing */
}

void ScrollBar::OnMouseMotion()
{
	/* Nothing */
}

void ScrollBar::OnValueChanged()
{
	/* Nothing */
}

void ScrollBar::DrawSelf()
{
	// We need to update some scrollbar params and dimensions
	ClampScrollBarPosition();

	UpdateScrollbarDimension();

	// Reset arrow icon visibility
	m_pLeftArrowDefault->SetVisible(false);
	m_pLeftArrowHover->SetVisible(false);
	m_pLeftArrowPressed->SetVisible(false);
	m_pLeftArrowDisabled->SetVisible(false);
	m_pRightArrowDefault->SetVisible(false);
	m_pRightArrowHover->SetVisible(false);
	m_pRightArrowPressed->SetVisible(false);
	m_pRightArrowDisabled->SetVisible(false);

	if(IsSelectedRight())
	{
		m_pRightArrowPressed->SetVisible(true);
	}
	else if(IsHoverRight())
	{
		m_pRightArrowHover->SetVisible(true);
	}
	else if(IsDisabled())
	{
		m_pRightArrowDisabled->SetVisible(true);
	}
	else
	{
		m_pRightArrowDefault->SetVisible(true);
	}

	if(IsSelectedLeft())
	{
		m_pLeftArrowPressed->SetVisible(true);
	}
	else if(IsHoverLeft())
	{
		m_pLeftArrowHover->SetVisible(true);
	}
	else if(IsDisabled())
	{
		m_pLeftArrowDisabled->SetVisible(true);
	}
	else
	{
		m_pLeftArrowDefault->SetVisible(true);
	}

	// Scrollbar visibility
	m_pScrollbarDefault->SetVisible(false);
	m_pScrollbarHover->SetVisible(false);
	m_pScrollbarPressed->SetVisible(false);
	m_pScrollbarDisabled->SetVisible(false);

	m_pBackbar->SetVisible(true);

	if(m_bDraggingScrollBar)
	{
		m_pScrollbarPressed->SetVisible(true);
	}
	else if(m_bHoverScrollBar)
	{
		m_pScrollbarHover->SetVisible(true);
	}
	else if(IsDisabled())
	{
		m_pScrollbarDisabled->SetVisible(true);
	}
	else
	{
		m_pScrollbarDefault->SetVisible(true);
	}
	
	/*
	m_pRenderer->PushMatrix();
		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
		if(m_bDraggingScrollBar)
		{
			m_pRenderer->ImmediateColourAlpha(1.0f, 0.0f, 0.0f, 1.0f);
		}
		else if(m_bHoverScrollBar)
		{
			m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 0.0f, 1.0f);
		}
		else if(IsDisabled())
		{
			m_pRenderer->ImmediateColourAlpha(0.75f, 0.75f, 0.75f, 1.0f);
		}
		else
		{
			m_pRenderer->ImmediateColourAlpha(0.0f, 0.0f, 1.0f, 1.0f);
		}		
		m_pRenderer->ImmediateVertex(l_scrollX1, l_scrollY1, l_depth);
		m_pRenderer->ImmediateVertex(l_scrollX2, l_scrollY1, l_depth);
		m_pRenderer->ImmediateVertex(l_scrollX2, l_scrollY2, l_depth);
		m_pRenderer->ImmediateVertex(l_scrollX1, l_scrollY2, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();

	// Draw the outline
	m_pRenderer->PushMatrix();
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->ImmediateColourAlpha(0.25f, 0.25f, 0.25f, 1.0f);
		m_pRenderer->EnableImmediateMode(IM_LINE_LOOP);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2 + 1, l_outlineY2, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY2, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
	*/
}

void ScrollBar::DrawChildren()
{
	Container::DrawChildren();

	/*
	// Render the scroll area
	int l_outlineX1 = m_scrollArea.m_x;
	int l_outlineX2 = m_scrollArea.m_x + m_scrollArea.m_width;
	int l_outlineY1 = m_scrollArea.m_y;
	int l_outlineY2 = m_scrollArea.m_y + m_scrollArea.m_height;
	int l_depth = static_cast<int>(m_depth);

	m_pRenderer->PushMatrix();
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
		m_pRenderer->EnableImmediateMode(IM_LINE_LOOP);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2 + 1, l_outlineY2, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY2, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
	*/

	Dimensions lScrollbarParent;
	if(GetParent() != nullptr)
	{
		lScrollbarParent = GetParent()->GetDimensions();
	}

	int lScissorX = lScrollbarParent.m_x + GetLocation().m_x + m_scrollArea.m_x;
	int lScissorWidth = m_scrollArea.m_width;
	int lScissorY = lScrollbarParent.m_y + GetLocation().m_y + m_scrollArea.m_y;
	int lScissorHeight = m_scrollArea.m_height;


	if(IsContainer() && HasChildren() && IsVisible())
	{
		// Draw all children
		ComponentList::const_iterator iterator;

		for(iterator = m_vpScrollAreaComponentList.begin(); iterator != m_vpScrollAreaComponentList.end(); ++iterator)
		{
			if((*iterator)->IsVisible())
			{
				if(m_scissorEnabled)
				{
					m_pRenderer->EnableScissorTest(lScissorX, lScissorY, lScissorWidth, lScissorHeight);;
				}

				(*iterator)->Draw();

				if(m_scissorEnabled)
				{
					m_pRenderer->DisableScissorTest();
				}
			}
		}
	}
}

void ScrollBar::_UpdateArrowButtons(void *lpData)
{
	ScrollBar *_this = (ScrollBar*)lpData;

	if(_this)
	{
		_this->UpdateArrowButtons();
	}
}

void ScrollBar::UpdateArrowButtons()
{
	float updateSpeed = 0.01f;

	if(IsSelectedLeft())
	{
		if(m_eScrollDirection == EScrollBarDirection_Horizontal)
		{
			m_scrollPosition -= updateSpeed;
		}
		else
		{
			m_scrollPosition += updateSpeed;
		}

		OnValueChanged();

		if(m_ValueChangedCallback)
		{
			m_ValueChangedCallback(m_pValueChangedCallbackData);
		}
	}

	if(IsSelectedRight())
	{
		if(m_eScrollDirection == EScrollBarDirection_Horizontal)
		{
			m_scrollPosition += updateSpeed;
		}
		else
		{
			m_scrollPosition -= updateSpeed;
		}

		OnValueChanged();

		if(m_ValueChangedCallback)
		{
			m_ValueChangedCallback(m_pValueChangedCallbackData);
		}
	}

	ClampScrollBarPosition();
}