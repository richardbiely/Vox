// ******************************************************************************
// Filename:    Interpolator.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 23/02/12
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "Interpolator.h"

#include "../Maths/3dGeometry.h"

#include <stdio.h>
#include <algorithm>

#pragma comment (lib, "Winmm.lib")


// Initialize the singleton instance
Interpolator *Interpolator::c_instance = 0;

Interpolator* Interpolator::GetInstance()
{
	if(c_instance == 0)
		c_instance = new Interpolator;

	return c_instance;
}

void Interpolator::Destroy()
{
	if(c_instance)
	{
		ClearInterpolators();

		delete c_instance;
	}
}

Interpolator::Interpolator()
{
	m_paused = false;
}

void Interpolator::ClearInterpolators()
{
	// Float
	for(unsigned int i = 0; i < m_vpFloatInterpolations.size(); i++)
	{
		FloatInterpolation* lpNext = m_vpFloatInterpolations[i]->m_pNextInterpolation;
		while(lpNext != nullptr)
		{
			FloatInterpolation* lpThisOne = lpNext;
			lpNext = lpNext->m_pNextInterpolation;
			
			if(lpThisOne->m_variable != 0)
			{
				delete lpThisOne;
				lpThisOne->m_variable = 0;
				lpThisOne->m_pNextInterpolation = 0;
				lpThisOne = 0;
			}
		}

		if(m_vpFloatInterpolations[i]->m_variable != 0)
		{
			delete m_vpFloatInterpolations[i];
			m_vpFloatInterpolations[i]->m_variable = 0;
			m_vpFloatInterpolations[i]->m_pNextInterpolation = 0;
			m_vpFloatInterpolations[i] = 0;
		}
	}
	m_vpFloatInterpolations.clear();

	for(unsigned int i = 0; i < m_vpCreateFloatInterpolations.size(); i++)
	{
		FloatInterpolation* lpNext = m_vpCreateFloatInterpolations[i]->m_pNextInterpolation;
		while(lpNext != nullptr)
		{
			FloatInterpolation* lpThisOne = lpNext;
			lpNext = lpNext->m_pNextInterpolation;

			if(lpThisOne->m_variable != 0)
			{
				delete lpThisOne;
				lpThisOne->m_variable = 0;
				lpThisOne->m_pNextInterpolation = 0;
				lpThisOne = 0;
			}
		}

		if(m_vpCreateFloatInterpolations[i]->m_variable != 0)
		{
			delete m_vpCreateFloatInterpolations[i];
			m_vpCreateFloatInterpolations[i]->m_variable = 0;
			m_vpCreateFloatInterpolations[i]->m_pNextInterpolation = 0;
			m_vpCreateFloatInterpolations[i] = 0;
		}
	}
	m_vpCreateFloatInterpolations.clear();

	// Int
	for(unsigned int i = 0; i < m_vpIntInterpolations.size(); i++)
	{
		IntInterpolation* lpNext = m_vpIntInterpolations[i]->m_pNextInterpolation;
		while(lpNext != nullptr)
		{
			IntInterpolation* lpThisOne = lpNext;
			lpNext = lpNext->m_pNextInterpolation;

			if(lpThisOne->m_variable != 0)
			{
				delete lpThisOne;
				lpThisOne->m_variable = 0;
				lpThisOne->m_pNextInterpolation = 0;
				lpThisOne = 0;
			}
		}

		if(m_vpIntInterpolations[i]->m_variable != 0)
		{
			delete m_vpIntInterpolations[i];
			m_vpIntInterpolations[i]->m_variable = 0;
			m_vpIntInterpolations[i]->m_pNextInterpolation = 0;
			m_vpIntInterpolations[i] = 0;
		}
	}
	m_vpIntInterpolations.clear();

	for(unsigned int i = 0; i < m_vpCreateIntInterpolations.size(); i++)
	{
		IntInterpolation* lpNext = m_vpCreateIntInterpolations[i]->m_pNextInterpolation;
		while(lpNext != nullptr)
		{
			IntInterpolation* lpThisOne = lpNext;
			lpNext = lpNext->m_pNextInterpolation;

			if(lpThisOne->m_variable != 0)
			{
				delete lpThisOne;
				lpThisOne->m_variable = 0;
				lpThisOne->m_pNextInterpolation = 0;
				lpThisOne = 0;
			}
		}

		if(m_vpCreateIntInterpolations[i]->m_variable != 0)
		{
			delete m_vpCreateIntInterpolations[i];
			m_vpCreateIntInterpolations[i]->m_variable = 0;
			m_vpCreateIntInterpolations[i]->m_pNextInterpolation = 0;
			m_vpCreateIntInterpolations[i] = 0;
		}
	}
	m_vpCreateIntInterpolations.clear();
}

