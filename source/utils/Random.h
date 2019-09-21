// ******************************************************************************
// Filename:    Random.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//	 A selection of helper functions to make generating random numbers easier.
//
// Revision History:
//   Initial Revision - 20/02/11
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include <time.h>
#include <math.h>

inline void SeedRandomNumberGenerator()
{
	srand ( (unsigned int)time(nullptr) );
}

inline void SeedRandomNumberGeneratorInt(int seed)
{
	srand ( seed );
}

// Get a random integer number in the range from lower to higher. INCLUSIVE
inline int GetRandomNumber(int lower, int higher)
{
	if(lower > higher)
	{
		int temp = lower;
		lower = higher;
		higher = temp;
	}
	int diff = (higher+1) - lower;
	return (rand() % diff + lower);
}

// Get a random floating point number in the range from lower to higher. INCLUSIVE
// Precision defines how many significant numbers there are after the point
inline float GetRandomNumber(int lower, int higher, int precision)
{
	float lPrecisionPow = pow(10.0f, precision);
	float lRand = (float)GetRandomNumber((int)(lower * lPrecisionPow), (int)(higher * lPrecisionPow));

	return (lRand / lPrecisionPow);
}
