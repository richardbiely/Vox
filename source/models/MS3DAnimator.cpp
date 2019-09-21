#include "MS3DAnimator.h"

#include <assert.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
using namespace std;


MS3DAnimator::MS3DAnimator(Renderer *lpRenderer, MS3DModel* pModel)
{
	mpRenderer = lpRenderer;

	mpModel = pModel;

	numJointAnimations = 0;
	pJointAnimations = NULL;

	numAnimations = 0;
	pAnimations = NULL;

	// Once we have some model data, create out joint animations
	CreateJointAnimations();

	// Calculate the initial bounding box
	CalculateBoundingBox();

	mCurrentAnimationIndex = 0;
	mCurrentAnimationStartTime = 0.0;
	mCurrentAnimationEndTime = 0.0;

	mCurrentAnimationRightWeaponTrailStartTime = 0.0;
	mCurrentAnimationRightWeaponTrailEndTime = 0.0;
	mCurrentAnimationLeftWeaponTrailStartTime = 0.0;
	mCurrentAnimationLeftWeaponTrailEndTime = 0.0;

	m_timer = 0;
	mblooping = false;

	m_bPaused = false;
	m_bLooped = false;
	m_bFinished = false;

	m_bBlending = false;
	m_blendTime = 0.0f;
	m_blendTimer = 0.0f;
	m_blendEndAnimationIndex = 0;
}

MS3DAnimator::~MS3DAnimator()
{
	numJointAnimations = 0;
	if(pJointAnimations != NULL)
	{
		delete[] pJointAnimations;
		pJointAnimations = NULL;
	}

	numAnimations = 0;
	if(pAnimations != NULL)
	{
		delete[] pAnimations;
		pAnimations = NULL;
	}
}

MS3DModel* MS3DAnimator::GetModel()
{
	return mpModel;
}

void MS3DAnimator::CreateJointAnimations()
{
	numJointAnimations = mpModel->numJoints;
	pJointAnimations = new JointAnimation[numJointAnimations];

	for(int i = 0; i < numJointAnimations; i++)
	{
		pJointAnimations[i].currentTranslationKeyframe = 0;
		pJointAnimations[i].currentRotationKeyframe = 0;

		pJointAnimations[i].final = mpModel->pJoints[i].absolute;
	}
}

bool MS3DAnimator::LoadAnimations(const char *animationFileName)
{
	ifstream file;

	// Open the file
	file.open(animationFileName, ios::in);
	if(file.is_open())
	{
		string tempString;

		// Read in the number of animations
		file >> tempString >> numAnimations;

		// Create the animation storage space
		pAnimations = new Animation[numAnimations];

		// Read in each animation
		for(int i = 0; i < numAnimations; i++)
		{
			// Animation name
			file >> tempString  >> pAnimations[i].animationName;

			// Looping
			file >> tempString  >> pAnimations[i].looping;

			// Start frame
			file >> tempString  >> pAnimations[i].startFrame;

			// End frame
			file >> tempString  >> pAnimations[i].endFrame;

			// Blend frame
			file >> tempString  >> pAnimations[i].blendFrame;

			// Start right weapon trail
			file >> tempString >> pAnimations[i].startRightWeaponTrailFrame;

			// End right weapon trail
			file >> tempString >> pAnimations[i].endRightWeaponTrailFrame;

			// Start left weapon trail
			file >> tempString >> pAnimations[i].startLeftWeaponTrailFrame;

			// End left weapon trail
			file >> tempString >> pAnimations[i].endLeftWeaponTrailFrame;

			// Work out the start time and end time
			pAnimations[i].startTime = pAnimations[i].startFrame * 1000.0f / mpModel->mAnimationFPS;
			pAnimations[i].endTime = pAnimations[i].endFrame * 1000.0f / mpModel->mAnimationFPS;
			pAnimations[i].startRightWeaponTrailTime = pAnimations[i].startRightWeaponTrailFrame * 1000.0f / mpModel->mAnimationFPS;
			pAnimations[i].endRightWeaponTrailTime = pAnimations[i].endRightWeaponTrailFrame * 1000.0f / mpModel->mAnimationFPS;
			pAnimations[i].startLeftWeaponTrailTime = pAnimations[i].startLeftWeaponTrailFrame * 1000.0f / mpModel->mAnimationFPS;
			pAnimations[i].endLeftWeaponTrailTime = pAnimations[i].endLeftWeaponTrailFrame * 1000.0f / mpModel->mAnimationFPS;
		}

		// Close the file
		file.close();

		return true;
	}

	return false;
}

