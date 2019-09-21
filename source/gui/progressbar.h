// ******************************************************************************
//
// Filename:	progressbar.h
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

#pragma once

#include "container.h"
#include "renderrectangle.h"

enum EProgressDirection
{
	EProgressDirection_Horizontal = 0,
	EProgressDirection_Vertical,
};


class ProgressBar : public Container
{
public:
	/* Public methods */
	ProgressBar(Renderer* pRenderer, unsigned int GUIFont, float min, float max);

	virtual ~ProgressBar();

	float GetMinValue();
	float GetMaxValue();
	float GetCurrentValue();

	void SetMinValue(float minValue);
	void SetMaxValue(float maxValue);
	void SetCurrentValue(float currentValue);

	EProgressDirection GetProgressDirection();
	void SetProgressDirection(EProgressDirection lDirection);

	void SetProgressFiller(RenderRectangle *icon);
	void SetProgressBackground(RenderRectangle *icon);

	EComponentType GetComponentType() const override;

protected:
	/* Protected methods */
	virtual void OnMinValueReached();
	virtual void OnMaxValueReached();

	void DrawSelf() override;

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	float m_minValue;
	float m_maxValue;

	float m_currentValue;

	EProgressDirection m_eProgressDirection;

	RenderRectangle	*m_pProgressBackground;
	RenderRectangle	*m_pProgressFiller;
};
