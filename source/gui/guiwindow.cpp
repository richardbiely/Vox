// ******************************************************************************
//
// Filename:	guiwindow.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A window container class, defines basic window functionality.
//
// Revision History:
//   Initial Revision - 26/09/06
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "guiwindow.h"

#include "openglgui.h"


GUIWindow::GUIWindow(Renderer* pRenderer, unsigned int GUIFont, const std::string &title)
  : Container(pRenderer),
    m_GUIFont(GUIFont)
{
	m_titleBar = new TitleBar(pRenderer, GUIFont, title);

	// We created the title bar, so we own it (Must delete it)
	mb_ownsTitleBar = true;

	m_titleBar->SetParent(this); // The base class component parent functionality
	m_titleBar->SetGUIWindowParent(this);
	m_titleBar->SetDepth(2.0f);

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

	Add(m_titleBar);

	m_bMinimized = false;

	m_bAllowMoving = true;
	m_bAllowClosing = true;
	m_bAllowMinimizing = true;
	m_bAllowScrolling = true;
    m_bSnapToWindow = true;

	m_bRenderTitleBar = true;
	m_bRenderWindowBackground = true;

	m_applicationBorderLeft = 0;
	m_applicationBorderRight = 0;
	m_applicationBorderTop = 0;
	m_applicationBorderBottom = 0;

	m_outlineRender = false;

	m_pParentGUI = nullptr;
}

GUIWindow::GUIWindow(Renderer* pRenderer, unsigned int GUIFont, TitleBar* ptitleBar)
  : Container(pRenderer),
    m_GUIFont(GUIFont)
{
	m_titleBar = ptitleBar;
	
	// We didn't create the title bar, so we don't own it. (Cant delete it)
	mb_ownsTitleBar = false;

	m_titleBar->SetParent(this);  // The base class component parent functionality
	m_titleBar->SetGUIWindowParent(this);
	m_titleBar->SetDepth(2.0f);

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

	Add(m_titleBar);

	m_bMinimized = false;

	m_bAllowMoving = true;
	m_bAllowClosing = true;
	m_bAllowMinimizing = true;
	m_bAllowScrolling = true;

	m_applicationBorderLeft = 0;
	m_applicationBorderRight = 0;
	m_applicationBorderTop = 0;
	m_applicationBorderBottom = 0;

	m_pParentGUI = nullptr;

	m_outlineRender = false;
}

GUIWindow::~GUIWindow()
{
	if(m_pParentGUI)
	{
		m_pParentGUI->RemoveWindow(this);
	}

	if(mb_ownsTitleBar)
	{
		// If we own the title bar, we are allowed to delete it
		delete m_titleBar;
	}

	Remove(m_pBackgroundIcon);
	delete m_pBackgroundIcon;

	m_vpGUIWindowList.clear();
}

void GUIWindow::AddEventListeners()
{
	AddMouseListener(this);

	// Also add all children's event listeners
	Container::AddEventListeners();

	//m_titleBar->AddEventListeners();
}

void GUIWindow::RemoveEventListeners()
{
	RemoveMouseListener(this);

	// Also remove all children's event listeners
	Container::RemoveEventListeners();

	//m_titleBar->RemoveEventListeners();
}

bool GUIWindow::IsRootContainer() const
{
	return true;
}

void GUIWindow::AddGUIWindow(GUIWindow *window)
{
	window->SetParent(this);
	
	window->SetDepth(m_depth + 1.0f);

	m_vpGUIWindowList.push_back(window);

	// Sort the GUIWindow vector list, by depth
	DepthSortGUIWindowChildren();
}

void GUIWindow::RemoveGUIWindow(GUIWindow *window)
{
	// Find this window in the vector
	GUIWindowList::iterator iter = std::find(m_vpGUIWindowList.begin(), m_vpGUIWindowList.end(), window);

	if(iter != m_vpGUIWindowList.end())
	{
		(*iter)->SetParent(0);

		// Erase the window
		m_vpGUIWindowList.erase(iter);
	}
}

void GUIWindow::RemoveAllGUIWindows()
{
	m_vpGUIWindowList.clear();
}

