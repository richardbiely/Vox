// ******************************************************************************
//
// Filename:	textbox.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A text entry box that accepts focus events and key events.
//
// Revision History:
//   Initial Revision - 11/07/08
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

typedef void(*FunctionCallback)(void *lpData);


class TextBox : public Container, public MouseListener, public FocusListener, public KeyListener
{
public:
	/* Public methods */
	TextBox(Renderer* pRenderer, unsigned int GUIFont, const std::string &defaultText, const std::string &name);

	~TextBox();

	void AddEventListeners() override;
	void RemoveEventListeners() override;

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	void SetLabelPositionOffset(int lOffsetX, int lOffsetY);

	void SetTextColour(Colour lColour);

	void SetBackgroundIcon(RenderRectangle *icon);

	void SetPipHeight(int height);
	void SetTextIndent(int indent);
	void SetMaxTextLength(int length);
	void SetPipeColour(Colour pipeColour);

	void SetText(const std::string &label);
	const std::string& GetText() const;

	void SetHiddenText(bool lbHidden);
	bool IsHiddenText() const;

	void SetHideCharacter(char lHideChar);
	char GetHideCharacter() const;

	void SetDontLoseFocus(bool dontLoseFocus);
	bool IsDontLoseFocusSet();

	void SetCallBackFunction_OnReturnPressed(FunctionCallback lFunction);
	void SetCallBackData_OnReturnPressed(void *lpData);

	EComponentType GetComponentType() const override;

protected:
	/* Protected methods */
	void KeyPressed(const KeyEvent& lEvent);
	void KeyReleased(const KeyEvent& lEvent);
	void KeyTyped(const KeyEvent& lEvent);
	void CharacterEntered(const KeyEvent& lEvent);

	void FocusLost(const FocusEvent& lEvent);
	void FocusGained(const FocusEvent& lEvent);

	virtual void OnReturnPressed();

	void DrawSelf() override;
	void DrawChildren() override;

private:
	/* Private methods */

	static void _TogglePipeDisplay(void *lpData);
	void TogglePipeDisplay();

public:
	/* Public members */

protected:
	/* Protected members */
	FunctionCallback m_Callback_OnReturnPressed;
	void *m_pCallbackData_OnReturnPressed;

private:
	/* Private members */
	Label m_text;
	Label m_name;

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

	bool m_bShiftHeld;

	bool m_bHiddenText;
	char m_HideCharacter;

	bool m_bDontLoseFocus;
};
