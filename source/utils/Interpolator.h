// ******************************************************************************
// Filename:    Interpolator.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//	 An interpolator helper class that will manage all the interpolations for
//   your variables.
//
// Revision History:
//   Initial Revision - 23/02/12
//
// Copyright (c) 2005-20016, Steven Ball
// ******************************************************************************

#pragma once

#include <vector>
#include <stddef.h>


typedef void(*FunctionCallback)(void *lpData);

class FloatInterpolation
{
public:
	float *m_variable;

	float m_start;
	float m_end;
	float m_time;

	float m_easing;

	float m_elapsed;

	bool m_erase;

	FloatInterpolation* m_pNextInterpolation;

	FunctionCallback m_Callback;
	void *m_pCallbackData;
};

class IntInterpolation
{
public:
	int *m_variable;

	int m_start;
	int m_end;
	float m_time;

	float m_easing;

	float m_elapsed;

	bool m_erase;

	IntInterpolation* m_pNextInterpolation;

	FunctionCallback m_Callback;
	void *m_pCallbackData;
};

typedef std::vector<FloatInterpolation*> FloatInterpolationList;
typedef std::vector<IntInterpolation*> IntInterpolationList;


class Interpolator
{
public:
	/* Public methods */
	static Interpolator* GetInstance();
	void Destroy();

	void ClearInterpolators();

	FloatInterpolation* CreateFloatInterpolation(float *val, float start, float end, float time, float easing, FloatInterpolation* aNext = nullptr, FunctionCallback aCallback = nullptr, void *aData = nullptr);
	void LinkFloatInterpolation(FloatInterpolation* aFirst, FloatInterpolation* aSecond);
	void AddFloatInterpolation(FloatInterpolation* aInterpolation);
	void AddFloatInterpolation(float *val, float start, float end, float time, float easing, FloatInterpolation* aNext = nullptr, FunctionCallback aCallback = nullptr, void *aData = nullptr);
	void RemoveFloatInterpolationByVariable(float *val);

	IntInterpolation* CreateIntInterpolation(int *val, int start, int end, float time, float easing, IntInterpolation* aNext = nullptr, FunctionCallback aCallback = nullptr, void *aData = nullptr);
	void LinkIntInterpolation(IntInterpolation* aFirst, IntInterpolation* aSecond);
	void AddIntInterpolation(IntInterpolation* aInterpolation);
	void AddIntInterpolation(int *val, int start, int end, float time, float easing, IntInterpolation* aNext = nullptr, FunctionCallback aCallback = nullptr, void *aData = nullptr);
	void RemoveIntInterpolationByVariable(int *val);

	void SetPaused(bool pause);
	bool IsPaused();

	void Update(float dt);
	void UpdateFloatInterpolators(float delta);
	void UpdateIntInterpolators(float delta);

protected:
	/* Protected methods */
	Interpolator();
	Interpolator(const Interpolator&);
	Interpolator &operator=(const Interpolator&);

private:
	/* Private methods */
	void RemoveCreateFloatInterpolation(FloatInterpolation* aInterpolation);
	void RemoveFloatInterpolation(FloatInterpolation* aInterpolation);

	void RemoveCreateIntInterpolation(IntInterpolation* aInterpolation);
	void RemoveIntInterpolation(IntInterpolation* aInterpolation);

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */

	// A dynamic array of our float interpolation variables
	FloatInterpolationList m_vpFloatInterpolations;
	FloatInterpolationList m_vpCreateFloatInterpolations;

	// A dynamic array of our int interpolation variables
	IntInterpolationList m_vpIntInterpolations;
	IntInterpolationList m_vpCreateIntInterpolations;

	// Singleton instance
	static Interpolator *c_instance;

	// Flag to control if we are paused or not
	bool m_paused;
};
