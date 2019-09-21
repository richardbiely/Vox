// ******************************************************************************
//
// Filename:	component.h
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

#pragma once

#include <vector>
#include <string>
#include <algorithm>

#include "dimensions.h"
#include "point.h"

#include "mouselistener.h"
#include "keylistener.h"
#include "focuslistener.h"

#include "../Renderer/Renderer.h"
#include "../Renderer/colour.h"

typedef std::vector<MouseListener*> MouseListenerList;
typedef std::vector<KeyListener*> KeyListenerList;
typedef std::vector<FocusListener*> FocusListenerList;


enum EVerticalAlignment
{
	eVerticalAlignment_Top = 0,
	eVerticalAlignment_Center,
	eVerticalAlignment_Bottom
};

enum EHorizontalAlignment
{
	eHorizontalAlignment_Left = 0,
	eHorizontalAlignment_Center,
	eHorizontalAlignment_Right
};

enum EComponentType
{
	EComponentType_Component = 0,
	EComponentType_Container,
	EComponentType_AbstractButton,
	EComponentType_Button,
	EComponentType_CheckBox,
	EComponentType_OptionBox,
	EComponentType_OptionController,
	EComponentType_MenuBar,
	EComponentType_Menu,
	EComponentType_MenuItem,
	EComponentType_Label,
	EComponentType_RenderRectangle,
	EComponentType_Icon,
	EComponentType_MultiTextureIcon,
	EComponentType_DirectDrawRectangle,
	EComponentType_DraggableRenderRectangle,
	EComponentType_Slider,
	EComponentType_TitleBar,
	EComponentType_TextBox,
	EComponentType_MultiLineTextBox,
	EComponentType_ProgressBar,
	EComponentType_ScrollBar,
	EComponentType_PulldownMenu,
	EComponentType_GUIWindow,
	EComponentType_TreeView,

	EComponentType_Num,
};


class Component
{
public:
	/* Public methods */
	Component(Renderer* pRenderer);

	virtual ~Component();

	virtual void SetVisible(bool Visible);
	bool IsVisible() const;

	virtual void SetEnabled(bool enabled);
	bool IsEnabled() const;

	bool IsParentVisible() const;
	bool IsParentEnabled() const;

	void SetTransparency(float transparency);
	float GetTransparency() const;

	void SetDepth(float depth);
	float GetDepth() const;
		
	void SetParent(Component *pParent);
	Component* GetParent() const;

	virtual EComponentType GetComponentType() const;

	virtual void SetLocation(int x, int y);
	virtual void SetLocation(const Point& p);

	void SetX(int x);
	void SetY(int y);
	void GetLocation(int& x, int& y) const;
	Point GetLocation() const;
	Point GetLocationOnScreen() const;

	void SetSize(int width, int height);
	void GetSize(int& width, int& height);

	virtual void SetDebugRender(bool debug);
	bool IsDebugRender();

	virtual void SetDimensions(int x, int y, int width, int height);
	virtual void SetDimensions(const Dimensions& r);
	virtual void SetWidth(int width);
	virtual void SetHeight(int height);
	Dimensions GetDimensions() const;
	Dimensions* GetDimensionsPointer();

	virtual void AddEventListeners();
	virtual void RemoveEventListeners();

	bool HasMouseListener() const;
	void AddMouseListener(MouseListener* listener);
	void RemoveMouseListener(MouseListener* listener);
	void RemoveMouseListeners();
	void ProcessMouseEvent(const MouseEvent& lEvent);

	bool HasKeyListener() const;
	void AddKeyListener(KeyListener* listener);
	void RemoveKeyListener(KeyListener* listener);
	void RemoveKeyListeners();
	void ProcessKeyEvent(const KeyEvent& lEvent);

	bool HasFocusListener() const;
	void AddFocusListener(FocusListener* listener);
	void RemoveFocusListener(FocusListener* listener);
	void RemoveFocusListeners();
	void ProcessFocusEvent(const FocusEvent& lEvent);

	bool HasFocus() const;
	void SetFocus(bool enable);

  virtual void SetAudio(bool set);
  virtual void SetAudioVolume(float volume);

	virtual bool IsContainer() const;
	virtual bool IsRootContainer() const;

	virtual void Update(float deltaTime);

	void Draw();

	// < Operator (Used for component depth sorting)
	bool operator<(const Component &w) const;
	static bool DepthLessThan(const Component *lhs, const Component *rhs);

protected:
	/* Protected methods */
	virtual void DrawSelf();
	virtual void DrawChildren();

private:
	/* Private methods */
	void DrawDebug();

public:
	/* Public members */

protected:
	/* Protected members */
	Dimensions m_dimensions;

	Renderer* m_pRenderer;

	float m_depth;

	float m_audioVolume;
  bool m_audio;  

private:
	/* Private members */
	bool m_bVisible;
	bool m_bEnabled;
	bool m_bFocused;

	bool m_bDegubRender;

	float m_fTransparency;

	MouseListenerList m_vpMouseListenerList;
	KeyListenerList m_vpKeyListenerList;
	FocusListenerList m_vpFocusListenerList;

	bool m_bMouseListenerRemoved;
	bool m_bKeyListenerRemoved;
	bool m_bFocusListenerRemoved;

	Component* m_pParent;
};