void MS3DAnimator::CalculateBoundingBox()
{
	for(int i = 0; i < mpModel->numVertices; i++)
	{
		if(mpModel->pVertices[i].boneID == -1)
		{
			// Don't use this vertex if it doesnt have a valid boneID, since this will mess up the matrix math
			continue;
		}

		Matrix4x4& final = pJointAnimations[mpModel->pVertices[i].boneID].final;
		vec3 newVertex( mpModel->pVertices[i].location[0], mpModel->pVertices[i].location[1], mpModel->pVertices[i].location[2]);

		newVertex = final * newVertex;

		float tempVertex[3];
		tempVertex[0] = newVertex.x;
		tempVertex[1] = newVertex.y;
		tempVertex[2] = newVertex.z;

		// Always set the first vertex values
		if(i == 0)
		{
			m_BoundingBox.mMinX = tempVertex[0];
			m_BoundingBox.mMinY = tempVertex[1];
			m_BoundingBox.mMinZ = tempVertex[2];

			m_BoundingBox.mMaxX = tempVertex[0];
			m_BoundingBox.mMaxY = tempVertex[1];
			m_BoundingBox.mMaxZ = tempVertex[2];
		}
		else
		{
			if(tempVertex[0] < m_BoundingBox.mMinX)
			{
				m_BoundingBox.mMinX = tempVertex[0];
			}

			if(tempVertex[1] < m_BoundingBox.mMinY)
			{
				m_BoundingBox.mMinY = tempVertex[1];
			}

			if(tempVertex[2] < m_BoundingBox.mMinZ)
			{
				m_BoundingBox.mMinZ = tempVertex[2];
			}

			if(tempVertex[0] > m_BoundingBox.mMaxX)
			{
				m_BoundingBox.mMaxX = tempVertex[0];
			}

			if(tempVertex[1] > m_BoundingBox.mMaxY)
			{
				m_BoundingBox.mMaxY = tempVertex[1];
			}

			if(tempVertex[2] > m_BoundingBox.mMaxZ)
			{
				m_BoundingBox.mMaxZ = tempVertex[2];
			}
		}
	}
}

BoundingBox* MS3DAnimator::GetBoundingBox()
{
	return &m_BoundingBox;
}

void MS3DAnimator::PlayAnimation(int lAnimationIndex)
{
	assert(lAnimationIndex >= 0 && lAnimationIndex < numAnimations);

	// Keep track of the current animation index
	mCurrentAnimationIndex = lAnimationIndex;

	// Set the animation params
	mCurrentAnimationStartTime = pAnimations[lAnimationIndex].startTime;
	mCurrentAnimationEndTime = pAnimations[lAnimationIndex].endTime;
	mblooping = pAnimations[lAnimationIndex].looping;

	// Set the weapon trail start and end params
	mCurrentAnimationRightWeaponTrailStartTime = pAnimations[lAnimationIndex].startRightWeaponTrailTime;
	mCurrentAnimationRightWeaponTrailEndTime = pAnimations[lAnimationIndex].endRightWeaponTrailTime;
	mCurrentAnimationLeftWeaponTrailStartTime = pAnimations[lAnimationIndex].startLeftWeaponTrailTime;
	mCurrentAnimationLeftWeaponTrailEndTime = pAnimations[lAnimationIndex].endLeftWeaponTrailTime;

	// Reset the timer to the start of the animation
	Restart();

	m_bPaused = false;
	m_bLooped = false;
	m_bFinished = false;
	m_bBlending = false;
}

void MS3DAnimator::PlayAnimation(const char *lAnimationName)
{
	int lAnimationIndex = -1;
	for(int i = 0; i < numAnimations; i++)
	{
#ifdef _WIN32
		if (_strcmpi(lAnimationName, pAnimations[i].animationName) == 0)
#else
		if (strcasecmp(lAnimationName, pAnimations[i].animationName) == 0)
#endif //_WIN32
		{
			lAnimationIndex = i;

			break;
		}
	}

	if(lAnimationIndex >= 0 && lAnimationIndex < numAnimations)
	{
		PlayAnimation(lAnimationIndex);
	}
}

void MS3DAnimator::PauseAnimation()
{
	m_bPaused = true;
}

void MS3DAnimator::ResumeAnimation()
{
	m_bPaused = false;
}

int MS3DAnimator::GetCurrentAnimationIndex() const
{
	return mCurrentAnimationIndex;
}

int MS3DAnimator::GetNumAnimations() const
{
	return numAnimations;
}

const char *MS3DAnimator::GetAnimationName(const int index)
{
	if(index < 0 || index >= numAnimations)
	{
		return "";
	}

	return pAnimations[index].animationName;
}

