// ******************************************************************************
// Filename:    VoxelCharacter.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 24/07/14
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "VoxelCharacter.h"

#include "../utils/Interpolator.h"
#include "../utils/Random.h"

#include <glm/detail/func_geometric.hpp>

#include <fstream>
#include <ostream>
#include <iostream>
#include <string>
using namespace std;


VoxelCharacter::VoxelCharacter(Renderer* pRenderer, QubicleBinaryManager* pQubicleBinaryManager)
{
	m_pRenderer = pRenderer;
	m_pQubicleBinaryManager = pQubicleBinaryManager;

	Reset();
}

VoxelCharacter::~VoxelCharacter()
{
	UnloadCharacter();
	Reset();
}

void VoxelCharacter::Reset()
{
	m_loaded = false;

	m_loadedFaces = false;

	m_updateAnimator = true;

	m_renderRightWeapon = false;
	m_renderLeftWeapon = false;

	m_characterScale = 1.0f;

	m_characterAlpha = 1.0f;

	m_lookRotationAngle = 0.0f;
	m_zLookTranslate = 0.0f;

	m_currentFrame = 0;

	m_pVoxelModel = nullptr;
	m_pCharacterModel = nullptr;
	for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
	{
		m_pCharacterAnimator[i] = nullptr;
	}	

	m_pRightWeapon = nullptr;
	m_pLeftWeapon = nullptr;
	m_rightWeaponLoaded = false;
	m_leftWeaponLoaded = false;

	// Breathing animation
	m_bBreathingAnimationEnabled = false;
	m_bBreathingAnimationStarted = false;
	m_breathingBodyYOffset = 0.0f;
	m_breathingHandsYOffset = 0.0f;
	Interpolator::GetInstance()->RemoveFloatInterpolationByVariable(&m_breathingBodyYOffset);
	Interpolator::GetInstance()->RemoveFloatInterpolationByVariable(&m_breathingHandsYOffset);
	m_breathingAnimationInitialWaitTime = GetRandomNumber(0, 100, 2) * 0.01f;

	// Facial expressions
	m_numFacialExpressions = 0;
	m_pFacialExpressions = nullptr;
	m_faceEyesTexture = -1;
	m_faceMouthTexture = -1;	
	m_eyesOffset = vec3(0.0f, 0.0f, 0.0f);
	m_mouthOffset = vec3(0.0f, 0.0f, 0.0f);
	m_currentFacialExpression = 0;
	m_eyesBoneName.clear();
	m_mouthBoneName.clear();
	m_eyesBone = -1;
	m_mouthBone = -1;
	m_eyesMatrixIndex = -1;
	m_mouthMatrixIndex = -1;
	m_eyesTextureWidth = 9.0f;
	m_eyesTextureHeight = 9.0f;
	m_mouthTextureWidth = 9.0f;
	m_mouthTextureHeight = 9.0f;
	
	// Face looking direction
	m_faceLookingDirection = vec3(0.0f, 0.0f, 1.0f);
	m_faceLookingDirection = normalize(m_faceLookingDirection);
	m_faceTargetDirection = vec3(0.0f, 0.0f, 1.0f);
	m_faceTargetDirection = normalize(m_faceTargetDirection);
	m_faceLookToTargetSpeedMultiplier = 1.0f;
	m_headBoneIndex = -1;
	m_bodyBoneIndex = -1;
	m_leftShoulderBoneIndex = -1;
	m_leftHandBoneIndex = -1;
	m_rightShoulderBoneIndex = -1;
	m_rightHandBoneIndex = -1;
	m_legsBoneIndex = -1;
	m_rightFootBoneIndex = -1;
	m_leftFootBoneIndex = -1;
	m_bRandomLookDirectionEnabled = false;

	// Wink animation
	m_bWinkAnimationEnabled = false;
	m_faceEyesWinkTexture = -1;
	m_wink = false;
	m_winkWaitTimer = 4.0f + GetRandomNumber(-2, 2, 2);
	m_winkStayTime = 0.15f;

	// Talking animation
	m_bTalkingAnimationEnabled = false;
	m_numTalkingMouths = 0;
	m_pTalkingAnimations = nullptr;
	m_currentTalkingTexture = 0;
	m_talkingWaitTimer = 0.0f;
	m_talkingWaitTime = 0.12f;
	m_randomMouthSelection = false;
	m_talkingPauseTimer = 0.0f;
	m_talkingPauseTime = 0.45f;
	m_talkingPauseMouthCounter = 0;
	m_talkingPauseMouthAmount = 6;
}

void VoxelCharacter::LoadVoxelCharacter(const char* characterType, const char *qbFilename, const char *modelFilename, const char *animatorFilename, const char *facesFilename, const char* characterFilename, const char *charactersBaseFolder, bool useQubicleManager)
{
	m_usingQubicleManager = useQubicleManager;

	// Qubicle model
	if(useQubicleManager)
	{
		m_pVoxelModel = m_pQubicleBinaryManager->GetQubicleBinaryFile(qbFilename, false);
	}
	else
	{
		m_pVoxelModel = new QubicleBinary(m_pRenderer);
		m_pVoxelModel->Import(qbFilename, true);
	}

	// MS3d model
	m_pCharacterModel = new MS3DModel(m_pRenderer);
	m_pCharacterModel->LoadModel(modelFilename);

	// Animators
	for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
	{
		m_pCharacterAnimator[i] = new MS3DAnimator(m_pRenderer, m_pCharacterModel);
		m_pCharacterAnimator[i]->LoadAnimations(animatorFilename);	
	}

	m_pCharacterAnimatorPaperdoll_Left = new MS3DAnimator(m_pRenderer, m_pCharacterModel);
	m_pCharacterAnimatorPaperdoll_Left->LoadAnimations(animatorFilename);
	m_pCharacterAnimatorPaperdoll_Left->PlayAnimation("BindPose");
	m_pCharacterAnimatorPaperdoll_Right = new MS3DAnimator(m_pRenderer, m_pCharacterModel);
	m_pCharacterAnimatorPaperdoll_Right->LoadAnimations(animatorFilename);
	m_pCharacterAnimatorPaperdoll_Right->PlayAnimation("BindPose");

	m_pVoxelModel->SetupMatrixBones(m_pCharacterAnimator[0]);

	// Faces
	LoadFaces(characterType, facesFilename, charactersBaseFolder);
	SetupFacesBones();

	// Character file
	LoadCharacterFile(characterFilename);

	m_pRightWeapon = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);
	m_pLeftWeapon = new VoxelWeapon(m_pRenderer, m_pQubicleBinaryManager);

	m_rightWeaponLoaded = true;
	m_leftWeaponLoaded = true;

	m_loaded = true;
}

void VoxelCharacter::SaveVoxelCharacter(const char *qbFilename, const char *facesFilename, const char* characterFilename)
{
	// Qubicle model
	m_pVoxelModel->Export(qbFilename);

	// Faces
	SaveFaces(facesFilename);

	// Character file
	SaveCharacterFile(characterFilename);
}

void VoxelCharacter::UnloadCharacter()
{
	if(m_loaded)
	{
		if(m_usingQubicleManager == false)
		{
			delete m_pVoxelModel;
		}

		m_pVoxelModel = nullptr;
		delete m_pCharacterModel;
		m_pCharacterModel = nullptr;
		for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
		{
			delete m_pCharacterAnimator[i];
			m_pCharacterAnimator[i] = nullptr;
		}

		delete m_pCharacterAnimatorPaperdoll_Left;
		delete m_pCharacterAnimatorPaperdoll_Right;

		delete[] m_pFacialExpressions;
		m_pFacialExpressions = nullptr;
		m_numFacialExpressions = 0;
	}

	if(m_pRightWeapon != nullptr)
	{
		delete m_pRightWeapon;
		m_pRightWeapon = nullptr;
		m_rightWeaponLoaded = false;		
	}
	if(m_pLeftWeapon != nullptr)
	{
		delete m_pLeftWeapon;
		m_pLeftWeapon = nullptr;
		m_leftWeaponLoaded = false;
	}

	m_renderRightWeapon = false;
	m_renderLeftWeapon = false;

	m_loaded = false;
}

// Rebuild
void VoxelCharacter::RebuildVoxelModel(bool faceMerge)
{
	m_pVoxelModel->RebuildMesh(faceMerge);

	if (m_pRightWeapon != nullptr)
	{
		m_pRightWeapon->RebuildVoxelModel(faceMerge);
	}
	if (m_pLeftWeapon != nullptr)
	{
		m_pLeftWeapon->RebuildVoxelModel(faceMerge);
	}
}

