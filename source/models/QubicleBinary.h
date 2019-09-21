// ******************************************************************************
// Filename:    QubicleBinary.h
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//   A Qubicle Binary (.qb) file loader and renderer, handles all the loading of
//   matrices and setting up the render meshes and static draw buffers.
//   Also connected to the Voxel character class, so that bones can be linked with
//   qubicle matrix oibjects, for animation and character rigging.
//
// Revision History:
//   Initial Revision - 10/07/14
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#pragma once

#include "MS3DModel.h"
#include "MS3DAnimator.h"

class VoxelCharacter;

enum MergedSide
{
	MergedSide_None = 0,

	MergedSide_X_Positive = 1,
	MergedSide_X_Negative = 2,
	MergedSide_Y_Positive = 4,
	MergedSide_Y_Negative = 8,
	MergedSide_Z_Positive = 16,
	MergedSide_Z_Negative = 32,
};

bool IsMergedXNegative(int *merged, int x, int y, int z, int width, int height);
bool IsMergedXPositive(int *merged, int x, int y, int z, int width, int height);
bool IsMergedYNegative(int *merged, int x, int y, int z, int width, int height);
bool IsMergedYPositive(int *merged, int x, int y, int z, int width, int height);
bool IsMergedZNegative(int *merged, int x, int y, int z, int width, int height);
bool IsMergedZPositive(int *merged, int x, int y, int z, int width, int height);

class QubicleMatrix
{
public:
	char m_nameLength;
	char* m_name;

	unsigned int m_matrixSizeX;
	unsigned int m_matrixSizeY;
	unsigned int m_matrixSizeZ;

	int m_matrixPosX;
	int m_matrixPosY;
	int m_matrixPosZ;

	unsigned int *m_pColour;

	int m_boneIndex;

	Matrix4x4 m_modelMatrix;

	float m_scale;
	float m_offsetX;
	float m_offsetY;
	float m_offsetZ;

	bool m_removed;

	OpenGLTriangleMesh* m_pMesh;

	void GetColour(int x, int y, int z, float* r, float* g, float* b, float* a) const
	{
		unsigned colour = m_pColour[x + m_matrixSizeX * (y + m_matrixSizeY * z)];
		unsigned int alpha = (colour & 0xFF000000) >> 24;
		unsigned int blue = (colour & 0x00FF0000) >> 16;
		unsigned int green = (colour & 0x0000FF00) >> 8;
		unsigned int red = (colour & 0x000000FF);

		*r = (float)(red / 255.0f);
		*g = (float)(green / 255.0f);
		*b = (float)(blue / 255.0f);

		*a = 1.0f;
	}

	unsigned int GetColourCompact(int x, int y, int z) const
	{
		return m_pColour[x + m_matrixSizeX * (y + m_matrixSizeY * z)];
	}

	bool GetActive(int x, int y, int z) const
	{
		unsigned colour = m_pColour[x + m_matrixSizeX * (y + m_matrixSizeY * z)];
		unsigned int alpha = (colour & 0xFF000000) >> 24;
		unsigned int blue = (colour & 0x00FF0000) >> 16;
		unsigned int green = (colour & 0x0000FF00) >> 8;
		unsigned int red = (colour & 0x000000FF);

		if(alpha == 0)
		{
			return false;
		}

		return true;
	}
};

typedef vector<QubicleMatrix*> QubicleMatrixList;


class QubicleBinary
{
public:
	/* Public methods */
	QubicleBinary(Renderer* pRenderer);
	~QubicleBinary();

	void SetNullLinkage(const QubicleBinary *pBinary);

	void Unload();
	void ClearMatrices();

	void Reset();

	string GetFileName() const;

	unsigned int GetMaterial() const;

	Matrix4x4 GetModelMatrix(int qubicleMatrixIndex);

	int GetMatrixIndexForName(const char* matrixName) const;
	void GetMatrixPosition(int index, int* aX, int* aY, int* aZ) const;

	bool Import(const char* fileName, bool faceMerging);
	bool Export(const char* fileName);

