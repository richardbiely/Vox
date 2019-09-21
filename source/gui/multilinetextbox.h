// ******************************************************************************
//
// Filename:	multilinetextbox.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A multi-linetext entry box that accepts focus events and key events.
//
// Revision History:
//   Initial Revision - 09/01/13
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "container.h"
#include "../utils/CountdownTimer.h"
#include "directdrawrectangle.h"
#include "multitextureicon.h"
#include "icon.h"
#include "label.h"
#include "scrollbar.h"

typedef void(*FunctionCallback)(void *lpData);


class MultiLineTextBox : public Container, public MouseListener, public FocusListener, public KeyListener
{
public:
	/* Public methods */
	MultiLineTextBox(Renderer* pRenderer, unsigned int GUIFont, const std::string &defaultText);

	~MultiLineTextBox();

	void AddEventListeners() override;
	void RemoveEventListeners() override;

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	void SetTextColour(Colour lColour);

	void SetBackgroundIcon(RenderRectangle *icon);

	void SetPipHeight(int height);
	void SetTextIndent(int indent);
	void SetMaxTextLength(int length);
	void SetPipeColour(Colour pipeColour);

	void SetText(const std::string &label);
	const std::string& GetText() const;

	void SetVerticalAlignment(EVerticalAlignment alignment);
	EVerticalAlignment GetVerticalAlignment() const;

	void SetHorizontalAlignment(EHorizontalAlignment alignment);
	EHorizontalAlignment GetHorizontalAlignment() const;

	void SetHiddenText(bool lbHidden);
	bool IsHiddenText() const;

	void SetHideCharacter(char lHideChar);
	char GetHideCharacter() const;

	void SetCallBackFunction_OnReturnPressed(FunctionCallback lFunction);
	void SetCallBackData_OnReturnPressed(void *lpData);

	EComponentType GetComponentType() const override;

	void UpdateText();

	void ReAlignText();

	void UpdateScrollbar();

protected:
	/* Protected methods */
	void KeyPressed(const KeyEvent& lEvent);
	void KeyReleased(const KeyEvent& lEvent);
	void KeyTyped(const KeyEvent& lEvent);
	void CharacterEntered(const KeyEvent& lEvent);

	void FocusLost(const FocusEvent& lEvent);
	void FocusGained(const FocusEvent& lEvent);

	virtual void OnReturnPressed();

	void ParseText();

	void ClearLabels();

	void DrawChildren() override;
	void DrawSelf() override;

private:
	/* Private methods */
	static void _TogglePipeDisplay(void *lpData);
	void TogglePipeDisplay();

	static void _ScrollbarChanged(void *lpData);
	void ScrollbarChanged();

public:
	/* Public members */

protected:
	/* Protected members */
	FunctionCallback m_Callback_OnReturnPressed;
	void *m_pCallbackData_OnReturnPressed;

private:
	/* Private members */
	std::string m_text;

	std::vector<Label> LabelList;

	int m_NumLines;

	EVerticalAlignment m_eVerticalAlignment;
	EHorizontalAlignment m_eHorizontalAlignment;

	RenderRectangle	*m_pBackgroundIcon;

	unsigned int m_GUIFont;

	int m_HighlightStartIndex;
	int m_HighlightEndIndex;

	int m_PipeCharacterIndex;
	bool m_bDisplayPipe;
	CountdownTimer* m_pPipeDisplayCountDown;

	int m_pipeHeight;
	int m_textIndent;
	int m_maxTextLength;
	Colour m_pipeColour;
	int m_pipeYOffset;

	bool m_newLinePipeRender;

	bool m_bShiftHeld;

	bool m_bHiddenText;
	char m_HideCharacter;

	ScrollBar mScrollBar;
};
