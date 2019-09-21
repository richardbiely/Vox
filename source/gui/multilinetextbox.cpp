// ******************************************************************************
//
// Filename:	multilinetextbox.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A multi-line text entry box that accepts focus events and key events.
//
// Revision History:
//   Initial Revision - 09/01/13
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "multilinetextbox.h"
#include "focusmanager.h"
#include "../utils/TimeManager.h"


MultiLineTextBox::MultiLineTextBox(Renderer* pRenderer, unsigned int GUIFont, const std::string &defaultText)
  : Container(pRenderer),
    m_GUIFont(GUIFont),
	mScrollBar(pRenderer)
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

	mScrollBar.SetScrollDirection(EScrollBarDirection_Vertical);
	mScrollBar.SetScrollPosition(0.75f);

	mScrollBar.SetValueChangedCallback(_ScrollbarChanged);
	mScrollBar.SetValueChangedCallBackData(this);

	Add(&mScrollBar);

	//m_text.SetLocation(0, 0);
	//m_text.SetDepth(2.0f);
	//Add(&m_text);

	m_pipeYOffset = 0;

	SetText(defaultText);

	UpdateText();

	m_HighlightStartIndex = 0;
	if(LabelList.size() >= 1)
	{
		m_PipeCharacterIndex = (int)m_text.size();
	}

	m_HighlightEndIndex = 0;

	m_PipeCharacterIndex = 0;
	m_bDisplayPipe = false;

	m_newLinePipeRender = false;

	m_pipeHeight = 0;
	m_textIndent = 0;
	m_maxTextLength = 0;

	m_bShiftHeld = false;

	m_bHiddenText = false;
	m_HideCharacter = '*';

	m_Callback_OnReturnPressed = nullptr;

	m_NumLines = 0;

	m_eVerticalAlignment = eVerticalAlignment_Top;
	m_eHorizontalAlignment = eHorizontalAlignment_Left;

	m_pPipeDisplayCountDown = new CountdownTimer();
	m_pPipeDisplayCountDown->SetCountdownTime(0.5f);
	m_pPipeDisplayCountDown->SetLooping(true);
	m_pPipeDisplayCountDown->SetCallBackFunction(_TogglePipeDisplay);
	m_pPipeDisplayCountDown->SetCallBackData(this);
	m_pPipeDisplayCountDown->StartCountdown();
}

MultiLineTextBox::~MultiLineTextBox()
{
	TimeManager::GetInstance()->RemoveCountdownTimer(m_pPipeDisplayCountDown);

	delete m_pBackgroundIcon;

	// Clear the label list, since we still have labels present
	LabelList.clear();
}

void MultiLineTextBox::AddEventListeners()
{
	Container::AddEventListeners();

	AddMouseListener(this);
	AddFocusListener(this);
	AddKeyListener(this);
}

void MultiLineTextBox::RemoveEventListeners()
{
	Container::RemoveEventListeners();

	RemoveMouseListener(this);
	RemoveFocusListener(this);
	RemoveKeyListener(this);
}

void MultiLineTextBox::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);

	// Also set the dimensions of the icons
	m_pBackgroundIcon->SetDimensions(0, 0, width, height);

	mScrollBar.SetDimensions(width, 0, 20, height);
}

void MultiLineTextBox::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void MultiLineTextBox::SetTextColour(Colour lColour)
{
	//m_text.SetColour(lColour);
}

void MultiLineTextBox::SetBackgroundIcon(RenderRectangle *icon)
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

void MultiLineTextBox::SetPipHeight(int height)
{
	m_pipeHeight = height;
}

void MultiLineTextBox::SetTextIndent(int indent)
{
	m_textIndent = indent;
}

void MultiLineTextBox::SetMaxTextLength(int length)
{
	m_maxTextLength = length;
}

void MultiLineTextBox::SetPipeColour(Colour pipeColour)
{
	m_pipeColour = pipeColour;
}