FloatInterpolation* Interpolator::CreateFloatInterpolation(float *val, float start, float end, float time, float easing, FloatInterpolation* aNext, FunctionCallback aCallback, void *aData)
{
	FloatInterpolation* floatInterp = new FloatInterpolation();
	floatInterp->m_variable = val;
	floatInterp->m_start = start;
	floatInterp->m_end = end;
	floatInterp->m_time = time;

	floatInterp->m_easing = easing;

	floatInterp->m_pNextInterpolation = aNext;

	floatInterp->m_elapsed = 0.0f;
	floatInterp->m_erase = false;

	floatInterp->m_Callback = aCallback;
	floatInterp->m_pCallbackData = aData;

	return floatInterp;
}

void Interpolator::LinkFloatInterpolation(FloatInterpolation* aFirst, FloatInterpolation* aSecond)
{
	aFirst->m_pNextInterpolation = aSecond;
}

void Interpolator::AddFloatInterpolation(FloatInterpolation* aInterpolation)
{
	// Reset the elapsed time
	aInterpolation->m_elapsed = 0.0f;

	// Rest the erase flag
	aInterpolation->m_erase = false;

	m_vpCreateFloatInterpolations.push_back(aInterpolation);
}

void Interpolator::AddFloatInterpolation(float *val, float start, float end, float time, float easing, FloatInterpolation* aNext, FunctionCallback aCallback, void *aData)
{
	FloatInterpolation* floatInterp = CreateFloatInterpolation(val, start, end, time, easing, aNext, aCallback, aData);

	AddFloatInterpolation(floatInterp);
}

void Interpolator::RemoveCreateFloatInterpolation(FloatInterpolation* aInterpolation)
{
	m_vpCreateFloatInterpolations.erase(std::remove(m_vpCreateFloatInterpolations.begin(), m_vpCreateFloatInterpolations.end(), aInterpolation), m_vpCreateFloatInterpolations.end());
}

void Interpolator::RemoveFloatInterpolation(FloatInterpolation* aInterpolation)
{
	m_vpFloatInterpolations.erase(std::remove(m_vpFloatInterpolations.begin(), m_vpFloatInterpolations.end(), aInterpolation), m_vpFloatInterpolations.end());
}

void Interpolator::RemoveFloatInterpolationByVariable(float *val)
{
	// Creation list
	FloatInterpolationList l_vpRemoveCreateFloatInterpolations;
	for(unsigned int i = 0; i < (int)m_vpCreateFloatInterpolations.size(); i++)
	{
		FloatInterpolation* pInterpolator = m_vpCreateFloatInterpolations[i];
		if(pInterpolator->m_variable == val)
		{
			l_vpRemoveCreateFloatInterpolations.push_back(pInterpolator);
		}
	}
	for(unsigned int i = 0; i < (int)l_vpRemoveCreateFloatInterpolations.size(); i++)
	{
		RemoveCreateFloatInterpolation(l_vpRemoveCreateFloatInterpolations[i]);
	}


	// Normal list
	FloatInterpolationList l_vpRemoveFloatInterpolations;
	for(unsigned int i = 0; i < (int)m_vpFloatInterpolations.size(); i++)
	{
		FloatInterpolation* pInterpolator = m_vpFloatInterpolations[i];
		if(pInterpolator->m_variable == val)
		{
			l_vpRemoveFloatInterpolations.push_back(pInterpolator);
		}
	}
	for(unsigned int i = 0; i < (int)l_vpRemoveFloatInterpolations.size(); i++)
	{
		RemoveFloatInterpolation(l_vpRemoveFloatInterpolations[i]);
	}
}

