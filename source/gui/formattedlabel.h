// ******************************************************************************
//
// Filename:	formattedlabel.h
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A formatted label component that can render formatted text strings.
//	 Constructs multiple simple labels when the formatted text is parsed and
//	 is in charge of managing and rendering these labels.
//
// Revision History:
//   Initial Revision - 15/03/09
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#pragma once

#include "component.h"
#include "label.h"


static const int MAX_NUM_LINES = 30;


class FormattedLabel : public Component
{
public:
	/* Public methods */
	FormattedLabel(Renderer* pRenderer, unsigned int NormalFont, unsigned int BoldFont, unsigned int ItalicFont, const std::string &label);

	~FormattedLabel();

	void SetColour(Colour lColour);
	void SetOutlineColour(Colour lColour);
	void SetOutlineFont(unsigned int OutlineGUIFont);

	void SetOutline(bool outline);

	void SetDimensions(int x, int y, int width, int height) override;
	void SetDimensions(const Dimensions& r) override;

	void SetStyle();

	void SetVerticalAlignment(EVerticalAlignment alignment);
	EVerticalAlignment GetVerticalAlignment() const;

	void SetHorizontalAlignment(EHorizontalAlignment alignment);
	EHorizontalAlignment GetHorizontalAlignment() const;

	void SetWordWrap(bool lWrap);
	bool IsWordWrapEnabled() const;

	void SetNewLineSpacer(int lSpacer);

	void SetText(const std::string &label);
	const std::string& GetText() const;

	bool DoesTextExist(const char *lpText);
	Label* GetLabel(const char *lpText);

	int GetNumberOfLines() const;
	int GetNumberOfLabels() const;
	int GetMinimumTextHeight() const;

	void UpdateText();

	void ReAlignText();

protected:
	/* Protected methods */
	void ParseText();

	void ClearLabels();

	void DrawSelf() override;

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	std::string m_text;

	Colour m_colour;
	Colour m_outlineColour;

	bool m_outline;

	unsigned int m_NormalFont;
	unsigned int m_BoldFont;
	unsigned int m_ItalicFont;
	unsigned int m_OutlineGUIFont;

	std::vector<Label> LabelList;

	int m_NumLines;

	EVerticalAlignment m_eVerticalAlignment;
	EHorizontalAlignment m_eHorizontalAlignment;

	bool m_bWordWrap;

	int m_NewLineSpacer;

	bool m_bBold;
	bool m_bItalic;
};
