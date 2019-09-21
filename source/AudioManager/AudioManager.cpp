// ******************************************************************************
// Filename:    AudioManager.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 04/05/12
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "AudioManager.h"
#include <iostream>
#include "fmod/include/fmod_errors.h"

// Initialize the singleton instance
AudioManager *AudioManager::c_instance = 0;

void ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		std::cout << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
		//printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
}

AudioManager* AudioManager::GetInstance()
{
	if(c_instance == 0)
	{
		c_instance = new AudioManager;

		c_instance->Setup();
	}

	return c_instance;
}

void AudioManager::Destroy()
{
	if(c_instance)
	{
				Shutdown();

		delete c_instance;
	}
}

void AudioManager::Setup()
{
	m_FMODResult = FMOD::System_Create(&m_FMODSystem);
	ERRCHECK(m_FMODResult);

	m_FMODResult = m_FMODSystem->getVersion(&m_FMODVersion);
	ERRCHECK(m_FMODResult);

	if (m_FMODVersion < FMOD_VERSION)
	{
		std::cout << "Error!  You are using an old version of FMOD " << m_FMODVersion <<".  This program requires " << FMOD_VERSION << std::endl;
		//printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		return;
	}

	m_FMODResult = m_FMODSystem->init(32, FMOD_INIT_NORMAL, 0);
	ERRCHECK(m_FMODResult);

	float distanceFactor = 1.0f;
	m_FMODResult = m_FMODSystem->set3DSettings(1.0f, distanceFactor, 1.0f);
	ERRCHECK(m_FMODResult);
}

void AudioManager::Shutdown()
{
	m_FMODResult = m_FMODSystem->close();
	ERRCHECK(m_FMODResult);

	m_FMODResult = m_FMODSystem->release();
	ERRCHECK(m_FMODResult);
}

AudioManager::AudioManager()
{
		SetEnableAudio(true);
}

void AudioManager::Update(vec3 listenerPos, vec3 listenerForward, vec3 listenerUp)
{
	FMOD_VECTOR listenerpos = { listenerPos.x, listenerPos.y, listenerPos.z };
	FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
	FMOD_VECTOR forward = { listenerForward.x, listenerForward.y, listenerForward.z };
	FMOD_VECTOR up = { listenerUp.x, listenerUp.y, listenerUp.z };

	m_FMODResult = m_FMODSystem->set3DListenerAttributes(0, &listenerpos, &vel, &forward, &up);
	ERRCHECK(m_FMODResult);

	m_FMODResult = m_FMODSystem->update();
	ERRCHECK(m_FMODResult);
}

void AudioManager::SetEnableAudio(bool enable)
{
		m_audioEnabled = enable;
}

void AudioManager::StopAllSounds()
{
	if(m_audioEnabled)
	{
	}
}

FMOD::Sound* AudioManager::PlaySound2D(FMOD::Channel** pChannel, const char* filename, bool looping, bool stream)
{
	if (m_audioEnabled)
	{
		FMOD::Sound* sound;
		*pChannel = nullptr;
		FMOD::ChannelGroup *channelGroup = nullptr;

		if (stream)
		{
			m_FMODResult = m_FMODSystem->createStream(filename, FMOD_DEFAULT | FMOD_2D, 0, &sound);
		}
		else
		{
			m_FMODResult = m_FMODSystem->createSound(filename, FMOD_DEFAULT | FMOD_2D, 0, &sound);
		}
		ERRCHECK(m_FMODResult);

		m_FMODResult = sound->setMode(looping == true ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
		ERRCHECK(m_FMODResult);

		m_FMODResult = m_FMODSystem->playSound(sound, channelGroup, false, pChannel);
		ERRCHECK(m_FMODResult);

		return sound;
	}

	return nullptr;
}

FMOD::Sound* AudioManager::PlaySound3D(FMOD::Channel** pChannel, const char* filename, vec3 position, bool looping, bool stream)
{
	if (m_audioEnabled)
	{
		FMOD::Sound* sound;
		*pChannel = nullptr;
		FMOD::ChannelGroup *channelGroup = nullptr;

		if (stream)
		{
			m_FMODResult = m_FMODSystem->createStream(filename, FMOD_DEFAULT | FMOD_3D, 0, &sound);
		}
		else
		{
			m_FMODResult = m_FMODSystem->createSound(filename, FMOD_DEFAULT | FMOD_3D, 0, &sound);
		}
		ERRCHECK(m_FMODResult);

		float distanceFactor = 1.0f;
		m_FMODResult = sound->set3DMinMaxDistance(1.0f * distanceFactor, 300.0f * distanceFactor);
		ERRCHECK(m_FMODResult);

		m_FMODResult = sound->setMode(looping == true ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
		ERRCHECK(m_FMODResult);

		m_FMODResult = m_FMODSystem->playSound(sound, channelGroup, false, pChannel);
		ERRCHECK(m_FMODResult);

		FMOD_VECTOR pos = { position.x, position.y, position.z };
		FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };

		m_FMODResult = (*pChannel)->set3DAttributes(&pos, &vel);
		ERRCHECK(m_FMODResult);

		return sound;
	}

	return nullptr;
}

void AudioManager::StopSound(FMOD::Channel* pChannel)
{
	if (m_audioEnabled)
	{
		pChannel->stop();
	}
}