bool MS3DAnimator::IsAnimationPaused() const
{
	return m_bPaused;
}

bool MS3DAnimator::HasAnimationLooped() const
{
	return m_bLooped;
}

bool MS3DAnimator::HasAnimationFinished() const
{
	return m_bFinished;
}

int MS3DAnimator::GetStartFrame(const char *lAnimationName)
{
	int lAnimationIndex = -1;
	for(int i = 0; i < numAnimations; i++)
	{
#ifdef _WIN32
		if (_strcmpi(lAnimationName, pAnimations[i].animationName) == 0)
#else
		if (strcasecmp(lAnimationName, pAnimations[i].animationName) == 0)
#endif //_WIN32
		{
			return pAnimations[i].startFrame;
		}
	}

	return -1;
}

int MS3DAnimator::GetEndFrame(const char *lAnimationName)
{
	int lAnimationIndex = -1;
	for(int i = 0; i < numAnimations; i++)
	{
#ifdef _WIN32
		if (_strcmpi(lAnimationName, pAnimations[i].animationName) == 0)
#else
		if (strcasecmp(lAnimationName, pAnimations[i].animationName) == 0)
#endif //_WIN32
		{
			return pAnimations[i].endFrame;
		}
	}

	return -1;
}

int MS3DAnimator::GetCurrentFrame()
{
	Joint *pJoint = &(mpModel->pJoints[0]);
	JointAnimation *pJointAnimation = &(pJointAnimations[0]);

	int frame = pJointAnimation->currentTranslationKeyframe;
	while ( frame < pJoint->numTranslationKeyframes && pJoint->pTranslationKeyframes[frame].time <= m_timer )
	{
		frame++;
	}

	return frame;
}

float MS3DAnimator::GetAnimationTimer()
{
	return (float)m_timer;
}

int MS3DAnimator::GetNumKeyframes()
{
	Joint *pJoint = &(mpModel->pJoints[0]);
	int numFrames = pJoint->numTranslationKeyframes;

	return numFrames;
}