IntInterpolation* Interpolator::CreateIntInterpolation(int *val, int start, int end, float time, float easing, IntInterpolation* aNext, FunctionCallback aCallback, void *aData)
{
	IntInterpolation* intInterp = new IntInterpolation();
	intInterp->m_variable = val;
	intInterp->m_start = start;
	intInterp->m_end = end;
	intInterp->m_time = time;

	intInterp->m_easing = easing;

	intInterp->m_pNextInterpolation = aNext;

	intInterp->m_elapsed = 0.0f;
	intInterp->m_erase = false;

	intInterp->m_Callback = aCallback;
	intInterp->m_pCallbackData = aData;

	return intInterp;
}

void Interpolator::LinkIntInterpolation(IntInterpolation* aFirst, IntInterpolation* aSecond)
{
	aFirst->m_pNextInterpolation = aSecond;
}

void Interpolator::AddIntInterpolation(IntInterpolation* aInterpolation)
{
	// Reset the elapsed time
	aInterpolation->m_elapsed = 0.0f;

	// Rest the erase flag
	aInterpolation->m_erase = false;

	m_vpCreateIntInterpolations.push_back(aInterpolation);
}

void Interpolator::AddIntInterpolation(int *val, int start, int end, float time, float easing, IntInterpolation* aNext, FunctionCallback aCallback, void *aData)
{
	IntInterpolation* intInterp = CreateIntInterpolation(val, start, end, time, easing, aNext, aCallback, aData);

	AddIntInterpolation(intInterp);
}

void Interpolator::RemoveCreateIntInterpolation(IntInterpolation* aInterpolation)
{
	m_vpCreateIntInterpolations.erase(std::remove(m_vpCreateIntInterpolations.begin(), m_vpCreateIntInterpolations.end(), aInterpolation), m_vpCreateIntInterpolations.end());
}

void Interpolator::RemoveIntInterpolation(IntInterpolation* aInterpolation)
{
	m_vpIntInterpolations.erase(std::remove(m_vpIntInterpolations.begin(), m_vpIntInterpolations.end(), aInterpolation), m_vpIntInterpolations.end());
}

void Interpolator::RemoveIntInterpolationByVariable(int *val)
{
	// Creation list
	IntInterpolationList l_vpRemoveCreateIntInterpolations;
	for(unsigned int i = 0; i < (int)m_vpCreateIntInterpolations.size(); i++)
	{
		IntInterpolation* pInterpolator = m_vpCreateIntInterpolations[i];
		if(pInterpolator->m_variable == val)
		{
			l_vpRemoveCreateIntInterpolations.push_back(pInterpolator);
		}
	}
	for(unsigned int i = 0; i < (int)l_vpRemoveCreateIntInterpolations.size(); i++)
	{
		RemoveCreateIntInterpolation(l_vpRemoveCreateIntInterpolations[i]);
	}


	// Normal list
	IntInterpolationList l_vpRemoveIntInterpolations;
	for(unsigned int i = 0; i < (int)m_vpIntInterpolations.size(); i++)
	{
		IntInterpolation* pInterpolator = m_vpIntInterpolations[i];
		if(pInterpolator->m_variable == val)
		{
			l_vpRemoveIntInterpolations.push_back(pInterpolator);
		}
	}
	for(unsigned int i = 0; i < (int)l_vpRemoveIntInterpolations.size(); i++)
	{
		RemoveIntInterpolation(l_vpRemoveIntInterpolations[i]);
	}
}

