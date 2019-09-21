// ******************************************************************************
//
// Filename:	openglgui.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   Class to manage everything related to the GUI. Has methods
//   that must be called for the GUI to function properly such as
//   Render() and Update(). Also requires access to input events.
//
// Revision History:
//   Initial Revision - 27/09/06
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "openglgui.h"
#include "selectionmanager.h"
#include "focusmanager.h"

#include "../Renderer/Renderer.h"


OpenGLGUI::OpenGLGUI(Renderer* pRenderer)
  : m_pRenderer(pRenderer)
{
	m_mouseX = 0;
	m_mouseY = 0;

	m_currentWindowDepth = 1.0f;

	m_bDegubRender = false;

	m_pFocusedWindow = nullptr;

	m_pDraggingComponentPriority = nullptr;

	m_audio = true;
	m_audioVolume = 1.0f;

	// Create the gui font
	m_pRenderer->CreateFreeTypeFont("media/fonts/arial.ttf", 17, &m_GUIFont);
}

OpenGLGUI::~OpenGLGUI()
{
	SelectionManager::GetInstance()->Destroy();
	FocusManager::GetInstance()->Destroy();
}

bool OpenGLGUI::IsMouseInteractingWithGUIComponent(bool ignoreGUIWindows) const
{
	Component* lCurrentComponent = SelectionManager::GetInstance()->GetCurrentComponent();
	if(lCurrentComponent != 0 && lCurrentComponent->IsVisible() && (ignoreGUIWindows == true ? lCurrentComponent->GetComponentType() != EComponentType_GUIWindow : true))
	{
		return true;
	}

	return false;
}

bool OpenGLGUI::IsKeyboardInteractingWithGUIComponent() const
{
	Component* lCurrentComponent = FocusManager::GetInstance()->GetFocusOwner();
	if(lCurrentComponent != 0)
	{
		return true;
	}

	return false;
}

Component* OpenGLGUI::GetCurrentMouseInteractionComponent() const
{
	Component* lCurrentComponent = SelectionManager::GetInstance()->GetCurrentComponent();
	if(lCurrentComponent != 0 && lCurrentComponent->IsVisible())
	{
		return lCurrentComponent;
	}

	return nullptr;
}

void OpenGLGUI::AddWindow(GUIWindow* window)
{
	window->SetDepth(m_currentWindowDepth);

	window->SetAudio(m_audio);
	window->SetAudioVolume(m_audioVolume);

	// Windows are seperated by a depth of 5
	m_currentWindowDepth += 10.0f;

	window->SetDepth(m_currentWindowDepth);

	m_vpGUIWindowList.push_back(window);

	window->SetGUIParent(this);

	// Make sure the last added window is the focus window
	SetFocusedWindow(window);

	window->AddEventListeners();

	DepthSortGUIWindowChildren();
}

void OpenGLGUI::RemoveWindow(GUIWindow* window)
{
	window->RemoveEventListeners();

	window->SetGUIParent(nullptr);

	m_vpGUIWindowList.erase(std::remove(m_vpGUIWindowList.begin(), m_vpGUIWindowList.end(), window), m_vpGUIWindowList.end());

	// Windows are seperated by a depth of 5
	m_currentWindowDepth -= 10.0f;

	DepthSortGUIWindowChildren();
}

void OpenGLGUI::SetFocusedWindow(GUIWindow* window)
{
	if(window == m_pFocusedWindow)
	{
		// Don't set the same focus window multiple times
		return;
	}

	m_pFocusedWindow = window;

	float lPreviousDepth = window->GetDepth();

	// Draw all the windows which we contain
	GUIWindowList::const_iterator iter_window;
	for(iter_window = m_vpGUIWindowList.begin(); iter_window != m_vpGUIWindowList.end(); ++iter_window)
	{
		if((*iter_window) == window)
		{
			continue;
		}

		if((*iter_window)->GetDepth() >= lPreviousDepth)
		{
			// Reduce the depth of all un-focused windows that had a higher depth
			(*iter_window)->SetDepth((*iter_window)->GetDepth() - 10.0f);
		}
	}

	// Make the focused window have the greatest depth
	m_pFocusedWindow->SetDepth(m_currentWindowDepth);

	// Sort the GUIWindow vector list, by depth
	DepthSortGUIWindowChildren();
}

void OpenGLGUI::SetComponentFocus(Component* component)
{
	FocusManager::GetInstance()->SetFocusOwner(component);
}

void OpenGLGUI::ResetFocus()
{
	FocusManager::GetInstance()->SetFocusOwner(0);
}

void OpenGLGUI::AddComponent(Component* component)
{
	component->SetAudio(m_audio);
	component->SetAudioVolume(m_audioVolume);

	m_vpComponentList.push_back(component);

	// Sort the component vector list, by depth
	DepthSortComponentChildren();

	component->AddEventListeners();
}

void OpenGLGUI::RemoveComponent(Component *component)
{
	component->RemoveEventListeners();

	m_vpComponentList.erase(std::remove(m_vpComponentList.begin(), m_vpComponentList.end(), component), m_vpComponentList.end());
}

Component* OpenGLGUI::GetComponent(Component *component)
{
	ComponentList::const_iterator iter_component;
	for(iter_component = m_vpComponentList.begin(); iter_component != m_vpComponentList.end(); ++iter_component)
	{
		if(component == (*iter_component))
		{
			return component;
		}
	}

	return nullptr;
}