void MultiLineTextBox::SetText(const std::string &label)
{
	m_text = label;

	UpdateText();

	if(LabelList.size() >= 1)
	{
		m_PipeCharacterIndex = (int)m_text.size();
	}
}

const std::string& MultiLineTextBox::GetText() const
{
	return m_text;
}

void MultiLineTextBox::SetVerticalAlignment(EVerticalAlignment alignment)
{
	m_eVerticalAlignment = alignment;

	// Also update the text, since we have changed vertical alignment
	UpdateText();
}

EVerticalAlignment MultiLineTextBox::GetVerticalAlignment() const
{
	return m_eVerticalAlignment;
}

void MultiLineTextBox::SetHorizontalAlignment(EHorizontalAlignment alignment)
{
	m_eHorizontalAlignment = alignment;

	// Also update the text, since we have changed horizontal alignment
	UpdateText();
}

EHorizontalAlignment MultiLineTextBox::GetHorizontalAlignment() const
{
	return m_eHorizontalAlignment;
}

void MultiLineTextBox::SetHiddenText(bool lbHidden)
{
	m_bHiddenText = lbHidden;
}

bool MultiLineTextBox::IsHiddenText() const
{
	return m_bHiddenText;
}

void MultiLineTextBox::SetHideCharacter(char lHideChar)
{
	m_HideCharacter = lHideChar;
}

char MultiLineTextBox::GetHideCharacter() const
{
	return m_HideCharacter;
}

void MultiLineTextBox::SetCallBackFunction_OnReturnPressed(FunctionCallback lFunction)
{
	m_Callback_OnReturnPressed = lFunction;
}

void MultiLineTextBox::SetCallBackData_OnReturnPressed(void *lpData)
{
	m_pCallbackData_OnReturnPressed = lpData;
}

EComponentType MultiLineTextBox::GetComponentType() const
{
	return EComponentType_MultiLineTextBox;
}

void MultiLineTextBox::KeyPressed(const KeyEvent& lEvent)
{
	if(lEvent.GetKeyCode() == 0x08)
	{
		int l_NumHighlightChars = m_HighlightEndIndex - m_HighlightStartIndex;

		if(l_NumHighlightChars > 0)
		{
			//m_text.Erase(m_HighlightStartIndex, l_NumHighlightChars);

			m_PipeCharacterIndex = m_HighlightStartIndex;
		}
		else
		{
			// Remove the previous character and update the pipe character index
			if(m_PipeCharacterIndex > 0)
			{
				m_text.erase(m_PipeCharacterIndex - 1, 1);
				m_PipeCharacterIndex -= 1;
			}
		}

		m_HighlightStartIndex = m_PipeCharacterIndex;
		m_HighlightEndIndex = m_PipeCharacterIndex;

		m_bDisplayPipe = true;
		m_pPipeDisplayCountDown->ResetCountdown();
	}
	/*
	if(lEvent.GetKeyCode() == VK_DELETE)
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

	if(lEvent.GetKeyCode() == VK_HOME)
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

	if(lEvent.GetKeyCode() == VK_END)
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
		m_PipeDisplayCountDown.ResetCountdown();
	}
	*/

	if(lEvent.GetKeyCode() == 0x0D)
	{
		char lChar[2];

		lChar[0] = '\n';
		lChar[1] = 0; // Make sure we include a terminator

		m_text.insert(m_PipeCharacterIndex, &lChar[0]);

		// Update the pipe character index
		m_PipeCharacterIndex += 1;

		m_bDisplayPipe = true;
		m_pPipeDisplayCountDown->ResetCountdown();
	}

	/*
	if(lEvent.GetKeyCode() == VK_LEFT)
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

	if(lEvent.GetKeyCode() == VK_RIGHT)
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

	if(lEvent.GetKeyCode() == VK_SHIFT)
	{
		m_bShiftHeld = true;
	}
	*/

	UpdateText();
}

