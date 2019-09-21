// ******************************************************************************
//
// Filename:	component.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   An abstract class for GUI components.
//
// Revision History:
//   Initial Revision - 17/09/06
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "component.h"
#include "selectionmanager.h"
#include "../utils/TimeManager.h"


Component::Component(Renderer* pRenderer)
  : m_pRenderer(pRenderer)
{
	m_bVisible = true;
	m_bEnabled = true;
	m_bFocused = false;

	m_bDegubRender = false;

	m_fTransparency = 1.0f;

	m_depth = 1.0f;

    m_audio = true;
    m_audioVolume = 1.0f;

	m_pParent = nullptr;

	m_dimensions = Dimensions(0, 0, 0, 0);

	m_bMouseListenerRemoved = false;
	m_bKeyListenerRemoved = false;
	m_bFocusListenerRemoved = false;
}

Component::~Component()
{
	/* Nothing */
}

void Component::SetVisible(bool Visible)
{
	m_bVisible = Visible;
}

bool Component::IsVisible() const
{
	return m_bVisible;
}

void Component::SetEnabled(bool enabled)
{
	m_bEnabled = enabled;
}

bool Component::IsEnabled() const
{
	return m_bEnabled;
}

bool Component::IsParentVisible() const
{
	if(m_pParent == nullptr)
	{
		return IsVisible();
	}

	return m_pParent->IsParentVisible();
}

bool Component::IsParentEnabled() const
{
	if(m_pParent == nullptr)
	{
		return IsEnabled();
	}

	return m_pParent->IsParentEnabled();
}

void Component::SetTransparency(float transparency)
{
	m_fTransparency = transparency;
}

float Component::GetTransparency() const
{
	return m_fTransparency;
}

void Component::SetDepth(float depth)
{
	m_depth = depth;
}

float Component::GetDepth() const
{
	float depth = m_depth;

	if(m_pParent != nullptr)
	{
		depth += m_pParent->GetDepth();
	}

	return depth;
}

void Component::SetParent(Component *pParent)
{
	m_pParent = pParent;
}

Component* Component::GetParent() const
{
	return m_pParent;
}

EComponentType Component::GetComponentType() const
{
	return EComponentType_Component;
}

void Component::SetLocation(int x, int y)
{
	m_dimensions.m_x = x;
	m_dimensions.m_y = y;
}

void Component::SetLocation(const Point& p)
{
	m_dimensions.m_x = p.m_x;
	m_dimensions.m_y = p.m_y;
}

void Component::SetX(int x)
{
	m_dimensions.m_x = x;
}

void Component::SetY(int y)
{
	m_dimensions.m_y = y;
}

void Component::GetLocation(int& x, int& y) const
{
	x = m_dimensions.m_x;
	y = m_dimensions.m_y;
}

Point Component::GetLocation() const
{
	return Point(m_dimensions.m_x, m_dimensions.m_y);
}

void Component::SetSize(int width, int height)
{
	m_dimensions.m_width = width;
	m_dimensions.m_height = height;
}

void Component::GetSize(int& width, int& height)
{
	width = m_dimensions.m_width;
	height = m_dimensions.m_height;
}

void Component::SetDebugRender(bool debug)
{
	m_bDegubRender = debug;
}

bool Component::IsDebugRender()
{
	return m_bDegubRender;
}

Point Component::GetLocationOnScreen() const
{
	// Start with this component's location, and add all parents locations
	Point location = GetLocation();

	for(Component* parent = GetParent(); parent != 0;)
	{
		Point parentLocation = parent->GetLocation();

		location.m_x += parentLocation.m_x;
		location.m_y += parentLocation.m_y;

		parent = parent->GetParent();
	}

	return location;
}

void Component::SetDimensions(int x, int y, int width, int height)
{
	m_dimensions.m_x = x;
	m_dimensions.m_y = y;
	m_dimensions.m_width = width;
	m_dimensions.m_height = height;
}

void Component::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void Component::SetWidth(int width)
{
	m_dimensions.m_width = width;
}

void Component::SetHeight(int height)
{
	m_dimensions.m_height = height;
}

Dimensions Component::GetDimensions() const
{
	return m_dimensions;
}

