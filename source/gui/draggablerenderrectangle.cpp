// ******************************************************************************
//
// Filename:	draggablerenderrectangle.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 31/07/12
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "draggablerenderrectangle.h"
#include "guiwindow.h"


DraggableRenderRectangle::DraggableRenderRectangle(Renderer* pRenderer)
  : Container(pRenderer)
{
	m_pIcon = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pIcon;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	Add(m_pIcon);

	m_bDragging = false;

	m_nextX = 0;
	m_nextY = 0;

    m_PressedCallback = nullptr;
    m_pPressedCallbackData = nullptr;
    
    m_ReleasedCallback = nullptr;
    m_pReleasedCallbackData = nullptr;

    m_EnterCallback = nullptr;
    m_pEnterCallbackData = nullptr;

    m_ExitCallback = nullptr;
    m_pExitCallbackData = nullptr;
}

DraggableRenderRectangle::~DraggableRenderRectangle()
{
	Remove(m_pIcon);

	delete m_pIcon;
	m_pIcon = nullptr;

    for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
    {
		Remove(m_vpAddedComponentList[i]);

        delete m_vpAddedComponentList[i];
        m_vpAddedComponentList[i] = 0;
    }
	m_vpAddedComponentList.clear();
}

void DraggableRenderRectangle::SetIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pIcon)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pIcon);

		delete m_pIcon;
		m_pIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pIcon = lpNewDirectDraw;
	}

	// Re-add the icon to the component list
	Add(m_pIcon);
}

void DraggableRenderRectangle::AddText(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, Colour colour, int xOffset, int yOffset, bool outline, Colour outlineColour)
{
    Label* lpNewLabel = new Label(pRenderer, GUIFont, label, colour);
    lpNewLabel->SetLocation(xOffset, yOffset);
    lpNewLabel->SetDepth(lpNewLabel->GetDepth()+1.0f);

    if(outline)
    {
        lpNewLabel->SetOutline(true);
        lpNewLabel->SetOutlineColour(outlineColour);
        lpNewLabel->SetOutlineFont(OutlineGUIFont);
    }

    m_vpAddedComponentList.push_back(lpNewLabel);

    Add(lpNewLabel);
}

void DraggableRenderRectangle::AddIcon(Renderer* pRenderer, const std::string &fileName, int texWidth, int texHeight, int width, int height, int xOffset, int yOffset, float depth)
{
	// texWidth and texHeight should always be a power of 2.

    Icon* lpNewIcon = new Icon(pRenderer, fileName, texWidth, texHeight);
    lpNewIcon->SetDimensions(xOffset, yOffset, width, height);
    lpNewIcon->SetDepth(depth);

    m_vpAddedComponentList.push_back(lpNewIcon);

    Add(lpNewIcon);
}

void DraggableRenderRectangle::RemoveIcon(const std::string &fileName)
{
	bool removed = false;
	for(unsigned int i = 0; i < m_vpAddedComponentList.size() && removed == false; i++)
	{
		if(m_vpAddedComponentList[i]->GetComponentType() == EComponentType_Icon)
		{
			Icon* pIcon = (Icon*)m_vpAddedComponentList[i];
			if(pIcon->GetFileName() == fileName)
			{
				Remove(pIcon);

				ComponentList::iterator iter = std::find(m_vpAddedComponentList.begin(), m_vpAddedComponentList.end(), pIcon);
				if(iter != m_vpAddedComponentList.end())
				{
					// Erase the component
					m_vpAddedComponentList.erase(iter);
				}

				delete pIcon;

				removed = true;
			}
		}
	}
}

void DraggableRenderRectangle::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);

	// Also set the dimensions of the icon
	m_pIcon->SetDimensions(0, 0, width, height);
}

void DraggableRenderRectangle::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void DraggableRenderRectangle::AddEventListeners()
{
	Container::AddEventListeners();

	AddMouseListener(this);
}

void DraggableRenderRectangle::RemoveEventListeners()
{
	Container::RemoveEventListeners();

	RemoveMouseListener(this);
}

void DraggableRenderRectangle::SetPressedCallBackFunction(FunctionCallback lFunction)
{
    m_PressedCallback = lFunction;
}