void MultiLineTextBox::KeyReleased(const KeyEvent& lEvent)
{
	if(lEvent.GetKeyCode() == 0x10 || lEvent.GetKeyCode() == 0xA0 || lEvent.GetKeyCode() == 0xA1)
	{
		m_bShiftHeld = false;
	}
}

void MultiLineTextBox::KeyTyped(const KeyEvent& lEvent)
{
}

void MultiLineTextBox::CharacterEntered(const KeyEvent& lEvent)
{
	/*
	int l_NumHighlightChars = m_HighlightEndIndex - m_HighlightStartIndex;

	if(l_NumHighlightChars > 0)
	{
		m_text.Erase(m_HighlightStartIndex, l_NumHighlightChars);

		m_PipeCharacterIndex = m_HighlightStartIndex;
	}
	*/

	int lCharacterCode = lEvent.GetKeyCode();

	if(m_maxTextLength == 0 || m_text.size() < m_maxTextLength)
	{
		// NOTE : This range excludes all the control characters such as backspace
		// and carriage return (code < 31), these should be handled in key events.
		if(lCharacterCode > 31 && lCharacterCode < 255)
		{
			char lChar[2];

			lChar[0] = lEvent.GetKeyCode();
			lChar[1] = 0; // Make sure we include a terminator


			m_text.insert(m_PipeCharacterIndex, &lChar[0]);

			// Update the pipe character index
			m_PipeCharacterIndex += 1;

			m_bDisplayPipe = true;
			m_pPipeDisplayCountDown->ResetCountdown();
		}
	}

	m_HighlightStartIndex = m_PipeCharacterIndex;
	m_HighlightEndIndex = m_PipeCharacterIndex;

	UpdateText();

	mScrollBar.SetScrollPosition(0.0f);
}

void MultiLineTextBox::FocusLost(const FocusEvent& lEvent)
{
	m_HighlightStartIndex = m_PipeCharacterIndex;
	m_HighlightEndIndex = m_PipeCharacterIndex;
}

void MultiLineTextBox::FocusGained(const FocusEvent& lEvent)
{
	m_bDisplayPipe = true;
	m_pPipeDisplayCountDown->ResetCountdown();

	m_HighlightStartIndex = m_PipeCharacterIndex;
	m_HighlightEndIndex = m_PipeCharacterIndex;
}

void MultiLineTextBox::OnReturnPressed()
{
	/* Nothing */
}

void MultiLineTextBox::UpdateText()
{
	// Parse the text into separate label components that we will manage
	ParseText();

	// Re-align the label components
	ReAlignText();

	// Update the scrollbar
	UpdateScrollbar();

	DepthSortComponentChildren();
}

void MultiLineTextBox::ReAlignText()
{
	// We will need to know the height of the text
	int lTextHeight = m_pRenderer->GetFreeTypeTextHeight(m_GUIFont, "a");

	// Now we can work out the X and Y modifications to apply to each label component depending on what alignment we have...

	int XModification = 0;
	int YModification = 0;

	// Vertical alignment is easy to calculate and we only need to know how many lines we have in total to work out the Y modification
	int lBottomTextY = m_dimensions.m_height - (m_NumLines * lTextHeight);
	if(m_eVerticalAlignment == eVerticalAlignment_Top)
	{
		YModification = 0;
	}
	else if(m_eVerticalAlignment == eVerticalAlignment_Center)
	{
		YModification = lBottomTextY / 2;
	}
	else if(m_eVerticalAlignment == eVerticalAlignment_Bottom)
	{
		YModification = lBottomTextY;
	}

	std::vector<Label>::iterator iterator;
	for(iterator = LabelList.begin(); iterator != LabelList.end(); ++iterator)
	{
		// Apply the position modifications to the label component
		int lX = (*iterator).GetLocation().m_x + XModification;
		int lY = (*iterator).GetLocation().m_y - YModification;

		(*iterator).SetLocation(lX, lY);
		(*iterator).SetDepth(3.0f);
	}
}