void OpenGLGUI::SetDraggingComponentPriority(Component* pDraggingComponentPriority)
{
	m_pDraggingComponentPriority = pDraggingComponentPriority;
}

void OpenGLGUI::SetDebugRender(bool debug)
{
	m_bDegubRender = debug;

	// Set debug draw on all the windows which we contain
	GUIWindowList::const_iterator iter_window;
	for(iter_window = m_vpGUIWindowList.begin(); iter_window != m_vpGUIWindowList.end(); ++iter_window)
	{
		(*iter_window)->SetDebugRender(m_bDegubRender);
	}

	// Then Set debug draw on all the standalone components we contain
	ComponentList::const_iterator iter_component;
	for(iter_component = m_vpComponentList.begin(); iter_component != m_vpComponentList.end(); ++iter_component)
	{
		(*iter_component)->SetDebugRender(m_bDegubRender);
	}
}

bool OpenGLGUI::IsDebugRender()
{
	return m_bDegubRender;
}

void OpenGLGUI::Render()
{
	// Sort the GUI window vector list, by depth
	DepthSortGUIWindowChildren();

	// Sort the component vector list, by depth
	DepthSortComponentChildren();

	// Draw all the standalone components we contain
	ComponentList::const_iterator iter_component;
	for(iter_component = m_vpComponentList.begin(); iter_component != m_vpComponentList.end(); ++iter_component)
	{
		if((*iter_component) == m_pDraggingComponentPriority)
			continue;

		(*iter_component)->Draw();
	}

	// Draw all the windows which we contain
	GUIWindowList::const_iterator iter_window;
	for(iter_window = m_vpGUIWindowList.begin(); iter_window != m_vpGUIWindowList.end(); ++iter_window)
	{
		(*iter_window)->Draw();
	}

	// Draw the dragging icon with priority last, due to transparency issues with windows that are drawn before.
	if(m_pDraggingComponentPriority != nullptr)
	{
		m_pDraggingComponentPriority->Draw();
	}
}

void OpenGLGUI::ResetSelectionManager()
{
	SelectionManager::GetInstance()->ResetCurrentComponent();
}

void OpenGLGUI::Update(float deltaTime)
{
	// Update the selection manager
	SelectionManager::GetInstance()->Update(m_mouseX, m_mouseY);


	// Update all the windows which we contain
	GUIWindowList::const_iterator iter_window;
	for(iter_window = m_vpGUIWindowList.begin(); iter_window != m_vpGUIWindowList.end(); ++iter_window)
	{
		if((*iter_window)->IsVisible())
		{
			(*iter_window)->Update(deltaTime);
		}
	}

	// Then update all the standalone components we contain
	ComponentList::const_iterator iter_component;
	for(iter_component = m_vpComponentList.begin(); iter_component != m_vpComponentList.end(); ++iter_component)
	{
		if((*iter_component)->IsVisible())
		{
			(*iter_component)->Update(deltaTime);
		}
	}
}

void OpenGLGUI::ImportMouseMotion(int x, int y)
{
	m_mouseX = x;
	m_mouseY = y;

	// Forward to the selection manager
	SelectionManager::GetInstance()->MouseMotion(x, y);
}

void OpenGLGUI::MousePressed(int button)
{
	// Forward to the selection manager
	SelectionManager::GetInstance()->MousePressed(button);
}

void OpenGLGUI::MouseReleased(int button)
{
	// Forward to the selection manager
	SelectionManager::GetInstance()->MouseReleased(button);
}

void OpenGLGUI::KeyPressed(int keyCode, int modifier)
{
	// Forward to the focus manager
	FocusManager::GetInstance()->KeyPressed(keyCode, modifier);
}

void OpenGLGUI::KeyReleased(int keyCode, int modifier)
{
	// Forward to the focus manager
	FocusManager::GetInstance()->KeyReleased(keyCode, modifier);
}

void OpenGLGUI::CharacterEntered(int characterCode)
{
	// Forward to the focus manager
	FocusManager::GetInstance()->CharacterEntered(characterCode);
}

void OpenGLGUI::DepthSortGUIWindowChildren()
{
	sort(m_vpGUIWindowList.begin(), m_vpGUIWindowList.end(), GUIWindow::DepthLessThan);
}

void OpenGLGUI::DepthSortComponentChildren()
{
	sort(m_vpComponentList.begin(), m_vpComponentList.end(), Component::DepthLessThan);
}

void OpenGLGUI::SetAudio(bool set)
{
	m_audio = set;

	GUIWindowList::const_iterator iter_window;
	for (iter_window = m_vpGUIWindowList.begin(); iter_window != m_vpGUIWindowList.end(); ++iter_window)
	{
		(*iter_window)->SetAudio(set);
	}

	ComponentList::const_iterator iter_component;
	for (iter_component = m_vpComponentList.begin(); iter_component != m_vpComponentList.end(); ++iter_component)
	{
		(*iter_component)->SetAudio(set);
	}
}

void OpenGLGUI::SetAudioVolume(float volume)
{
	m_audioVolume = volume;

    GUIWindowList::const_iterator iter_window;
    for(iter_window = m_vpGUIWindowList.begin(); iter_window != m_vpGUIWindowList.end(); ++iter_window)
    {
        (*iter_window)->SetAudioVolume(volume);
    }

    ComponentList::const_iterator iter_component;
    for(iter_component = m_vpComponentList.begin(); iter_component != m_vpComponentList.end(); ++iter_component)
    {
        (*iter_component)->SetAudioVolume(volume);
    }
}