void MS3DAnimator::StartBlendAnimation(int startIndex, int endIndex, float blendTime)
{
	m_bBlending = true;
	m_bPaused = false;
	m_bLooped = false;
	m_bFinished = false;
	m_blendTime = blendTime;
	m_blendTimer = 0.0f;
	m_blendStartAnimationIndex = startIndex;
	m_blendEndAnimationIndex = endIndex;

	double startTime = 0.0;

	if(startIndex != -1)
	{
		startTime = pAnimations[startIndex].startTime;
	}

	for ( int i = 0; i < mpModel->numJoints; i++ )
	{
		int frame;
		Joint *pJoint = &(mpModel->pJoints[i]);
		JointAnimation *pJointAnimation = &(pJointAnimations[i]);

		if ( pJoint->numRotationKeyframes == 0 && pJoint->numTranslationKeyframes == 0 )
		{
			pJointAnimation->startBlendTrans[0] = pJoint->localTranslation[0];
			pJointAnimation->startBlendTrans[1] = pJoint->localTranslation[1];
			pJointAnimation->startBlendTrans[2] = pJoint->localTranslation[2];
			pJointAnimation->endBlendTrans[0] = pJoint->localTranslation[0];
			pJointAnimation->endBlendTrans[1] = pJoint->localTranslation[1];
			pJointAnimation->endBlendTrans[2] = pJoint->localTranslation[2];
			pJointAnimation->startBlendRot[0] = pJoint->localRotation[0];
			pJointAnimation->startBlendRot[1] = pJoint->localRotation[1];
			pJointAnimation->startBlendRot[2] = pJoint->localRotation[2];
			pJointAnimation->endBlendRot[0] = pJoint->localRotation[0];
			pJointAnimation->endBlendRot[1] = pJoint->localRotation[1];
			pJointAnimation->endBlendRot[2] = pJoint->localRotation[2];

			continue;
		}

		// Translation
		// Start
		if(startIndex == -1)
		{
			// We don't want to set the start trans values, since we will use the stored values of the current animation state
			pJointAnimation->startBlendTrans[0] = pJointAnimation->currentBlendTrans[0];
			pJointAnimation->startBlendTrans[1] = pJointAnimation->currentBlendTrans[1];
			pJointAnimation->startBlendTrans[2] = pJointAnimation->currentBlendTrans[2];
		}
		else
		{
			frame = 1;
			while ( frame < pJoint->numTranslationKeyframes && pJoint->pTranslationKeyframes[frame].time < startTime )
			{
				frame++;
			}
			if (frame == pJoint->numTranslationKeyframes)
			{
				frame--;
			}

			if(pJoint->numTranslationKeyframes == 0)
			{
				pJointAnimation->startBlendTrans[0] = 0.0f;
				pJointAnimation->startBlendTrans[1] = 0.0f;
				pJointAnimation->startBlendTrans[2] = 0.0f;
			}
			else
			{
				memcpy(pJointAnimation->startBlendTrans, pJoint->pTranslationKeyframes[frame-1].parameter, sizeof ( float )*3);
			}
		}

		// End
		frame = pAnimations[endIndex].blendFrame;

		if(pJoint->numTranslationKeyframes == 0)
		{
			pJointAnimation->endBlendTrans[0] = 0.0f;
			pJointAnimation->endBlendTrans[1] = 0.0f;
			pJointAnimation->endBlendTrans[2] = 0.0f;
		}
		else
		{
			memcpy(pJointAnimation->endBlendTrans, pJoint->pTranslationKeyframes[frame].parameter, sizeof ( float )*3);
		}

		// Rotation
		// Start
		if(startIndex == -1)
		{
			// We don't want to set the start rot values, since we will use the stored values of the current animation state
			pJointAnimation->startBlendRot[0] = pJointAnimation->currentBlendRot[0];
			pJointAnimation->startBlendRot[1] = pJointAnimation->currentBlendRot[1];
			pJointAnimation->startBlendRot[2] = pJointAnimation->currentBlendRot[2];
		}
		else
		{
			frame = 1;
			while ( frame < pJoint->numRotationKeyframes && pJoint->pRotationKeyframes[frame].time < startTime )
			{
				frame++;
			}
			if (frame == pJoint->numRotationKeyframes)
			{
				frame--;
			}

			if(pJoint->numRotationKeyframes == 0)
			{
				pJointAnimation->startBlendRot[0] = 0.0f;
				pJointAnimation->startBlendRot[1] = 0.0f;
				pJointAnimation->startBlendRot[2] = 0.0f;
			}
			else
			{
				memcpy(pJointAnimation->startBlendRot, pJoint->pRotationKeyframes[frame-1].parameter, sizeof ( float )*3);
			}
		}


		// End
		frame = pAnimations[endIndex].blendFrame;

		if(pJoint->numRotationKeyframes == 0)
		{
			pJointAnimation->endBlendRot[0] = 0.0f;
			pJointAnimation->endBlendRot[1] = 0.0f;
			pJointAnimation->endBlendRot[2] = 0.0f;
		}
		else
		{
			memcpy(pJointAnimation->endBlendRot, pJoint->pRotationKeyframes[frame].parameter, sizeof ( float )*3);
		}
	}
}

void MS3DAnimator::StartBlendAnimation(const char *lStartAnimationName, const char *lEndAnimationName, float blendTime)
{
	int lStartIndex = -1;
	int lEndIndex = -1;
	for(int i = 0; i < numAnimations; i++)
	{
#ifdef _WIN32
		if (_strcmpi(lStartAnimationName, pAnimations[i].animationName) == 0)
#else
		if (strcasecmp(lStartAnimationName, pAnimations[i].animationName) == 0)
#endif //_WIN32
		{
			lStartIndex = i;
		}

#ifdef _WIN32
		if (_strcmpi(lEndAnimationName, pAnimations[i].animationName) == 0)
#else
		if (strcasecmp(lEndAnimationName, pAnimations[i].animationName) == 0)
#endif //_WIN32
		{
			lEndIndex = i;
		}
	}

	if(lStartIndex >= 0 && lStartIndex < numAnimations && lEndIndex >= 0 && lEndIndex < numAnimations)
	{
		StartBlendAnimation(lStartIndex, lEndIndex, blendTime);
	}
}

void MS3DAnimator::BlendIntoAnimation(const char *lAnimationName, float blendTime)
{
	int lIndex = -1;
	for(int i = 0; i < numAnimations; i++)
	{
#ifdef _WIN32
		if (_strcmpi(lAnimationName, pAnimations[i].animationName) == 0)
#else
		if (strcasecmp(lAnimationName, pAnimations[i].animationName) == 0)
#endif //_WIN32
		{
			lIndex = i;
			break;
		}
	}

	if(lIndex >= 0 && lIndex < numAnimations)
	{
		StartBlendAnimation(-1, lIndex, blendTime);
	}
}


void MS3DAnimator::GetCurrentBlendTranslation(int jointIndex, float* x, float* y, float* z)
{
	JointAnimation *pJointAnimation = &(pJointAnimations[jointIndex]);
	*x = pJointAnimation->currentBlendTrans[0];
	*y = pJointAnimation->currentBlendTrans[1];
	*z = pJointAnimation->currentBlendTrans[2];
}