void MultiLineTextBox::ParseText()
{
	// First clear out any labels we already have
	ClearLabels();

	if(strcmp(m_text.c_str(), "") == 0)
	{
		// If we have no text then set some default values and early out
		m_NumLines = 0;

		return;
	}

	m_NumLines = 1;  // We have at least 1 line

	unsigned int lCurrentFontID = m_GUIFont;  // Start off with the normal font

	// Keep track of the text height, needed when we create new lines of text
	int lTextHeight = m_pRenderer->GetFreeTypeTextHeight(lCurrentFontID, "a");

	// Next go though every character in the text
	int lCharIndex = 0;
	int lStartLineIndex = 0;
	int lLastGoodSpaceIndex = 0;
	bool lbFoundGoodSpace = false;
	bool lFinalCharFound = false;
	bool lLongWordwrapFound = false;
	m_newLinePipeRender = false;
	const char *lpString = m_text.c_str();

	// Our current label position
	int lCurrentTextX = m_textIndent;
	int lCurrentTextPosition = m_textIndent;
	int lCurrentTextY = m_dimensions.m_height - m_textIndent - lTextHeight;

	// Current label colour
	Colour lCurrentColour(1.0f, 1.0f, 1.0f);

	while(lpString[lCharIndex] != 0)
	{
		char lpChar = lpString[lCharIndex];

		// Word wrap
		//if(m_bWordWrap)
		{
			lLongWordwrapFound = false;
			m_newLinePipeRender = false;

			// Check for word wrapping on a single line with a really long string that goes out of bounds without a space or newline
			//if(lbFoundGoodSpace == false)
			{
				std::string lString(m_text.substr(lStartLineIndex, lCharIndex+1 - lStartLineIndex));
				if(lbFoundGoodSpace)
				{
					lString = m_text.substr(lLastGoodSpaceIndex+1, (lCharIndex+1) - (lLastGoodSpaceIndex+1));
				}

				int lTextLineWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());

				// If the current X position, plus our new text length is greater than the width, then we know we will go out of bounds
				if(lCurrentTextPosition + lTextLineWidth > m_dimensions.m_width)
				{
					//char lpNextChar = lpString[lCharIndex+1];
					//int indexToUse = (lCharIndex+1);
					//if(lpNextChar == '\n' || lpNextChar == 0)
					//{
						int indexToUse = lCharIndex;
					//}

					std::string lString(m_text.substr(lStartLineIndex, indexToUse - lStartLineIndex));
					if(lbFoundGoodSpace)
					{
						lString = m_text.substr(lStartLineIndex, lLastGoodSpaceIndex - lStartLineIndex);
					}

					Label lNewLabel(m_pRenderer, lCurrentFontID, lString, lCurrentColour);
					int lTextWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());
					lNewLabel.SetLocation(lCurrentTextX, lCurrentTextY);
					lNewLabel.SetDepth(3.0f);
					LabelList.push_back(lNewLabel);

					// Modify our Y position, since we are moving down to a new line
					lCurrentTextY -= lTextHeight;

					// Skip over the new line, else we will detect it on the next loop t
					lStartLineIndex = indexToUse;
					if(lbFoundGoodSpace)
					{
						lStartLineIndex = lLastGoodSpaceIndex + 1;
					}

					// Reset the current text X position, since we have created a new line
					lCurrentTextX = m_textIndent;
					lCurrentTextPosition = m_textIndent;

					// Increase the line number count
					m_NumLines++;

					// Reset the good space flag, since we have created a new line
					lbFoundGoodSpace = false;

					//m_newLinePipeRender = true;

					lLongWordwrapFound = true;
				}
			}

			// Check for spaces
			if(lLongWordwrapFound == false && lpChar == ' ')
			{
				std::string lString(m_text.substr(lStartLineIndex, lCharIndex - lStartLineIndex));

				int lTextLineWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());

				// If the current X position, plus our new text length is greater than the width, then we know we will go out of bounds
				if(lCurrentTextX + lTextLineWidth > m_dimensions.m_width)
				{
					std::string lString;

					if(lbFoundGoodSpace)
					{
						// We need to create a new line now, but only include up to the last good space in the previous label that we are making
						lString = std::string (m_text.substr(lStartLineIndex, lLastGoodSpaceIndex - lStartLineIndex));
					}
					else
					{
						// We haven't been able to find a good space, that means that that a whole word is longer than the width of the box
						lString = std::string (m_text.substr(lStartLineIndex, lCharIndex - lStartLineIndex));
					}

					Label lNewLabel(m_pRenderer, lCurrentFontID, lString, lCurrentColour);
					int lTextWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());
					lNewLabel.SetLocation(lCurrentTextX, lCurrentTextY);
					lNewLabel.SetDepth(3.0f);
					LabelList.push_back(lNewLabel);

					// Modify our Y position, since we are moving down to a new line
					lCurrentTextY -= lTextHeight;

					if(lbFoundGoodSpace)
					{
						lCharIndex = lLastGoodSpaceIndex;
					}

					// Skip over the space, we don't want to start the new line with a leading space
					lStartLineIndex = lCharIndex + 1;

					// Reset the current text X position, since we have created a new line
					lCurrentTextX = m_textIndent;
					lCurrentTextPosition = m_textIndent;

					// Increase the line number count
					m_NumLines++;

					// Reset the good space flag, since we have created a new line
					lbFoundGoodSpace = false;
				}
				else
				{
					std::string lString(m_text.substr(lStartLineIndex, lCharIndex - lStartLineIndex));

					int lTextLineWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());

					lCurrentTextPosition = m_textIndent + lTextLineWidth;

					lbFoundGoodSpace = true;

					lLastGoodSpaceIndex = lCharIndex;
				}
			}

			// Check for new lines, being at the end of a word-wrapped line, this usually means the last word doesn't get wrapped properly
			if(lLongWordwrapFound == false && lpChar == '\n')
			{
				std::string lString(m_text.substr(lStartLineIndex, lCharIndex - lStartLineIndex));

				int lTextLineWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());

				// If the current X position, plus our new text length is greater than the width, then we know we will go out of bounds
				if(lCurrentTextPosition + lTextLineWidth > m_dimensions.m_width)
				{
					if(lbFoundGoodSpace)
					{
						std::string lString(m_text.substr(lStartLineIndex, lLastGoodSpaceIndex - lStartLineIndex));

						Label lNewLabel(m_pRenderer, lCurrentFontID, lString, lCurrentColour);
						int lTextWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());
						lNewLabel.SetLocation(lCurrentTextX, lCurrentTextY);
						lNewLabel.SetDepth(3.0f);
						LabelList.push_back(lNewLabel);

						// Modify our Y position, since we are moving down to a new line
						lCurrentTextY -= lTextHeight;

						// Skip over the new line, else we will detect it on the next loop t
						lStartLineIndex = lLastGoodSpaceIndex + 1;

						// Reset the current text X position, since we have created a new line
						lCurrentTextX = m_textIndent;
						lCurrentTextPosition = m_textIndent;

						// Increase the line number count
						m_NumLines++;
					}

					// Reset the good space flag, since we have created a new line
					lbFoundGoodSpace = false;
				}
			}

			// Check for the end of the string, need to make sure the final word gets wrapped, if it is needed
			char lpFinalChar = lpString[lCharIndex+1];
			if(lpFinalChar == 0)
			{
				// Make sure we don't try to include new line characters in the width calculations
				char lpPreviousChar = lpString[lCharIndex];
				int indexToUse = lCharIndex;
				//if(lpPreviousChar == '\n')
				//{
				//	indexToUse = (lCharIndex+1);
				//}

				std::string lString(m_text.substr(lStartLineIndex, indexToUse - lStartLineIndex));
				if(lbFoundGoodSpace)
				{
					lString = m_text.substr(lLastGoodSpaceIndex, indexToUse);
				}

				int lTextLineWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());

				// If the current X position, plus our new text length is greater than the width, then we know we will go out of bounds
				if(lCurrentTextPosition + lTextLineWidth > m_dimensions.m_width)
				{
					if(lbFoundGoodSpace)
					{
						std::string lString(m_text.substr(lStartLineIndex, lLastGoodSpaceIndex - lStartLineIndex));
						
						Label lNewLabel(m_pRenderer, lCurrentFontID, lString, lCurrentColour);
						int lTextWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());
						lNewLabel.SetLocation(lCurrentTextX, lCurrentTextY);
						lNewLabel.SetDepth(3.0f);
						LabelList.push_back(lNewLabel);

						// Modify our Y position, since we are moving down to a new line
						lCurrentTextY -= lTextHeight;

						// Skip over the new line, else we will detect it on the next loop t
						lStartLineIndex = lLastGoodSpaceIndex + 1;

						// Reset the current text X position, since we have created a new line
						lCurrentTextX = m_textIndent;
						lCurrentTextPosition = m_textIndent;

						// Increase the line number count
						m_NumLines++;
					}

					// Reset the good space flag, since we have created a new line
					lbFoundGoodSpace = false;
				}

				// Make sure we don't try to render new line characters
				indexToUse = (lCharIndex+1);
				if(lpPreviousChar == '\n')
				{
					indexToUse = lCharIndex;
				}

				lString = string(m_text.substr(lStartLineIndex, indexToUse - lStartLineIndex));

				// Create the final label component
				Label lNewLabel(m_pRenderer, lCurrentFontID, lString, lCurrentColour);

				// Set the final label position
				int lTextWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());

				lNewLabel.SetLocation(lCurrentTextX, lCurrentTextY);
				lNewLabel.SetDepth(3.0f);

				// Add the final label to the list
				LabelList.push_back(lNewLabel);

				// Make sure we don't process any more rules.
				lFinalCharFound = true;
			}
		}


		// Checking for new lines
		if(lpChar == '\n')
		{
			// We have encountered a new line...

			if(lFinalCharFound == true)
			{
				// Modify our Y position, since we are moving down to a new line
				lCurrentTextY -= lTextHeight;

				// Reset the current text X position, since we have created a new line
				lCurrentTextX = m_textIndent;
				lCurrentTextPosition = m_textIndent;

				// Increase the line number count
				m_NumLines++;

				m_newLinePipeRender = true;
			}
			else
			{
				// Create the substring of the text for our new label component
				std::string lString(m_text.substr(lStartLineIndex, lCharIndex - lStartLineIndex));

				// Create the new label component using the new string we have just generated
				Label lNewLabel(m_pRenderer, lCurrentFontID, lString, lCurrentColour);

				// Set the location of this new label component
				int lTextWidth = m_pRenderer->GetFreeTypeTextWidth(lCurrentFontID, "%s", lString.c_str());
				lNewLabel.SetLocation(lCurrentTextX, lCurrentTextY);
				lNewLabel.SetDepth(3.0f);

				// Add the new label to the label list
				LabelList.push_back(lNewLabel);

				// Set the new starting index, REMEMBERING to skip over the new line char
				lStartLineIndex = lCharIndex + 1;

				// Modify our Y position, since we are moving down to a new line
				lCurrentTextY -= lTextHeight;

				// Reset the current text X position, since we have created a new line
				lCurrentTextX = m_textIndent;
				lCurrentTextPosition = m_textIndent;

				// Increase the line number count
				m_NumLines++;

				// Reset the good space flag, since we have created a new line
				lbFoundGoodSpace = false;
			}
		}

		lCharIndex++;
	}
}