// Faces
bool VoxelCharacter::LoadFaces(const char* characterType, const char *facesFileName, const char *charactersBaseFolder)
{
	ifstream file;

	// Open the file
	file.open(facesFileName, ios::in);
	if(file.is_open())
	{
		string tempString;
		int lTextureWidth, lTextureHeight, lTextureWidth2, lTextureHeight2;

		float offsetX;
		float offsetY;
		float offsetZ;

		file >> tempString >> offsetX >> offsetY >> offsetZ;
		m_eyesOffset = vec3(offsetX, offsetY, offsetZ);

		file >> tempString >> offsetX >> offsetY >> offsetZ;
		m_mouthOffset = vec3(offsetX, offsetY, offsetZ);

		file >> tempString >> m_eyesTextureWidth >> m_eyesTextureHeight;
		file >> tempString >> m_mouthTextureWidth >> m_mouthTextureHeight;

		file >> tempString >> m_winkTextureFilename;

		char winkFilename[128];
		sprintf(winkFilename, "%s/%s/%s", charactersBaseFolder, characterType, m_winkTextureFilename.c_str());
		m_pRenderer->LoadTexture(winkFilename, &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_faceEyesWinkTexture);

		file >> tempString >> m_eyesBoneName;
		file >> tempString >> m_mouthBoneName;

		file >> tempString >> m_numFacialExpressions;

		// Create the facial expressions objects
		m_pFacialExpressions = new FacialExpression[m_numFacialExpressions];

		for(int i = 0; i < m_numFacialExpressions; i++)
		{
			char eyesFilename[128];
			char mouthFilename[128];
			file >> m_pFacialExpressions[i].m_facialExpressionName >> m_pFacialExpressions[i].m_eyesTextureFile >> m_pFacialExpressions[i].m_mouthTextureFile;

			sprintf(eyesFilename, "%s/%s/%s", charactersBaseFolder, characterType, m_pFacialExpressions[i].m_eyesTextureFile.c_str());
			m_pRenderer->LoadTexture(eyesFilename, &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_pFacialExpressions[i].m_eyeTexture);

			sprintf(mouthFilename, "%s/%s/%s", charactersBaseFolder, characterType, m_pFacialExpressions[i].m_mouthTextureFile.c_str());
			m_pRenderer->LoadTexture(mouthFilename, &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_pFacialExpressions[i].m_mouthTexture);
		}

		if(m_numFacialExpressions > 0)
		{
			m_faceEyesTexture = m_pFacialExpressions[0].m_eyeTexture;
			m_faceMouthTexture = m_pFacialExpressions[0].m_mouthTexture;
		}

		file >> tempString >> m_numTalkingMouths;
		m_pTalkingAnimations = new TalkingAnimation[m_numTalkingMouths];

		for(int i = 0; i < m_numTalkingMouths; i++)
		{
			char talkingMouthFilename[128];
			file >> m_pTalkingAnimations[i].m_talkingAnimationTextureFile;

			sprintf(talkingMouthFilename, "%s/%s/%s", charactersBaseFolder, characterType, m_pTalkingAnimations[i].m_talkingAnimationTextureFile.c_str());
			m_pRenderer->LoadTexture(talkingMouthFilename, &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_pTalkingAnimations[i].m_talkingAnimationTexture);
		}

		file.close();

		m_loadedFaces = true;

		return true;
	}

	return false;
}

bool VoxelCharacter::SaveFaces(const char *facesFileName)
{
	ofstream file;

	// Open the file
	file.open(facesFileName, ios::out);
	if(file.is_open())
	{
		file << "eyesOffset: " << m_eyesOffset.x  << " " << m_eyesOffset.y << " " << m_eyesOffset.z << "\n";
		file << "mouthOffset: " << m_mouthOffset.x  << " " << m_mouthOffset.y << " " << m_mouthOffset.z << "\n";

		file << "eyesSize: " << m_eyesTextureWidth  << " " << m_eyesTextureHeight << "\n";
		file << "mouthSize: " << m_mouthTextureWidth  << " " << m_mouthTextureHeight << "\n";

		file << "eyesWinkTexture: " << m_winkTextureFilename.c_str() << "\n";

		file << "eyesBone: " << m_eyesBoneName.c_str() << "\n";
		file << "mouthBone: " << m_mouthBoneName.c_str() << "\n\n";

		file << "numfaces: " << m_numFacialExpressions << "\n";

		for(int i = 0; i < m_numFacialExpressions; i++)
		{
			file << m_pFacialExpressions[i].m_facialExpressionName.c_str() << " " << m_pFacialExpressions[i].m_eyesTextureFile.c_str() << " " << m_pFacialExpressions[i].m_mouthTextureFile.c_str() << "\n";
		}
		file << "\n";

		file << "numTalkingMouths: " << m_numTalkingMouths << "\n";

		for(int i = 0; i < m_numTalkingMouths; i++)
		{
			file << m_pTalkingAnimations[i].m_talkingAnimationTextureFile.c_str() << "\n";
		}
		file << "\n";

		file.close();

		return true;
	}

	return false;
}

void VoxelCharacter::SetupFacesBones()
{
	const MS3DModel* pModel = m_pCharacterAnimator[AnimationSections_FullBody]->GetModel();
	m_eyesBone = pModel->GetBoneIndex(m_eyesBoneName.c_str());
	m_mouthBone = pModel->GetBoneIndex(m_mouthBoneName.c_str());

	m_eyesMatrixIndex = m_pVoxelModel->GetMatrixIndexForName(m_eyesBoneName.c_str());
	m_mouthMatrixIndex = m_pVoxelModel->GetMatrixIndexForName(m_mouthBoneName.c_str());

	m_headBoneIndex = pModel->GetBoneIndex("Head");
	m_bodyBoneIndex = pModel->GetBoneIndex("Body");
	m_leftShoulderBoneIndex = pModel->GetBoneIndex("Left_Shoulder");
	m_leftHandBoneIndex = pModel->GetBoneIndex("Left_Hand");
	m_rightShoulderBoneIndex = pModel->GetBoneIndex("Right_Shoulder");
	m_rightHandBoneIndex = pModel->GetBoneIndex("Right_Hand");
	m_legsBoneIndex = pModel->GetBoneIndex("Legs");
	m_rightFootBoneIndex = pModel->GetBoneIndex("Right_Foot");
	m_leftFootBoneIndex = pModel->GetBoneIndex("Left_Foot");
}

void VoxelCharacter::ModifyEyesTextures(const char *charactersBaseFolder, const char* characterType, const char* eyeTextureFolder)
{
	int lTextureWidth, lTextureHeight, lTextureWidth2, lTextureHeight2;

	char winkFilename[128];

	// For saving to the faces file we need a stripped down version of the full path
	sprintf(winkFilename, "faces/%s/face_eyes_wink.tga", eyeTextureFolder);
	m_winkTextureFilename = winkFilename;

	// Generate full path for texture loading
	sprintf(winkFilename, "%s/%s/faces/%s/face_eyes_wink.tga", charactersBaseFolder, characterType, eyeTextureFolder);
	m_pRenderer->LoadTexture(winkFilename, &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_faceEyesWinkTexture);

	for(int i = 0; i < m_numFacialExpressions; i++)
	{
		char eyesFilename[128];

		int firstindex = (int)(m_pFacialExpressions[i].m_eyesTextureFile.find_last_of("/"));
		int lastindex = (int)(m_pFacialExpressions[i].m_eyesTextureFile.find_last_of("."));
		string fileWithoutExtension = m_pFacialExpressions[i].m_eyesTextureFile.substr(firstindex+1, lastindex); 

		// For saving to the faces file we need a stripped down version of the full path
		sprintf(eyesFilename, "faces/%s/%s", eyeTextureFolder, fileWithoutExtension.c_str());
		m_pFacialExpressions[i].m_eyesTextureFile = eyesFilename;

		// Generate full path for texture loading
		sprintf(eyesFilename, "%s/%s/faces/%s/%s", charactersBaseFolder, characterType, eyeTextureFolder, fileWithoutExtension.c_str());
		m_pRenderer->LoadTexture(eyesFilename, &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &m_pFacialExpressions[i].m_eyeTexture);
	}

	m_faceEyesTexture = m_pFacialExpressions[m_currentFacialExpression].m_eyeTexture;
	m_faceMouthTexture = m_pFacialExpressions[m_currentFacialExpression].m_mouthTexture;
}

