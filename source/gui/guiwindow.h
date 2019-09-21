// ******************************************************************************
//
// Filename:	guiwindow.h
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

#pragma once

#include "container.h"
#include "titlebar.h"


// Forward declaration of GUIWindowList
class GUIWindow;
class OpenGLGUI;

typedef std::vector<GUIWindow*> GUIWindowList;


class GUIWindow : public Container, public MouseListener
{
public:
	/* Public methods */
	GUIWindow(Renderer* pRenderer, unsigned int GUIFont, const std::string& title);

	GUIWindow(Renderer* pRenderer, unsigned int GUIFont, TitleBar* ptitleBar);

	~GUIWindow();

	void AddEventListeners() override;
	void RemoveEventListeners() override;

	virtual bool IsRootContainer() const override;

	void AddGUIWindow(GUIWindow* window);
	void RemoveGUIWindow(GUIWindow* window);
	void RemoveAllGUIWindows();

	const GUIWindowList& GetGUIWindows() const;

	TitleBar* GetTitleBar() const;

	void SetTitleBarDimensions(int xOffset, int yOffset, int width, int height);
	void SetTitleOffset(int xOffset, int yOffset);
	void SetTitlebarBackgroundIcon(RenderRectangle* icon);

	void SetBackgroundIcon(RenderRectangle* icon);

	void AddComponent(Component* component);
	void RemoveComponent(Component* component);

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	void SetLocation(int x, int y) override;
	void SetLocation(const Point& p) override;

	void SetTitle(const std::string& title);
	const std::string GetTitle() const;

	void SetDebugRender(bool debug);
	void SetOutlineRender(bool outline);

	void Show();
	void Hide();

	bool GetMinimized() const;
	void SetMinimized(bool minimized);

	void SetMinimizedDefaultIcon(RenderRectangle* icon);
	void SetMinimizedSelectedIcon(RenderRectangle* icon);
	void SetMinimizedHoverIcon(RenderRectangle* icon);
	void SetMinimizedDisabledIcon(RenderRectangle* icon);

	void SetCloseDefaultIcon(RenderRectangle* icon);
	void SetCloseSelectedIcon(RenderRectangle* icon);
	void SetCloseHoverIcon(RenderRectangle* icon);
	void SetCloseDisabledIcon(RenderRectangle* icon);

	void AllowMoving(bool val);
	void AllowClosing(bool val);
	void AllowMinimizing(bool val);
	void AllowScrolling(bool val);
	void SnapToApplication(bool val);

	void SetApplicationDimensions(int width, int height);

	void SetApplicationBorder(int left, int right, int top, int bottom);

	void SetRenderTitleBar(bool lbRender);
	void SetRenderWindowBackground(bool lbRender);

	void DepthSortGUIWindowChildren();

	EComponentType GetComponentType() const override;

	void SetGUIParent(OpenGLGUI* pParent);
	void SetFocusWindow();

	void Update(float deltaTime);

	// < Operator (Used for GUIWindow depth sorting)
	bool operator<(const GUIWindow& w) const;
	static bool DepthLessThan(const GUIWindow* lhs, const GUIWindow* rhs);

protected:
	/* Protected methods */
	void MousePressed(const MouseEvent& lEvent) override;

	void DrawSelf() override;
	void DrawChildrenFirst();
	void DrawChildren() override;

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	GUIWindowList m_vpGUIWindowList;

	TitleBar* m_titleBar;

	unsigned int m_GUIFont;

	bool m_bMinimized;

	// Bools to control functionality of window
	bool m_bAllowMoving;
	bool m_bAllowClosing;
	bool m_bAllowMinimizing;
	bool m_bAllowScrolling;
	bool m_bSnapToWindow;

	bool m_bRenderTitleBar;
	bool m_bRenderWindowBackground;

	bool mb_ownsTitleBar;

	int m_applicationWidth;
	int m_applicationHeight;

	RenderRectangle* m_pBackgroundIcon;

	bool m_outlineRender;

	int m_applicationBorderLeft;
	int m_applicationBorderRight;
	int m_applicationBorderTop;
	int m_applicationBorderBottom;

	OpenGLGUI* m_pParentGUI;

	// Friend classes
	friend class TitleBar;
	friend class GUIWindowMinimizeButton;
	friend class GUIWindowCloseButton;
};