bool needs_erasing_float(FloatInterpolation* aF)
{
	bool needsErase = aF->m_erase;

	if(needsErase == true)
	{
		delete aF;
	}

	return needsErase;
}

bool needs_erasing_int(IntInterpolation* aI)
{
	bool needsErase = aI->m_erase;

	if(needsErase == true)
	{
		delete aI;
	}

	return needsErase;
}

void Interpolator::SetPaused(bool pause)
{
	m_paused = pause;
}

bool Interpolator::IsPaused()
{
	return m_paused;
}

void Interpolator::Update(float dt)
{
	UpdateFloatInterpolators(dt);
	UpdateIntInterpolators(dt);
}

void Interpolator::UpdateFloatInterpolators(float delta)
{
	// Add any interpolators in the create list
	for(unsigned int i = 0; i < (int)m_vpCreateFloatInterpolations.size(); i++)
	{
		FloatInterpolation* pInterpolator = m_vpCreateFloatInterpolations[i];

		m_vpFloatInterpolations.push_back(pInterpolator);
	}
	m_vpCreateFloatInterpolations.clear();


	// Remove any interpolations that need to be erased
	m_vpFloatInterpolations.erase( remove_if(m_vpFloatInterpolations.begin(), m_vpFloatInterpolations.end(), needs_erasing_float), m_vpFloatInterpolations.end() );

	if(m_paused == false)
	{
		FloatInterpolationList m_vpNextFloatInterpolationsAddList;

		// Update the float interpolations
		for(unsigned int i = 0; i < (int)m_vpFloatInterpolations.size(); i++)
		{
			FloatInterpolation* pInterpolator = m_vpFloatInterpolations[i];
			if(pInterpolator != nullptr)
			{
				FloatInterpolation* pNextInterpolationToAdd = nullptr;

				if(pInterpolator->m_elapsed < pInterpolator->m_time)
				{
					float* lVar = pInterpolator->m_variable;
					if(lVar == nullptr)
					{
						continue;
					}

					float lTimeRatio = pInterpolator->m_elapsed / pInterpolator->m_time;
					float lDiff = (pInterpolator->m_end - pInterpolator->m_start);

					// Create a bezier curve to represent the acceleration/deceleration curve of the animation
					// NOTE : 0 = linear, 100 = full acceleration, -100 = full deceleration.
					float lX = (pInterpolator->m_easing * 0.005f) + 0.5f;
					float lY = 1.0f - lX;

					vec3 lStart = vec3(0.0f, 0.0f, 0.0f);
					vec3 lEnd = vec3(1.0f, 1.0f, 0.0f);
					vec3 lControl = vec3(lX, lY, 0.0f);

					Bezier3 lEaseBezier = Bezier3(lStart, lEnd, lControl);
					vec3 lVectorT = lEaseBezier.GetInterpolatedPoint(lTimeRatio);

					// Get our real T value that we are going to use to interpolate
					float lRealT = lVectorT.y;

					// Set the variable value
					(*lVar) = pInterpolator->m_start + (lDiff * lRealT);

					pInterpolator->m_elapsed += delta;
				}
				else
				{
					float* lVar = pInterpolator->m_variable;
					if(lVar == nullptr)
					{
						continue;
					}

					(*lVar) = pInterpolator->m_end;

					// If we have a callback, do it
					if(pInterpolator->m_Callback != nullptr)
					{
						pInterpolator->m_Callback(pInterpolator->m_pCallbackData);
					}

					// Are we chained to start another float interpolator?
					if(pInterpolator->m_pNextInterpolation != nullptr)
					{
						m_vpNextFloatInterpolationsAddList.push_back(pInterpolator->m_pNextInterpolation);
					}

					// Erase this interpolator since we have finished
					pInterpolator->m_erase = true;
				}
			}

		}

		// Add any chained interpolators to the list
		for(unsigned int j = 0; j < (int)m_vpNextFloatInterpolationsAddList.size(); j++)
		{
			FloatInterpolation* pInterpolator = m_vpNextFloatInterpolationsAddList[j];
			AddFloatInterpolation(pInterpolator);
		}
	}
}