const GUIWindowList& GUIWindow::GetGUIWindows() const
{
	return m_vpGUIWindowList;
}

TitleBar* GUIWindow::GetTitleBar() const
{
	return m_titleBar;
}

void GUIWindow::SetTitleBarDimensions(int xOffset, int yOffset, int width, int height)
{
	m_titleBar->SetDimensions(xOffset, GetDimensions().m_height + yOffset, width, height);
}

void GUIWindow::SetTitleOffset(int xOffset, int yOffset)
{
	m_titleBar->SetTitleOffset(xOffset, yOffset);
}

void GUIWindow::SetTitlebarBackgroundIcon(RenderRectangle *icon)
{
	m_titleBar->SetBackgroundIcon(icon);
}

void GUIWindow::SetBackgroundIcon(RenderRectangle *icon)
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

void GUIWindow::AddComponent(Component* component)
{
	Container::Add(component);

	component->SetVisible(IsVisible());

	component->SetAudio(m_audio);
	component->SetAudioVolume(m_audioVolume);
}

void GUIWindow::RemoveComponent(Component *component)
{
	component->RemoveEventListeners();

	Container::Remove(component);
}

void GUIWindow::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);

	// Also set the background
	m_pBackgroundIcon->SetDimensions(0, 0, width, height);

	// Also set the title bar dimensions
	int titleBarSpacer = 1;
	int titleBarHeight = m_pRenderer->GetFreeTypeTextHeight(m_GUIFont, "a");
	m_titleBar->SetDimensions(0, height, width, (titleBarHeight + titleBarSpacer*2));
}

void GUIWindow::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void GUIWindow::SetLocation(int x, int y)
{
	int widthToUse = m_dimensions.m_width;
	int heightAdjustment = 0;

	if(m_bMinimized)
	{
		widthToUse = m_titleBar->GetDimensions().m_width;
		heightAdjustment = m_dimensions.m_height;
	}

    if(x < 1 + m_applicationBorderLeft)
        x = 1 + m_applicationBorderLeft;
    if(x + 1 + widthToUse > m_applicationWidth - m_applicationBorderRight)
        x = m_applicationWidth - 1 - widthToUse - m_applicationBorderRight;
    if(y < 0 - heightAdjustment + m_applicationBorderBottom)
        y = 0 - heightAdjustment + m_applicationBorderBottom;
    if(y + 1 + m_dimensions.m_height + m_titleBar->GetDimensions().m_height > m_applicationHeight - m_applicationBorderTop)
        y = m_applicationHeight - 1 - m_dimensions.m_height - m_titleBar->GetDimensions().m_height - m_applicationBorderTop;

    m_dimensions.m_x = x;
    m_dimensions.m_y = y;
}

void GUIWindow::SetLocation(const Point& p)
{
    int x = p.m_x;
    int y = p.m_x;

	SetLocation(x, y);
}

void GUIWindow::SetTitle(const std::string &title)
{
	m_titleBar->SetTitle(title);
}

const std::string GUIWindow::GetTitle() const
{
	return m_titleBar->GetTitle();
}

void GUIWindow::SetDebugRender(bool debug)
{
	Container::SetDebugRender(debug);

	m_titleBar->SetDebugRender(debug);

	GUIWindowList::const_iterator iter;
	for(iter = m_vpGUIWindowList.begin(); iter != m_vpGUIWindowList.end(); ++iter)
	{
		(*iter)->SetDebugRender(debug);
	}
}

void GUIWindow::SetOutlineRender(bool outline)
{
	m_outlineRender = outline;

	m_titleBar->SetOutlineRender(outline);
}

void GUIWindow::Show()
{
	SetVisible(true);
	m_titleBar->SetVisible(true);

	// Show all our children components
	ComponentList::const_iterator iterator;
	for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
	{
		(*iterator)->SetVisible(true);
	}

	// Show all our children GUI Windows
	GUIWindowList::const_iterator iterator_window;
	for(iterator_window = m_vpGUIWindowList.begin(); iterator_window != m_vpGUIWindowList.end(); ++iterator_window)
	{
		(*iterator_window)->SetVisible(true);
	}
}