// Character file
void VoxelCharacter::LoadCharacterFile(const char* characterFilename)
{
	ifstream file;

	// Open the file
	file.open(characterFilename, ios::in);
	if(file.is_open())
	{
		string tempString;
		int numModifiers;

		float xBoneScale;
		float yBoneScale;
		float zBoneScale;
		file >> tempString >> xBoneScale >> yBoneScale >> zBoneScale;
		m_boneScale = vec3(xBoneScale, yBoneScale, zBoneScale);

		file >> tempString >> numModifiers;

		for(int i = 0; i < numModifiers; i++)
		{
			string matrixName;
			float scale;
			float xOffset;
			float yOffset;
			float zOffset;

			file >> tempString >> matrixName;
			file >> tempString >> scale;
			file >> tempString >> xOffset >> yOffset >> zOffset;

			m_pVoxelModel->SetScaleAndOffsetForMatrix(matrixName.c_str(), scale, xOffset, yOffset, zOffset);
		}

		file.close();
	}
}

void VoxelCharacter::SaveCharacterFile(const char* characterFilename)
{
	ofstream file;

	// Open the file
	file.open(characterFilename, ios::out);
	if(file.is_open())
	{
		file << "boneScale: " << m_boneScale.x << " " << m_boneScale.y << " " << m_boneScale.z << "\n\n";

		file << "numModifiers: " << m_pVoxelModel->GetNumMatrices() << "\n\n";

		for(int i = 0; i < m_pVoxelModel->GetNumMatrices(); i++)
		{
			vec3 offset = m_pVoxelModel->GetMatrixOffset(i);

			file << "boneName: " << m_pVoxelModel->GetMatrixName(i) << "\n";
			file << "Scale: " << m_pVoxelModel->GetMatrixScale(i) << "\n";
			file << "Offset: " << offset.x << " " << offset.y << " " << offset.z << "\n\n";
		}

		file.close();
	}
}

void VoxelCharacter::ResetMatrixParamsFromCharacterFile(const char* characterFilename, const char* matrixToReset)
{
	ifstream file;

	// Open the file
	file.open(characterFilename, ios::in);
	if(file.is_open())
	{
		string tempString;
		int numModifiers;

		file >> tempString >> numModifiers;

		for(int i = 0; i < numModifiers; i++)
		{
			string matrixName;
			float scale;
			float xOffset;
			float yOffset;
			float zOffset;

			file >> tempString >> matrixName;
			file >> tempString >> scale;
			file >> tempString >> xOffset >> yOffset >> zOffset;

			if(strcmp(matrixName.c_str(), matrixToReset) == 0)
			{
				m_pVoxelModel->SetScaleAndOffsetForMatrix(matrixName.c_str(), scale, xOffset, yOffset, zOffset);

				file.close();

				return;
			}
		}

		file.close();
	}
}

// Character scale
void VoxelCharacter::SetCharacterScale(float scale)
{
	m_characterScale = scale;
}

float VoxelCharacter::GetCharacterScale() const
{
	return m_characterScale;
}

// Character alpha
float VoxelCharacter::GetCharacterAlpha() const
{
	return m_characterAlpha;
}

// Upper body and head tilt look, for camera rotation
void VoxelCharacter::SetHeadAndUpperBodyLookRotation(float lookRotationAngle, float zLookTranslate)
{
	m_lookRotationAngle = lookRotationAngle;
	m_zLookTranslate = zLookTranslate;
}

float VoxelCharacter::GetHeadAndUpperBodyLookRotation() const
{
	return m_lookRotationAngle;
}

float VoxelCharacter::GetHeadAndUpperBodyLookzTranslate() const
{
	return m_zLookTranslate;
}

void VoxelCharacter::SetCharacterMatrixRenderParams(const char* matrixName, float scale, float xOffset, float yOffset, float zOffset)
{
	m_pVoxelModel->SetScaleAndOffsetForMatrix(matrixName, scale, xOffset, yOffset, zOffset);
}

float VoxelCharacter::GetBoneMatrixRenderScale(const char* matrixName) const
{
	return m_pVoxelModel->GetScale(matrixName);
}

vec3 VoxelCharacter::GetBoneMatrixRenderOffset(const char* matrixName) const
{
	return m_pVoxelModel->GetOffset(matrixName);
}

void VoxelCharacter::LoadRightWeapon(const char *weaponFilename)
{
	if(m_loaded)
	{
		m_pRightWeapon->SetVoxelCharacterParent(this);
		m_pRightWeapon->SetBoneAttachment("Right_Hand");
		m_pRightWeapon->LoadWeapon(weaponFilename);

		m_renderRightWeapon = true;
		m_rightWeaponLoaded = true;
	}
}

void VoxelCharacter::LoadLeftWeapon(const char *weaponFilename)
{
	if(m_loaded)
	{
		m_pLeftWeapon->SetVoxelCharacterParent(this);
		m_pLeftWeapon->SetBoneAttachment("Left_Hand");
		m_pLeftWeapon->LoadWeapon(weaponFilename);

		m_renderLeftWeapon = true;
		m_leftWeaponLoaded = true;
	}
}

VoxelWeapon* VoxelCharacter::GetRightWeapon()
{
	return m_pRightWeapon;
}

VoxelWeapon* VoxelCharacter::GetLeftWeapon()
{
	return m_pLeftWeapon;
}

void VoxelCharacter::UnloadRightWeapon()
{
	m_rightWeaponLoaded = false;
}

void VoxelCharacter::UnloadLeftWeapon()
{
	m_leftWeaponLoaded = false;
}

bool VoxelCharacter::IsRightWeaponLoaded() const
{
	return m_rightWeaponLoaded;
}

bool VoxelCharacter::IsLeftWeaponLoaded() const
{
	return m_leftWeaponLoaded;
}

// Setup animator and bones
void VoxelCharacter::SetUpdateAnimator(bool update)
{
	m_updateAnimator = update;
}

Matrix4x4 VoxelCharacter::GetBoneMatrix(AnimationSections section, int index) const
{
	if(m_loaded)
	{
		return m_pCharacterAnimator[section]->GetBoneMatrix(index);
	}

	Matrix4x4 empty;
	return empty;
}

Matrix4x4 VoxelCharacter::GetBoneMatrix(AnimationSections section, const char* boneName) const
{
	if(m_loaded)
	{
		int boneIndex = m_pCharacterAnimator[section]->GetModel()->GetBoneIndex(boneName);
		return GetBoneMatrix(section, boneIndex);
	}

	Matrix4x4 empty;
	return empty;
}

Matrix4x4 VoxelCharacter::GetBoneMatrixPaperdoll(int index, bool left) const
{
	if(m_loaded)
	{
		if (left)
		{
			return m_pCharacterAnimatorPaperdoll_Left->GetBoneMatrix(index);
		}
		else
		{
			return m_pCharacterAnimatorPaperdoll_Right->GetBoneMatrix(index);
		}
	}

	Matrix4x4 empty;
	return empty;
}

int VoxelCharacter::GetBoneIndex(const char* boneName) const
{
	if(m_loaded)
	{
		return m_pCharacterAnimator[AnimationSections_FullBody]->GetModel()->GetBoneIndex(boneName);
	}

	return -1;
}

int VoxelCharacter::GetMatrixIndexForName(const char* matrixName) const
{
	if(m_loaded)
	{
		return m_pVoxelModel->GetMatrixIndexForName(matrixName);
	}

	return -1;
}

const MS3DModel* VoxelCharacter::GetMS3DModel() const
{
	return m_pCharacterModel;
}

const MS3DAnimator* VoxelCharacter::GetMS3DAnimator(AnimationSections section) const
{
	return m_pCharacterAnimator[section];
}

const QubicleBinary* VoxelCharacter::GetQubicleModel() const
{
	return m_pVoxelModel;
}

vec3 VoxelCharacter::GetBoneScale() const
{
	return m_boneScale;
}

void VoxelCharacter::SetBoneScale(float scale)
{
	m_boneScale.x = scale;
	m_boneScale.y = scale;
	m_boneScale.z = scale;
}

