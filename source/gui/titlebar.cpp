// ******************************************************************************
//
// Filename:	titlebar.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A title bar that is used for gui windows, displays the window
//   title and any buttons for close, minimize, etc..
//
// Revision History:
//   Initial Revision - 22/10/06
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "titlebar.h"
#include "guiwindow.h"


TitleBar::TitleBar(Renderer* pRenderer, unsigned int GUIFont, const std::string &title)
  : Container(pRenderer),
    m_title(pRenderer, GUIFont, title, Colour(1.0f, 1.0f, 1.0f))
{
	m_closeButton = new GUIWindowCloseButton(pRenderer, GUIFont, "Close");
	m_minimizeButton = new GUIWindowMinimizeButton(pRenderer, GUIFont, "Minimize");

	mb_ownsButtons = true;

	m_pBackgroundIcon = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pBackgroundIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	Add(m_pBackgroundIcon);

	int lTitleBarSpace = 1;
	m_title.SetLocation(lTitleBarSpace, lTitleBarSpace);
	m_title.SetDepth(2.0f);

	Add(&m_title);
	Add(m_closeButton);
	Add(m_minimizeButton);

	m_nextX = 0;
	m_nextY = 0;

	m_lastX = 0;
	m_lastY = 0;

	m_bDragging = false;
	m_bFirstOutsideDrag = false;

	m_outlineRender = false;
}

TitleBar::TitleBar(Renderer* pRenderer, unsigned int GUIFont, const std::string &title, GUIWindowCloseButton* pCloseButton, GUIWindowMinimizeButton* pMinimizeButton)
  : Container(pRenderer),
    m_title(pRenderer, GUIFont, title, Colour(1.0f, 1.0f, 1.0f))
{
	m_closeButton = pCloseButton;
	m_minimizeButton = pMinimizeButton;

	mb_ownsButtons = false;

	m_pBackgroundIcon = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pBackgroundIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	Add(m_pBackgroundIcon);

	int lTitleBarSpace = 1;
	m_title.SetLocation(lTitleBarSpace, lTitleBarSpace);
	m_title.SetDepth(2.0f);

	Add(&m_title);
	Add(m_closeButton);
	Add(m_minimizeButton);

	m_nextX = 0;
	m_nextY = 0;

	m_lastX = 0;
	m_lastY = 0;

	m_bDragging = false;
	m_bFirstOutsideDrag = false;

	m_outlineRender = false;
}

TitleBar::~TitleBar()
{
	if(mb_ownsButtons)
	{
		delete m_closeButton;
		delete m_minimizeButton;
	}

	Remove(m_pBackgroundIcon);
	delete m_pBackgroundIcon;
}

void TitleBar::AddEventListeners()
{
	Container::AddEventListeners();

	//m_closeButton->AddEventListeners();
	//m_minimizeButton->AddEventListeners();

	AddMouseListener(this);
}

void TitleBar::RemoveEventListeners()
{
	Container::RemoveEventListeners();

	//m_closeButton->RemoveEventListeners();
	//m_minimizeButton->RemoveEventListeners();

	RemoveMouseListener(this);
}

GUIWindowCloseButton* TitleBar::GetCloseButton() const
{
	return m_closeButton;
}

GUIWindowMinimizeButton* TitleBar::GetMinimizeButton() const
{
	return m_minimizeButton;
}

void TitleBar::SetGUIWindowParent(GUIWindow* pParent)
{
	m_pParent = pParent;

	// Also set the close and minimize buttons to have the same GUI window parent
	m_closeButton->SetGUIWindowParent(pParent);
	m_minimizeButton->SetGUIWindowParent(pParent);
}

void TitleBar::MouseEntered(const MouseEvent& lEvent)
{
	m_bFirstOutsideDrag = false;

	OnMouseEnter();
}

void TitleBar::MouseExited(const MouseEvent& lEvent)
{
	OnMouseExit();
}

void TitleBar::MousePressed(const MouseEvent& lEvent)
{
	if(m_pParent != nullptr)
	{
		m_pParent->SetFocusWindow();
	}

	m_bDragging = true;

	OnMousePressed();
}

void TitleBar::MouseReleased(const MouseEvent& lEvent)
{
	m_bDragging = false;

	OnMouseReleased();
}

void TitleBar::MouseClicked(const MouseEvent& lEvent)
{
	OnMouseClicked();
}

void TitleBar::MouseDragged(const MouseEvent& lEvent)
{
	// Don't allow moving if we have disabled this
	if(m_pParent->m_bAllowMoving)
	{
		if(lEvent.GetSource() == this)
		{
			if(m_bDragging)
			{
				m_nextX = GetParent()->GetLocationOnScreen().m_x + lEvent.GetX();
				m_nextY = GetParent()->GetLocationOnScreen().m_y + lEvent.GetY();
				GetParent()->SetLocation(m_nextX, m_nextY);
			}
		}
	}

	OnMouseDragged();
}