void MS3DAnimator::GetCurrentBlendRotation(int jointIndex, float* x, float* y, float* z)
{
	JointAnimation *pJointAnimation = &(pJointAnimations[jointIndex]);
	*x = pJointAnimation->currentBlendRot[0];
	*y = pJointAnimation->currentBlendRot[1];
	*z = pJointAnimation->currentBlendRot[2];
}

bool MS3DAnimator::GetRightWeaponTrailActive()
{
	return (m_timer > mCurrentAnimationRightWeaponTrailStartTime) && (m_timer < mCurrentAnimationRightWeaponTrailEndTime);
}

bool MS3DAnimator::GetLeftWeaponTrailActive()
{
	return (m_timer > mCurrentAnimationLeftWeaponTrailStartTime) && (m_timer < mCurrentAnimationLeftWeaponTrailEndTime);
}

void MS3DAnimator::Restart()
{
	for ( int i = 0; i < numJointAnimations; i++ )
	{
		pJointAnimations[i].currentRotationKeyframe = pJointAnimations[i].currentTranslationKeyframe = 0;

		//pJointAnimations[i].final = mpModel->pJoints[i].absolute;
	}

	SetTimerForStartOfAnimation();
}

void MS3DAnimator::SetTimerForStartOfAnimation()
{
	m_timer = pAnimations[mCurrentAnimationIndex].startTime;
}

Matrix4x4 MS3DAnimator::GetBoneMatrix(int index)
{
	Matrix4x4& final = pJointAnimations[index].final;

	return final;
}