void GUIWindow::Hide()
{
	SetVisible(false);
	m_titleBar->SetVisible(false);

	// Hide all our children components
	ComponentList::const_iterator iterator;
	for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
	{
		(*iterator)->SetVisible(false);
	}

	// Hide all our children GUI Windows
	GUIWindowList::const_iterator iterator_window;
	for(iterator_window = m_vpGUIWindowList.begin(); iterator_window != m_vpGUIWindowList.end(); ++iterator_window)
	{
		(*iterator_window)->SetVisible(false);
	}
}

bool GUIWindow::GetMinimized() const
{
	return m_bMinimized;
}

void GUIWindow::SetMinimized(bool minimized)
{
	m_bMinimized = minimized;

	if(m_bMinimized)
	{
		// Hide all our children components when we are minimized
		ComponentList::const_iterator iterator;

		for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
		{
			(*iterator)->SetVisible(false);
			(*iterator)->SetEnabled(false);
		}
	}
	else
	{
		// Else show all our children components
		ComponentList::const_iterator iterator;

		for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
		{
			(*iterator)->SetVisible(true);
			(*iterator)->SetEnabled(true);
		}
	}

	m_titleBar->SetVisible(true);
	m_titleBar->SetEnabled(true);

	SetLocation(GetLocation().m_x, GetLocation().m_y);
}

void GUIWindow::SetMinimizedDefaultIcon(RenderRectangle *icon)
{
	m_titleBar->GetMinimizeButton()->SetDefaultIcon(icon);
}

void GUIWindow::SetMinimizedSelectedIcon(RenderRectangle *icon)
{
	m_titleBar->GetMinimizeButton()->SetSelectedIcon(icon);
}

void GUIWindow::SetMinimizedHoverIcon(RenderRectangle *icon)
{
	m_titleBar->GetMinimizeButton()->SetHoverIcon(icon);
}

void GUIWindow::SetMinimizedDisabledIcon(RenderRectangle *icon)
{
	m_titleBar->GetMinimizeButton()->SetDisabledIcon(icon);
}

void GUIWindow::SetCloseDefaultIcon(RenderRectangle *icon)
{
	m_titleBar->GetCloseButton()->SetDefaultIcon(icon);
}

void GUIWindow::SetCloseSelectedIcon(RenderRectangle *icon)
{
	m_titleBar->GetCloseButton()->SetSelectedIcon(icon);
}

void GUIWindow::SetCloseHoverIcon(RenderRectangle *icon)
{
	m_titleBar->GetCloseButton()->SetHoverIcon(icon);
}

void GUIWindow::SetCloseDisabledIcon(RenderRectangle *icon)
{
	m_titleBar->GetCloseButton()->SetDisabledIcon(icon);
}

void GUIWindow::AllowMoving(bool val)
{
	m_bAllowMoving = val;
}

void GUIWindow::AllowClosing(bool val)
{
	m_bAllowClosing = val;

	m_titleBar->AllowClosing(val);
}

void GUIWindow::AllowMinimizing(bool val)
{
	m_bAllowMinimizing = val;

	m_titleBar->AllowMinimizing(val);
}

void GUIWindow::AllowScrolling(bool val)
{
	m_bAllowScrolling = val;
}

void GUIWindow::SnapToApplication(bool val)
{
    m_bSnapToWindow = val;
}

void GUIWindow::SetApplicationDimensions(int width, int height)
{
    m_applicationWidth = width;
    m_applicationHeight = height;
}

void GUIWindow::SetApplicationBorder(int left, int right, int top, int bottom)
{
	m_applicationBorderLeft = left;
	m_applicationBorderRight = right;
	m_applicationBorderTop = top;
	m_applicationBorderBottom = bottom;
}

void GUIWindow::SetRenderTitleBar(bool lbRender)
{
	m_bRenderTitleBar = lbRender;

	if(m_bRenderTitleBar)
	{
		m_titleBar->SetVisible(true);
	}
	else
	{
		m_titleBar->SetVisible(false);
	}
}

