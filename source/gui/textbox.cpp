// ******************************************************************************
//
// Filename:	textbox.cpp
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

#include "textbox.h"
#include "focusmanager.h"
#include "../utils/TimeManager.h"

#include <glfw/glfw3.h>


TextBox::TextBox(Renderer* pRenderer, unsigned int GUIFont, const std::string &defaultText, const std::string &name)
  : Container(pRenderer),
    m_GUIFont(GUIFont),
	m_text(pRenderer, GUIFont, defaultText, Colour(0.0f, 0.0f, 0.0f)),
	m_name(pRenderer, GUIFont, name, Colour(0.0f, 0.0f, 0.0f))
{
	m_pBackgroundIcon = new DirectDrawRectangle(pRenderer);
	DirectDrawRectangle *lpDirectDrawRect = (DirectDrawRectangle *)m_pBackgroundIcon;

	lpDirectDrawRect->SetBackgroundColourTopLeft(Colour(0.7f, 0.7f, 0.85f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourTopRight(Colour(0.7f, 0.7f, 0.85f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomLeft(Colour(0.7f, 0.7f, 0.85f, 1.0f));
	lpDirectDrawRect->SetBackgroundColourBottomRight(Colour(0.7f, 0.7f, 0.85f, 1.0f));
	lpDirectDrawRect->SetOutlineColourTop(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourBottom(Colour(0.0f, 0.0f, 0.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourLeft(Colour(1.0f, 1.0f, 1.0f, 1.0f));
	lpDirectDrawRect->SetOutlineColourRight(Colour(0.0f, 0.0f, 0.0f, 1.0f));

	Add(m_pBackgroundIcon);
	m_pBackgroundIcon->SetDepth(1.0f);

	m_text.SetLocation(0, 0);
	m_text.SetDepth(2.0f);
	Add(&m_text);

	m_name.SetLocation(0, 0);
	m_name.SetDepth(2.0f);
	Add(&m_name);

	m_HighlightStartIndex = 0;
	m_HighlightEndIndex = 0;

	m_PipeCharacterIndex = (int)m_text.GetText().size();
	m_bDisplayPipe = false;

	m_pipeHeight = 0;
	m_textIndent = 0;
	m_maxTextLength = 0;

	SetText(defaultText);

	m_bShiftHeld = false;

	m_bHiddenText = false;
	m_HideCharacter = '*';

	m_bDontLoseFocus = false;

	m_Callback_OnReturnPressed = nullptr;

	m_pPipeDisplayCountDown = new CountdownTimer();
	m_pPipeDisplayCountDown->SetCountdownTime(0.5f);
	m_pPipeDisplayCountDown->SetLooping(true);
	m_pPipeDisplayCountDown->SetCallBackFunction(_TogglePipeDisplay);
	m_pPipeDisplayCountDown->SetCallBackData(this);
	m_pPipeDisplayCountDown->StartCountdown();
}

TextBox::~TextBox()
{
	TimeManager::GetInstance()->RemoveCountdownTimer(m_pPipeDisplayCountDown);

	delete m_pBackgroundIcon;
}

void TextBox::AddEventListeners()
{
	Container::AddEventListeners();

	AddMouseListener(this);
	AddFocusListener(this);
	AddKeyListener(this);
}

void TextBox::RemoveEventListeners()
{
	Container::RemoveEventListeners();

	RemoveMouseListener(this);
	RemoveFocusListener(this);
	RemoveKeyListener(this);
}

void TextBox::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);

	// Also set the dimensions of the icons
	m_pBackgroundIcon->SetDimensions(0, 0, width, height);

	int nameSpace = 5;
	int nameLabelWidth = m_pRenderer->GetFreeTypeTextWidth(m_GUIFont, "%s", m_name.GetText().c_str());
	int nameLabelHeight = m_pRenderer->GetFreeTypeTextHeight(m_GUIFont, "%s", m_name.GetText().c_str());
	int l_center_height = (height / 2) - (nameLabelHeight / 2);
	m_name.SetLocation(-nameLabelWidth - nameSpace, l_center_height);
}

void TextBox::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void TextBox::SetLabelPositionOffset(int lOffsetX, int lOffsetY)
{
	m_name.SetLocation(lOffsetX, lOffsetY);
}

void TextBox::SetTextColour(Colour lColour)
{
	m_text.SetColour(lColour);
}

void TextBox::SetBackgroundIcon(RenderRectangle *icon)
{
	MultiTextureIcon* lpMulti = dynamic_cast<MultiTextureIcon*>(icon);
	Icon* lpIcon = dynamic_cast<Icon*>(icon);
	DirectDrawRectangle* lpDirectDraw = dynamic_cast<DirectDrawRectangle*>(icon);

	if(m_pBackgroundIcon)
	{
		// If we already own a background icon, remove it from out component list and also delete it, since we will be replacing it
		Remove(m_pBackgroundIcon);

		delete m_pBackgroundIcon;
		m_pBackgroundIcon = nullptr;
	}

	// Check what type of render rectangle we have been given, and then create a new copy of it
	if(lpMulti)
	{
		MultiTextureIcon* lpNewMulti = new MultiTextureIcon((*lpMulti));
		m_pBackgroundIcon = lpNewMulti;
	}
	else if(lpIcon)
	{
		Icon* lpNewIcon = new Icon((*lpIcon));
		m_pBackgroundIcon = lpNewIcon;
	}
	else if(lpDirectDraw)
	{
		DirectDrawRectangle* lpNewDirectDraw = new DirectDrawRectangle((*lpDirectDraw));
		m_pBackgroundIcon = lpNewDirectDraw;
	}

	// Re-add this icon to the component list
	Add(m_pBackgroundIcon);

	m_pBackgroundIcon->SetDepth(1.0f);

	DepthSortComponentChildren();
}

void TextBox::SetPipHeight(int height)
{
	m_pipeHeight = height;
}

void TextBox::SetTextIndent(int indent)
{
	m_textIndent = indent;
}

void TextBox::SetMaxTextLength(int length)
{
	m_maxTextLength = length;
}

void TextBox::SetPipeColour(Colour pipeColour)
{
	m_pipeColour = pipeColour;
}

void TextBox::SetText(const std::string &label)
{
	m_text.SetText(label);
	m_PipeCharacterIndex = (int)m_text.GetText().size();
}

const std::string& TextBox::GetText() const
{
	return m_text.GetText();
}

void TextBox::SetHiddenText(bool lbHidden)
{
	m_bHiddenText = lbHidden;
}

bool TextBox::IsHiddenText() const
{
	return m_bHiddenText;
}

void TextBox::SetHideCharacter(char lHideChar)
{
	m_HideCharacter = lHideChar;
}

char TextBox::GetHideCharacter() const
{
	return m_HideCharacter;
}

void TextBox::SetDontLoseFocus(bool dontLoseFocus)
{
	m_bDontLoseFocus = dontLoseFocus;
}

bool TextBox::IsDontLoseFocusSet()
{
	return m_bDontLoseFocus;
}

void TextBox::SetCallBackFunction_OnReturnPressed(FunctionCallback lFunction)
{
	m_Callback_OnReturnPressed = lFunction;
}

void TextBox::SetCallBackData_OnReturnPressed(void *lpData)
{
	m_pCallbackData_OnReturnPressed = lpData;
}

EComponentType TextBox::GetComponentType() const
{
	return EComponentType_TextBox;
}

void TextBox::KeyPressed(const KeyEvent& lEvent)
{
	if(lEvent.GetKeyCode() == GLFW_KEY_BACKSPACE)
	{
		int l_NumHighlightChars = m_HighlightEndIndex - m_HighlightStartIndex;

		if(l_NumHighlightChars > 0)
		{
			m_text.Erase(m_HighlightStartIndex, l_NumHighlightChars);

			m_PipeCharacterIndex = m_HighlightStartIndex;
		}
		else
		{
			// Remove the previous character and update the pipe character index
			if(m_PipeCharacterIndex > 0)
			{
				m_text.Erase(m_PipeCharacterIndex - 1, 1);
				m_PipeCharacterIndex -= 1;
			}
		}

		m_HighlightStartIndex = m_PipeCharacterIndex;
		m_HighlightEndIndex = m_PipeCharacterIndex;

		m_bDisplayPipe = true;
		m_pPipeDisplayCountDown->ResetCountdown();
	}

	if(lEvent.GetKeyCode() == GLFW_KEY_DELETE)
	{
		int l_NumHighlightChars = m_HighlightEndIndex - m_HighlightStartIndex;

		if(l_NumHighlightChars > 0)
		{
			m_text.Erase(m_HighlightStartIndex, l_NumHighlightChars);

			m_PipeCharacterIndex = m_HighlightStartIndex;
		}
		else
		{
			// Remove the current character and leave the pipe character index the same
			int l_stringSize = (int)m_text.GetText().size();
			if(m_PipeCharacterIndex < l_stringSize)
			{
				m_text.Erase(m_PipeCharacterIndex, 1);
			}
		}

		m_HighlightStartIndex = m_PipeCharacterIndex;
		m_HighlightEndIndex = m_PipeCharacterIndex;

		m_bDisplayPipe = true;
		m_pPipeDisplayCountDown->ResetCountdown();
	}

	if(lEvent.GetKeyCode() == GLFW_KEY_HOME)
	{
		// Set pipe to start
		m_PipeCharacterIndex = 0;

		if(m_bShiftHeld)
		{
			m_HighlightEndIndex = m_HighlightStartIndex;
			m_HighlightStartIndex = m_PipeCharacterIndex;
		}
		else
		{
			m_HighlightStartIndex = m_PipeCharacterIndex;
			m_HighlightEndIndex = m_PipeCharacterIndex;
		}

		m_bDisplayPipe = true;
		m_pPipeDisplayCountDown->ResetCountdown();
	}

	if(lEvent.GetKeyCode() == GLFW_KEY_END)
	{
		// Set pipe to end
		int l_stringSize = (int)m_text.GetText().size();
		m_PipeCharacterIndex = l_stringSize;

		if(m_bShiftHeld)
		{
			m_HighlightStartIndex = m_HighlightEndIndex;
			m_HighlightEndIndex = m_PipeCharacterIndex;
		}
		else
		{
			m_HighlightStartIndex = m_PipeCharacterIndex;
			m_HighlightEndIndex = m_PipeCharacterIndex;
		}

		m_bDisplayPipe = true;
		m_pPipeDisplayCountDown->ResetCountdown();
	}

	if(lEvent.GetKeyCode() == GLFW_KEY_ENTER)
	{
		if(m_bDontLoseFocus == false)
		{
			FocusManager::GetInstance()->SetFocusOwner(0);
		}

		// Try to call our callback function, if we have one
		if(m_Callback_OnReturnPressed)
		{
			m_Callback_OnReturnPressed(m_pCallbackData_OnReturnPressed);
		}

		// Signal that we have pressed return, allows custom sub-class behaviors
		OnReturnPressed();
	}

	if(lEvent.GetKeyCode() == GLFW_KEY_LEFT)
	{
		if(m_PipeCharacterIndex > 0)
		{
			if(m_bShiftHeld)
			{
				if(m_HighlightStartIndex == m_PipeCharacterIndex)
				{
					m_HighlightStartIndex -= 1;
				}
				else
				{
					m_HighlightEndIndex -= 1;
				}
			}
			else
			{
				m_HighlightStartIndex = m_PipeCharacterIndex - 1;
				m_HighlightEndIndex = m_PipeCharacterIndex - 1;
			}

			m_PipeCharacterIndex -= 1;
		}
		else
		{
			if(!m_bShiftHeld)
			{
				m_HighlightStartIndex = m_PipeCharacterIndex;
				m_HighlightEndIndex = m_PipeCharacterIndex;
			}
		}

		m_bDisplayPipe = true;
		m_pPipeDisplayCountDown->ResetCountdown();
	}

	if(lEvent.GetKeyCode() == GLFW_KEY_RIGHT)
	{
		int l_stringSize = (int)m_text.GetText().size();
		if(m_PipeCharacterIndex < l_stringSize)
		{
			if(m_bShiftHeld)
			{
				if(m_HighlightEndIndex == m_PipeCharacterIndex)
				{
					m_HighlightEndIndex += 1;
				}
				else
				{
					m_HighlightStartIndex += 1;
				}
			}
			else
			{
				m_HighlightStartIndex = m_PipeCharacterIndex + 1;
				m_HighlightEndIndex = m_PipeCharacterIndex + 1;
			}

			m_PipeCharacterIndex += 1;
		}
		else
		{
			if(!m_bShiftHeld)
			{
				m_HighlightStartIndex = m_PipeCharacterIndex;
				m_HighlightEndIndex = m_PipeCharacterIndex;
			}
		}

		m_bDisplayPipe = true;
		m_pPipeDisplayCountDown->ResetCountdown();
	}

	if(lEvent.GetKeyCode() == GLFW_KEY_LEFT_SHIFT || lEvent.GetKeyCode() == GLFW_KEY_RIGHT_SHIFT)
	{
		m_bShiftHeld = true;
	}
}

void TextBox::KeyReleased(const KeyEvent& lEvent)
{
	if(lEvent.GetKeyCode() == GLFW_KEY_LEFT_SHIFT || lEvent.GetKeyCode() == GLFW_KEY_RIGHT_SHIFT)
	{
		m_bShiftHeld = false;
	}
}

void TextBox::KeyTyped(const KeyEvent& lEvent)
{
}

void TextBox::CharacterEntered(const KeyEvent& lEvent)
{
	int l_NumHighlightChars = m_HighlightEndIndex - m_HighlightStartIndex;

	if(l_NumHighlightChars > 0)
	{
		m_text.Erase(m_HighlightStartIndex, l_NumHighlightChars);

		m_PipeCharacterIndex = m_HighlightStartIndex;
	}

	int lCharacterCode = lEvent.GetKeyCode();

	if(m_maxTextLength == 0 || m_text.GetText().size() < (unsigned int)m_maxTextLength)
	{
		// NOTE : This range excludes all the control characters such as backspace
		// and carriage return (code < 31), these should be handled in key events.
		if(lCharacterCode > 31 && lCharacterCode < 255)
		{
			char lChar[2];

			lChar[0] = lEvent.GetKeyCode();
			lChar[1] = 0; // Make sure we include a terminator

			m_text.Insert(m_PipeCharacterIndex, &lChar[0]);

			// Update the pipe character index
			m_PipeCharacterIndex += 1;

			m_bDisplayPipe = true;
			m_pPipeDisplayCountDown->ResetCountdown();
		}
	}

	m_HighlightStartIndex = m_PipeCharacterIndex;
	m_HighlightEndIndex = m_PipeCharacterIndex;
}

void TextBox::FocusLost(const FocusEvent& lEvent)
{
	m_HighlightStartIndex = m_PipeCharacterIndex;
	m_HighlightEndIndex = m_PipeCharacterIndex;
}

void TextBox::FocusGained(const FocusEvent& lEvent)
{
	m_bDisplayPipe = true;
	m_pPipeDisplayCountDown->ResetCountdown();

	m_HighlightStartIndex = m_PipeCharacterIndex;
	m_HighlightEndIndex = m_PipeCharacterIndex;
}

void TextBox::OnReturnPressed()
{
	/* Nothing */
}

void TextBox::DrawSelf()
{
	int l_containerWidth = m_dimensions.m_width;
	int l_containerHeight = m_dimensions.m_height;
	int l_depth = static_cast<int>(GetDepth() + 1);

	int l_containerX1 = 0;
	int l_containerX2 = l_containerWidth;
	int l_containerY1 = 0;
	int l_containerY2 = l_containerHeight;	

	int l_outlineX1 = 0;
	int l_outlineX2 = l_containerWidth + 1;
	int l_outlineY1 = 0;
	int l_outlineY2 = l_containerHeight + 1;


	std::string l_DisplayText = m_text.GetText();

	int l_TextWidth = m_pRenderer->GetFreeTypeTextWidth(m_GUIFont, "%s", m_text.GetText().c_str());
	int l_TextHeight = m_pRenderer->GetFreeTypeTextHeight(m_GUIFont, "%s", m_text.GetText().c_str());

	int l_TextSpacer = 2;
	int PipeStringLength = 0;
	int ReversePipeStringLength = 0;
	if(m_text.GetText().size() > 0)
	{
		std::string l_PipeSubString = l_DisplayText;
		l_PipeSubString = l_PipeSubString.substr(0, m_PipeCharacterIndex);
		PipeStringLength = m_pRenderer->GetFreeTypeTextWidth(m_GUIFont, "%s", l_PipeSubString.c_str());;

		if(PipeStringLength < GetDimensions().m_width - (l_TextSpacer*2))
		{
			PipeStringLength = 0;
		}
		else
		{
			PipeStringLength = PipeStringLength - GetDimensions().m_width + (l_TextSpacer*2);
		}

		std::string l_PipeReverseSubString = l_DisplayText;
		l_PipeReverseSubString = l_PipeReverseSubString.substr(m_PipeCharacterIndex, m_text.GetText().size());
		ReversePipeStringLength = m_pRenderer->GetFreeTypeTextWidth(m_GUIFont, "%s", l_PipeReverseSubString.c_str());;
		if(ReversePipeStringLength < GetDimensions().m_width - (l_TextSpacer*2))
		{
			ReversePipeStringLength = 0;
		}
		else
		{
			ReversePipeStringLength = ReversePipeStringLength - GetDimensions().m_width + (l_TextSpacer*2);
		}
	}	
	int l_xDiff = -PipeStringLength;
	int l_xDiff2 = 0;//ReversePipeStringLength;
	int m_center_height = (GetDimensions().m_height / 2) - (l_TextHeight / 2);
	int l_posX = m_textIndent + l_TextSpacer + l_xDiff + l_xDiff2;
	int l_posY = m_center_height;

	m_text.SetLocation(l_posX, l_posY);


	// Do a scissor test here
	int lScissorX = GetLocation().m_x;
	int lScissorY = GetLocation().m_y;
	if(GetParent() != nullptr)
	{
		lScissorX = GetParent()->GetLocation().m_x + lScissorX;
		lScissorY = GetParent()->GetLocation().m_y + lScissorY;
	}	
	int lScissorWidth = GetDimensions().m_width;
	int lScissorHeight = GetDimensions().m_height;

	if(m_bHiddenText)
	{
		l_DisplayText.replace(0, l_DisplayText.size(), l_DisplayText.size(), m_HideCharacter);
	}

	if(HasFocus() && m_bDisplayPipe)
	{
		std::string l_PipSubString = l_DisplayText;
		l_PipSubString = l_PipSubString.substr(0, m_PipeCharacterIndex);

		int l_HeightSpace = 3;
		int l_PipeWidth = 2;
		int l_TextWidth = l_TextSpacer + m_pRenderer->GetFreeTypeTextWidth(m_GUIFont, "%s", l_PipSubString.c_str());
		int l_pipeX1 = m_textIndent + l_TextWidth + l_xDiff + l_xDiff2;
		int l_pipeX2 = m_textIndent + l_TextWidth + l_PipeWidth + l_xDiff + l_xDiff2;
		int l_pipeY1 = 0;
		int l_pipeY2 = 0;
		if(m_pipeHeight == 0)
		{
			l_pipeY1 = l_HeightSpace;
			l_pipeY2 = l_containerHeight - l_HeightSpace;
		}
		else
		{
			l_pipeY1 = (int)(((l_containerHeight - l_HeightSpace)*0.5f)-(m_pipeHeight*0.5f));
			l_pipeY2 = (int)(((l_containerHeight - l_HeightSpace)*0.5f)+(m_pipeHeight*0.5f));
		}

		// Draw the pipe (vertical bar)
		m_pRenderer->PushMatrix();
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateColourAlpha(m_pipeColour.GetRed(), m_pipeColour.GetGreen(), m_pipeColour.GetBlue(), 1.0f);
			m_pRenderer->ImmediateVertex(l_pipeX1, l_pipeY1, l_depth);
			m_pRenderer->ImmediateVertex(l_pipeX2, l_pipeY1, l_depth);
			m_pRenderer->ImmediateVertex(l_pipeX2, l_pipeY2, l_depth);
			m_pRenderer->ImmediateVertex(l_pipeX1, l_pipeY2, l_depth);
			m_pRenderer->DisableImmediateMode();
		m_pRenderer->PopMatrix();
	}

	bool lbNeedScissor = true;

	if(lbNeedScissor)
	{
		m_pRenderer->EnableScissorTest(lScissorX, lScissorY, lScissorWidth, lScissorHeight);;
	}

	// Draw the highlighted area
	if(m_HighlightEndIndex > m_HighlightStartIndex)
	{
		std::string l_HighlightSubStringStart = l_DisplayText;
		l_HighlightSubStringStart = l_HighlightSubStringStart.substr(0, m_HighlightStartIndex);

		std::string l_HighlightSubStringEnd = l_DisplayText;
		int lNumCharsToHighlight = m_HighlightEndIndex - m_HighlightStartIndex;
		l_HighlightSubStringEnd = l_HighlightSubStringEnd.substr(m_HighlightStartIndex, lNumCharsToHighlight);

		int l_HeightSpace = 3;
		int l_HighlightStart = l_TextSpacer + m_pRenderer->GetFreeTypeTextWidth(m_GUIFont, "%s", l_HighlightSubStringStart.c_str());
		int l_HighlightWidth = m_pRenderer->GetFreeTypeTextWidth(m_GUIFont, "%s", l_HighlightSubStringEnd.c_str());
		int l_HighlightX1 = m_textIndent + l_HighlightStart + l_xDiff + l_xDiff2;
		int l_HighlightX2 = m_textIndent + l_HighlightStart + l_HighlightWidth + l_xDiff + l_xDiff2;
		int l_HighlightY1 = 0;
		int l_HighlightY2 = 0;
		if(m_pipeHeight == 0)
		{
			l_HighlightY1 = l_HeightSpace;
			l_HighlightY2 = l_containerHeight - l_HeightSpace;
		}
		else
		{
			l_HighlightY1 = (int)(((l_containerHeight - l_HeightSpace)*0.5f)-(m_pipeHeight*0.5f));
			l_HighlightY2 = (int)(((l_containerHeight - l_HeightSpace)*0.5f)+(m_pipeHeight*0.5f));
		}


		// Draw the highlighted area
		m_pRenderer->PushMatrix();
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateColourAlpha(0.52f, 0.53f, 0.91f, 1.0f);
			m_pRenderer->ImmediateVertex(l_HighlightX1, l_HighlightY1, l_depth);
			m_pRenderer->ImmediateVertex(l_HighlightX2, l_HighlightY1, l_depth);
			m_pRenderer->ImmediateVertex(l_HighlightX2, l_HighlightY2, l_depth);
			m_pRenderer->ImmediateVertex(l_HighlightX1, l_HighlightY2, l_depth);
			m_pRenderer->DisableImmediateMode();
		m_pRenderer->PopMatrix();
	}

	if(lbNeedScissor)
	{
		m_pRenderer->DisableScissorTest();
	}

	m_name.SetVisible(true);

	/*
	// Draw the background
	m_pRenderer->PushMatrix();
		m_pRenderer->SetRenderMode(RM_SOLID);
		m_pRenderer->EnableImmediateMode(IM_QUADS);
		if(HasFocus())
		{
			m_pRenderer->ImmediateColourAlpha(0.75f, 0.75f, 0.75f, 1.0f);
		}
		else
		{
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
		}
		m_pRenderer->ImmediateVertex(l_containerX1, l_containerY1, l_depth);
		m_pRenderer->ImmediateVertex(l_containerX2, l_containerY1, l_depth);
		m_pRenderer->ImmediateVertex(l_containerX2, l_containerY2, l_depth);
		m_pRenderer->ImmediateVertex(l_containerX1, l_containerY2, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();

	// Draw the outline
	m_pRenderer->PushMatrix();
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->ImmediateColourAlpha(0.0f, 0.0f, 0.0f, 1.0f);
		m_pRenderer->EnableImmediateMode(IM_LINE_LOOP);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY1, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY2, l_depth);
		m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY2, l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();

	m_pRenderer->RenderFreeTypeText(m_GUIFont, (float)l_posX, (float)l_posY, GetDepth()+1.0f, m_textColour, "%s", l_DisplayText.c_str());
	*/
}

void TextBox::DrawChildren()
{
	// Do a scissor test here
	int lScissorX = GetLocation().m_x-1;
	int lScissorY = GetLocation().m_y-1;
	if(GetParent() != nullptr)
	{
		lScissorX = GetParent()->GetLocation().m_x + lScissorX;
		lScissorY = GetParent()->GetLocation().m_y + lScissorY;
	}	
	int lScissorWidth = GetDimensions().m_width+2;
	int lScissorHeight = GetDimensions().m_height+2;
	int l_depth = static_cast<int>(GetDepth());

	bool lbNeedScissor = true;

	if(lbNeedScissor)
	{
		m_pRenderer->EnableScissorTest(lScissorX, lScissorY, lScissorWidth, lScissorHeight);;
	}

	Container::DrawChildren();

	/*
	// DEBUG : Draw the scissor region
	m_pRenderer->PushMatrix();
		m_pRenderer->SetLineWidth(1.0f);
		m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
		m_pRenderer->EnableImmediateMode(IM_LINE_LOOP);
		m_pRenderer->ImmediateVertex((lScissorX - GetLocation().m_x),						(lScissorY - GetLocation().m_y),					l_depth);
		m_pRenderer->ImmediateVertex((lScissorX - GetLocation().m_x) + lScissorWidth,		(lScissorY - GetLocation().m_y),					l_depth);
		m_pRenderer->ImmediateVertex((lScissorX - GetLocation().m_x) + lScissorWidth + 1,	(lScissorY - GetLocation().m_y ) + lScissorHeight,	l_depth);
		m_pRenderer->ImmediateVertex((lScissorX - GetLocation().m_x),						(lScissorY - GetLocation().m_y ) + lScissorHeight,	l_depth);
		m_pRenderer->DisableImmediateMode();
	m_pRenderer->PopMatrix();
	*/

	if(IsContainer() && HasChildren() && IsVisible())
	{
		ComponentList::const_iterator iterator;
		for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
		{
			if((*iterator) == &m_name)
			{
				continue;  // Don't render name header inside scissor test
			}

			if((*iterator)->IsVisible())
			{
				(*iterator)->Draw();
			}
		}
	}

	if(lbNeedScissor)
	{
		m_pRenderer->DisableScissorTest();
	}

	// Render name header outside of scissor test
	m_name.Draw();
}

void TextBox::_TogglePipeDisplay(void *lpData)
{
	TextBox *_this = (TextBox*)lpData;

	if(_this)
	{
		_this->TogglePipeDisplay();
	}
}

void TextBox::TogglePipeDisplay()
{
	m_bDisplayPipe = !m_bDisplayPipe;
}
