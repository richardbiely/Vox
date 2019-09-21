#include "renderrectangle.h"
#include "icon.h"
#include "label.h"

RenderRectangle::RenderRectangle(Renderer* pRenderer)
  :  Container(pRenderer)
{
}

RenderRectangle::~RenderRectangle()
{
	for(unsigned int i = 0; i < m_vpAddedComponentList.size(); i++)
	{
		Remove(m_vpAddedComponentList[i]);

		delete m_vpAddedComponentList[i];
		m_vpAddedComponentList[i] = 0;
	}
	m_vpAddedComponentList.clear();
}

void RenderRectangle::AddText(Renderer* pRenderer, unsigned int GUIFont, unsigned int OutlineGUIFont, const std::string &label, Colour colour, int xOffset, int yOffset, bool outline, Colour outlineColour)
{
	Label* lpNewLabel = new Label(pRenderer, GUIFont, label, colour);
	lpNewLabel->SetLocation(xOffset, yOffset);
	lpNewLabel->SetDepth(lpNewLabel->GetDepth()+1.0f);

	if(outline)
	{
		lpNewLabel->SetOutline(true);
		lpNewLabel->SetOutlineColour(outlineColour);
		lpNewLabel->SetOutlineFont(OutlineGUIFont);
	}

	m_vpAddedComponentList.push_back(lpNewLabel);

	Add(lpNewLabel);
}

void RenderRectangle::AddIcon(Renderer* pRenderer, const std::string &fileName, int texWidth, int texHeight, int width, int height, int xOffset, int yOffset, float depth)
{
	// texWidth and texHeight should always be a power of 2.

	Icon* lpNewIcon = new Icon(pRenderer, fileName, texWidth, texHeight);
	lpNewIcon->SetDimensions(xOffset, yOffset, width, height);
	lpNewIcon->SetDepth(depth);

	m_vpAddedComponentList.push_back(lpNewIcon);

	Add(lpNewIcon);
}

EComponentType RenderRectangle::GetComponentType() const
{
	return EComponentType_RenderRectangle;
}

void RenderRectangle::DrawSelf()
{
}

void RenderRectangle::DrawChildren()
{
	Container::DrawChildren();
}