// Update
void MS3DAnimator::Update(float dt)
{
	if(m_bBlending)
	{
		UpdateBlending(dt);
		return;
	}

	if(!m_bPaused)
	{
		m_timer += dt * 1000.0;
	}

	if ( m_timer > mCurrentAnimationEndTime )
	{
		if ( mblooping )
		{
			Restart();
			m_timer = mCurrentAnimationStartTime;

			m_bLooped = true;
		}
		else
		{
			m_timer = mCurrentAnimationEndTime;

			m_bFinished = true;
		}
	}

	for ( int i = 0; i < mpModel->numJoints; i++ )
	{
		float transVec[3];
		float rotVec[3];
		Matrix4x4 transform;
		int frame;
		Joint *pJoint = &(mpModel->pJoints[i]);
		JointAnimation *pJointAnimation = &(pJointAnimations[i]);

		if ( pJoint->numRotationKeyframes == 0 && pJoint->numTranslationKeyframes == 0 )
		{
			pJointAnimation->final = pJoint->absolute;

			continue;
		}

		// Translation
		frame = pJointAnimation->currentTranslationKeyframe;
		while ( frame < pJoint->numTranslationKeyframes && pJoint->pTranslationKeyframes[frame].time < m_timer )
		{
			frame++;
		}
		pJointAnimation->currentTranslationKeyframe = frame;

		if(pJoint->numTranslationKeyframes == 0)
		{
			transVec[0] = 0.0f;
			transVec[1] = 0.0f;
			transVec[2] = 0.0f;
		}
		else
		{
			if ( frame == 0 )
			{
				memcpy( transVec, pJoint->pTranslationKeyframes[0].parameter, sizeof ( float )*3 );
			}
			else if ( frame == pJoint->numTranslationKeyframes )
			{
				memcpy( transVec, pJoint->pTranslationKeyframes[frame-1].parameter, sizeof ( float )*3 );
			}
			else
			{
				const Keyframe& curFrame = pJoint->pTranslationKeyframes[frame];
				const Keyframe& prevFrame = pJoint->pTranslationKeyframes[frame-1];

				float timeDelta = curFrame.time-prevFrame.time;
				float interpValue = ( float )(( m_timer-prevFrame.time )/timeDelta );

				transVec[0] = prevFrame.parameter[0]+( curFrame.parameter[0]-prevFrame.parameter[0] )*interpValue;
				transVec[1] = prevFrame.parameter[1]+( curFrame.parameter[1]-prevFrame.parameter[1] )*interpValue;
				transVec[2] = prevFrame.parameter[2]+( curFrame.parameter[2]-prevFrame.parameter[2] )*interpValue; 
			}
		}

		// Rotation
		frame = pJointAnimation->currentRotationKeyframe;
		while ( frame < pJoint->numRotationKeyframes && pJoint->pRotationKeyframes[frame].time < m_timer )
		{
			frame++;
		}
		pJointAnimation->currentRotationKeyframe = frame;

		if(pJoint->numRotationKeyframes == 0)
		{
			// Do nothing
		}
		else
		{
			if ( frame == 0 )
			{
				rotVec[0] = pJoint->pRotationKeyframes[0].parameter[0];
				rotVec[1] = pJoint->pRotationKeyframes[0].parameter[1];
				rotVec[2] = pJoint->pRotationKeyframes[0].parameter[2];

				transform.SetRotationRadians( rotVec );
			}
			else if ( frame == pJoint->numRotationKeyframes )
			{
				rotVec[0] = pJoint->pRotationKeyframes[frame-1].parameter[0];
				rotVec[1] = pJoint->pRotationKeyframes[frame-1].parameter[1];
				rotVec[2] = pJoint->pRotationKeyframes[frame-1].parameter[2];

				transform.SetRotationRadians( rotVec );
			}
			else
			{
				const Keyframe& curFrame = pJoint->pRotationKeyframes[frame];
				const Keyframe& prevFrame = pJoint->pRotationKeyframes[frame-1];

				float timeDelta = curFrame.time-prevFrame.time;
				float interpValue = ( float )(( m_timer-prevFrame.time )/timeDelta );

				quat q1(vec3(prevFrame.parameter[0], prevFrame.parameter[1], prevFrame.parameter[2]));
				quat q2(vec3(curFrame.parameter[0], curFrame.parameter[1], curFrame.parameter[2]));
				quat q3 = slerp(q1, q2, interpValue);
				
				mat4 trans = mat4_cast(q3);
				float *pM = value_ptr(trans);
				transform.SetValues(pM);

				// To preserve blending, since the matrix-to-angles functionality is broken
				rotVec[0] = prevFrame.parameter[0]+( curFrame.parameter[0]-prevFrame.parameter[0] )*interpValue;
				rotVec[1] = prevFrame.parameter[1]+( curFrame.parameter[1]-prevFrame.parameter[1] )*interpValue;
				rotVec[2] = prevFrame.parameter[2]+( curFrame.parameter[2]-prevFrame.parameter[2] )*interpValue;
			}
		}
		
		// Combine and create the final animation matrix
		transform.SetTranslation( transVec );

		Matrix4x4 relativeFinal( pJoint->relative );
		relativeFinal.PostMultiply( transform );

		if ( pJoint->parent == -1 )
		{
			pJointAnimation->final = relativeFinal;
		}
		else
		{
			pJointAnimation->final = pJointAnimations[pJoint->parent].final;

			pJointAnimation->final.PostMultiply( relativeFinal );
		}

		// Also store the current trans and rot values in the start blend variables, in case we want to start a new blend.
		const vec3& v = transform.GetTranslationVector();
		pJointAnimation->currentBlendTrans[0] =v.x;
		pJointAnimation->currentBlendTrans[1] =v.y;
		pJointAnimation->currentBlendTrans[2] =v.z;
		pJointAnimation->currentBlendRot[0] = rotVec[0];
		pJointAnimation->currentBlendRot[1] = rotVec[1];
		pJointAnimation->currentBlendRot[2] = rotVec[2];
	}

	// Also re-calculate the bounding box, since vertices *might* now have new positions, given that we have updated all the bones!
	CalculateBoundingBox();
}