void MultiLineTextBox::UpdateScrollbar()
{
	int l_TextHeight = m_pRenderer->GetFreeTypeTextHeight(m_GUIFont, "%s", "a");

	int visibleSize = GetDimensions().m_height;
	int neededHeight = m_NumLines * l_TextHeight;
	int heightDiff = neededHeight - visibleSize;
	if(heightDiff > 0)
	{
		mScrollBar.SetScrollSize((float)visibleSize / (float)neededHeight);
		mScrollBar.SetDisabled(false);

		float scrollRatio = mScrollBar.GetScrollRatio();
		int offsetLabelY = (int)(heightDiff * scrollRatio);

		m_pipeYOffset = offsetLabelY;

		for(unsigned int i = 0; i < (int)LabelList.size(); i++)
		{
			int xPos = m_textIndent;
			int yPos = GetDimensions().m_height - ((i+1)*l_TextHeight);

			LabelList[i].SetLocation(xPos, yPos + offsetLabelY);
		}
	}
	else
	{
		mScrollBar.SetScrollSize(0.0f);
		mScrollBar.SetDisabled(true);
	}
}

void MultiLineTextBox::ClearLabels()
{
	LabelList.clear();
}

void MultiLineTextBox::DrawChildren()
{
	// Do a scissor test here
	int lScissorX = GetLocation().m_x - 1;
	int lScissorY = GetLocation().m_y - 1;
	if(GetParent() != nullptr)
	{
		lScissorX = GetParent()->GetLocation().m_x + lScissorX;
		lScissorY = GetParent()->GetLocation().m_y + lScissorY;
	}	
	int lScissorWidth = GetDimensions().m_width + 20 + 2;
	int lScissorHeight = GetDimensions().m_height + 2;

	bool lbNeedScissor = true;

	if(lbNeedScissor)
	{
		m_pRenderer->EnableScissorTest(lScissorX, lScissorY, lScissorWidth, lScissorHeight);;
	}

	Container::DrawChildren();

	// Render all the normal label components that we contain
	std::vector<Label>::iterator iterator;
	for(iterator = LabelList.begin(); iterator != LabelList.end(); ++iterator)
	{
		(*iterator).Draw();
	}

	if(lbNeedScissor)
	{
		m_pRenderer->DisableScissorTest();
	}
}

