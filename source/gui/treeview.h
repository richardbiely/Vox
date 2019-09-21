// ******************************************************************************
//
// Filename:	treeview.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A simple GUI component that simple displays a text label
//
// Revision History:
//   Initial Revision - 13/10/14
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "component.h"
#include "container.h"
#include "label.h"
#include "button.h"
#include "scrollbar.h"


class TreeNode;
class TreeView;
typedef std::vector<TreeNode*> TreeNodeList;

class TreeNode
{
public:
	TreeView* m_pTreeView;

	Label* m_pName;
	Button* m_pExpandButton;

	bool m_expanded;
	int m_x;
	int m_y;
	int m_width;
	int m_height;

	TreeNode* m_pParentNode;
	TreeNodeList m_vpChildren;
};


class TreeView : public Container
{
public:
	/* Public methods */
	TreeView(Renderer* pRenderer, unsigned int GUIFont, string rootName);
	~TreeView();

	void ClearTreeView();
	void ClearNode(TreeNode* pNode);

	void SetRootName(string rootName);

	TreeNode* FindNode(string name, TreeNode* pRoot);

	void AddRootNode(string name);
	void AddChildNode(string parentName, string name);

	bool GetNodeVisible(TreeNode* pNode);
	void SetNodeVisible(TreeNode* pNode, bool visible);
	void SetNodeDimensions(TreeNode* pNode, int x, int y, int width, int height, int *additionalYOffset);

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	EComponentType GetComponentType() const override;

	void UpdateScrollbar();

protected:
	/* Protected methods */
	void DrawChildren() override;
	void DrawSelf() override;

private:
	/* Private methods */
	static void _ExpandPressed(void *lpData);
	void ExpandPressed(TreeNode* pNode);

	static void _ScrollbarChanged(void *lpData);
	void ScrollbarChanged();

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	unsigned int m_GUIFont;

	int m_xIndent;
	int m_yOffset;

	RenderRectangle *m_pBackgroundDefault;
	RenderRectangle *m_pBackgroundDisabled;

	ScrollBar mScrollBar;

	int m_numOpenNodes;

	TreeNode* m_root;
};
