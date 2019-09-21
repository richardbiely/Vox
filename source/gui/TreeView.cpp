// ******************************************************************************
//
// Filename:	treeview.cpp
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

#include "treeview.h"

#include "../Renderer/Renderer.h"


TreeView::TreeView(Renderer* pRenderer, unsigned int GUIFont, string rootName)
  : Container(pRenderer),
    mScrollBar(pRenderer)
{
	m_GUIFont = GUIFont;

	m_root = nullptr;

	m_pBackgroundDefault = new DirectDrawRectangle(pRenderer);
	m_pBackgroundDisabled = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pBackgroundDefault;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pBackgroundDisabled;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	Add(m_pBackgroundDefault);
	Add(m_pBackgroundDisabled);

	mScrollBar.SetScrollDirection(EScrollBarDirection_Vertical);
	mScrollBar.SetScrollPosition(0.75f);

	mScrollBar.SetValueChangedCallback(_ScrollbarChanged);
	mScrollBar.SetValueChangedCallBackData(this);

	Add(&mScrollBar);

	m_numOpenNodes = 0;

	AddRootNode(rootName);

	m_xIndent = 8;
	m_yOffset = -20;
}

TreeView::~TreeView()
{
	ClearTreeView();
}

void TreeView::ClearTreeView()
{
	ClearNode(m_root);

	m_root = nullptr;
}

void TreeView::ClearNode(TreeNode* pNode)
{
	for(int i = 0; i < pNode->m_vpChildren.size(); i++)
	{
		ClearNode(pNode->m_vpChildren[i]);
	}
	pNode->m_vpChildren.clear();

	Remove(pNode->m_pName);
	delete pNode->m_pName;
	pNode->m_pName = nullptr;

	Remove(pNode->m_pExpandButton);
	delete pNode->m_pExpandButton;
	pNode->m_pExpandButton = nullptr;

	delete pNode;
	pNode = nullptr;
}

void TreeView::SetRootName(string rootName)
{
	m_root->m_pName->SetText(rootName);
}

TreeNode* TreeView::FindNode(string name, TreeNode* pNode)
{
	if(pNode != nullptr)
	{
		if(pNode->m_pName->GetText() == name)
		{
			return pNode;
		}

		for(int i = 0; i < pNode->m_vpChildren.size(); i++)
		{
			TreeNode* foundNode = FindNode(name, pNode->m_vpChildren[i]);

			if(foundNode != nullptr)
			{
				return foundNode;
			}
		}
	}

	return nullptr;
}

void TreeView::AddRootNode(string name)
{
	if(m_root == nullptr)
	{
		m_root = new TreeNode();
		m_root->m_pTreeView = this;
		m_root->m_pName = new Label(m_pRenderer, m_GUIFont, name, Colour(0.0f, 0.0f, 0.0f, 1.0f));
		m_root->m_pExpandButton = new Button(m_pRenderer, m_GUIFont, "+");
		m_root->m_pExpandButton->SetCallBackFunction(_ExpandPressed);
		m_root->m_pExpandButton->SetCallBackData(m_root);
		m_root->m_pExpandButton->SetLabelOffset(0, 0);
		m_root->m_expanded = false;
		m_root->m_pParentNode = nullptr;

		Add(m_root->m_pName);
		Add(m_root->m_pExpandButton);

		m_root->m_pName->SetVisible(true);
		m_root->m_pExpandButton->SetVisible(true);
	}

	UpdateScrollbar();
}

void TreeView::AddChildNode(string parentName, string name)
{
	TreeNode* pNode = FindNode(parentName, m_root);

	if(pNode != nullptr)
	{
		TreeNode* pNewNode = new TreeNode();
		pNewNode->m_pTreeView = this;
		pNewNode->m_pName = new Label(m_pRenderer, m_GUIFont, name, Colour(0.0f, 0.0f, 0.0f, 1.0f));
		pNewNode->m_pExpandButton = new Button(m_pRenderer, m_GUIFont, "+");
		pNewNode->m_pExpandButton->SetCallBackFunction(_ExpandPressed);
		pNewNode->m_pExpandButton->SetCallBackData(pNewNode);
		pNewNode->m_pExpandButton->SetLabelOffset(0, 0);
		pNewNode->m_expanded = false;
		pNewNode->m_pParentNode = pNode;

		Add(pNewNode->m_pName);
		Add(pNewNode->m_pExpandButton);

		pNewNode->m_pName->SetVisible(false);
		pNewNode->m_pExpandButton->SetVisible(false);

		pNode->m_vpChildren.push_back(pNewNode);
	}

	// Reset all component dimensions starting from the root node
	int yCul = -20 + m_yOffset;
	m_numOpenNodes = 0;
	SetNodeDimensions(m_root, m_dimensions.m_x, m_dimensions.m_y, m_dimensions.m_width, m_dimensions.m_height, &yCul);

	UpdateScrollbar();
}

bool TreeView::GetNodeVisible(TreeNode* pNode)
{
	bool visible = true;

	if(pNode->m_pParentNode != nullptr)
	{
		if(pNode->m_pParentNode->m_expanded == false)
			return false;

		bool parentVisible = GetNodeVisible(pNode->m_pParentNode);

		return parentVisible;
	}

	return visible;
}

void TreeView::SetNodeVisible(TreeNode* pNode, bool visible)
{
	pNode->m_pName->SetVisible(visible);
	pNode->m_pExpandButton->SetVisible(visible);

	if(pNode->m_vpChildren.size() == 0)
	{
		pNode->m_pExpandButton->SetVisible(false);
	}

	for(int i = 0; i < pNode->m_vpChildren.size(); i++)
	{
		if(pNode->m_expanded && visible)
		{
			SetNodeVisible(pNode->m_vpChildren[i], true);
		}
		else
		{
			SetNodeVisible(pNode->m_vpChildren[i], false);
		}
	}
}

