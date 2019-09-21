// ******************************************************************************
//
// Filename:	selectionmanager.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   Handles the picking of components, events are sent to the currently
//   selected component. This is worked out using component bounding boxes.
//
// Note:
//   The actual algorithm for working out what component is currently
//   selected doesn't have to be mathematically, we could use the opengl
//   selection buffer, colour picking or a number of other methods.
//
// Revision History:
//   Initial Revision - 15/10/06
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "selectionmanager.h"
#include "focusmanager.h"
#include "guiwindow.h"
#include "scrollbar.h"

// Initialize the singleton instance
SelectionManager *SelectionManager::c_instance = 0;

SelectionManager* SelectionManager::GetInstance()
{
	if(c_instance == 0)
		c_instance = new SelectionManager;

	return c_instance;
}

void SelectionManager::Destroy()
{
	if(c_instance)
	{
		delete c_instance;
	}
}

void SelectionManager::ResetCurrentComponent()
{
	if(m_foundComponent != 0)
	{
		MouseEvent exitEvent(m_foundComponent, MOUSE_EXITED, m_mouseX, m_mouseY, 0);
		m_foundComponent->ProcessMouseEvent(exitEvent);
	}
}

SelectionManager::SelectionManager()
  : m_foundComponent(0),
    m_hoverOverComponent(0),
	m_firstClickedComponent(0),
	m_bPressed(false)
{
	m_lastX = 0;
	m_lastY = 0;
}

void SelectionManager::AddComponent(Component* component)
{
	m_vpComponentList.push_back(component);
}

void SelectionManager::RemoveComponent(Component* component)
{
	if(component == m_foundComponent)
	{
		m_foundComponent = nullptr;
	}
	if(component == m_hoverOverComponent)
	{
		m_hoverOverComponent = nullptr;
	}
	if(component == m_firstClickedComponent)
	{
		m_firstClickedComponent = nullptr;
	}

	ComponentList::iterator iter = std::find(m_vpComponentList.begin(), m_vpComponentList.end(), component);
	if(iter != m_vpComponentList.end())
	{
		// Erase the component
		m_vpComponentList.erase(iter);
	}
}

void SelectionManager::Update(int x, int y)
{
	m_mouseX = x;
	m_mouseY = y;

	m_foundComponent = GetComponentAt(x, y);
}

Component* SelectionManager::GetComponentAt(int x, int y)
{
	// ----------------------
	//         TODO
	// This needs much work:
	// * Depth testing
	// * Focus ordering
	// ----------------------

	ComponentList::const_iterator iterator;

	Component* lpWindowComponent = 0;
	float lCurrentDepth = 0;

	for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
	{
		// We have only found a selection if the component is enabled
		if((*iterator)->IsEnabled() && (*iterator)->IsVisible() && (*iterator)->IsParentEnabled() && (*iterator)->IsParentVisible())
		{
			Point l_location = (*iterator)->GetLocationOnScreen();
			Dimensions l_dimensions = (*iterator)->GetDimensions();
			int l_left = l_location.m_x;
			int l_right = l_location.m_x + l_dimensions.m_width;
			int l_top = l_location.m_y + l_dimensions.m_height;
			int l_bottom = l_location.m_y;
	
			if( (x < l_right && x >= l_left) && (y <= l_top && y > l_bottom) )
			{
				if((*iterator)->GetComponentType() == EComponentType_GUIWindow)
				{
					// Hack : If we have found a window component, then dont return it just yet, let us see if we can find something else
					GUIWindow* lpWindow = (GUIWindow*)(*iterator);

					if(!lpWindow->GetMinimized())
					{
						// ONLY if we are not minimized... if we are minimized then don't return this window

						if(lpWindow->GetDepth() > lCurrentDepth)
						{
							lCurrentDepth = lpWindow->GetDepth();
							lpWindowComponent = (*iterator);
						}

					}
				}
				else
				{
					if((*iterator)->GetDepth() > lCurrentDepth)
					{
						if((*iterator)->GetParent() && (*iterator)->GetParent()->GetComponentType() == EComponentType_ScrollBar)
						{
							// Check scroll area of parent
							ScrollBar* lpScrollBar = (ScrollBar*)(*iterator)->GetParent();

							Dimensions lScrollbarParent;
							if(lpScrollBar->GetParent() != nullptr)
							{
								lScrollbarParent = lpScrollBar->GetParent()->GetDimensions();
							}
							Dimensions lScrollbar = lpScrollBar->GetDimensions();
							Dimensions lScrollArea = lpScrollBar->GetScrollArea();
							int l_scrollleft = lScrollbarParent.m_x + lScrollbar.m_x + lScrollArea.m_x;
							int l_scrollright = lScrollbarParent.m_x + lScrollbar.m_x + lScrollArea.m_x + lScrollArea.m_width;
							int l_scrolltop = lScrollbarParent.m_y + lScrollbar.m_y + lScrollArea.m_y + lScrollArea.m_height;
							int l_scrollbottom = lScrollbarParent.m_y + lScrollbar.m_y + lScrollArea.m_y;

							if( x > l_scrollright || x < l_scrollleft || y > l_scrolltop || y < l_scrollbottom )
							{
								continue;
							}
						}

						lCurrentDepth = (*iterator)->GetDepth();
						lpWindowComponent = (*iterator);
					}
				}
			}
		}
	}

	// If we haven't found any components, but we DO have a GUI window, then return that, else NOTHING
	return lpWindowComponent;
}