void MS3DAnimator::UpdateBlending(float dt)
{
	if(!m_bPaused)
	{
		m_blendTimer += dt;
	}

	if (m_blendTimer > m_blendTime)
	{
		// Finished blending
		m_bBlending = false;
		PlayAnimation(m_blendEndAnimationIndex);
	}

	for (int i = 0; i < mpModel->numJoints; i++)
	{
		float transVec[3];
		float rotVec[3];
		Matrix4x4 transform;
		Joint *pJoint = &(mpModel->pJoints[i]);
		JointAnimation *pJointAnimation = &(pJointAnimations[i]);

		float interpValue = (float)(m_blendTimer/m_blendTime);

		transVec[0] = pJointAnimation->startBlendTrans[0]+( pJointAnimation->endBlendTrans[0]-pJointAnimation->startBlendTrans[0] )*interpValue;
		transVec[1] = pJointAnimation->startBlendTrans[1]+( pJointAnimation->endBlendTrans[1]-pJointAnimation->startBlendTrans[1] )*interpValue;
		transVec[2] = pJointAnimation->startBlendTrans[2]+( pJointAnimation->endBlendTrans[2]-pJointAnimation->startBlendTrans[2] )*interpValue;
		// To preserve blending, since the matrix-to-angles functionality is broken
		rotVec[0] = pJointAnimation->startBlendRot[0]+( pJointAnimation->endBlendRot[0]-pJointAnimation->startBlendRot[0] )*interpValue;
		rotVec[1] = pJointAnimation->startBlendRot[1]+( pJointAnimation->endBlendRot[1]-pJointAnimation->startBlendRot[1] )*interpValue;
		rotVec[2] = pJointAnimation->startBlendRot[2]+( pJointAnimation->endBlendRot[2]-pJointAnimation->startBlendRot[2] )*interpValue;
		
		quat q1(vec3(pJointAnimation->startBlendRot[0], pJointAnimation->startBlendRot[1], pJointAnimation->startBlendRot[2]));
		quat q2(vec3(pJointAnimation->endBlendRot[0], pJointAnimation->endBlendRot[1], pJointAnimation->endBlendRot[2]));
		quat q3 = slerp(q1, q2, interpValue);

		mat4 trans = mat4_cast(q3);
		float *pM = value_ptr(trans);
		transform.SetValues(pM);

		transform.SetTranslation(transVec);

		Matrix4x4 relativeFinal(pJoint->relative);
		relativeFinal.PostMultiply(transform);

		if (pJoint->parent == -1)
		{
			pJointAnimation->final = relativeFinal;
		}
		else
		{
			pJointAnimation->final = pJointAnimations[pJoint->parent].final;

			pJointAnimation->final.PostMultiply(relativeFinal);
		}

		// Also store the current trans and rot values in the start blend variables, in case we want to start a new blend.
		const vec3& v = transform.GetTranslationVector();
		pJointAnimation->currentBlendTrans[0] = v.x;
		pJointAnimation->currentBlendTrans[1] = v.y;
		pJointAnimation->currentBlendTrans[2] = v.z;
		pJointAnimation->currentBlendRot[0] = rotVec[0];
		pJointAnimation->currentBlendRot[1] = rotVec[1];
		pJointAnimation->currentBlendRot[2] = rotVec[2];
	}
}

// Rendering
void MS3DAnimator::Render(bool lMesh, bool lNormals, bool lBones, bool lBoundingBox)
{
	if(lMesh)
	{
		RenderMesh();
	}
	if(lNormals)
	{
		RenderNormals();
	}
	if(lBones)
	{
		RenderBones();
	}
	if(lBoundingBox)
	{
		RenderBoundingBox();
	}
}

void MS3DAnimator::RenderMesh()
{
	//Draw by group
	for ( int i = 0; i < mpModel->numMeshes; i++ )
	{
		int materialIndex = mpModel->pMeshes[i].materialIndex;
		mpRenderer->BindTexture(mpModel->pMaterials[materialIndex].texture);

		glBegin( GL_TRIANGLES );
		{
			for ( int j = 0; j < mpModel->pMeshes[i].numTriangles; j++ )
			{
				int triangleIndex = mpModel->pMeshes[i].pTriangleIndices[j];
				const Triangle* pTri = &mpModel->pTriangles[triangleIndex];

				for ( int k = 0; k < 3; k++ )
				{
					int index = pTri->vertexIndices[k];

					if ( mpModel->pVertices[index].boneID == -1 )
					{
						//Dont draw the foot steps!
					}
					else
					{
						//Rotate according to transformation matrix
						Matrix4x4& final = pJointAnimations[mpModel->pVertices[index].boneID].final;

						// Texture co-ordinate
						glTexCoord2f( pTri->s[k], pTri->t[k] );

						// Normal
						vec3 newNormal( pTri->vertexNormals[k][0], pTri->vertexNormals[k][1], pTri->vertexNormals[k][2]);

						newNormal = final * newNormal;

						newNormal = normalize(newNormal);

						float tempNormal[3];
						tempNormal[0] = newNormal.x;
						tempNormal[1] = newNormal.y;
						tempNormal[2] = newNormal.z;
						glNormal3fv( tempNormal );

						// Vertex
						vec3 newVertex( mpModel->pVertices[index].location[0], mpModel->pVertices[index].location[1], mpModel->pVertices[index].location[2]);

						newVertex = final * newVertex;

						float tempVertex[3];
						tempVertex[0] = newVertex.x;
						tempVertex[1] = newVertex.y;
						tempVertex[2] = newVertex.z;
						glVertex3fv( tempVertex);
					}
				}
			}
		}
		glEnd();
		mpRenderer->DisableTexture();
	}
}