// Rendering modes
void VoxelCharacter::SetWireFrameRender(bool wireframe)
{
	if(m_pVoxelModel != nullptr)
	{
		m_pVoxelModel->SetWireFrameRender(wireframe);
	}

	if(m_pLeftWeapon != nullptr && m_leftWeaponLoaded)
	{
		m_pLeftWeapon->SetWireFrameRender(wireframe);
	}

	if(m_pRightWeapon != nullptr && m_rightWeaponLoaded)
	{
		m_pRightWeapon->SetWireFrameRender(wireframe);
	}
}

void VoxelCharacter::SetRenderRightWeapon(bool render)
{
	m_renderRightWeapon = render;
}

void VoxelCharacter::SetRenderLeftWeapon(bool render)
{
	m_renderLeftWeapon = render;
}

void VoxelCharacter::SetMeshAlpha(float alpha, bool force)
{
	if(force == false && alpha == m_characterAlpha)
	{
		// Early out, since we haven't changed alpha (much!)
		return;
	}

	m_characterAlpha = alpha;

	if(m_pVoxelModel)
	{
		m_pVoxelModel->SetMeshAlpha(m_characterAlpha);
	}

	if(m_pLeftWeapon && m_leftWeaponLoaded)
	{
		m_pLeftWeapon->SetMeshAlpha(m_characterAlpha);
	}

	if(m_pRightWeapon && m_rightWeaponLoaded)
	{
		m_pRightWeapon->SetMeshAlpha(m_characterAlpha);
	}
}

void VoxelCharacter::SetMeshSingleColour(float r, float g, float b)
{
	if(m_pVoxelModel)
	{
		m_pVoxelModel->SetMeshSingleColour(r, g, b);
	}

	if (m_pLeftWeapon && m_leftWeaponLoaded)
	{
		m_pLeftWeapon->SetMeshSingleColour(r, g, b);
	}

	if (m_pRightWeapon && m_rightWeaponLoaded)
	{
		m_pRightWeapon->SetMeshSingleColour(r, g, b);
	}
}

void VoxelCharacter::SetBreathingAnimationEnabled(bool enable)
{
	m_bBreathingAnimationEnabled = enable;
}

bool VoxelCharacter::IsBreathingAnimationEnabled() const
{
	return m_bBreathingAnimationEnabled;
}

bool VoxelCharacter::IsBreathingAnimationStarted() const
{
	return m_bBreathingAnimationStarted;
}

void VoxelCharacter::StartBreathAnimation()
{
	m_bBreathingAnimationStarted = true;

	auto* pInterpolator = Interpolator::GetInstance();

	FloatInterpolation* lBodyYInterpolation1 = pInterpolator->CreateFloatInterpolation(&m_breathingBodyYOffset, 0.0f, 0.35f, 1.5f, 100.0f);
	FloatInterpolation* lBodyYInterpolation2 = pInterpolator->CreateFloatInterpolation(&m_breathingBodyYOffset, 0.35f, 0.35f, 0.175f, 0.0f);
	FloatInterpolation* lBodyYInterpolation3 = pInterpolator->CreateFloatInterpolation(&m_breathingBodyYOffset, 0.35f, 0.0f, 1.5f, -100.0f);
	FloatInterpolation* lBodyYInterpolation4 = pInterpolator->CreateFloatInterpolation(&m_breathingBodyYOffset, 0.0f, 0.0f, 0.05f, 0.0f, nullptr, _BreathAnimationFinished, this);
	pInterpolator->LinkFloatInterpolation(lBodyYInterpolation1, lBodyYInterpolation2);
	pInterpolator->LinkFloatInterpolation(lBodyYInterpolation2, lBodyYInterpolation3);
	pInterpolator->LinkFloatInterpolation(lBodyYInterpolation3, lBodyYInterpolation4);

	FloatInterpolation* lHandsYInterpolation1 = pInterpolator->CreateFloatInterpolation(&m_breathingHandsYOffset, 0.0f, 0.0f, 0.5f, 0.0f);
	FloatInterpolation* lHandsYInterpolation2 = pInterpolator->CreateFloatInterpolation(&m_breathingHandsYOffset, 0.0f, 0.75f, 1.25f, 100.0f);
	FloatInterpolation* lHandsYInterpolation3 = pInterpolator->CreateFloatInterpolation(&m_breathingHandsYOffset, 0.75f, 0.75f, 0.125f, 0.0f);
	FloatInterpolation* lHandsYInterpolation4 = pInterpolator->CreateFloatInterpolation(&m_breathingHandsYOffset, 0.75f, 0.0f, 1.5f, -100.0f);
	pInterpolator->LinkFloatInterpolation(lHandsYInterpolation1, lHandsYInterpolation2);
	pInterpolator->LinkFloatInterpolation(lHandsYInterpolation2, lHandsYInterpolation3);
	pInterpolator->LinkFloatInterpolation(lHandsYInterpolation3, lHandsYInterpolation4);

	pInterpolator->AddFloatInterpolation(lBodyYInterpolation1);
	pInterpolator->AddFloatInterpolation(lHandsYInterpolation1);
}

float VoxelCharacter::GetBreathingAnimationOffsetForBone(int boneIndex) const
{
	auto* pFullBodyModel = m_pCharacterAnimator[AnimationSections_FullBody]->GetModel();

	if(strcmp(pFullBodyModel->GetNameFromBoneIndex(boneIndex), "Head") == 0)
	{
		return m_breathingBodyYOffset * 0.75f;
	}
	if(strcmp(pFullBodyModel->GetNameFromBoneIndex(boneIndex), "Body") == 0)
	{
		return m_breathingBodyYOffset;
	}
	if(strcmp(pFullBodyModel->GetNameFromBoneIndex(boneIndex), "Legs") == 0)
	{
		return m_breathingBodyYOffset * 0.5f;
	}
	if(strcmp(pFullBodyModel->GetNameFromBoneIndex(boneIndex), "Right_Shoulder") == 0)
	{
		return m_breathingHandsYOffset;
	}
	if(strcmp(pFullBodyModel->GetNameFromBoneIndex(boneIndex), "Left_Shoulder") == 0)
	{
		return m_breathingHandsYOffset;
	}
	if(strcmp(pFullBodyModel->GetNameFromBoneIndex(boneIndex), "Right_Hand") == 0)
	{
		return m_breathingHandsYOffset;
	}
	if(strcmp(pFullBodyModel->GetNameFromBoneIndex(boneIndex), "Left_Hand") == 0)
	{
		return m_breathingHandsYOffset;
	}

	return 0.0f;
}

int VoxelCharacter::GetNumFacialExpressions() const
{
	return m_numFacialExpressions;
}

const char* VoxelCharacter::GetFacialExpressionName(const int index) const
{
	return m_pFacialExpressions[index].m_facialExpressionName.c_str();
}

vec3 VoxelCharacter::GetEyesOffset() const
{
	return m_eyesOffset;
}

vec3 VoxelCharacter::GetMouthOffset() const
{
	return m_mouthOffset;
}

void VoxelCharacter::SetEyesOffset(vec3 offset)
{
	m_eyesOffset = offset;
}

void VoxelCharacter::SetMouthOffset(vec3 offset)
{
	m_mouthOffset = offset;
}

// Wink animation
void VoxelCharacter::SetWinkAnimationEnabled(bool enable)
{
	m_bWinkAnimationEnabled = enable;
}

bool VoxelCharacter::IsWinkAnimationEnabled() const
{
	return m_bWinkAnimationEnabled;
}

void VoxelCharacter::UpdateWinkAnimation(float dt)
{
	m_winkWaitTimer -= dt;
	if(m_winkWaitTimer <= 0.0f)
	{
		m_winkWaitTimer = 4.0f + GetRandomNumber(-2, 2, 2);
		m_wink = false;

		// Return eyes back to whatever they were before the wink
		m_faceEyesTexture = m_pFacialExpressions[m_currentFacialExpression].m_eyeTexture;
	}
	else if(m_winkWaitTimer <= m_winkStayTime)
	{
		m_wink = true;
		m_faceEyesTexture = m_faceEyesWinkTexture;
	}
}

// Talking animation
void VoxelCharacter::SetTalkingAnimationEnabled(bool enable)
{
	m_bTalkingAnimationEnabled = enable;

	if(m_loaded && m_loadedFaces)
	{
		if(m_bTalkingAnimationEnabled == false)
		{
			m_faceMouthTexture = m_pFacialExpressions[m_currentFacialExpression].m_mouthTexture;
		}
	}
}