void TreeView::SetNodeDimensions(TreeNode* pNode, int x, int y, int width, int height, int *additionalYOffset)
{
	int expanderSpacer = 3;
	int yOffset = 4;
	int buttonSize = 10;

	pNode->m_pName->SetLocation(x + expanderSpacer + expanderSpacer + buttonSize, y);
	pNode->m_pExpandButton->SetDimensions(x + expanderSpacer, y + yOffset, buttonSize, buttonSize);

	pNode->m_x = x;
	pNode->m_y = y;
	pNode->m_width = width;
	pNode->m_height = height;

	// Go through and add the correct dimensions for the children and set their visibility
	x += m_xIndent;
	for(int i = 0; i < pNode->m_vpChildren.size(); i++)
	{
		if(GetNodeVisible(pNode->m_vpChildren[i]))
		{
			y = (*additionalYOffset);
			(*additionalYOffset) += m_yOffset;

			m_numOpenNodes += 1;
		}

		SetNodeDimensions(pNode->m_vpChildren[i], x, y, width, height, additionalYOffset);

		if(GetNodeVisible(pNode->m_vpChildren[i]))
		{
			SetNodeVisible(pNode->m_vpChildren[i], true);
		}
		else
		{
			SetNodeVisible(pNode->m_vpChildren[i], false);
		}

		if(pNode->m_expanded)
		{
			pNode->m_pExpandButton->SetLabelOffset(1, 2);
			pNode->m_pExpandButton->SetLabel("-");
		}
		else
		{
			pNode->m_pExpandButton->SetLabelOffset(0, 0);
			pNode->m_pExpandButton->SetLabel("+");
		}
	}
}

void TreeView::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);

	m_pBackgroundDefault->SetDimensions(0, -height, width, height);
	m_pBackgroundDisabled->SetDimensions(0, -height, width, height);

	mScrollBar.SetDimensions(width, -height, 20, height);

	int yCul = m_yOffset;
	m_numOpenNodes = 0;
	SetNodeDimensions(m_root, 0, -20, width, height, &yCul);
}

void TreeView::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

EComponentType TreeView::GetComponentType() const
{
	return EComponentType_TreeView;
}

void TreeView::UpdateScrollbar()
{
	int visibleSize = GetDimensions().m_height;
	int l_TextHeight = m_pRenderer->GetFreeTypeTextHeight(m_GUIFont, "%s", "a");
	int yOffset = 4;

	int neededHeight = (m_numOpenNodes+1) * (l_TextHeight+yOffset);
	int heightDiff = neededHeight - visibleSize;

	float scrollRatio = mScrollBar.GetScrollRatio();
	int offsetLabelY = (int)(heightDiff * scrollRatio);

	if(heightDiff > 0)
	{
		mScrollBar.SetScrollSize((float)visibleSize / (float)neededHeight);
		mScrollBar.SetDisabled(false);

		int yCul = -20 + m_yOffset + offsetLabelY;
		m_numOpenNodes = 0;
		SetNodeDimensions(m_root, 0, -20 + offsetLabelY, m_dimensions.m_width, m_dimensions.m_height, &yCul);
	}
	else
	{
		mScrollBar.SetScrollSize(0.0f);
		mScrollBar.SetDisabled(true);
	}
}

void TreeView::DrawChildren()
{
	// Do a scissor test here
	int lScissorX = GetLocation().m_x-1;
	int lScissorY = GetLocation().m_y-1;
	if(GetParent() != nullptr)
	{
		lScissorX = GetParent()->GetLocation().m_x + lScissorX;
		lScissorY = GetParent()->GetLocation().m_y + lScissorY;
	}	
	int lScissorWidth = GetDimensions().m_width+2+20;
	int lScissorHeight = GetDimensions().m_height+2;
	int l_depth = static_cast<int>(GetDepth());

	lScissorY = (lScissorY - lScissorHeight) + 2;

	bool lbNeedScissor = true;

	if(lbNeedScissor)
	{
		m_pRenderer->EnableScissorTest(lScissorX, lScissorY, lScissorWidth, lScissorHeight);
	}

	Container::DrawChildren();

	if(lbNeedScissor)
	{
		m_pRenderer->DisableScissorTest();
	}
}

void TreeView::DrawSelf()
{
	m_pBackgroundDefault->SetVisible(false);
	m_pBackgroundDisabled->SetVisible(false);

	if(IsEnabled())
	{
		m_pBackgroundDefault->SetVisible(true);
	}
	else
	{
		m_pBackgroundDisabled->SetVisible(true);
	}

	mScrollBar.SetVisible(true);
}

void TreeView::_ExpandPressed(void *lpData)
{
	TreeNode *_this = (TreeNode*)lpData;

	if(_this)
	{
		_this->m_pTreeView->ExpandPressed(_this);
	}
}

void TreeView::ExpandPressed(TreeNode* pNode)
{
	if(pNode->m_expanded == false)
	{
		pNode->m_expanded = true;
	}
	else
	{
		pNode->m_expanded = false;
	}
	
	int yCul = -20 + m_yOffset;
	m_numOpenNodes = 0;
	SetNodeDimensions(m_root, 0, -20, m_dimensions.m_width, m_dimensions.m_height, &yCul);

	UpdateScrollbar();
}

void TreeView::_ScrollbarChanged(void *lpData)
{
	TreeView *_this = (TreeView*)lpData;
	_this->ScrollbarChanged();
}

void TreeView::ScrollbarChanged()
{
	UpdateScrollbar();
}