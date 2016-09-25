// ******************************************************************************
// Filename:    AudioManager.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//	 A singleton instance used as an entry point to play audio.
//   (Music and sound effects)
//
// Revision History:
//   Initial Revision - 04/05/12
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#pragma once

#include "../Maths/3dmaths.h"
#include "fmod/include/fmod.hpp"


class AudioManager
{
public:
	/* Public methods */
	static AudioManager* GetInstance();
	void Destroy();

    void Setup();
    void Shutdown();

	void Update(vec3 listenerPos, vec3 listenerForward, vec3 listenerUp);

    void SetEnableAudio(bool enable);

	void StopAllSounds();

	FMOD::Sound* PlaySound2D(FMOD::Channel** pChannel, const char* filename, bool looping, bool stream = false);
	FMOD::Sound* PlaySound3D(FMOD::Channel** pChannel, const char* filename, vec3 position, bool looping, bool stream = false);

	void StopSound(FMOD::Channel* pChannel);

protected:
	/* Protected methods */
	AudioManager();
	AudioManager(const AudioManager&);
	AudioManager &operator=(const AudioManager&);

private:
	/* Private methods */

public:
	/* Public members */

protected:
	/* Protected members */

private:
	/* Private members */
	FMOD::System *m_FMODSystem;

	FMOD_RESULT m_FMODResult;
	unsigned int m_FMODVersion;

    bool m_audioEnabled;

	// Singleton instance
	static AudioManager *c_instance;
};
