// ******************************************************************************
//
// Filename:	progressbar.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A progress bar component to show visual feedback of progress.
//
// Revision History:
//   Initial Revision - 13/07/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "progressbar.h"
#include "directdrawrectangle.h"
#include "multitextureicon.h"
#include "icon.h"


ProgressBar::ProgressBar(Renderer* pRenderer, unsigned int GUIFont, float min, float max)
  : Container(pRenderer),
    m_minValue(min),
    m_maxValue(max),
    m_eProgressDirection(EProgressDirection_Horizontal)
{
	m_currentValue = m_minValue;

	m_pProgressBackground = new DirectDrawRectangle(pRenderer);
	m_pProgressFiller = new DirectDrawRectangle(pRenderer);

	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pProgressBackground;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.75f, 0.75f, 0.75f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(0.25f, 0.25f, 0.25f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.25f, 0.25f, 0.25f, 1.0f));

	lpDirectDrawRect = (DirectDrawRectangle *)m_pProgressFiller;
	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.52f, 0.53f, 0.91f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	Add(m_pProgressFiller);
	Add(m_pProgressBackground);

	m_pProgressBackground->SetDepth(2.0f);
	m_pProgressFiller->SetDepth(3.0f);
}

ProgressBar::~ProgressBar()
{
	delete m_pProgressBackground;
	delete m_pProgressFiller;
}

float ProgressBar::GetMinValue()
{
	return m_minValue;
}

float ProgressBar::GetMaxValue()
{
	return m_maxValue;
}

float ProgressBar::GetCurrentValue()
{
	return m_currentValue;
}

void ProgressBar::SetMinValue(float minValue)
{
	m_minValue = minValue;
}

void ProgressBar::SetMaxValue(float maxValue)
{
	m_maxValue = maxValue;
}

void ProgressBar::SetCurrentValue(float currentValue)
{
	m_currentValue = currentValue;

	// Bounds checking
	if(m_currentValue < m_minValue)
	{
		m_currentValue = m_minValue;

		OnMinValueReached();
	}
	else if(m_currentValue > m_maxValue)
	{
		m_currentValue = m_maxValue;

		OnMaxValueReached();
	}
}

EProgressDirection ProgressBar::GetProgressDirection()
{
	return m_eProgressDirection;
}

void ProgressBar::SetProgressDirection(EProgressDirection lDirection)
{
	m_eProgressDirection = lDirection;
}

void ProgressBar::SetProgressFiller(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pProgressFiller)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pProgressFiller);

		delete m_pProgressFiller;
		m_pProgressFiller = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pProgressFiller = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pProgressFiller = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pProgressFiller);

	// Properly set the depth again, since this will have changed after we added the component again
	m_pProgressFiller->SetDepth(3.0f);
}

void ProgressBar::SetProgressBackground(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);

	if(m_pProgressBackground)
	{
		// If we already own an icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pProgressBackground);

		delete m_pProgressBackground;
		m_pProgressBackground = nullptr;
	}

	// Check what type of render rectangle we have been given, and then assign our new data
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pProgressBackground = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pProgressBackground = lpNewIcon;
	}

	// Re-add this icon to the component list
	Add(m_pProgressBackground);

	// Properly set the depth again, since this will have changed after we added the component again
	m_pProgressBackground->SetDepth(2.0f);
}

EComponentType ProgressBar::GetComponentType() const
{
	return EComponentType_ProgressBar;
}

void ProgressBar::OnMinValueReached()
{

}

void ProgressBar::OnMaxValueReached()
{
	// TEMP!
	m_currentValue = m_minValue;
}

void ProgressBar::DrawSelf()
{
	int l_containerWidth = m_dimensions.m_width;
	int l_containerHeight = m_dimensions.m_height;
	int l_depth = static_cast<int>(GetDepth());

	int l_backgroundX1;
	int l_backgroundX2;
	int l_backgroundY1;
	int l_backgroundY2;

	int l_progressionX1;
	int l_progressionX2;
	int l_progressionY1;
	int l_progressionY2;

	int l_outlineX1;
	int l_outlineX2;
	int l_outlineY1;
	int l_outlineY2;

	int lProgression;

	if(m_eProgressDirection == EProgressDirection_Horizontal)
	{
		lProgression = (int)(m_dimensions.m_width * ((m_currentValue - m_minValue) / (m_maxValue - m_minValue)));

		l_backgroundX1 = 0;
		l_backgroundX2 = l_containerWidth;
		l_backgroundY1 = 0;
		l_backgroundY2 = l_containerHeight;	

		l_progressionX1 = 0;
		l_progressionX2 = lProgression;
		l_progressionY1 = 0;
		l_progressionY2 = l_containerHeight;	

		l_outlineX1 = 0;
		l_outlineX2 = l_containerWidth + 1;
		l_outlineY1 = 0;
		l_outlineY2 = l_containerHeight + 1;

	}
	else //m_eProgressDirection == EProgressDirection_Vertical
	{
		lProgression = (int)(m_dimensions.m_height * ((m_currentValue - m_minValue) / (m_maxValue - m_minValue)));

		l_backgroundX1 = 0;
		l_backgroundX2 = l_containerWidth;
		l_backgroundY1 = 0;
		l_backgroundY2 = l_containerHeight;	

		l_progressionX1 = 0;
		l_progressionX2 = l_containerWidth;
		l_progressionY1 = 0;
		l_progressionY2 = lProgression;	

		l_outlineX1 = 0;
		l_outlineX2 = l_containerWidth + 1;
		l_outlineY1 = 0;
		l_outlineY2 = l_containerHeight + 1;
	}

	/*
	// Draw the progression
	m_pRenderer->PushMatrix();
		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
		m_pRenderer->ImmediateColourAlpha(1.0f, 0.0f, 0.0f, 1.0f);
		m_pRenderer->ImmediateVertex(l_progressionX1, l_progressionY1, l_depth);
		m_pRenderer->ImmediateVertex(l_progressionX2, l_progressionY1, l_depth);
		m_pRenderer->ImmediateVertex(l_progressionX2, l_progressionY2, l_depth);
		m_pRenderer->ImmediateVertex(l_progressionX1, l_progressionY2, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();

	// Draw the background
	m_pRenderer->PushMatrix();
		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
		m_pRenderer->ImmediateVertex(l_backgroundX1, l_backgroundY1, l_depth);
		m_pRenderer->ImmediateVertex(l_backgroundX2, l_backgroundY1, l_depth);
		m_pRenderer->ImmediateVertex(l_backgroundX2, l_backgroundY2, l_depth);
		m_pRenderer->ImmediateVertex(l_backgroundX1, l_backgroundY2, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();

	// Draw the outline
	m_pRenderer->PushMatrix();
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->EnableImmediateMode(IM_LINE_LOOP);
		m_pRenderer->ImmediateColourAlpha(0.0f, 0.0f, 0.0f, 1.0f);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY2, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY2, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
	*/

	m_pProgressBackground->SetDimensions(l_backgroundX1, l_backgroundY1, l_backgroundX2, l_backgroundY2);
	m_pProgressFiller->SetDimensions(l_progressionX1, l_progressionY1, l_progressionX2, l_progressionY2);

	m_pProgressBackground->SetVisible(true);
	m_pProgressFiller->SetVisible(true);
}