void DraggableRenderRectangle::SetPressedCallBackData(void *lpData)
{
    m_pPressedCallbackData = lpData;
}

void DraggableRenderRectangle::SetReleasedCallBackFunction(FunctionCallback lFunction)
{
    m_ReleasedCallback = lFunction;
}

void DraggableRenderRectangle::SetReleasedCallBackData(void *lpData)
{
    m_pReleasedCallbackData = lpData;
}

void DraggableRenderRectangle::SetEnterCallBackFunction(FunctionCallback lFunction)
{
    m_EnterCallback = lFunction;
}

void DraggableRenderRectangle::SetEnterCallBackData(void *lpData)
{
    m_pEnterCallbackData = lpData;
}

void DraggableRenderRectangle::SetExitCallBackFunction(FunctionCallback lFunction)
{
    m_ExitCallback = lFunction;
}

void DraggableRenderRectangle::SetExitCallBackData(void *lpData)
{
    m_pExitCallbackData = lpData;
}

void DraggableRenderRectangle::MouseEntered(const MouseEvent& lEvent)
{
	if(IsEnabled() == false)
	{
		return;
	}

	OnMouseEnter();

    // Call the callback function
    if(m_EnterCallback)
    {
        m_EnterCallback(m_pEnterCallbackData);
    }
}

void DraggableRenderRectangle::MouseExited(const MouseEvent& lEvent)
{
	if(IsEnabled() == false)
	{
		return;
	}

	OnMouseExit();

    // Call the callback function
    if(m_ExitCallback)
    {
        m_ExitCallback(m_pExitCallbackData);
    }
}

void DraggableRenderRectangle::MousePressed(const MouseEvent& lEvent)
{
	if(IsEnabled() == false)
	{
		return;
	}

	if(GetParent() != nullptr && GetParent()->GetComponentType() == EComponentType_GUIWindow)
	{
		GUIWindow* lpParentWindow = (GUIWindow *)GetParent();
		lpParentWindow->SetFocusWindow();
	}

	m_bDragging = true;

	OnMousePressed();

    // Call the callback function
    if(m_PressedCallback)
    {
        m_PressedCallback(m_pPressedCallbackData);
    }
}

void DraggableRenderRectangle::MouseReleased(const MouseEvent& lEvent)
{
	if(IsEnabled() == false)
	{
		return;
	}

	m_bDragging = false;

	OnMouseReleased();

    // Call the callback function
    if(m_ReleasedCallback)
    {
        m_ReleasedCallback(m_pReleasedCallbackData);
    }
}

void DraggableRenderRectangle::MouseClicked(const MouseEvent& lEvent)
{
	OnMouseClicked();
}

void DraggableRenderRectangle::MouseDragged(const MouseEvent& lEvent)
{
	if(IsEnabled() == false)
	{
		return;
	}

	if(lEvent.GetSource() == this)
	{
		if(m_bDragging)
		{
			Point location = GetLocation();

			for(Component* parent = GetParent(); parent != 0;)
			{
				Point parentLocation = parent->GetLocation();

				parent = parent->GetParent();
			}

			m_nextX = location.m_x + lEvent.GetX();
			m_nextY = location.m_y + lEvent.GetY();
			SetLocation(m_nextX, m_nextY);
		}
	}

	OnMouseDragged();
}

void DraggableRenderRectangle::MouseDraggedOutside(const MouseEvent& lEvent)
{
	OnMouseDraggedOutside();
}

EComponentType DraggableRenderRectangle::GetComponentType() const
{
	return EComponentType_DraggableRenderRectangle;
}

void DraggableRenderRectangle::OnMouseEnter()
{
	/* Nothing */
}

void DraggableRenderRectangle::OnMouseExit()
{
	/* Nothing */
}

void DraggableRenderRectangle::OnMousePressed()
{
	/* Nothing */
}

void DraggableRenderRectangle::OnMouseReleased()
{
	/* Nothing */
}

void DraggableRenderRectangle::OnMouseClicked()
{
	/* Nothing */
}

void DraggableRenderRectangle::OnMouseDragged()
{
	/* Nothing */
}

void DraggableRenderRectangle::OnMouseDraggedOutside()
{
	/* Nothing */
}

void DraggableRenderRectangle::DrawSelf()
{
}