void TitleBar::MouseDraggedOutside(const MouseEvent& lEvent)
{
	if(!m_bFirstOutsideDrag)
	{
		m_lastX = lEvent.GetX();
		m_lastY = lEvent.GetY();
		m_bFirstOutsideDrag = true;
	}

	if(m_pParent->m_bAllowMoving)
	{
		if(lEvent.GetSource() == this)
		{
			if(m_bDragging)
			{
				int lNewX = lEvent.GetX();
				int lNewY = lEvent.GetY();

				int l_ChangeX = lNewX - m_lastX;
				int l_ChangeY = lNewY - m_lastY;

				if(l_ChangeX == 0 && l_ChangeY == 0)
					return;

				m_nextX = GetParent()->GetLocationOnScreen().m_x + l_ChangeX;
				m_nextY = GetParent()->GetLocationOnScreen().m_y + l_ChangeY;
				GetParent()->SetLocation(m_nextX, m_nextY);

				m_lastX = lNewX;
				m_lastY = lNewY;
			}
		}
	}

	OnMouseDraggedOutside();
}

void TitleBar::DrawSelf()
{
	if(m_outlineRender)
	{
		int l_containerWidth = m_dimensions.m_width;
		int l_containerHeight = m_dimensions.m_height;
		float l_depth = (GetDepth());

		float l_containerX1 = 0.5f;
		float l_containerX2 = (float)l_containerWidth;
		float l_containerY1 = 0.5f;
		float l_containerY2 = (float)l_containerHeight;

		float l_outlineX1 = -0.5f;
		float l_outlineX2 = l_containerWidth + 0.5f;
		float l_outlineY1 = -0.0f;
		float l_outlineY2 = l_containerHeight + 0.5f;

		// Draw the background
		m_pRenderer->PushMatrix();
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->EnableImmediateMode(IM_QUADS);
				m_pRenderer->ImmediateColourAlpha(0.039f, 0.141f, 0.415f, 1.0f);
				m_pRenderer->ImmediateVertex(l_containerX1, l_containerY1, l_depth);
				m_pRenderer->ImmediateColourAlpha(0.650f, 0.792f, 0.941f, 1.0f);
				m_pRenderer->ImmediateVertex(l_containerX2, l_containerY1, l_depth);
				m_pRenderer->ImmediateVertex(l_containerX2, l_containerY2, l_depth);
				m_pRenderer->ImmediateColourAlpha(0.039f, 0.141f, 0.415f, 1.0f);
				m_pRenderer->ImmediateVertex(l_containerX1, l_containerY2, l_depth);
			m_pRenderer->DisableImmediateMode();
		m_pRenderer->PopMatrix();

		// Draw the outline
		m_pRenderer->PushMatrix();
			m_pRenderer->SetLineWidth(1.0f);
			m_pRenderer->EnableImmediateMode(IM_LINE_LOOP);
				m_pRenderer->ImmediateColourAlpha(0.650f, 0.792f, 0.941f, 1.0f);
				m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY1, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY1, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY2, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY2, l_depth);
			m_pRenderer->DisableImmediateMode();
		m_pRenderer->PopMatrix();
	}
}

void TitleBar::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);
	
	// Also set the background
	m_pBackgroundIcon->SetDimensions(0, 0, width, height);

	// Also set the dimensions of the close button
	int distanceFromBorder = 2;
	int buttonHeight = height - (distanceFromBorder * 2);
	int buttonX = width;
	
	if(m_pParent->m_bAllowClosing)
	{
		buttonX -= (buttonHeight + distanceFromBorder);
		m_closeButton->SetDimensions(buttonX, distanceFromBorder, buttonHeight, buttonHeight);
	}

	if(m_pParent->m_bAllowMinimizing)
	{
		buttonX -= (buttonHeight + distanceFromBorder*2);
		m_minimizeButton->SetDimensions(buttonX, distanceFromBorder, buttonHeight, buttonHeight);
	}
}

void TitleBar::SetTitleOffset(int xOffset, int yOffset)
{
	m_title.SetLocation(xOffset, yOffset);
}

void TitleBar::SetBackgroundIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pBackgroundIcon)
	{
		// If we already own a background icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pBackgroundIcon);

		delete m_pBackgroundIcon;
		m_pBackgroundIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pBackgroundIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pBackgroundIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pBackgroundIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pBackgroundIcon);
}

void TitleBar::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void TitleBar::SetTitle(const std::string &title)
{
	m_title.SetText(title);
}

const std::string TitleBar::GetTitle() const
{
	return m_title.GetText();
}

void TitleBar::SetOutlineRender(bool outline)
{
	m_outlineRender = outline;

	m_pBackgroundIcon->SetVisible(outline == false);
}

void TitleBar::AllowMinimizing(bool val)
{
	m_minimizeButton->SetVisible(val);
}

void TitleBar::AllowClosing(bool val)
{
	m_closeButton->SetVisible(val);
}

EComponentType TitleBar::GetComponentType() const
{
	return EComponentType_TitleBar;
}

void TitleBar::OnMouseEnter()
{
	/* Nothing */
}

void TitleBar::OnMouseExit()
{
	/* Nothing */
}

void TitleBar::OnMousePressed()
{
	/* Nothing */
}

void TitleBar::OnMouseReleased()
{
	/* Nothing */
}

void TitleBar::OnMouseClicked()
{
	/* Nothing */
}

void TitleBar::OnMouseDragged()
{
	/* Nothing */
}

void TitleBar::OnMouseDraggedOutside()
{
	/* Nothing */
}