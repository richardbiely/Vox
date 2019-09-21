// ******************************************************************************
// Filename:    CountdownTimer.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 15/07/08
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "CountdownTimer.h"
#include "TimeManager.h"


CountdownTimer::CountdownTimer()
{
	TimeManager::GetInstance()->AddCountdownTimer(this);

	m_elapsedTime = 0.0f;
	m_timeOutTime = 0.0f;

	m_Callback = nullptr;
	m_pCallbackData = nullptr;

	m_bStarted = false;

	m_bPaused = true;

	m_bFinished = false;
}

CountdownTimer::~CountdownTimer()
{
	//TimeManager::GetInstance()->RemoveCountdownTimer(this);
}

void CountdownTimer::SetCallBackFunction(FunctionCallback lFunction)
{
	m_Callback = lFunction;
}

void CountdownTimer::SetCallBackData(void *lpData)
{
	m_pCallbackData = lpData;
}

void CountdownTimer::StartCountdown()
{
	m_elapsedTime = 0.0f;

	m_bPaused = false;

	m_bFinished = false;

	m_bStarted = true;
}

void CountdownTimer::ResetCountdown()
{
	m_elapsedTime = 0.0f;

	m_bFinished = false;
}

void CountdownTimer::PauseCountdown()
{
	m_bPaused = true;
}

void CountdownTimer::ResumeCountdown()
{
	m_bPaused = false;
}

bool CountdownTimer::IsPaused() const
{
	return m_bPaused;
}

float CountdownTimer::GetElapsedTime() const
{
	return m_elapsedTime;
}

float CountdownTimer::GetRemainingTime() const
{
	return (m_timeOutTime - m_elapsedTime);
}

void CountdownTimer::SetCountdownTime(float lTimeOut)
{
	m_timeOutTime = lTimeOut;
}

void CountdownTimer::SetLooping(bool lLoop)
{
	m_bLooping = lLoop;
}

void CountdownTimer::UpdateCountdown(float dt)
{
	if(!m_bStarted)
	{
		return;
	}

	if(!m_bPaused)
	{
		m_elapsedTime += dt;
	}

	if(m_elapsedTime >= m_timeOutTime)
	{
		if(!m_bFinished)
		{
			// We have reached our countdown time, call our function callback
			if(m_Callback)
			{
				m_Callback(m_pCallbackData);
			}

			if(m_bLooping)
			{
				// If we are a looping timer, then just reset the elapsed time
				m_elapsedTime = 0.0f;
			}
			else
			{
				// We are not looping, so set our finished flag
				m_bFinished = true;
			}
		}
	}
}