void MultiLineTextBox::DrawSelf()
{
	// Do a scissor test here
	int lScissorX = GetLocation().m_x - 1;
	int lScissorY = GetLocation().m_y - 1;
	if(GetParent() != nullptr)
	{
		lScissorX = GetParent()->GetLocation().m_x + lScissorX;
		lScissorY = GetParent()->GetLocation().m_y + lScissorY;
	}	
	int lScissorWidth = GetDimensions().m_width + 20 + 2;
	int lScissorHeight = GetDimensions().m_height + 2;

	bool lbNeedScissor = true;

	if(lbNeedScissor)
	{
		m_pRenderer->EnableScissorTest(lScissorX, lScissorY, lScissorWidth, lScissorHeight);;
	}

	int l_containerWidth = m_dimensions.m_width;
	int l_containerHeight = m_dimensions.m_height;
	float l_depth = GetDepth() + 1.0f;

	float l_containerX1 = 0.5f;
	float l_containerX2 = (float)l_containerWidth;
	float l_containerY1 = 0.5f;
	float l_containerY2 = (float)l_containerHeight;

	float l_outlineX1 = -0.5f;
	float l_outlineX2 = l_containerWidth + 0.5f;
	float l_outlineY1 = 0.0f;
	float l_outlineY2 = l_containerHeight + 0.5f;

	int l_TextSpacer = 2;
	int l_TextHeight = m_pRenderer->GetFreeTypeTextHeight(m_GUIFont, "%s", "a");
	int m_center_height = (GetDimensions().m_height / 2) - (l_TextHeight / 2);
	int l_posX = m_textIndent + l_TextSpacer;
	int l_posY = m_center_height;

	std::string l_DisplayText = "";
	if(LabelList.size() >= 1)
	{
		l_DisplayText = LabelList[LabelList.size()-1].GetText();
	}

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
		if(m_newLinePipeRender)
		{
			l_TextWidth = 0;
		}
		int l_TextHeight = m_pRenderer->GetFreeTypeTextHeight(m_GUIFont, "%s", "a");
		int l_pipeX1 = m_textIndent + l_TextWidth;
		int l_pipeX2 = m_textIndent + l_TextWidth + l_PipeWidth;
		int l_pipeY1 = 0;
		int l_pipeY2 = 0;
		if(m_pipeHeight == 0)
		{
			l_pipeY1 = l_HeightSpace + m_pipeYOffset;
			l_pipeY2 = l_containerHeight - l_HeightSpace + m_pipeYOffset;
		}
		else
		{
			l_pipeY1 = l_containerHeight - l_TextHeight + m_pipeYOffset;
			l_pipeY2 = l_containerHeight - l_TextHeight + m_pipeHeight + m_pipeYOffset;
		}

		if(LabelList.size() >= 1)
		{
			l_pipeY1 = l_containerHeight - (l_TextHeight*m_NumLines) + m_pipeYOffset;
			l_pipeY2 = l_containerHeight - (l_TextHeight*m_NumLines) + m_pipeHeight + m_pipeYOffset;
		}

		// Draw the pipe (vertical bar)
		m_pRenderer->PushMatrix();
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->EnableImmediateMode(IM_QUADS);
			m_pRenderer->ImmediateColourAlpha(m_pipeColour.GetRed(), m_pipeColour.GetGreen(), m_pipeColour.GetBlue(), 1.0f);
			m_pRenderer->ImmediateVertex(l_pipeX1, l_pipeY1, (int)l_depth);
			m_pRenderer->ImmediateVertex(l_pipeX2, l_pipeY1, (int)l_depth);
			m_pRenderer->ImmediateVertex(l_pipeX2, l_pipeY2, (int)l_depth);
			m_pRenderer->ImmediateVertex(l_pipeX1, l_pipeY2, (int)l_depth);
			m_pRenderer->DisableImmediateMode();
		m_pRenderer->PopMatrix();
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
		int l_HighlightX1 = m_textIndent + l_HighlightStart;
		int l_HighlightX2 = m_textIndent + l_HighlightStart + l_HighlightWidth;
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
			m_pRenderer->ImmediateVertex(l_HighlightX1, l_HighlightY1, (int)l_depth);
			m_pRenderer->ImmediateVertex(l_HighlightX2, l_HighlightY1, (int)l_depth);
			m_pRenderer->ImmediateVertex(l_HighlightX2, l_HighlightY2, (int)l_depth);
			m_pRenderer->ImmediateVertex(l_HighlightX1, l_HighlightY2, (int)l_depth);
			m_pRenderer->DisableImmediateMode();
		m_pRenderer->PopMatrix();
	}

	if(lbNeedScissor)
	{
		m_pRenderer->DisableScissorTest();
	}

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

void MultiLineTextBox::_TogglePipeDisplay(void *lpData)
{
	MultiLineTextBox *_this = (MultiLineTextBox*)lpData;

	if(_this)
	{
		_this->TogglePipeDisplay();
	}
}

void MultiLineTextBox::TogglePipeDisplay()
{
	m_bDisplayPipe = !m_bDisplayPipe;
}

void MultiLineTextBox::_ScrollbarChanged(void *lpData)
{
	MultiLineTextBox *_this = (MultiLineTextBox*)lpData;
	_this->ScrollbarChanged();
}

void MultiLineTextBox::ScrollbarChanged()
{
	UpdateScrollbar();
}