bool VoxelCharacter::IsTalkingAnimationEnabled() const
{
	return m_bTalkingAnimationEnabled;
}

void VoxelCharacter::SetRandomMouthSelection(bool random)
{
	m_randomMouthSelection = random;
}

bool VoxelCharacter::IsRandomMouthSelectionEnabled() const
{
	return m_randomMouthSelection;
}

void VoxelCharacter::UpdateTalkingAnimation(float dt)
{
	m_talkingWaitTimer -= dt;
	m_talkingPauseTimer -= dt;
	if(m_talkingWaitTimer <= 0.0f && m_talkingPauseTimer <= 0.0f)
	{
		if(m_randomMouthSelection)
		{
			// Random mouth selection
			m_currentTalkingTexture = GetRandomNumber(0, m_numTalkingMouths-1);
		}
		else
		{
			// Ordered mouth selection
			m_currentTalkingTexture++;
			if(m_currentTalkingTexture >= m_numTalkingMouths)
			{
				m_currentTalkingTexture = 0;
			}
		}

		// Should we take a pause from cycling talking mouths, to simulate normal conversation stops...
		m_talkingPauseMouthCounter++;
		if(m_talkingPauseMouthCounter == m_talkingPauseMouthAmount)
		{
			m_talkingPauseMouthCounter = 0;
			m_talkingPauseMouthAmount = 6 + GetRandomNumber(-2, 5);

			float randomTimeAddtion = GetRandomNumber(-25, 40, 2) * 0.01f;
			m_talkingPauseTimer = m_talkingPauseTime + randomTimeAddtion;
		}

		if(m_talkingPauseTimer > 0.0f)
		{
			if(GetRandomNumber(0, 100, 1) > 50)
			{
				// Revert back to the face pose mouth
				m_faceMouthTexture = m_pFacialExpressions[m_currentFacialExpression].m_mouthTexture;
			}
			else
			{
				// Leave the current mouth on pause for this wait period
			}			
		}
		else
		{
			m_faceMouthTexture = m_pTalkingAnimations[m_currentTalkingTexture].m_talkingAnimationTexture;

			float randomTimeAddtion = GetRandomNumber(-10, 50, 2) * 0.00225f;
			m_talkingWaitTimer = m_talkingWaitTime + randomTimeAddtion;
		}
	}
}

void VoxelCharacter::PlayFacialExpression(const char *lFacialExpressionName)
{
	if(m_loadedFaces)
	{
		int lExpressionIndex = -1;
		for(int i = 0; i < m_numFacialExpressions; i++)
		{
#ifdef _WIN32
			if (_strcmpi(lFacialExpressionName, m_pFacialExpressions[i].m_facialExpressionName.c_str()) == 0)
#else
			if (strcasecmp(lFacialExpressionName, m_pFacialExpressions[i].m_facialExpressionName.c_str()) == 0)
#endif //_WIN32
			{
				lExpressionIndex = i;

				break;
			}
		}

		if(lExpressionIndex != -1)
		{
			PlayFacialExpression(lExpressionIndex);
		}
	}
}

void VoxelCharacter::PlayFacialExpression(int facialAnimationIndex)
{
	if(m_loadedFaces)
	{
		if(facialAnimationIndex >= 0 && facialAnimationIndex < m_numFacialExpressions)
		{
			m_currentFacialExpression = facialAnimationIndex;

			m_faceEyesTexture = m_pFacialExpressions[m_currentFacialExpression].m_eyeTexture;
			m_faceMouthTexture = m_pFacialExpressions[m_currentFacialExpression].m_mouthTexture;
		}
	}
}

int VoxelCharacter::GetCurrentFacialAnimation() const
{
	return m_currentFacialExpression;
}

void VoxelCharacter::SetEyesBone(string eyesBoneName)
{
	if(m_loadedFaces)
	{
		m_eyesBoneName = eyesBoneName;

		SetupFacesBones();
	}
}

void VoxelCharacter::SetMouthBone(string mouthBoneName)
{
	if(m_loadedFaces)
	{
		m_mouthBoneName = mouthBoneName;

		SetupFacesBones();
	}
}

string VoxelCharacter::GetEyesBoneName() const
{
	return m_eyesBoneName;
}

string VoxelCharacter::GetMouthBoneName() const
{
	return m_mouthBoneName;
}

int VoxelCharacter::GetEyesBone() const
{
	return m_eyesBone;
}

int VoxelCharacter::GetMouthBone() const
{
	return m_mouthBone;
}

int VoxelCharacter::GetEyesMatrixIndex() const
{
	return m_eyesMatrixIndex;
}

int VoxelCharacter::GetMouthMatrixIndex() const
{
	return m_mouthMatrixIndex;
}

void VoxelCharacter::SetEyesTextureSize(float width, float height)
{
	m_eyesTextureWidth = width;
	m_eyesTextureHeight = height;
}

void VoxelCharacter::SetMouthTextureSize(float width, float height)
{
	m_mouthTextureWidth = width;
	m_mouthTextureHeight = height;
}

float VoxelCharacter::GetEyeTextureWidth() const
{
	return m_eyesTextureWidth;
}

float VoxelCharacter::GetEyeTextureHeight() const
{
	return m_eyesTextureHeight;
}

float VoxelCharacter::GetMouthTextureWidth() const
{
	return m_mouthTextureWidth;
}

float VoxelCharacter::GetMouthTextureHeight() const
{
	return m_mouthTextureHeight;
}

// Face looking
vec3 VoxelCharacter::GetFaceLookingDirection() const
{
	return m_faceLookingDirection;
}

void VoxelCharacter::SetFaceLookingDirection(vec3 looking)
{
	if(length(looking) > 0.001f)
	{
		m_faceLookingDirection = normalize(looking);
	}
	else
	{
		m_faceLookingDirection = vec3(0.0f, 0.0f, 1.0f);
	}
}

vec3 VoxelCharacter::GetFaceTargetDirection() const
{
	return m_faceTargetDirection;
}

void VoxelCharacter::SetFaceTargetDirection(vec3 target)
{
	if(length(target) > 0.001f)
	{
		m_faceTargetDirection = normalize(target);
	}
	else
	{
		m_faceTargetDirection = vec3(0.0f, 0.0f, 1.0f);
	}	
}

float VoxelCharacter::GetFaceLookToTargetSpeedMultiplier() const
{
	return m_faceLookToTargetSpeedMultiplier;
}

void VoxelCharacter::SetFaceLookToTargetSpeedMultiplier(float speedMultiplier)
{
	m_faceLookToTargetSpeedMultiplier = speedMultiplier;
}

int VoxelCharacter::GetHeadBoneIndex() const
{
	return m_headBoneIndex;
}

int VoxelCharacter::GetBodyBoneIndex() const
{
	return m_bodyBoneIndex;
}

int VoxelCharacter::GetLeftShoulderBoneIndex() const
{
	return m_leftShoulderBoneIndex;
}

int VoxelCharacter::GetLeftHandBoneIndex() const
{
	return m_leftHandBoneIndex;
}

int VoxelCharacter::GetRightShoulderBoneIndex() const
{
	return m_rightShoulderBoneIndex;
}

int VoxelCharacter::GetRightHandBoneIndex() const
{
	return m_rightHandBoneIndex;
}

int VoxelCharacter::GetLegsBoneIndex() const
{
	return m_legsBoneIndex;
}

int VoxelCharacter::GetRightFootBoneIndex() const
{
	return m_rightFootBoneIndex;
}

int VoxelCharacter::GetLeftFootBoneIndex() const
{
	return m_leftFootBoneIndex;
}

void VoxelCharacter::SetRandomLookDirection(bool enable)
{
	m_bRandomLookDirectionEnabled = enable;
}

bool VoxelCharacter::IsRandomLookDirectionEnabled() const
{
	return m_bRandomLookDirectionEnabled;
}

int VoxelCharacter::GetNumAnimations() const
{
	if(m_pCharacterAnimator[AnimationSections_FullBody] == nullptr)
	{
		return 0;
	}

	return m_pCharacterAnimator[AnimationSections_FullBody]->GetNumAnimations();
}

const char* VoxelCharacter::GetAnimationName(int index) const
{
	if(m_pCharacterAnimator[AnimationSections_FullBody] == nullptr)
	{
		return "";
	}

	return m_pCharacterAnimator[AnimationSections_FullBody]->GetAnimationName(index);
}