void MS3DAnimator::RenderNormals()
{
	//Make the colour cyan
	glColor3ub(0, 255, 255);

	glDisable(GL_LIGHTING);
	mpRenderer->SetRenderMode(RM_SOLID);

	for ( int i = 0; i < mpModel->numMeshes; i++ )
	{
		glBegin( GL_LINES );
		{
			for ( int j = 0; j < mpModel->pMeshes[i].numTriangles; j++ )
			{
				int triangleIndex = mpModel->pMeshes[i].pTriangleIndices[j];
				const Triangle* pTri = &mpModel->pTriangles[triangleIndex];

				for ( int k = 0; k < 3; k++ )
				{
					int index = pTri->vertexIndices[k];

					if ( mpModel->pVertices[index].boneID == -1 )
					{
						//Dont draw the foot steps!
					}
					else
					{
						//Rotate according to transformation matrix
						Matrix4x4& final = mpModel->pJoints[mpModel->pVertices[index].boneID].absolute;

						// Normal
						vec3 newNormal( pTri->vertexNormals[k][0], pTri->vertexNormals[k][1], pTri->vertexNormals[k][2]);

						newNormal = final * newNormal;

						newNormal = normalize(newNormal);
						newNormal *= 0.3f; // Scale normal down

						float tempNormal[3];
						tempNormal[0] = newNormal.x;
						tempNormal[1] = newNormal.y;
						tempNormal[2] = newNormal.z;

						// Vertex
						vec3 newVertex( mpModel->pVertices[index].location[0], mpModel->pVertices[index].location[1], mpModel->pVertices[index].location[2]);

						newVertex = final * newVertex;

						float tempVertex[3];
						tempVertex[0] = newVertex.x;
						tempVertex[1] = newVertex.y;
						tempVertex[2] = newVertex.z;

						// Draw a line for the normal
						glVertex3f( tempVertex[0], tempVertex[1], tempVertex[2]);
						glVertex3f( tempVertex[0] + tempNormal[0], tempVertex[1] + tempNormal[1], tempVertex[2] + tempNormal[2]);
					}
				}
			}
		}
		glEnd();
	}
}

void MS3DAnimator::RenderBones()
{
	//Make the colour white
	glColor3ub(255, 255, 255);

	for ( int i = 0; i < numJointAnimations; i++ )
	{
		glBegin( GL_LINES );
		{
			vec3 newVertex;

			newVertex = pJointAnimations[i].final * newVertex;

			float tempVertex[3];
			tempVertex[0] = newVertex.x;
			tempVertex[1] = newVertex.y;
			tempVertex[2] = newVertex.z;
			glVertex3fv( tempVertex);

			if ( mpModel->pJoints[i].parent != -1 )
			{
				Matrix4x4& final = pJointAnimations[mpModel->pJoints[i].parent].final;
				vec3 newPVertex;

				newPVertex =  final * newPVertex;

				float tempPVertex[3];
				tempPVertex[0] = newPVertex.x;
				tempPVertex[1] = newPVertex.y;
				tempPVertex[2] = newPVertex.z;
				glVertex3fv( tempPVertex );
			}
		}

		glEnd();
	}
}

void MS3DAnimator::RenderBoundingBox()
{
	mpRenderer->PushMatrix();
		mpRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);

		mpRenderer->EnableImmediateMode(IM_LINES);
		// Bottom
		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMinY, m_BoundingBox.mMinZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMinY, m_BoundingBox.mMinZ);

		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMinY, m_BoundingBox.mMinZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMinY, m_BoundingBox.mMaxZ);

		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMinY, m_BoundingBox.mMaxZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMinY, m_BoundingBox.mMaxZ);

		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMinY, m_BoundingBox.mMaxZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMinY, m_BoundingBox.mMinZ);

		// Top
		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMaxY, m_BoundingBox.mMinZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMaxY, m_BoundingBox.mMinZ);

		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMaxY, m_BoundingBox.mMinZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMaxY, m_BoundingBox.mMaxZ);

		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMaxY, m_BoundingBox.mMaxZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMaxY, m_BoundingBox.mMaxZ);

		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMaxY, m_BoundingBox.mMaxZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMaxY, m_BoundingBox.mMinZ);

		// Sides
		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMinY, m_BoundingBox.mMinZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMaxY, m_BoundingBox.mMinZ);

		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMinY, m_BoundingBox.mMinZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMaxY, m_BoundingBox.mMinZ);

		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMinY, m_BoundingBox.mMaxZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMaxX, m_BoundingBox.mMaxY, m_BoundingBox.mMaxZ);

		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMinY, m_BoundingBox.mMaxZ);
		mpRenderer->ImmediateVertex(m_BoundingBox.mMinX, m_BoundingBox.mMaxY, m_BoundingBox.mMaxZ);

		mpRenderer->DisableImmediateMode();
	mpRenderer->PopMatrix();
}