	void GetColour(int matrixIndex, int x, int y, int z, float* r, float* g, float* b, float* a) const;
	unsigned int GetColourCompact(int matrixIndex, int x, int y, int z) const;
	bool GetSingleMeshColour(float* r, float* g, float* b, float* a) const;
	bool GetActive(int matrixIndex, int x, int y, int z) const;

	void SetMeshAlpha(float alpha);
	void SetMeshSingleColour(float r, float g, float b);

	void CreateMesh(bool lDoFaceMerging);
	void RebuildMesh(bool lDoFaceMerging);
	void UpdateMergedSide(int *merged, int matrixIndex, int blockx, int blocky, int blockz, int width, int height, vec3 *p1, vec3 *p2, vec3 *p3, vec3 *p4, int startX, int startY, int maxX, int maxY, bool positive, bool zFace, bool xFace, bool yFace);

	int GetNumMatrices() const;
	QubicleMatrix* SetQubicleMatrix(int index);
	const QubicleMatrix* GetQubicleMatrix(int index) const;
	QubicleMatrix* SetQubicleMatrix(const char* matrixName);
	const QubicleMatrix* GetQubicleMatrix(const char* matrixName) const;
	const char* GetMatrixName(int index) const;
	float GetMatrixScale(int index) const;
	vec3 GetMatrixOffset(int index) const;

	void SetupMatrixBones(MS3DAnimator* pSkeleton);
	
	void SetScaleAndOffsetForMatrix(const char* matrixName, float scale, float xOffset, float yOffset, float zOffset);
	float GetScale(const char* matrixName) const;
	vec3 GetOffset(const char* matrixName) const;

	void SwapMatrix(const char* matrixName, QubicleMatrix* pMatrix, bool copyMatrixParams);
	void AddQubicleMatrix(QubicleMatrix* pNewMatrix, bool copyMatrixParams);
	void RemoveQubicleMatrix(const char* matrixName);
	void SetQubicleMatrixRender(const char* matrixName, bool render);

	// Sub selection
	string GetSubSelectionName(int pickingId) const;

	// Rendering modes
	void SetWireFrameRender(bool wireframe);

	// Update
	void Update(float dt);

	// Rendering
	void Render(bool renderOutline, bool reflection, bool silhouette, Colour OutlineColour);
	void RenderWithAnimator(MS3DAnimator** pSkeleton, VoxelCharacter* pVoxelCharacter, bool renderOutline, bool reflection, bool silhouette, Colour OutlineColour, bool subSelectionNamePicking);
	void RenderSingleMatrix(MS3DAnimator** pSkeleton, VoxelCharacter* pVoxelCharacter, string matrixName, bool renderOutline, bool silhouette, Colour OutlineColour);
	void RenderFace(MS3DAnimator* pSkeleton, VoxelCharacter* pVoxelCharacter, bool transparency, bool useScale = true, bool useTranslate = true);
	void RenderPaperdoll(MS3DAnimator* pSkeleton_Left, MS3DAnimator* pSkeleton_Right, VoxelCharacter* pVoxelCharacter);
	void RenderPortrait(MS3DAnimator* pSkeleton, VoxelCharacter* pVoxelCharacter, string matrixName);

protected:
	/* Protected methods */

private:
	/* Private methods */

public:
	/* Public members */
	static const float BLOCK_RENDER_SIZE;
	static const int SUBSELECTION_NAMEPICKING_OFFSET = 10000000;

protected:
	/* Protected members */

private:
	/* Private members */
	Renderer* m_pRenderer;

	// Loaded flag
	bool m_loaded;

	// Filename
	string m_fileName;

	// Qubicle binary file information
	char m_version[4];
	unsigned int m_colourFormat;
	unsigned int m_zAxisOrientation;
	unsigned int m_compressed;
	unsigned int m_visibilityMaskEncoded;
	unsigned int m_numMatrices;

	// Matrix data for file
	QubicleMatrixList m_vpMatrices;

	// Render modes
	bool m_renderWireFrame;

	// Alpha
	float m_meshAlpha;

	// Single colour
	bool m_singleMeshColour;
	float m_meshSingleColourR;
	float m_meshSingleColourG;
	float m_meshSingleColourB;

	// Material
	unsigned int m_materialID;
};