void VoxelCharacter::PlayAnimation(AnimationSections section, bool waitForComplete, AnimationSections syncWithSection, const char *lAnimationName)
{
	if(m_pCharacterAnimator[AnimationSections_FullBody] == nullptr)
	{
		return;
	}

	if(section == AnimationSections_FullBody)
	{
		bool syncComplete = (m_pCharacterAnimator[syncWithSection]->HasAnimationFinished() || m_pCharacterAnimator[syncWithSection]->HasAnimationLooped());

		for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
		{
			if(waitForComplete == false || (m_pCharacterAnimator[i]->HasAnimationFinished() || m_pCharacterAnimator[i]->HasAnimationLooped()))
			{
				if(syncWithSection == section || syncComplete)
				{
					m_pCharacterAnimator[i]->PlayAnimation(lAnimationName);

					// Stop weapon trails
					if (m_pRightWeapon != nullptr)
					{
						m_pRightWeapon->StopWeaponTrails();
					}
					if (m_pLeftWeapon != nullptr)
					{
						m_pLeftWeapon->StopWeaponTrails();
					}
				}
			}
		}
	}
	else
	{
		m_pCharacterAnimator[section]->PlayAnimation(lAnimationName);

		// Stop weapon trails
		if (section == AnimationSections_Right_Arm_Hand)
		{
			if (m_pRightWeapon != nullptr)
			{
				m_pRightWeapon->StopWeaponTrails();
			}
		}
		if (section == AnimationSections_Left_Arm_Hand)
		{
			if (m_pLeftWeapon != nullptr)
			{
				m_pLeftWeapon->StopWeaponTrails();
			}
		}
	}
}

int VoxelCharacter::GetCurrentAnimationIndex(AnimationSections section)
{
	if(m_pCharacterAnimator[AnimationSections_FullBody] == nullptr)
	{
		return -1;
	}

	return m_pCharacterAnimator[section]->GetCurrentAnimationIndex();
}

void VoxelCharacter::SetBlendAnimation(AnimationSections section, bool waitForComplete, AnimationSections syncWithSection, const char *lStartAnimationName, const char *lEndAnimationName, float blendTime)
{
	if(m_pCharacterAnimator[AnimationSections_FullBody] == nullptr)
	{
		return;
	}

	if(section == AnimationSections_FullBody)
	{
		bool syncComplete = (m_pCharacterAnimator[syncWithSection]->HasAnimationFinished() || m_pCharacterAnimator[syncWithSection]->HasAnimationLooped());

		for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
		{
			if(waitForComplete == false || (m_pCharacterAnimator[i]->HasAnimationFinished() || m_pCharacterAnimator[i]->HasAnimationLooped()))
			{
				if(syncWithSection == section || syncComplete)
				{
					m_pCharacterAnimator[i]->StartBlendAnimation(lStartAnimationName, lEndAnimationName, blendTime);

					// Stop weapon trails
					if (m_pRightWeapon != nullptr)
					{
						m_pRightWeapon->StopWeaponTrails();
					}
					if (m_pLeftWeapon != nullptr)
					{
						m_pLeftWeapon->StopWeaponTrails();
					}
				}
			}			
		}
	}
	else
	{
		m_pCharacterAnimator[section]->StartBlendAnimation(lStartAnimationName, lEndAnimationName, blendTime);

		// Stop weapon trails
		if (section == AnimationSections_Right_Arm_Hand)
		{
			if (m_pRightWeapon != nullptr)
			{
				m_pRightWeapon->StopWeaponTrails();
			}
		}
		if (section == AnimationSections_Left_Arm_Hand)
		{
			if (m_pLeftWeapon != nullptr)
			{
				m_pLeftWeapon->StopWeaponTrails();
			}
		}
	}
}

void VoxelCharacter::BlendIntoAnimation(AnimationSections section, bool waitForComplete, AnimationSections syncWithSection, const char *lAnimationName, float blendTime)
{
	if(m_pCharacterAnimator[AnimationSections_FullBody] == nullptr)
	{
		return;
	}

	if(section == AnimationSections_FullBody)
	{
		bool syncComplete = (m_pCharacterAnimator[syncWithSection]->HasAnimationFinished() || m_pCharacterAnimator[syncWithSection]->HasAnimationLooped());

		for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
		{
			if(waitForComplete == false || (m_pCharacterAnimator[i]->HasAnimationFinished() || m_pCharacterAnimator[i]->HasAnimationLooped()))
			{
				if(syncWithSection == section || syncComplete)
				{
					m_pCharacterAnimator[i]->BlendIntoAnimation(lAnimationName, blendTime);

					// Stop weapon trails
					if (m_pRightWeapon != nullptr)
					{
						m_pRightWeapon->StopWeaponTrails();
					}
					if (m_pLeftWeapon != nullptr)
					{
						m_pLeftWeapon->StopWeaponTrails();
					}
				}
			}
		}
	}
	else
	{
		if(waitForComplete == false || (m_pCharacterAnimator[section]->HasAnimationFinished() || m_pCharacterAnimator[section]->HasAnimationLooped()))
		{
			if(syncWithSection == section || (m_pCharacterAnimator[syncWithSection]->HasAnimationFinished() || m_pCharacterAnimator[syncWithSection]->HasAnimationLooped()))
			{
				m_pCharacterAnimator[section]->BlendIntoAnimation(lAnimationName, blendTime);

				// Stop weapon trails
				if (section == AnimationSections_Right_Arm_Hand)
				{
					if (m_pRightWeapon != nullptr)
					{
						m_pRightWeapon->StopWeaponTrails();
					}
				}
				if (section == AnimationSections_Left_Arm_Hand)
				{
					if (m_pLeftWeapon != nullptr)
					{
						m_pLeftWeapon->StopWeaponTrails();
					}
				}
			}
		}
	}
}

bool VoxelCharacter::HasAnimationFinished(AnimationSections section) const
{
	if(m_pCharacterAnimator[AnimationSections_FullBody] == nullptr)
	{
		return false;
	}

	return m_pCharacterAnimator[section]->HasAnimationFinished();
}

bool VoxelCharacter::HasAnimationLooped(AnimationSections section) const
{
	if(m_pCharacterAnimator[AnimationSections_FullBody] == nullptr)
	{
		return false;
	}

	return m_pCharacterAnimator[section]->HasAnimationLooped();
}

void VoxelCharacter::StepAnimationFrame(float dt)
{
	for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
	{
		if(m_pCharacterAnimator[i] != nullptr)
		{
			m_pCharacterAnimator[i]->Update(dt);
		}
	}
}

int VoxelCharacter::GetStartFrame(const char *lAnimationName) const
{
	return m_pCharacterAnimator[AnimationSections_FullBody]->GetStartFrame(lAnimationName);
}

int VoxelCharacter::GetEndFrame(const char *lAnimationName) const
{
	return m_pCharacterAnimator[AnimationSections_FullBody]->GetEndFrame(lAnimationName);
}

int VoxelCharacter::GetCurrentFrame() const
{
	return m_pCharacterAnimator[AnimationSections_FullBody]->GetCurrentFrame();
}

int VoxelCharacter::GetNumJoints() const
{
	return m_pCharacterModel->GetNumJoints();
}

const Joint* VoxelCharacter::GetJoint(int index) const
{
	return m_pCharacterModel->GetJoint(index);
}

const Joint* VoxelCharacter::GetJoint(const char* jointName) const
{
	return m_pCharacterModel->GetJoint(jointName);
}

void VoxelCharacter::PlayAnimationOnPaperDoll(const char *lAnimationName, bool left)
{
	if (left)
	{
		m_pCharacterAnimatorPaperdoll_Left->PlayAnimation(lAnimationName);
	}
	else
	{
		m_pCharacterAnimatorPaperdoll_Right->PlayAnimation(lAnimationName);
	}
}

// Matrices
int VoxelCharacter::GetNumModelMatrices() const
{
	return m_pVoxelModel->GetNumMatrices();
}

const char* VoxelCharacter::GetModelMatrixName(int index) const
{
	return m_pVoxelModel->GetMatrixName(index);
}

// Swapping and adding new matrices
void VoxelCharacter::SwapBodyPart(const char* bodyPartName, QubicleMatrix* pMatrix, bool copyMatrixParams)
{
	m_pVoxelModel->SwapMatrix(bodyPartName, pMatrix, copyMatrixParams);
}