void GUIWindow::SetRenderWindowBackground(bool lbRender)
{
	m_bRenderWindowBackground = lbRender;
}

void GUIWindow::DepthSortGUIWindowChildren()
{

}

EComponentType GUIWindow::GetComponentType() const
{
	return EComponentType_GUIWindow;
}

void GUIWindow::SetGUIParent(OpenGLGUI* pParent)
{
	m_pParentGUI = pParent;
}

void GUIWindow::SetFocusWindow()
{
	if(m_pParentGUI)
	{
		m_pParentGUI->SetFocusedWindow(this);
	}
}

void GUIWindow::Update(float deltaTime)
{
	// Update our components
	Container::Update(deltaTime);

	GUIWindowList::const_iterator iter;

	// Update all the windows which we contain
	for(iter = m_vpGUIWindowList.begin(); iter != m_vpGUIWindowList.end(); ++iter)
	{
		if((*iter)->IsVisible())
		{
			(*iter)->Update(deltaTime);
		}
	}
}

// < Operator (Used for GUIWindow depth sorting)
bool GUIWindow::operator<(const GUIWindow &w) const
{
	return(GetDepth() < w.GetDepth());
}

bool GUIWindow::DepthLessThan(const GUIWindow *lhs, const GUIWindow *rhs)
{
	if ((*lhs) < (*rhs)) { return true; }
	if ((*rhs) < (*lhs)) { return false; }
	return (*lhs) < (*rhs);
}

void GUIWindow::MousePressed(const MouseEvent& lEvent)
{
	m_pParentGUI->SetFocusedWindow(this);
}

void GUIWindow::DrawSelf()
{
	m_pBackgroundIcon->SetVisible(false);

	if(m_outlineRender == false)
	{
		if(!m_bMinimized && m_bRenderWindowBackground)
		{
			m_pBackgroundIcon->SetVisible(true);
		}
	}

	if(m_outlineRender)
	{
		int l_containerWidth = m_dimensions.m_width;
		int l_containerHeight = m_dimensions.m_height;
		float l_depth = m_depth;

		float l_containerX1 = 0.5f;
		float l_containerX2 = (float)l_containerWidth;
		float l_containerY1 = 0.5f;
		float l_containerY2 = (float)l_containerHeight;

		float l_outlineX1 = -0.5f;
		float l_outlineX2 = l_containerWidth + 0.5f;
		float l_outlineY1 = -0.0f;
		float l_outlineY2 = l_containerHeight + 0.5f;

		if(!m_bMinimized && m_bRenderWindowBackground)
		{
			// Draw the background
			m_pRenderer->PushMatrix();
				m_pRenderer->SetRenderMode(RM_SOLID);
				m_pRenderer->EnableImmediateMode(IM_QUADS);
					m_pRenderer->ImmediateColourAlpha(0.288f, 0.288f, 0.288f, 1.0f);
					m_pRenderer->ImmediateVertex(l_containerX1, l_containerY1, l_depth);
					m_pRenderer->ImmediateVertex(l_containerX2, l_containerY1, l_depth);
					m_pRenderer->ImmediateColourAlpha(0.784f, 0.784f, 0.784f, 1.0f);
					m_pRenderer->ImmediateVertex(l_containerX2, l_containerY2, l_depth);
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

	if(m_bMinimized)
	{
		// Draw the window title
		if(m_bRenderTitleBar)
		{
			m_titleBar->Draw();
		}
	}

	// Draw the GUIWindow's children before the titlebar 
	DrawChildrenFirst();
}

void GUIWindow::DrawChildrenFirst()
{
	if(!m_bMinimized)
	{
		// Draw our components
		Container::DrawChildren();

		GUIWindowList::const_iterator iter;

		// Draw all the windows which we contain
		for(iter = m_vpGUIWindowList.begin(); iter != m_vpGUIWindowList.end(); ++iter)
		{
			if((*iter)->IsVisible())
			{
				(*iter)->Draw();
			}
		}
	}
}

void GUIWindow::DrawChildren()
{

}