Dimensions* Component::GetDimensionsPointer()
{
	return &m_dimensions;
}

void Component::AddEventListeners()
{
	/* Nothing... */
}

void Component::RemoveEventListeners()
{
	/* Nothing... */
}

bool Component::HasMouseListener() const
{
	return(m_vpMouseListenerList.size() > 0);
}

void Component::AddMouseListener(MouseListener* listener)
{
	m_vpMouseListenerList.push_back(listener);
	SelectionManager::GetInstance()->AddComponent(this);
}

void Component::RemoveMouseListener(MouseListener* listener)
{
	m_vpMouseListenerList.erase(std::remove(m_vpMouseListenerList.begin(), m_vpMouseListenerList.end(), listener), m_vpMouseListenerList.end());
	SelectionManager::GetInstance()->RemoveComponent(this);

	m_bMouseListenerRemoved = true;
}

void Component::RemoveMouseListeners()
{
	m_vpMouseListenerList.clear();
	SelectionManager::GetInstance()->RemoveComponent(this);

	m_bMouseListenerRemoved = true;
}

void Component::ProcessMouseEvent(const MouseEvent& lEvent)
{
	if(!IsEnabled())
	{
		return;
	}

	MouseListenerList::iterator iterator;

	for(iterator = m_vpMouseListenerList.begin(); iterator != m_vpMouseListenerList.end(); ++iterator)
	{
		switch(lEvent.GetID())
		{
		case MOUSE_PRESSED:
			{
				(*iterator)->MousePressed(lEvent);
				break;
			}
		case MOUSE_RELEASED:
			{
				(*iterator)->MouseReleased(lEvent);
				break;
			}
		case MOUSE_RELEASED_OUTSIDE:
			{
				(*iterator)->MouseReleasedOutside(lEvent);
				break;
			}
		case MOUSE_ENTERED:
			{
				(*iterator)->MouseEntered(lEvent);
				break;
			}
		case MOUSE_EXITED:
			{
				(*iterator)->MouseExited(lEvent);
				break;
			}
		case MOUSE_CLICKED:
			{
				(*iterator)->MouseClicked(lEvent);
				break;
			}
		case MOUSE_DRAGGED:
			{
				(*iterator)->MouseDragged(lEvent);
				break;
			}
		case MOUSE_DRAGGED_OUTSIDE: // HACK
			{
				(*iterator)->MouseDraggedOutside(lEvent);
				break;
			}
		case MOUSE_MOTION:
			{
				(*iterator)->MouseMotion(lEvent);
				break;
			}
		}

		if(m_bMouseListenerRemoved)
		{
			// We have removed a mouse listener while processing this iterator, all following iterators will be invalid, lets just break out the process loop
			m_bMouseListenerRemoved = false;
			break;
		}
	}
}

bool Component::HasKeyListener() const
{
	return(m_vpKeyListenerList.size() > 0);
}

void Component::AddKeyListener(KeyListener* listener)
{
	m_vpKeyListenerList.push_back(listener);
}

void Component::RemoveKeyListener(KeyListener* listener)
{
	m_vpKeyListenerList.erase(std::remove(m_vpKeyListenerList.begin(), m_vpKeyListenerList.end(), listener), m_vpKeyListenerList.end());

	m_bKeyListenerRemoved = true;
}

void Component::RemoveKeyListeners()
{
	m_vpKeyListenerList.clear();

	m_bKeyListenerRemoved = true;
}

void Component::ProcessKeyEvent(const KeyEvent& lEvent)
{
	if(!IsEnabled())
	{
		return;
	}

	KeyListenerList::iterator iter;

	for(iter = m_vpKeyListenerList.begin(); iter != m_vpKeyListenerList.end(); ++iter)
	{
		switch(lEvent.GetID())
		{
		case KEY_PRESSED:
			{
				(*iter)->KeyPressed(lEvent);
				break;
			}
		case KEY_RELEASED:
			{
				(*iter)->KeyReleased(lEvent);
				break;
			}
		case KEY_TYPED:
			{
				(*iter)->KeyTyped(lEvent);
				break;
			}

		case CHARACTER_ENTERED:
			{
				(*iter)->CharacterEntered(lEvent);
				break;
			}
		}

		if(m_bKeyListenerRemoved)
		{
			// We have removed a key listener while processing this iterator, all following iterators will be invalid, lets just break out the process loop
			m_bKeyListenerRemoved = false;
			break;
		}
	}
}

