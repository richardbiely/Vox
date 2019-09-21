// ******************************************************************************
//
// Filename:	slider.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A slider GUI component to allow quick selection of range values.
//
// Revision History:
//   Initial Revision - 21/03/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "container.h"
#include "label.h"
#include "icon.h"
#include "button.h"


enum ESliderDirection
{
	ESliderDirection_Horizontal = 0,
	ESliderDirection_Vertical,
};

typedef void(*FunctionCallback)(void *lpData);

class Slider : public Container, public MouseListener
{
public:
	/* Public methods */
	Slider(Renderer* pRenderer, unsigned int GUIFont, const std::string &label, float min, float max, float defaultValue, bool addIncrementButtons = false, float incrementAmount = 0.01f);
	Slider(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, float min, float max, float defaultValue, Colour fontColour, Colour outlineColour, bool addIncrementButtons = false, float incrementAmount = 0.01f);

	virtual ~Slider();

	void AddEventListeners() override;
	void RemoveEventListeners() override;

	void SetScrollerDefaultIcon(RenderRectangle *icon);
	void SetScrollerHoverIcon(RenderRectangle *icon);
	void SetScrollerDraggingIcon(RenderRectangle *icon);
	void SetScrollerDisabledIcon(RenderRectangle *icon);
	void SetScrollBackbarDefault(RenderRectangle *icon);
	void SetScrollBackbarPressed(RenderRectangle *icon);
	void SetScrollBackbarIncrementIconDefault(RenderRectangle *icon);
	void SetScrollBackbarIncrementIconHover(RenderRectangle *icon);
	void SetScrollBackbarIncrementIconPressed(RenderRectangle *icon);
	void SetScrollBackbarIncrementIconDisabled(RenderRectangle *icon);
	void SetScrollBackbarDecrementIconDefault(RenderRectangle *icon);
	void SetScrollBackbarDecrementIconHover(RenderRectangle *icon);
	void SetScrollBackbarDecrementIconPressed(RenderRectangle *icon);
	void SetScrollBackbarDecrementIconDisabled(RenderRectangle *icon);

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	void SetShowValue(bool lShow);
	void SetValuePrecision(int lPresision);
	void SetValuePositionOffset(int lOffsetX, int lOffsetY);

	void SetLabelPositionOffset(int lOffsetX, int lOffsetY);

	void SetIncrementButtonsOffset(int lOffsetX, int lOffsetY);

	void UpdateTextString();

	void SetSliderParams(int barHeight, int sliderWidth, ESliderDirection lDirection);

	float GetMinValue();
	float GetMaxValue();
	float GetDefaultValue();
	float GetCurrentValue();

	void SetMinValue(float minValue);
	void SetMaxValue(float maxValue);
	void SetDefaultValue(float defaultValue);
	void SetCurrentValue(float currentValue, bool supressCallback = false);

    void SetRenderBackgroundBar(bool render);
	void SetDynamicBackgroundColour(Colour top_left, Colour bottom_left, Colour top_right, Colour bottom_right);

	ESliderDirection GetSliderDirection();
	void SetSliderDirection(ESliderDirection lDirection);

	void SetDisabled(bool val);

	EComponentType GetComponentType() const override;

    void SetVaueChangedCallBackFunction(FunctionCallback lFunction);
    void SetVaueChangedCallBackData(void *lpData);

protected:
	/* Protected methods */
	void MouseEntered(const MouseEvent& lEvent) override;
	void MouseExited(const MouseEvent& lEvent) override;
	void MousePressed(const MouseEvent& lEvent) override;
	void MouseReleased(const MouseEvent& lEvent) override;
	void MouseReleasedOutside(const MouseEvent& lEvent);
	void MouseClicked(const MouseEvent& lEvent) override;
	void MouseDragged(const MouseEvent& lEvent);
	void MouseDraggedOutside(const MouseEvent& lEvent);
	void MouseMotion(const MouseEvent& lEvent);

	virtual void OnMouseEnter();
	virtual void OnMouseExit();
	virtual void OnMousePressed();
	virtual void OnMouseReleased();
	virtual void OnMouseClicked();

	virtual void OnValueChanged();

	void DrawSelf() override;

private:
	/* Private methods */
	void ValueChanged(bool supressCallback = false);

	static void _PlusPressed(void *lpData);
	void PlusPressed();

	static void _MinusPressed(void *lpData);
	void MinusPressed();

public:
	/* Public members */

protected:
	/* Protected members */
	Label	m_value;
	Label	m_name;

private:
	/* Private members */
	float m_minValue;
	float m_maxValue;
	float m_defaultValue;

	float m_currentValue;

	ESliderDirection m_eSliderDirection;

	bool m_bShowValue;
	int m_ValuePrecision;

	int m_barHeight;
	int m_sliderWidth;

	bool m_bDragging;
	bool m_bAllowDragging;
	bool m_bHover;
	bool m_bDragginOutside;
	bool m_bFirstOutsideDrag;
	bool m_bAllowDragginOutside;
	bool m_bPressedBar;
	bool m_bAllowReleasingOnBar;
	bool m_bOverBar;

    bool m_bRenderBackgroundBar;

	bool m_bDynamicColourBackground;
	Colour m_backgroundColour_topLeft;
	Colour m_backgroundColour_topRight;
	Colour m_backgroundColour_BottomLeft;
	Colour m_backgroundColour_BottomRight;

	int m_lastX;
	int m_lastY;

	bool m_disabled;

	unsigned int m_fontID;

	RenderRectangle	*m_pScrollerDefault;
	RenderRectangle	*m_pScrollerHover;
	RenderRectangle	*m_pScrollerDragging;
	RenderRectangle	*m_pScrollerDisabled;
	RenderRectangle	*m_pScrollBackbarDefault;
	RenderRectangle	*m_pScrollBackbarPressed;

	Button	*m_pPlusButton;
	Button	*m_pMinusButton;

	bool m_incrementButtons;
	float m_incrementAmount;

	int m_labelOffset_X;
	int m_labelOffset_Y;
	int m_valueOffset_X;
	int m_valueOffset_Y;
	int m_incrementButtonsOffset_X;
	int m_incrementButtonsOffset_Y;

    // Function callback for when this slider has a value changed event
    FunctionCallback m_ValueChangedCallback;
    void *m_pValueChangedCallbackData;
};