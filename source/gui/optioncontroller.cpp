// ******************************************************************************
//
// Filename:	optioncontroller.cpp
// Project:     Vox
// Author:		Steven Ball
//
// Purpose:
//   A controller for a group of option boxes
//
// Revision History:
//   Initial Revision - 02/02/08
//
// Copyright (c) 2005-2006, Steven Ball
//
// ******************************************************************************

#include "optioncontroller.h"


OptionController::OptionController(Renderer* pRenderer, unsigned int GUIFont, const std::string &label)
  : Container(pRenderer),
    m_label(pRenderer, GUIFont, label, Colour(1.0f, 1.0f, 1.0f))
{
	m_label.SetLocation(0, 0);
	Add(&m_label);

	m_labelIndent = 5;

	m_pActiveOption = nullptr;

	mbDisplayLabel = false;
	mbDisplayBorder = false;
}

OptionController::OptionController(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, Colour fontColour, Colour outlineColour)
  : Container(pRenderer),
    m_label(pRenderer, GUIFont, label, Colour(1.0f, 1.0f, 1.0f))
{
	m_label.SetLocation(0, 0);
	m_label.SetColour(fontColour);
	m_label.SetOutline(true);
	m_label.SetOutlineColour(outlineColour);
	m_label.SetOutlineFont(OutlineGUIFont);
	m_label.SetDepth(3.0f);
	Add(&m_label);

	m_labelIndent = 5;

	m_pActiveOption = nullptr;

	mbDisplayLabel = false;
	mbDisplayBorder = false;
}

OptionController::~OptionController()
{
	/* Nothing */
}

void OptionController::SetVisible(bool Visible)
{
	// Also set the visibility of all our children too
	ComponentList::const_iterator iterator;
	for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
	{
		if((*iterator)->GetComponentType() == EComponentType_OptionBox)
		{
			OptionBox* lpOptionBox = (OptionBox *)(*iterator);

			lpOptionBox->SetVisible(Visible);
		}
	}
}

void OptionController::SetDimensions(int x, int y, int width, int height)
{
	Component::SetDimensions(x, y, width, height);

	// Also set the location of the label
	m_label.SetLocation(m_labelIndent, height);
}

void OptionController::SetDimensions(const Dimensions& r)
{
	SetDimensions(r.m_x, r.m_y, r.m_width, r.m_height);
}

void OptionController::SetDisplayLabel(bool display)
{
	mbDisplayLabel = display;
}

void OptionController::SetDisplayBorder(bool display)
{
	mbDisplayBorder = display;
}

void OptionController::SetActiveOption(OptionBox *lpOption)
{
	m_pActiveOption = lpOption;
}

OptionBox *OptionController::GetActiveOption() const
{
	return m_pActiveOption;
}

void OptionController::ResetAllOptions()
{
	m_pActiveOption = nullptr;

	ComponentList::const_iterator iterator;

	for(iterator = m_vpComponentList.begin(); iterator != m_vpComponentList.end(); ++iterator)
	{
		if((*iterator)->GetComponentType() == EComponentType_OptionBox)
		{
			OptionBox* lpOptionBox = (OptionBox *)(*iterator);

			lpOptionBox->SetToggled(false);
		}
	}
}

EComponentType OptionController::GetComponentType() const
{
	return EComponentType_OptionController;
}

void OptionController::DrawSelf()
{
	if(mbDisplayBorder)
	{
		int l_containerWidth = m_dimensions.m_width;
		int l_containerHeight = m_dimensions.m_height;
		float l_depth = GetDepth();

		float l_outlineX1 = 0.5f;
		float l_outlineX2 = l_containerWidth + 0.5f;
		float l_outlineY1 = 0.5f;
		float l_outlineY2 = l_containerHeight + 0.5f;

		int l_LabelheightAdjust = m_pRenderer->GetFreeTypeTextHeight(m_label.GetFontID(), "%s", m_label.GetText().c_str()) / 2;
		int l_labelWidth = m_pRenderer->GetFreeTypeTextWidth(m_label.GetFontID(), "%s", m_label.GetText().c_str());

		if(mbDisplayLabel)
		{
			m_pRenderer->PushMatrix();
				m_pRenderer->SetLineWidth(1.0f);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
				m_pRenderer->EnableImmediateMode(IM_LINES);
				m_pRenderer->ImmediateVertex(l_outlineX1 + m_labelIndent, l_outlineY2 + l_LabelheightAdjust, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY2 + l_LabelheightAdjust, l_depth);

				m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY2 + l_LabelheightAdjust, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY1, l_depth);

				m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY1, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY1, l_depth);

				m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY1, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY2 + l_LabelheightAdjust, l_depth);

				m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY2 + l_LabelheightAdjust, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX1 + m_labelIndent + l_labelWidth + 2, l_outlineY2 + l_LabelheightAdjust, l_depth);
				m_pRenderer->DisableImmediateMode();
			m_pRenderer->PopMatrix();
		}
		else
		{
			m_pRenderer->PushMatrix();
				m_pRenderer->SetLineWidth(1.0f);
				m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
				m_pRenderer->EnableImmediateMode(IM_LINE_LOOP);
				m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY1, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX2, l_outlineY1, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX2 + 1, l_outlineY2, l_depth);
				m_pRenderer->ImmediateVertex(l_outlineX1, l_outlineY2, l_depth);
				m_pRenderer->DisableImmediateMode();
			m_pRenderer->PopMatrix();
		}
	
	}

	// Set the label visibility
	m_label.SetVisible(mbDisplayLabel);
}