void Interpolator::UpdateIntInterpolators(float delta)
{
	// Add any interpolators in the create list
	for(unsigned int i = 0; i < (int)m_vpCreateIntInterpolations.size(); i++)
	{
		IntInterpolation* pInterpolator = m_vpCreateIntInterpolations[i];

		m_vpIntInterpolations.push_back(pInterpolator);
	}
	m_vpCreateIntInterpolations.clear();


	// Remove any interpolations that need to be erased
	m_vpIntInterpolations.erase( remove_if(m_vpIntInterpolations.begin(), m_vpIntInterpolations.end(), needs_erasing_int), m_vpIntInterpolations.end() );

	if(m_paused == false)
	{
		IntInterpolationList m_vpNextIntInterpolationsAddList;

		// Update the int interpolations
		for(unsigned int i = 0; i < (int)m_vpIntInterpolations.size(); i++)
		{
			IntInterpolation* pInterpolator = m_vpIntInterpolations[i];
			if(pInterpolator != nullptr)
			{
				IntInterpolation* pNextInterpolationToAdd = nullptr;

				if(pInterpolator->m_elapsed < pInterpolator->m_time)
				{
					int* lVar = pInterpolator->m_variable;
					if(lVar == nullptr)
					{
						continue;
					}

					float lTimeRatio = pInterpolator->m_elapsed / pInterpolator->m_time;
					float lDiff = (float)(pInterpolator->m_end - pInterpolator->m_start);

					// Create a bezier curve to represent the acceleration/deceleration curve of the animation
					// NOTE : 0 = linear, 100 = full acceleration, -100 = full deceleration.
					float lX = (pInterpolator->m_easing * 0.005f) + 0.5f;
					float lY = 1.0f - lX;

					vec3 lStart = vec3(0.0f, 0.0f, 0.0f);
					vec3 lEnd = vec3(1.0f, 1.0f, 0.0f);
					vec3 lControl = vec3(lX, lY, 0.0f);

					Bezier3 lEaseBezier = Bezier3(lStart, lEnd, lControl);
					vec3 lVectorT = lEaseBezier.GetInterpolatedPoint(lTimeRatio);

					// Get our real T value that we are going to use to interpolate
					float lRealT = lVectorT.y;

					// Set the variable value
					(*lVar) = (int)(pInterpolator->m_start + (lDiff * lRealT));

					pInterpolator->m_elapsed += delta;
				}
				else
				{
					int* lVar = pInterpolator->m_variable;
					if(lVar == nullptr)
					{
						continue;
					}

					(*lVar) = pInterpolator->m_end;

					// If we have a callback, do it
					if(pInterpolator->m_Callback != nullptr)
					{
						pInterpolator->m_Callback(pInterpolator->m_pCallbackData);
					}

					// Are we chained to start another int interpolator?
					if(pInterpolator->m_pNextInterpolation != nullptr)
					{
						m_vpNextIntInterpolationsAddList.push_back(pInterpolator->m_pNextInterpolation);
					}

					// Erase this interpolator since we have finished
					pInterpolator->m_erase = true;
				}
			}

		}

		// Add any chained interpolators to the list
		for(unsigned int j = 0; j < (int)m_vpNextIntInterpolationsAddList.size(); j++)
		{
			IntInterpolation* pInterpolator = m_vpNextIntInterpolationsAddList[j];
			AddIntInterpolation(pInterpolator);
		}
	}
}