Component* SelectionManager::GetCurrentComponent() const
{
	return m_foundComponent;
}

void SelectionManager::MouseMotion(int x, int y)
{
	// Mouse Dragging
	if( (m_foundComponent != 0) && (m_foundComponent == m_firstClickedComponent) && (m_bPressed) )
	{
		if(m_lastX == 0 && m_lastY == 0)
		{
			m_lastX = x;
			m_lastY = y;
		}

		int draggedX = x - m_lastX;
		int draggedY = y - m_lastY;

		MouseEvent dragEvent(m_foundComponent, MOUSE_DRAGGED, draggedX, draggedY, m_buttonPressed);
		m_foundComponent->ProcessMouseEvent(dragEvent);
	}
	else
	{
		// HACK - For components to work 'outside' of their boundaries
		if(m_firstClickedComponent && m_bPressed)
		{
			MouseEvent dragEvent(m_firstClickedComponent, MOUSE_DRAGGED_OUTSIDE, x, y, m_buttonPressed);
			m_firstClickedComponent->ProcessMouseEvent(dragEvent);
		}
	}


	// Mouse motion
	if(m_foundComponent != 0)
	{
		MouseEvent motionEvent(m_foundComponent, MOUSE_MOTION, x, y, 0);
		m_foundComponent->ProcessMouseEvent(motionEvent);
	}

	// Mouse enter and exit
	if(m_foundComponent != m_hoverOverComponent)
	{
		// Exit the current component that is set as hover
		if(m_hoverOverComponent != 0)
		{
			MouseEvent exitEvent(m_hoverOverComponent, MOUSE_EXITED, m_mouseX, m_mouseY, 0);
			m_hoverOverComponent->ProcessMouseEvent(exitEvent);
		}

		// Enter the new component
		if(m_foundComponent != 0)
		{
			MouseEvent enterEvent(m_foundComponent, MOUSE_ENTERED, m_mouseX, m_mouseY, 0);
			m_foundComponent->ProcessMouseEvent(enterEvent);

			if(m_foundComponent == m_firstClickedComponent && m_bPressed)
			{
				MouseEvent pressedEvent(m_foundComponent, MOUSE_PRESSED, m_mouseX, m_mouseY, 0);
				m_foundComponent->ProcessMouseEvent(pressedEvent);
			}
		}

		m_hoverOverComponent = m_foundComponent;
	}
	else if(m_foundComponent == 0)
	{
		// Exit the current component that is set as hover
		if(m_hoverOverComponent != 0)
		{
			MouseEvent exitEvent(m_hoverOverComponent, MOUSE_EXITED, m_mouseX, m_mouseY, 0);
			m_hoverOverComponent->ProcessMouseEvent(exitEvent);
		}
	}

	m_lastX = x;
	m_lastY = y;
}

void SelectionManager::MousePressed(int button)
{
	if (button != MOUSE_BUTTON1)
	{
		// Only allow GUI interactions with the left mouse button
		return;
	}

	if(!m_bPressed)
	{
		m_firstClickedComponent = m_foundComponent;
	}

	if(m_foundComponent != 0)
	{
		if(m_firstClickedComponent == m_foundComponent)
		{
			// Mouse button is pressed down
			MouseEvent l_mouseEvent(m_foundComponent, MOUSE_PRESSED, m_mouseX, m_mouseY, button);
			m_foundComponent->ProcessMouseEvent(l_mouseEvent);
		}

		// Also set focus
		if(m_foundComponent->HasFocusListener())
		{
			// Only visible and enabled components can have focus
			if(m_foundComponent->IsEnabled())
			{
				FocusManager::GetInstance()->SetFocusOwner(m_foundComponent);
			}
		}
		else
		{
			FocusManager::GetInstance()->SetFocusOwner(0);
		}
	}
	else
	{
		FocusManager::GetInstance()->SetFocusOwner(0);
	}

	m_bPressed = true;
	m_buttonPressed = button;
}

void SelectionManager::MouseReleased(int button)
{
	if (button != MOUSE_BUTTON1)
	{
		// Only allow GUI interactions with the left mouse button
		return;
	}

	if(m_foundComponent != 0)
	{
		// Mouse button is released
		MouseEvent l_mouseEvent(m_foundComponent, MOUSE_RELEASED, m_mouseX, m_mouseY, button);
		m_foundComponent->ProcessMouseEvent(l_mouseEvent);

		// Only allow a mouse click if we release on the same component that we initially clicked
		if(m_foundComponent == m_firstClickedComponent)
		{
			if(m_foundComponent != 0)
			{
				// Mouse button has been clicked
				MouseEvent l_mouseClickedEvent(m_foundComponent, MOUSE_CLICKED, m_mouseX, m_mouseY, button);
				m_foundComponent->ProcessMouseEvent(l_mouseClickedEvent);
			}
		}
	}

	// HACK - For components to work 'outside' of their boundaries
	if((m_firstClickedComponent != 0) && (m_firstClickedComponent != m_foundComponent))
	{
		// Mouse button is released
		MouseEvent l_mouseEvent(m_firstClickedComponent, MOUSE_RELEASED_OUTSIDE, m_mouseX, m_mouseY, button);
		m_firstClickedComponent->ProcessMouseEvent(l_mouseEvent);

		m_firstClickedComponent = 0;
	}

	m_bPressed = false;
	m_buttonPressed = -1;
	
	m_lastX = 0;
	m_lastY = 0;	
}