bool Component::HasFocusListener() const
{
	return(m_vpFocusListenerList.size() > 0);
}

void Component::AddFocusListener(FocusListener* listener)
{
	m_vpFocusListenerList.push_back(listener);
}

void Component::RemoveFocusListener(FocusListener* listener)
{
	m_vpFocusListenerList.erase(std::remove(m_vpFocusListenerList.begin(), m_vpFocusListenerList.end(), listener), m_vpFocusListenerList.end());

	m_bFocusListenerRemoved = true;
}

void Component::RemoveFocusListeners()
{
	m_vpFocusListenerList.clear();

	m_bFocusListenerRemoved = true;
}

void Component::ProcessFocusEvent(const FocusEvent& lEvent)
{
	FocusListenerList::iterator iter;

	for(iter = m_vpFocusListenerList.begin(); iter != m_vpFocusListenerList.end(); ++iter)
	{
		switch(lEvent.GetID())
		{
		case FOCUS_LOST:
			{
				if(HasFocus())
				{
					(*iter)->FocusLost(lEvent);
				}
				break;
			}
		case FOCUS_GAINED:
			{
				if(!HasFocus())
				{
					(*iter)->FocusGained(lEvent);
				}
				break;
			}
		}

		if(m_bFocusListenerRemoved)
		{
			// We have removed a focus listener while processing this iterator, all following iterators will be invalid, lets just break out the process loop
			m_bFocusListenerRemoved = false;
			break;
		}
	}

	if(lEvent.GetID() == FOCUS_GAINED)
	{
		if(!HasFocus())
		{
			SetFocus(true);
		}
	}
	else
	{
		if(HasFocus())
		{
			SetFocus(false);
		}
	}
}

bool Component::HasFocus() const
{
	return m_bFocused;
}

void Component::SetFocus(bool enable)
{
	m_bFocused = enable;
}

void Component::SetAudio(bool set)
{
    m_audio = set;
}

void Component::SetAudioVolume(float volume)
{
    m_audioVolume = volume;
}

bool Component::IsContainer() const
{
	return false;
}

bool Component::IsRootContainer() const
{
	return false;
}

void Component::Update(float deltaTime)
{
}

void Component::Draw()
{
	if(IsVisible() && m_dimensions.m_width > 0 && m_dimensions.m_height > 0)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->TranslateWorldMatrix((float)m_dimensions.m_x, (float)m_dimensions.m_y, 0.0f);

			m_pRenderer->PushMatrix();
				// First draw ourself
				DrawSelf();

				// Draw the base component parts
				Component::DrawSelf();

				// Debug render
				if(m_bDegubRender)
				{
					DrawDebug();
				}

				// Then draw our children
				DrawChildren();

			m_pRenderer->PopMatrix();

		m_pRenderer->PopMatrix();
	}
}

// < Operator (Used for window pane sorting)
bool Component::operator<(const Component &w) const
{
	return(GetDepth() < w.GetDepth());
}

bool Component::DepthLessThan(const Component *lhs, const Component *rhs)
{
	if ((*lhs) < (*rhs)) { return true; }
	if ((*rhs) < (*lhs)) { return false; }
	return (*lhs) < (*rhs);
}

void Component::DrawSelf()
{
	/* Nothing */
}

void Component::DrawChildren()
{
	/* Nothing */
}

void Component::DrawDebug()
{
	int l_containerWidth = m_dimensions.m_width;
	int l_containerHeight = m_dimensions.m_height;
	float l_depth = m_depth + 2.0f;

	float l_outlineX1 = 0.5f;
	float l_outlineX2 = l_containerWidth + 0.5f;
	float l_outlineY1 = 0.5f;
	float l_outlineY2 = l_containerHeight + 0.5f;

	// Draw the outline
	m_pRenderer->PushMatrix();
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->EnableImmediateMode(IM_LINE_LOOP);
		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY2, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY2, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
}