void VoxelCharacter::AddQubicleMatrix(QubicleMatrix* pNewMatrix, bool copyMatrixParams)
{
	m_pVoxelModel->AddQubicleMatrix(pNewMatrix, copyMatrixParams);
}

void VoxelCharacter::RemoveQubicleMatrix(const char* matrixName)
{
	m_pVoxelModel->RemoveQubicleMatrix(matrixName);
}

void VoxelCharacter::SetQubicleMatrixRender(const char* matrixName, bool render)
{
	m_pVoxelModel->SetQubicleMatrixRender(matrixName, render);
}

string VoxelCharacter::GetSubSelectionName(int pickingId) const
{
	return m_pVoxelModel->GetSubSelectionName(pickingId);
}

// Update
void VoxelCharacter::Update(float dt, float animationSpeed[AnimationSections_NUMSECTIONS])
{
	if(m_loaded == false)
	{
		return;
	}

	// Update skeleton animation
	for(int i = 0; i < AnimationSections_NUMSECTIONS; i++)
	{
		if(m_pCharacterAnimator[i] != nullptr)
		{
			if(m_updateAnimator)
			{
				m_pCharacterAnimator[i]->Update(dt * animationSpeed[i]);
			}
		}
	}

	// Update paperdoll animator
	if(m_updateAnimator)
	{
		if (m_pCharacterAnimatorPaperdoll_Left != nullptr)
		{
			m_pCharacterAnimatorPaperdoll_Left->Update(dt);
		}
		if (m_pCharacterAnimatorPaperdoll_Right != nullptr)
		{
			m_pCharacterAnimatorPaperdoll_Right->Update(dt);
		}
	}

	// Breathing animations
	if(m_bBreathingAnimationEnabled && m_bBreathingAnimationStarted == false)
	{
		if(m_breathingAnimationInitialWaitTime <= 0.0f) // So we have an initial delay, do all characters are not in sync
		{
			StartBreathAnimation();
		}
		else
		{
			m_breathingAnimationInitialWaitTime -= dt;
		}
	}

	// Facial animation
	if(m_loadedFaces)
	{
		if(m_bWinkAnimationEnabled || m_wink == true)
		{
			UpdateWinkAnimation(dt);
		}

		if(m_bTalkingAnimationEnabled)
		{
			UpdateTalkingAnimation(dt);
		}
	}

	// Face looking
	if(glm::distance(m_faceLookingDirection, m_faceTargetDirection) <= 0.01f)
	{
		if(m_bRandomLookDirectionEnabled)
		{
			m_faceTargetDirection = vec3(GetRandomNumber(-1, 1, 2)*0.65f, GetRandomNumber(-1, 1, 2)*0.175f, GetRandomNumber(0, 3, 2)+0.35f);
			m_faceTargetDirection = normalize(m_faceTargetDirection);
		}
	}
	else
	{
		vec3 toTarget = m_faceTargetDirection - m_faceLookingDirection;
		m_faceLookingDirection += (toTarget * dt) * m_faceLookToTargetSpeedMultiplier;
		m_faceLookingDirection = normalize(m_faceLookingDirection);
	}

	// Animated weapons
	if(m_pLeftWeapon != nullptr)
	{
		if(m_leftWeaponLoaded)
		{
			m_pLeftWeapon->Update(dt);
		}
	}

	if(m_pRightWeapon != nullptr)
	{
		if(m_rightWeaponLoaded)
		{
			m_pRightWeapon->Update(dt);
		}
	}

	if (m_pLeftWeapon != nullptr)
	{
		m_pLeftWeapon->CreateWeaponTrailPoint();
	}
	if (m_pRightWeapon != nullptr)
	{
		m_pRightWeapon->CreateWeaponTrailPoint();
	}

	// Update the weapon trails, based on the animation frames
	if (m_pCharacterAnimator[AnimationSections_Left_Arm_Hand] != nullptr)
	{
		bool leftWeaponTrailActive = m_pCharacterAnimator[AnimationSections_Left_Arm_Hand]->GetLeftWeaponTrailActive();
		if (m_pLeftWeapon->IsWeaponTrailsActive() == false && leftWeaponTrailActive == true)
		{
			m_pLeftWeapon->StartWeaponTrails();
		}
		else if (m_pLeftWeapon->IsWeaponTrailsActive() == true && leftWeaponTrailActive == false)
		{
			m_pLeftWeapon->StopWeaponTrails();
		}
	}
	if (m_pCharacterAnimator[AnimationSections_Right_Arm_Hand] != nullptr)
	{
		bool rightWeaponTrailActive = m_pCharacterAnimator[AnimationSections_Right_Arm_Hand]->GetRightWeaponTrailActive();
		if (m_pRightWeapon->IsWeaponTrailsActive() == false && rightWeaponTrailActive == true)
		{
			m_pRightWeapon->StartWeaponTrails();
		}
		else if (m_pRightWeapon->IsWeaponTrailsActive() == true && rightWeaponTrailActive == false)
		{
			m_pRightWeapon->StopWeaponTrails();
		}
	}
}

void VoxelCharacter::SetWeaponTrailsOriginMatrix(float dt, Matrix4x4 originMatrix)
{
	if(m_pLeftWeapon != nullptr)
	{
		if(m_leftWeaponLoaded)
		{
			m_pLeftWeapon->SetWeaponTrailsParams(originMatrix, m_characterScale);
		}
	}

	if(m_pRightWeapon != nullptr)
	{
		if(m_rightWeaponLoaded)
		{
			m_pRightWeapon->SetWeaponTrailsParams(originMatrix, m_characterScale);
		}
	}
}

// Rendering
void VoxelCharacter::Render(bool renderOutline, bool reflection, bool silhouette, Colour OutlineColour, bool subSelectionNamePicking)
{
	if(m_pVoxelModel != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->ScaleWorldMatrix(m_characterScale, m_characterScale, m_characterScale);
			m_pVoxelModel->RenderWithAnimator(m_pCharacterAnimator, this, renderOutline, reflection, silhouette, OutlineColour, subSelectionNamePicking);
		m_pRenderer->PopMatrix();
	}
}

void VoxelCharacter::RenderSubSelection(string subSelection, bool renderOutline, bool silhouette, Colour OutlineColour)
{
	if(m_pVoxelModel != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->ScaleWorldMatrix(m_characterScale, m_characterScale, m_characterScale);
			m_pVoxelModel->RenderSingleMatrix(m_pCharacterAnimator, this, subSelection, renderOutline, silhouette, OutlineColour);
		m_pRenderer->PopMatrix();
	}
}

void VoxelCharacter::RenderBones()
{
	if(m_pVoxelModel != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->ScaleWorldMatrix(m_boneScale.x, m_boneScale.y, m_boneScale.z);
			m_pRenderer->ScaleWorldMatrix(m_characterScale, m_characterScale, m_characterScale);
			m_pRenderer->SetRenderMode(RM_SOLID);
			m_pRenderer->SetLineWidth(3.0f);
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
			m_pCharacterAnimator[AnimationSections_FullBody]->RenderBones();
		m_pRenderer->PopMatrix();
	}
}

void VoxelCharacter::RenderFace()
{
	if(m_loadedFaces == false)
	{
		return;
	}

	if(m_pVoxelModel != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->ScaleWorldMatrix(m_characterScale, m_characterScale, m_characterScale);
			m_pVoxelModel->RenderFace(m_pCharacterAnimator[AnimationSections_Head_Body], this, true);			
		m_pRenderer->PopMatrix();
	}
}

void VoxelCharacter::RenderFacingDebug()
{
	if(m_pVoxelModel != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->ScaleWorldMatrix(m_characterScale, m_characterScale, m_characterScale);

			// Debug render facing vector
			m_pRenderer->PushMatrix();
				Matrix4x4 headMatrix = GetBoneMatrix(AnimationSections_Head_Body, "Head");
				m_pRenderer->MultiplyWorldMatrix(headMatrix);

				// Rotation due to 3dsmax export affecting the bone rotations
				m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f);

				m_pRenderer->EnableMaterial(m_pVoxelModel->GetMaterial());
				m_pRenderer->SetRenderMode(RM_SOLID);
				m_pRenderer->SetLineWidth(3.0f);
				m_pRenderer->EnableImmediateMode(IM_LINES);
					m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 0.0f, 1.0f);
					m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
					m_pRenderer->ImmediateVertex(m_faceLookingDirection.x*15.0f, m_faceLookingDirection.y*15.0f, m_faceLookingDirection.z*15.0f);

					m_pRenderer->ImmediateColourAlpha(0.0f, 1.0f, 0.0f, 1.0f);
					m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
					m_pRenderer->ImmediateVertex(m_faceTargetDirection.x*15.0f, m_faceTargetDirection.y*15.0f, m_faceTargetDirection.z*15.0f);					
				m_pRenderer->DisableImmediateMode();
			m_pRenderer->PopMatrix();

		m_pRenderer->PopMatrix();
	}
}

void VoxelCharacter::RenderFaceTextures(bool eyesTexture, bool wireframe, bool transparency)
{
	if(m_pCharacterModel == nullptr || m_pCharacterAnimator == nullptr)
	{
		return;
	}

	if(m_faceEyesTexture == -1 || m_faceMouthTexture == -1)
	{
		return;
	}

	float texture_w = 1.0f;
	float texture_h = 1.0f;

	float width = 1.0f;
	float height = 1.0f;

	if(eyesTexture)
	{
		width = m_eyesTextureWidth;
		height = m_eyesTextureHeight;
	}
	else
	{
		width = m_mouthTextureWidth;
		height = m_mouthTextureHeight;
	}

	m_pRenderer->PushMatrix();
	{
		if (transparency)
		{
			m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
		}

		// Store cull mode
		CullMode cullMode = m_pRenderer->GetCullMode();

		if (wireframe)
		{
			m_pRenderer->SetLineWidth(1.0f);
			m_pRenderer->SetRenderMode(RM_WIREFRAME);
			m_pRenderer->SetCullMode(CM_NOCULL);
		}
		else
		{
			m_pRenderer->SetRenderMode(RM_TEXTURED);
		}

		if (eyesTexture)
		{
			m_pRenderer->BindTexture(m_faceEyesTexture);
		}
		else
		{
			m_pRenderer->BindTexture(m_faceMouthTexture);
		}

		if (transparency)
		{
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, m_characterAlpha);
		}
		else
		{
			m_pRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);
		}

		m_pRenderer->EnableMaterial(m_pVoxelModel->GetMaterial());

		m_pRenderer->EnableImmediateMode(IM_QUADS);
		{
			//m_pRenderer->ImmediateNormal(0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, texture_h);
			m_pRenderer->ImmediateVertex(0.0f, 0.0f, 0.0f);
			//m_pRenderer->ImmediateNormal(0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(texture_w, texture_h);
			m_pRenderer->ImmediateVertex(width, 0.0f, 0.0f);
			//m_pRenderer->ImmediateNormal(0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(texture_w, 0.0f);
			m_pRenderer->ImmediateVertex(width, height, 0.0f);
			//m_pRenderer->ImmediateNormal(0.0f, 0.0f, 1.0f);
			m_pRenderer->ImmediateTextureCoordinate(0.0f, 0.0f);
			m_pRenderer->ImmediateVertex(0.0f, height, 0.0f);
		}
		m_pRenderer->DisableImmediateMode();
		m_pRenderer->DisableTexture();

		if (transparency)
		{
			m_pRenderer->DisableTransparency();
		}

		// Restore cull mode
		m_pRenderer->SetCullMode(cullMode);
	}
	m_pRenderer->PopMatrix();
}

void VoxelCharacter::RenderWeapons(bool renderOutline, bool reflection, bool silhouette, Colour OutlineColour)
{
	if(m_pLeftWeapon != nullptr)
	{
		if(m_leftWeaponLoaded && m_renderLeftWeapon)
		{
			m_pRenderer->PushMatrix();
				m_pRenderer->ScaleWorldMatrix(m_characterScale, m_characterScale, m_characterScale);
				m_pLeftWeapon->Render(renderOutline, reflection, silhouette, OutlineColour);
			m_pRenderer->PopMatrix();
		}
	}
	if(m_pRightWeapon != nullptr)
	{
		if(m_rightWeaponLoaded && m_renderRightWeapon)
		{
			m_pRenderer->PushMatrix();
				m_pRenderer->ScaleWorldMatrix(m_characterScale, m_characterScale, m_characterScale);
				m_pRightWeapon->Render(renderOutline, reflection, silhouette, OutlineColour);
			m_pRenderer->PopMatrix();
		}
	}
}

void VoxelCharacter::RenderWeaponTrails()
{
	if(m_pLeftWeapon != nullptr)
	{
		if(m_leftWeaponLoaded && m_renderLeftWeapon)
		{
			m_pRenderer->PushMatrix();
				m_pLeftWeapon->RenderWeaponTrails();
			m_pRenderer->PopMatrix();
		}
	}
	if(m_pRightWeapon != nullptr)
	{
		if(m_rightWeaponLoaded && m_renderRightWeapon)
		{
			m_pRenderer->PushMatrix();
				m_pRightWeapon->RenderWeaponTrails();
			m_pRenderer->PopMatrix();
		}
	}
}

void VoxelCharacter::RenderPaperdoll()
{
	if(m_pVoxelModel != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->ScaleWorldMatrix(0.08f, 0.08f, 0.08f);
			m_pVoxelModel->RenderPaperdoll(m_pCharacterAnimatorPaperdoll_Left, m_pCharacterAnimatorPaperdoll_Right, this);
		m_pRenderer->PopMatrix();
	}
}

void VoxelCharacter::RenderPortrait()
{
	if(m_pVoxelModel != nullptr)
	{
		Colour OulineColour(1.0f, 1.0f, 0.0f, 1.0f);

		m_pRenderer->PushMatrix();
			m_pRenderer->ScaleWorldMatrix(0.08f, 0.08f, 0.08f);
			m_pVoxelModel->RenderPortrait(m_pCharacterAnimatorPaperdoll_Right, this, "Head");
			m_pVoxelModel->RenderPortrait(m_pCharacterAnimatorPaperdoll_Right, this, "Helm");
		m_pRenderer->PopMatrix();
	}
}

void VoxelCharacter::RenderFacePaperdoll()
{
	if(m_loadedFaces == false)
	{
		return;
	}

	if(m_pVoxelModel != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->ScaleWorldMatrix(0.08f, 0.08f, 0.08f);
			m_pVoxelModel->RenderFace(m_pCharacterAnimatorPaperdoll_Right, this, true);
		m_pRenderer->PopMatrix();
	}
}

void VoxelCharacter::RenderFacePortrait()
{
	if(m_loadedFaces == false)
	{
		return;
	}

	if(m_pVoxelModel != nullptr)
	{
		m_pRenderer->PushMatrix();
			m_pRenderer->ScaleWorldMatrix(0.08f, 0.08f, 0.08f);
			// NOTE : DON'T scale for portrait, we want a default size
			m_pVoxelModel->RenderFace(m_pCharacterAnimatorPaperdoll_Right, this, true, false, true);
		m_pRenderer->PopMatrix();
	}
}

void VoxelCharacter::RenderWeaponsPaperdoll()
{
	if(m_pLeftWeapon != nullptr)
	{
		if(m_leftWeaponLoaded && m_renderLeftWeapon)
		{
			Colour OutlineColour(1.0f, 1.0f, 0.0f, 1.0f);

			m_pRenderer->PushMatrix();
				m_pRenderer->ScaleWorldMatrix(0.08f, 0.08f, 0.08f);
				m_pLeftWeapon->RenderPaperdoll();
			m_pRenderer->PopMatrix();
		}
	}

	if(m_pRightWeapon != nullptr)
	{
		if(m_rightWeaponLoaded && m_renderRightWeapon)
		{
			Colour OutlineColour(1.0f, 1.0f, 0.0f, 1.0f);

			m_pRenderer->PushMatrix();
				m_pRenderer->ScaleWorldMatrix(0.08f, 0.08f, 0.08f);
				m_pRightWeapon->RenderPaperdoll();
			m_pRenderer->PopMatrix();
		}
	}
}

void VoxelCharacter::_BreathAnimationFinished(void *apData)
{
	VoxelCharacter* lpVoxelCharacter = (VoxelCharacter*)apData;
	lpVoxelCharacter->BreathAnimationFinished();
}

void VoxelCharacter::BreathAnimationFinished()
{
	m_bBreathingAnimationStarted = false;
}