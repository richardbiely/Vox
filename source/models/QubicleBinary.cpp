// ******************************************************************************
// Filename:    QubicleBinary.cpp
// Project:     Vox
// Author:      Steven Ball
//
// Purpose:
//
// Revision History:
//   Initial Revision - 10/07/14
//
// Copyright (c) 2005-2015, Steven Ball
// ******************************************************************************

#include "QubicleBinary.h"
#include "VoxelCharacter.h"
#include "../utils/FileUtils.h"

const float QubicleBinary::BLOCK_RENDER_SIZE = 0.5f;


QubicleBinary::QubicleBinary(Renderer* pRenderer)
{
	m_pRenderer = pRenderer;

	Reset();

	m_renderWireFrame = false;

	pRenderer->CreateMaterial(Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(1.0f, 1.0f, 1.0f, 1.0f), Colour(0.0f, 0.0f, 0.0f, 1.0f), 64, &m_materialID);

	float l_length = 0.5f; 
	float l_height = 0.5f;
	float l_width = 0.5f;

	m_meshAlpha = 1.0f;

	m_meshSingleColourR = 1.0f;
	m_meshSingleColourG = 1.0f;
	m_meshSingleColourB = 1.0f;
	m_singleMeshColour = false;
}

QubicleBinary::~QubicleBinary()
{
	Unload();

	Reset();
}

void QubicleBinary::SetNullLinkage(const QubicleBinary *pBinary)
{
	for (int i = 0; i < (int)m_vpMatrices.size(); i++)
	{
		for (int j = 0; j < pBinary->GetNumMatrices(); j++)
		{
			if (m_vpMatrices[i] == pBinary->GetQubicleMatrix(j))
			{
				m_vpMatrices[i] = NULL;
			}
		}
	}
}

void QubicleBinary::Unload()
{
	if(m_loaded)
	{
		ClearMatrices();
	}
}

void QubicleBinary::ClearMatrices()
{
	for(unsigned int i = 0; i < m_vpMatrices.size(); i++)
	{
		if (m_vpMatrices[i] == NULL)
		{
			continue;
		}

		if (m_vpMatrices[i]->m_removed == true)
		{
			continue;
		}

		m_pRenderer->ClearMesh(m_vpMatrices[i]->m_pMesh);
		m_vpMatrices[i]->m_pMesh = NULL;

		delete [] m_vpMatrices[i]->m_pColour;

		delete m_vpMatrices[i];
		m_vpMatrices[i] = 0;
	}
	m_vpMatrices.clear();
}

void QubicleBinary::Reset()
{
	m_version[0] = 0;
	m_version[1] = 0;
	m_version[2] = 0;
	m_version[3] = 0;
	m_colourFormat = 0;
	m_zAxisOrientation = 0;
	m_compressed = 0;
	m_visibilityMaskEncoded = 0;
	m_numMatrices = 0;

	m_loaded = false;

	ClearMatrices();

	m_renderWireFrame = false;
}

string QubicleBinary::GetFileName() const
{
	return m_fileName;
}

unsigned int QubicleBinary::GetMaterial() const
{
	return m_materialID;
}

Matrix4x4 QubicleBinary::GetModelMatrix(int qubicleMatrixIndex)
{
	if(m_loaded == false)
	{
		return Matrix4x4();
	}

	return m_vpMatrices[qubicleMatrixIndex]->m_modelMatrix;
}

int QubicleBinary::GetMatrixIndexForName(const char* matrixName) const
{
	for(unsigned int i = 0; i < m_numMatrices; i++)
	{
		if(strcmp(m_vpMatrices[i]->m_name, matrixName) == 0)
		{
			return i;
		}
	}

	return -1;
}

void QubicleBinary::GetMatrixPosition(int index, int* aX, int* aY, int* aZ) const
{
	*aX = m_vpMatrices[index]->m_matrixPosX;
	*aY = m_vpMatrices[index]->m_matrixPosY;
	*aZ = m_vpMatrices[index]->m_matrixPosZ;
}

bool QubicleBinary::Import(const char* fileName, bool faceMerging)
{
	m_fileName = fileName;

	char qbFilename[256];
	sprintf(qbFilename, "%s", fileName);;

	FILE* pQBfile = NULL;
	fopen_s(&pQBfile, qbFilename, "rb");

	const unsigned int CODEFLAG = 2;
	const unsigned int NEXTSLICEFLAG = 6;

	if (pQBfile != NULL)
	{
		size_t ok = 0;
		ok = fread(&m_version[0], sizeof(char)*4, 1, pQBfile) == 1;
		ok = fread(&m_colourFormat, sizeof(unsigned int), 1, pQBfile) == 1;
		ok = fread(&m_zAxisOrientation, sizeof(unsigned int), 1, pQBfile) == 1;
		ok = fread(&m_compressed, sizeof(unsigned int), 1, pQBfile) == 1;
		ok = fread(&m_visibilityMaskEncoded, sizeof(unsigned int), 1, pQBfile) == 1;
		ok = fread(&m_numMatrices, sizeof(unsigned int), 1, pQBfile) == 1;

		for(unsigned int i = 0; i < m_numMatrices; i++)
		{
			QubicleMatrix* pNewMatrix = new QubicleMatrix();

			ok = fread((char *)&pNewMatrix->m_nameLength, sizeof(char), 1, pQBfile) == 1;
			pNewMatrix->m_name = new char[pNewMatrix->m_nameLength+1];
			ok = fread(&pNewMatrix->m_name[0], sizeof(char)*pNewMatrix->m_nameLength, 1, pQBfile) == 1;
			pNewMatrix->m_name[pNewMatrix->m_nameLength] = 0;

			ok = fread(&pNewMatrix->m_matrixSizeX, sizeof(unsigned int), 1, pQBfile) == 1;
			ok = fread(&pNewMatrix->m_matrixSizeY, sizeof(unsigned int), 1, pQBfile) == 1;
			ok = fread(&pNewMatrix->m_matrixSizeZ, sizeof(unsigned int), 1, pQBfile) == 1;
			
			ok = fread(&pNewMatrix->m_matrixPosX, sizeof(int), 1, pQBfile) == 1;
			ok = fread(&pNewMatrix->m_matrixPosY, sizeof(int), 1, pQBfile) == 1;
			ok = fread(&pNewMatrix->m_matrixPosZ, sizeof(int), 1, pQBfile) == 1;

			pNewMatrix->m_boneIndex = -1;
			pNewMatrix->m_pMesh = NULL;

			pNewMatrix->m_scale = 1.0f;
			pNewMatrix->m_offsetX = 0.0f;
			pNewMatrix->m_offsetY = 0.0f;
			pNewMatrix->m_offsetZ = 0.0f;

			pNewMatrix->m_removed = false;

			pNewMatrix->m_pColour = new unsigned int[pNewMatrix->m_matrixSizeX * pNewMatrix->m_matrixSizeY * pNewMatrix->m_matrixSizeZ];

			if(m_compressed == 0)
			{
				for(unsigned int z = 0; z < pNewMatrix->m_matrixSizeZ; z++)
				{
					for(unsigned int y = 0; y < pNewMatrix->m_matrixSizeY; y++)
					{
						for(unsigned int x = 0; x < pNewMatrix->m_matrixSizeX; x++)
						{
							unsigned int colour = 0;
							ok = fread(&colour, sizeof(unsigned int), 1, pQBfile) == 1;

							pNewMatrix->m_pColour[x + pNewMatrix->m_matrixSizeX * (y + pNewMatrix->m_matrixSizeY * z)] = colour;
						}
					}
				}
			}
			else
			{
				unsigned int z = 0;

				while (z < pNewMatrix->m_matrixSizeZ) 
				{
					unsigned int index = 0;

					while(true)
					{
						unsigned int data = 0;
						ok = fread(&data, sizeof(unsigned int), 1, pQBfile) == 1;

						if (data == NEXTSLICEFLAG)
							break;
						else if (data == CODEFLAG) 
						{
							unsigned int count = 0;
							ok = fread(&count, sizeof(unsigned int), 1, pQBfile) == 1;
							ok = fread(&data, sizeof(unsigned int), 1, pQBfile) == 1;

							for(unsigned int j = 0; j < count; j++) 
							{
								unsigned int x = index % pNewMatrix->m_matrixSizeX;
								unsigned int y = index / pNewMatrix->m_matrixSizeX;

								pNewMatrix->m_pColour[x + pNewMatrix->m_matrixSizeX * (y + pNewMatrix->m_matrixSizeY * z)] = data;

								index++;
							}
						}
						else
						{
							unsigned int x = index % pNewMatrix->m_matrixSizeX;
							unsigned int y = index / pNewMatrix->m_matrixSizeX;

							pNewMatrix->m_pColour[x + pNewMatrix->m_matrixSizeX * (y + pNewMatrix->m_matrixSizeY * z)] = data;

							index++;
						}
					}

					z++;
				}
			}

			m_vpMatrices.push_back(pNewMatrix);
		}

		fclose(pQBfile);

		CreateMesh(faceMerging);

		m_loaded = true;

		return true;
	}

	return false;
}

bool QubicleBinary::Export(const char* fileName)
{
	char qbFilename[256];
	sprintf(qbFilename, "%s", fileName);;

	FILE* pQBfile = NULL;
	fopen_s(&pQBfile, qbFilename, "wb");

	const unsigned int CODEFLAG = 2;
	const unsigned int NEXTSLICEFLAG = 6;

	if (qbFilename != NULL)
	{
		int ok = 0;
		ok = fwrite(&m_version[0], sizeof(char)*4, 1, pQBfile) == 1;
		ok = fwrite(&m_colourFormat, sizeof(unsigned int), 1, pQBfile) == 1;
		ok = fwrite(&m_zAxisOrientation, sizeof(unsigned int), 1, pQBfile) == 1;
		ok = fwrite(&m_compressed, sizeof(unsigned int), 1, pQBfile) == 1;
		ok = fwrite(&m_visibilityMaskEncoded, sizeof(unsigned int), 1, pQBfile) == 1;
		ok = fwrite(&m_numMatrices, sizeof(unsigned int), 1, pQBfile) == 1;

		for(unsigned int i = 0; i < m_numMatrices; i++)
		{
			QubicleMatrix* pMatrix = m_vpMatrices[i];

			ok = fwrite((char *)&pMatrix->m_nameLength, sizeof(char), 1, pQBfile) == 1;
			ok = fwrite(&pMatrix->m_name[0], sizeof(char)*pMatrix->m_nameLength, 1, pQBfile) == 1;

			ok = fwrite(&pMatrix->m_matrixSizeX, sizeof(unsigned int), 1, pQBfile) == 1;
			ok = fwrite(&pMatrix->m_matrixSizeY, sizeof(unsigned int), 1, pQBfile) == 1;
			ok = fwrite(&pMatrix->m_matrixSizeZ, sizeof(unsigned int), 1, pQBfile) == 1;

			ok = fwrite(&pMatrix->m_matrixPosX, sizeof(int), 1, pQBfile) == 1;
			ok = fwrite(&pMatrix->m_matrixPosY, sizeof(int), 1, pQBfile) == 1;
			ok = fwrite(&pMatrix->m_matrixPosZ, sizeof(int), 1, pQBfile) == 1;

			if(m_compressed == 0)
			{
				for(unsigned int z = 0; z < pMatrix->m_matrixSizeZ; z++)
				{
					for(unsigned int y = 0; y < pMatrix->m_matrixSizeY; y++)
					{
						for(unsigned int x = 0; x < pMatrix->m_matrixSizeX; x++)
						{
							unsigned int colour = pMatrix->m_pColour[x + pMatrix->m_matrixSizeX * (y + pMatrix->m_matrixSizeY * z)];
							ok = fwrite(&colour, sizeof(unsigned int), 1, pQBfile) == 1;
						}
					}
				}
			}
			else
			{
				// Simple RLE compression
				unsigned int previousColour = 0;
				int runLength = 0;

				for(unsigned int z = 0; z < pMatrix->m_matrixSizeZ; z++)
				{
					for(unsigned int y = 0; y < pMatrix->m_matrixSizeY; y++)
					{
						for(unsigned int x = 0; x < pMatrix->m_matrixSizeX; x++)
						{
							unsigned int colour = pMatrix->m_pColour[x + pMatrix->m_matrixSizeX * (y + pMatrix->m_matrixSizeY * z)];

							if(x == 0 && y == 0)
							{
								// Start slice
								previousColour = colour;
								runLength = 1;
							}
							else if(x == pMatrix->m_matrixSizeX-1 && y == pMatrix->m_matrixSizeY-1 && z == pMatrix->m_matrixSizeZ-1)
							{
								// End
								ok = fwrite(&CODEFLAG, sizeof(unsigned int), 1, pQBfile) == 1;
								ok = fwrite(&runLength, sizeof(unsigned int), 1, pQBfile) == 1;
								ok = fwrite(&previousColour, sizeof(unsigned int), 1, pQBfile) == 1;

								previousColour = colour;
								runLength = 1;
							}
							else
							{
								if(colour == previousColour)
								{
									// Match
									runLength++;
									continue;
								}
								else
								{
									// No match
									ok = fwrite(&CODEFLAG, sizeof(unsigned int), 1, pQBfile) == 1;
									ok = fwrite(&runLength, sizeof(unsigned int), 1, pQBfile) == 1;
									ok = fwrite(&previousColour, sizeof(unsigned int), 1, pQBfile) == 1;

									previousColour = colour;
									runLength = 1;
								}
							}
						}
					}

					// Next slice
					ok = fwrite(&CODEFLAG, sizeof(unsigned int), 1, pQBfile) == 1;
					ok = fwrite(&runLength, sizeof(unsigned int), 1, pQBfile) == 1;
					ok = fwrite(&previousColour, sizeof(unsigned int), 1, pQBfile) == 1;
					ok = fwrite(&NEXTSLICEFLAG, sizeof(unsigned int), 1, pQBfile) == 1;
				}
			}
		}

		fclose(pQBfile);

		return true;
	}

	return false;
}

void QubicleBinary::GetColour(int matrixIndex, int x, int y, int z, float* r, float* g, float* b, float* a) const
{
	if(m_singleMeshColour)
	{
		*r = m_meshSingleColourR;
		*g = m_meshSingleColourG;
		*b = m_meshSingleColourB;
		*a = 1.0f;
	}
	else
	{
		QubicleMatrix* pMatrix = m_vpMatrices[matrixIndex];

		pMatrix->GetColour(x, y, z, r, g, b, a);
	}	
}

unsigned int QubicleBinary::GetColourCompact(int matrixIndex, int x, int y, int z) const
{
	QubicleMatrix* pMatrix = m_vpMatrices[matrixIndex];

	return pMatrix->GetColourCompact(x, y, z);
}

bool QubicleBinary::GetSingleMeshColour(float* r, float* g, float* b, float* a) const
{
	*r = m_meshSingleColourR;
	*g = m_meshSingleColourG;
	*b = m_meshSingleColourB;
	*a = 1.0f;

	return m_singleMeshColour;
}

bool QubicleBinary::GetActive(int matrixIndex, int x, int y, int z) const
{
	QubicleMatrix* pMatrix = m_vpMatrices[matrixIndex];

	return pMatrix->GetActive(x, y, z);
}

void QubicleBinary::SetMeshAlpha(float alpha)
{
	m_meshAlpha = alpha;

	for(unsigned int i = 0; i < m_vpMatrices.size(); i++)
	{
		m_pRenderer->ModifyMeshAlpha(alpha, m_vpMatrices[i]->m_pMesh);
	}
}

void QubicleBinary::SetMeshSingleColour(float r, float g, float b)
{
	m_singleMeshColour = true;
	m_meshSingleColourR = r;
	m_meshSingleColourG = g;
	m_meshSingleColourB = b;

	for(unsigned int i = 0; i < m_vpMatrices.size(); i++)
	{
		m_pRenderer->ModifyMeshColour(r, g, b, m_vpMatrices[i]->m_pMesh);
	}
}


bool IsMergedXNegative(int *merged, int x, int y, int z, int width, int height) { return (merged[x + y*width + z*width*height] & MergedSide_X_Negative) == MergedSide_X_Negative; }
bool IsMergedXPositive(int *merged, int x, int y, int z, int width, int height) { return (merged[x + y*width + z*width*height] & MergedSide_X_Positive) == MergedSide_X_Positive; }
bool IsMergedYNegative(int *merged, int x, int y, int z, int width, int height) { return (merged[x + y*width + z*width*height] & MergedSide_Y_Negative) == MergedSide_Y_Negative; }
bool IsMergedYPositive(int *merged, int x, int y, int z, int width, int height) { return (merged[x + y*width + z*width*height] & MergedSide_Y_Positive) == MergedSide_Y_Positive; }
bool IsMergedZNegative(int *merged, int x, int y, int z, int width, int height) { return (merged[x + y*width + z*width*height] & MergedSide_Z_Negative) == MergedSide_Z_Negative; }
bool IsMergedZPositive(int *merged, int x, int y, int z, int width, int height) { return (merged[x + y*width + z*width*height] & MergedSide_Z_Positive) == MergedSide_Z_Positive; }

void QubicleBinary::CreateMesh(bool lDoFaceMerging)
{
	for(unsigned int matrixIndex = 0; matrixIndex < m_vpMatrices.size(); matrixIndex++)
	{
		QubicleMatrix* pMatrix = m_vpMatrices[matrixIndex];

		int *l_merged;

		l_merged = new int[pMatrix->m_matrixSizeX*pMatrix->m_matrixSizeY*pMatrix->m_matrixSizeZ];

		for(unsigned int i = 0; i < pMatrix->m_matrixSizeX*pMatrix->m_matrixSizeY*pMatrix->m_matrixSizeZ; i++)
		{
			l_merged[i] = MergedSide_None;
		}

		if(pMatrix->m_pMesh == NULL)
		{
			pMatrix->m_pMesh = m_pRenderer->CreateMesh(OGLMeshType_Textured);
		}

		float r = 1.0f;
		float g = 1.0f;
		float b = 1.0f;
		float a = 1.0f;	

		for(unsigned int x = 0; x < pMatrix->m_matrixSizeX; x++)
		{
			for(unsigned int y = 0; y < pMatrix->m_matrixSizeY; y++)
			{
				for(unsigned int z = 0; z < pMatrix->m_matrixSizeZ; z++)
				{
					if(GetActive(matrixIndex, x, y, z) == false)
					{
						continue;
					}
					else
					{
						GetColour(matrixIndex, x, y, z, &r, &g, &b, &a);

						a = 1.0f;

						vec3 p1(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						vec3 p2(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						vec3 p3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						vec3 p4(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						vec3 p5(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						vec3 p6(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						vec3 p7(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						vec3 p8(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);

						vec3 n1;
						unsigned int v1, v2, v3, v4;
						unsigned int t1, t2, t3, t4;

						bool doXPositive = (IsMergedXPositive(l_merged, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY) == false);
						bool doXNegative = (IsMergedXNegative(l_merged, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY) == false);
						bool doYPositive = (IsMergedYPositive(l_merged, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY) == false);
						bool doYNegative = (IsMergedYNegative(l_merged, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY) == false);
						bool doZPositive = (IsMergedZPositive(l_merged, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY) == false);
						bool doZNegative = (IsMergedZNegative(l_merged, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY) == false);

						// Front
						if(doZPositive && ((z == pMatrix->m_matrixSizeZ-1) || z < pMatrix->m_matrixSizeZ-1 && GetActive(matrixIndex, x, y, z+1) == false))
						{
							int endX = pMatrix->m_matrixSizeX;
							int endY = pMatrix->m_matrixSizeY;
							
							if (lDoFaceMerging)
							{
								UpdateMergedSide(l_merged, matrixIndex, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY, &p1, &p2, &p3, &p4, x, y, endX, endY, true, true, false, false);
							}

							n1 = vec3(0.0f, 0.0f, 1.0f);
							v1 = m_pRenderer->AddVertexToMesh(p1, n1, r, g, b, a, pMatrix->m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, pMatrix->m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p2, n1, r, g, b, a, pMatrix->m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, pMatrix->m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p3, n1, r, g, b, a, pMatrix->m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, pMatrix->m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p4, n1, r, g, b, a, pMatrix->m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, pMatrix->m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, pMatrix->m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, pMatrix->m_pMesh);
						}

						p1 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p2 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p3 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p4 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p5 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p6 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p7 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p8 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);

						// Back
						if(doZNegative && ((z == 0) || (z > 0 && GetActive(matrixIndex, x, y, z-1) == false)))
						{
							int endX = pMatrix->m_matrixSizeX;
							int endY = pMatrix->m_matrixSizeY;

							if (lDoFaceMerging)
							{
								UpdateMergedSide(l_merged, matrixIndex, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY, &p6, &p5, &p8, &p7, x, y, endX, endY, false, true, false, false);
							}

							n1 = vec3(0.0f, 0.0f, -1.0f);
							v1 = m_pRenderer->AddVertexToMesh(p5, n1, r, g, b, a, pMatrix->m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, pMatrix->m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p6, n1, r, g, b, a, pMatrix->m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, pMatrix->m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p7, n1, r, g, b, a, pMatrix->m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, pMatrix->m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p8, n1, r, g, b, a, pMatrix->m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, pMatrix->m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, pMatrix->m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, pMatrix->m_pMesh);
						}

						p1 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p2 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p3 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p4 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p5 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p6 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p7 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p8 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);

						// Right
						if(doXPositive && ((x == pMatrix->m_matrixSizeX-1) || (x < pMatrix->m_matrixSizeX-1 && GetActive(matrixIndex, x+1, y, z) == false)))
						{
							int endX = pMatrix->m_matrixSizeZ;
							int endY = pMatrix->m_matrixSizeY;

							if (lDoFaceMerging)
							{
								UpdateMergedSide(l_merged, matrixIndex, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY, &p5, &p2, &p3, &p8, z, y, endX, endY, true, false, true, false);
							}

							n1 = vec3(1.0f, 0.0f, 0.0f);
							v1 = m_pRenderer->AddVertexToMesh(p2, n1, r, g, b, a, pMatrix->m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, pMatrix->m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p5, n1, r, g, b, a, pMatrix->m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, pMatrix->m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p8, n1, r, g, b, a, pMatrix->m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, pMatrix->m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p3, n1, r, g, b, a, pMatrix->m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, pMatrix->m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, pMatrix->m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, pMatrix->m_pMesh);
						}

						p1 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p2 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p3 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p4 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p5 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p6 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p7 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p8 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);

						// Left
						if(doXNegative && ((x == 0) || (x > 0 && GetActive(matrixIndex, x-1, y, z) == false)))
						{
							int endX = pMatrix->m_matrixSizeZ;
							int endY = pMatrix->m_matrixSizeY;

							if (lDoFaceMerging)
							{
								UpdateMergedSide(l_merged, matrixIndex, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY, &p6, &p1, &p4, &p7, z, y, endX, endY, false, false, true, false);
							}

							n1 = vec3(-1.0f, 0.0f, 0.0f);
							v1 = m_pRenderer->AddVertexToMesh(p6, n1, r, g, b, a, pMatrix->m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, pMatrix->m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p1, n1, r, g, b, a, pMatrix->m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, pMatrix->m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p4, n1, r, g, b, a, pMatrix->m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, pMatrix->m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p7, n1, r, g, b, a, pMatrix->m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, pMatrix->m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, pMatrix->m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, pMatrix->m_pMesh);
						}

						p1 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p2 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p3 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p4 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p5 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p6 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p7 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p8 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);

						// Top
						if(doYPositive && ((y == pMatrix->m_matrixSizeY-1) || (y < pMatrix->m_matrixSizeY-1 && GetActive(matrixIndex, x, y+1, z) == false)))
						{
							int endX = pMatrix->m_matrixSizeX;
							int endY = pMatrix->m_matrixSizeZ;

							if (lDoFaceMerging)
							{
								UpdateMergedSide(l_merged, matrixIndex, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY, &p7, &p8, &p3, &p4, x, z, endX, endY, true, false, false, true);
							}

							n1 = vec3(0.0f, 1.0f, 0.0f);
							v1 = m_pRenderer->AddVertexToMesh(p4, n1, r, g, b, a, pMatrix->m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, pMatrix->m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p3, n1, r, g, b, a, pMatrix->m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, pMatrix->m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p8, n1, r, g, b, a, pMatrix->m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, pMatrix->m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p7, n1, r, g, b, a, pMatrix->m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, pMatrix->m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, pMatrix->m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, pMatrix->m_pMesh);
						}

						p1 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p2 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p3 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p4 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z+BLOCK_RENDER_SIZE);
						p5 = vec3(x+BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p6 = vec3(x-BLOCK_RENDER_SIZE, y-BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p7 = vec3(x-BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);
						p8 = vec3(x+BLOCK_RENDER_SIZE, y+BLOCK_RENDER_SIZE, z-BLOCK_RENDER_SIZE);

						// Bottom
						if(doYNegative && ((y == 0) || (y > 0 && GetActive(matrixIndex, x, y-1, z) == false)))
						{
							int endX = pMatrix->m_matrixSizeX;
							int endY = pMatrix->m_matrixSizeZ;

							if (lDoFaceMerging)
							{
								UpdateMergedSide(l_merged, matrixIndex, x, y, z, pMatrix->m_matrixSizeX, pMatrix->m_matrixSizeY, &p6, &p5, &p2, &p1, x, z, endX, endY, false, false, false, true);
							}

							n1 = vec3(0.0f, -1.0f, 0.0f);
							v1 = m_pRenderer->AddVertexToMesh(p6, n1, r, g, b, a, pMatrix->m_pMesh);
							t1 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 0.0f, pMatrix->m_pMesh);
							v2 = m_pRenderer->AddVertexToMesh(p5, n1, r, g, b, a, pMatrix->m_pMesh);
							t2 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 0.0f, pMatrix->m_pMesh);
							v3 = m_pRenderer->AddVertexToMesh(p2, n1, r, g, b, a, pMatrix->m_pMesh);
							t3 = m_pRenderer->AddTextureCoordinatesToMesh(1.0f, 1.0f, pMatrix->m_pMesh);
							v4 = m_pRenderer->AddVertexToMesh(p1, n1, r, g, b, a, pMatrix->m_pMesh);
							t4 = m_pRenderer->AddTextureCoordinatesToMesh(0.0f, 1.0f, pMatrix->m_pMesh);

							m_pRenderer->AddTriangleToMesh(v1, v2, v3, pMatrix->m_pMesh);
							m_pRenderer->AddTriangleToMesh(v1, v3, v4, pMatrix->m_pMesh);
						}
					}
				}
			}
		}

		m_pRenderer->FinishMesh(-1, m_materialID, pMatrix->m_pMesh);

		// Delete the merged array
		delete [] l_merged;
	}
}

void QubicleBinary::RebuildMesh(bool lDoFaceMerging)
{
	for (unsigned int i = 0; i < m_vpMatrices.size(); i++)
	{
		m_pRenderer->ClearMesh(m_vpMatrices[i]->m_pMesh);
		m_vpMatrices[i]->m_pMesh = NULL;
	}

	CreateMesh(lDoFaceMerging);
}

void QubicleBinary::UpdateMergedSide(int *merged, int matrixIndex, int blockx, int blocky, int blockz, int width, int height, vec3 *p1, vec3 *p2, vec3 *p3, vec3 *p4, int startX, int startY, int maxX, int maxY, bool positive, bool zFace, bool xFace, bool yFace)
{
	QubicleMatrix* pMatrix = m_vpMatrices[matrixIndex];

	bool doMore = true;
	unsigned int incrementX = 0;
	unsigned int incrementZ = 0;
	unsigned int incrementY = 0;

	int change = 1;
	if(positive == false)
	{
		//change = -1;
	}

	if(zFace || yFace)
	{
		incrementX = 1;
		incrementY = 1;
	}
	if(xFace)
	{
		incrementZ = 1;
		incrementY = 1;
	}

	// 1st phase
	int incrementer = 1;
	while(doMore)
	{
		if(startX + incrementer >= maxX)
		{
			doMore = false;
		}
		else
		{
			bool doPhase1Merge = true;
			float r1, r2, g1, g2, b1, b2, a1, a2;
			GetColour(matrixIndex, blockx, blocky, blockz, &r1, &g1, &b1, &a1);
			GetColour(matrixIndex, blockx + incrementX, blocky, blockz + incrementZ, &r2, &g2, &b2, &a2);
			//if(m_pBlocks[blockx][blocky][blockz].GetBlockType() != m_pBlocks[blockx + incrementX][blocky][blockz + incrementZ].GetBlockType())
			//{
				// Don't do any phase 1 merging if we don't have the same block type.
			//	doPhase1Merge = false;
			//	doMore = false;
			//}
			/*//else*/ if((r1 != r2 || g1 != g2 || b1 != b2 || a1 != a2) /*&& allMerge == false*/)
			{
				// Don't do any phase 1 merging if we don't have the same colour variation
				doPhase1Merge = false;
				doMore = false;
			}
			else
			{
				if((xFace && positive && blockx + incrementX+1 == pMatrix->m_matrixSizeX) ||
				   (xFace && !positive && blockx + incrementX == 0) ||
				   (yFace && positive && blocky+1 == pMatrix->m_matrixSizeY) ||
				   (yFace && !positive && blocky == 0) ||
				   (zFace && positive && blockz + incrementZ+1 == pMatrix->m_matrixSizeZ) ||
				   (zFace && !positive && blockz + incrementZ == 0))
				{
					doPhase1Merge = false;
					doMore = false;
				}
				// Don't do any phase 1 merging if we find an inactive block or already merged block in our path
				else if(xFace && positive && (blockx + incrementX+1) < pMatrix->m_matrixSizeX && GetActive(matrixIndex, blockx + incrementX+1, blocky, blockz + incrementZ) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if(xFace && !positive && (blockx + incrementX) > 0 && GetActive(matrixIndex, blockx + incrementX-1, blocky, blockz + incrementZ) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if(yFace && positive && (blocky+1) < (int)pMatrix->m_matrixSizeY && GetActive(matrixIndex, blockx + incrementX, blocky+1, blockz + incrementZ) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if(yFace && !positive && blocky > 0 && GetActive(matrixIndex, blockx + incrementX, blocky-1, blockz + incrementZ) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if(zFace && positive && (blockz + incrementZ+1) < pMatrix->m_matrixSizeZ && GetActive(matrixIndex, blockx + incrementX, blocky, blockz + incrementZ+1) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if(zFace && !positive && (blockz + incrementZ) > 0 && GetActive(matrixIndex, blockx + incrementX, blocky, blockz + incrementZ-1) == true)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else if(GetActive(matrixIndex, blockx + incrementX, blocky, blockz + incrementZ) == false)
				{
					doPhase1Merge = false;
					doMore = false;
				}
				else
				{
					if(xFace)
					{
						doPhase1Merge = positive ? (IsMergedXPositive(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false) : (IsMergedXNegative(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false);
					}
					if(zFace)
					{
						doPhase1Merge = positive ? (IsMergedZPositive(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false) : (IsMergedZNegative(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false);
					}
					if(yFace)
					{
						doPhase1Merge = positive ? (IsMergedYPositive(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false) : (IsMergedYNegative(merged, blockx + incrementX, blocky, blockz + incrementZ, width, height) == false);
					}
				}

				if(doPhase1Merge)
				{
					if(zFace || yFace)
					{
						(*p2).x += change * (BLOCK_RENDER_SIZE * 2.0f);
						(*p3).x += change * (BLOCK_RENDER_SIZE * 2.0f);
					}
					if(xFace)
					{
						(*p2).z += change * (BLOCK_RENDER_SIZE * 2.0f);
						(*p3).z += change * (BLOCK_RENDER_SIZE * 2.0f);
					}

					if(positive)
					{
						if(zFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_Z_Positive;
						}
						if(xFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_X_Positive;
						}
						if(yFace)
						{
							merged[(blockx + incrementX)+ blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_Y_Positive;
						}
					}
					else
					{
						if(zFace)
						{
							merged[(blockx + incrementX)+ blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_Z_Negative;
						}
						if(xFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_X_Negative;
						}
						if(yFace)
						{
							merged[(blockx + incrementX) + blocky*width + (blockz + incrementZ)*width*height] |= MergedSide_Y_Negative;
						}
					}
				}
				else
				{
					doMore = false;
				}
			}
		}

		if(zFace || yFace)
		{
			incrementX += change;
		}
		if(xFace)
		{
			incrementZ += change;
		}

		incrementer += change;
	}


	// 2nd phase
	int loop = incrementer;
	incrementer = incrementY;

	doMore = true;
	while(doMore)
	{
		if(startY + incrementer >= maxY)
		{
			doMore = false;
		}
		else
		{
			for(int i = 0; i < loop-1; i++)
			{
				// Don't do any phase 2 merging is we have any inactive blocks or already merged blocks on the row
				if(zFace)
				{
					float r1, r2, g1, g2, b1, b2, a1, a2;
					GetColour(matrixIndex, blockx, blocky, blockz, &r1, &g1, &b1, &a1);
					GetColour(matrixIndex, blockx + i, blocky + incrementY, blockz, &r2, &g2, &b2, &a2);

					if(positive && (blockz+1) < (int)pMatrix->m_matrixSizeZ && GetActive(matrixIndex, blockx + i, blocky + incrementY, blockz+1) == true)
					{
						doMore = false;
					}
					else if(!positive && blockz > 0 && GetActive(matrixIndex, blockx + i, blocky + incrementY, blockz-1) == true)
					{
						doMore = false;
					}
					else if(GetActive(matrixIndex, blockx + i, blocky + incrementY, blockz) == false || (positive ? (IsMergedZPositive(merged, blockx + i, blocky + incrementY, blockz, width, height) == true) : (IsMergedZNegative(merged, blockx + i, blocky + incrementY, blockz, width, height) == true)))
					{
						// Failed active or already merged check
						doMore = false;
					}
					/*else if(m_pBlocks[blockx][blocky][blockz].GetBlockType() != m_pBlocks[blockx + i][blocky + incrementY][blockz].GetBlockType())
					{
						// Failed block type check
						doMore = false;
					}
					*/
					else if((r1 != r2 || g1 != g2 || b1 != b2 || a1 != a2) /*&& allMerge == false*/)
					{
						// Failed colour check
						doMore = false;
					}
				}
				if(xFace)
				{
					float r1, r2, g1, g2, b1, b2, a1, a2;
					GetColour(matrixIndex, blockx, blocky, blockz, &r1, &g1, &b1, &a1);
					GetColour(matrixIndex, blockx, blocky + incrementY, blockz + i, &r2, &g2, &b2, &a2);

					if(positive && (blockx+1) < (int)pMatrix->m_matrixSizeX && GetActive(matrixIndex, blockx+1, blocky + incrementY, blockz + i) == true)
					{
						doMore = false;
					}
					else if(!positive && (blockx) > 0 && GetActive(matrixIndex, blockx-1, blocky + incrementY, blockz + i) == true)
					{
						doMore = false;
					}
					else if(GetActive(matrixIndex, blockx, blocky + incrementY, blockz + i) == false || (positive ? (IsMergedXPositive(merged, blockx, blocky + incrementY, blockz + i, width, height) == true) : (IsMergedXNegative(merged, blockx, blocky + incrementY, blockz + i, width, height) == true)))
					{
						// Failed active or already merged check
						doMore = false;
					}
					/*else if(m_pBlocks[blockx][blocky][blockz].GetBlockType() != m_pBlocks[blockx][blocky + incrementY][blockz + i].GetBlockType())
					{
						// Failed block type check
						doMore = false;
					}
					*/
					else if((r1 != r2 || g1 != g2 || b1 != b2 || a1 != a2) /*&& allMerge == false*/)
					{
						// Failed colour check
						doMore = false;
					}
				}
				if(yFace)
				{
					float r1, r2, g1, g2, b1, b2, a1, a2;
					GetColour(matrixIndex, blockx, blocky, blockz, &r1, &g1, &b1, &a1);
					GetColour(matrixIndex, blockx + i, blocky, blockz + incrementY, &r2, &g2, &b2, &a2);

					if(positive && (blocky+1) < (int)pMatrix->m_matrixSizeY && GetActive(matrixIndex, blockx + i, blocky+1, blockz + incrementY) == true)
					{
						doMore = false;
					}
					else if(!positive && blocky > 0 && GetActive(matrixIndex, blockx + i, blocky-1, blockz + incrementY) == true)
					{
						doMore = false;
					}
					else if(GetActive(matrixIndex, blockx + i, blocky, blockz + incrementY) == false || (positive ? (IsMergedYPositive(merged, blockx + i, blocky, blockz + incrementY, width, height) == true) : (IsMergedYNegative(merged, blockx + i, blocky, blockz + incrementY, width, height) == true)))
					{
						// Failed active or already merged check
						doMore = false;
					}
					/*else if(m_pBlocks[blockx][blocky][blockz].GetBlockType() != m_pBlocks[blockx + i][blocky][blockz + incrementY].GetBlockType())
					{
						// Failed block type check
						doMore = false;
					}
					*/
					else if((r1 != r2 || g1 != g2 || b1 != b2 || a1 != a2) /*&& allMerge == false*/)
					{
						// Failed colour check
						doMore = false;
					}
				}
			}

			if(doMore == true)
			{
				if(zFace || xFace)
				{
					(*p3).y += change * (BLOCK_RENDER_SIZE * 2.0f);
					(*p4).y += change * (BLOCK_RENDER_SIZE * 2.0f);
				}
				if(yFace)
				{
					(*p3).z += change * (BLOCK_RENDER_SIZE * 2.0f);
					(*p4).z += change * (BLOCK_RENDER_SIZE * 2.0f);
				}

				for(int i = 0; i < loop-1; i++)
				{
					if(positive)
					{
						if(zFace)
						{
							merged[(blockx + i) + (blocky + incrementY)*width + blockz*width*height] |= MergedSide_Z_Positive;
						}
						if(xFace)
						{
							merged[blockx + (blocky + incrementY)*width + (blockz + i)*width*height] |= MergedSide_X_Positive;
						}
						if(yFace)
						{
							merged[(blockx + i) + blocky*width + (blockz + incrementY)*width*height] |= MergedSide_Y_Positive;
						}
					}
					else
					{
						if(zFace)
						{
							merged[(blockx + i) + (blocky + incrementY)*width + blockz*width*height] |= MergedSide_Z_Negative;
						}
						if(xFace)
						{
							merged[blockx + (blocky + incrementY)*width + (blockz + i)*width*height] |= MergedSide_X_Negative;
						}
						if(yFace)
						{
							merged[(blockx + i) + blocky*width + (blockz + incrementY)*width*height] |= MergedSide_Y_Negative;
						}
					}
				}
			}
		}

		incrementY += change;
		incrementer += change;
	}
}

int QubicleBinary::GetNumMatrices() const
{
	return m_numMatrices;
}

const QubicleMatrix* QubicleBinary::GetQubicleMatrix(int index) const
{
	return m_vpMatrices[index];
}

QubicleMatrix* QubicleBinary::SetQubicleMatrix(int index)
{
	return m_vpMatrices[index];
}

const QubicleMatrix* QubicleBinary::GetQubicleMatrix(const char* matrixName) const
{
	for(unsigned int i = 0; i < m_numMatrices; i++)
	{
		if(strcmp(m_vpMatrices[i]->m_name, matrixName) == 0)
		{
			return GetQubicleMatrix(i);
		}
	}

	return NULL;
}

QubicleMatrix* QubicleBinary::SetQubicleMatrix(const char* matrixName)
{
	for (unsigned int i = 0; i < m_numMatrices; i++)
	{
		if (strcmp(m_vpMatrices[i]->m_name, matrixName) == 0)
		{
			return SetQubicleMatrix(i);
		}
	}

	return NULL;
}

const char* QubicleBinary::GetMatrixName(int index) const
{
	return m_vpMatrices[index]->m_name;
}

float QubicleBinary::GetMatrixScale(int index) const
{
	return m_vpMatrices[index]->m_scale;
}

vec3 QubicleBinary::GetMatrixOffset(int index) const
{
	return vec3(m_vpMatrices[index]->m_offsetX, m_vpMatrices[index]->m_offsetY, m_vpMatrices[index]->m_offsetZ);
}

void QubicleBinary::SetupMatrixBones(MS3DAnimator* pSkeleton)
{
	for(unsigned int i = 0; i < m_numMatrices; i++)
	{
		int boneIndex = pSkeleton->GetModel()->GetBoneIndex(m_vpMatrices[i]->m_name);

		if(boneIndex != -1)
		{
			m_vpMatrices[i]->m_boneIndex = boneIndex;
		}
	}
}

void QubicleBinary::SetScaleAndOffsetForMatrix(const char* matrixName, float scale, float xOffset, float yOffset, float zOffset)
{
	for(unsigned int i = 0; i < m_numMatrices; i++)
	{
		if(strcmp(m_vpMatrices[i]->m_name, matrixName) == 0)
		{
			m_vpMatrices[i]->m_scale = scale;
			m_vpMatrices[i]->m_offsetX = xOffset;
			m_vpMatrices[i]->m_offsetY = yOffset;
			m_vpMatrices[i]->m_offsetZ = zOffset;
		}
	}
}

float QubicleBinary::GetScale(const char* matrixName) const
{
	for(unsigned int i = 0; i < m_numMatrices; i++)
	{
		if(strcmp(m_vpMatrices[i]->m_name, matrixName) == 0)
		{
			return m_vpMatrices[i]->m_scale;
		}
	}

	return 1.0f;
}

vec3 QubicleBinary::GetOffset(const char* matrixName) const
{
	for(unsigned int i = 0; i < m_numMatrices; i++)
	{
		if(strcmp(m_vpMatrices[i]->m_name, matrixName) == 0)
		{
			return vec3(m_vpMatrices[i]->m_offsetX, m_vpMatrices[i]->m_offsetY, m_vpMatrices[i]->m_offsetZ);
		}
	}

	return vec3(0.0f, 0.0f, 0.0f);
}

void QubicleBinary::SwapMatrix(const char* matrixName, QubicleMatrix* pMatrix, bool copyMatrixParams)
{
	int matrixIndex = GetMatrixIndexForName(matrixName);

	if(matrixIndex != -1)
	{
		if(copyMatrixParams)
		{
			pMatrix->m_nameLength = m_vpMatrices[matrixIndex]->m_nameLength;
			pMatrix->m_name = m_vpMatrices[matrixIndex]->m_name;
			pMatrix->m_boneIndex = m_vpMatrices[matrixIndex]->m_boneIndex;
			pMatrix->m_scale = m_vpMatrices[matrixIndex]->m_scale;
			pMatrix->m_offsetX = m_vpMatrices[matrixIndex]->m_offsetX;
			pMatrix->m_offsetY = m_vpMatrices[matrixIndex]->m_offsetY;
			pMatrix->m_offsetZ = m_vpMatrices[matrixIndex]->m_offsetZ;
		}

		m_vpMatrices[matrixIndex]->m_removed = false;
		m_vpMatrices[matrixIndex] = pMatrix;
	}
}

void QubicleBinary::AddQubicleMatrix(QubicleMatrix* pNewMatrix, bool copyMatrixParams)
{
	// First check if this matrix already exists
	const QubicleMatrix* pExistingMatrix = GetQubicleMatrix(pNewMatrix->m_name);
	if(pExistingMatrix != NULL)
	{
		// Replace existing matrix
		SwapMatrix(pNewMatrix->m_name, pNewMatrix, copyMatrixParams);
	}
	else
	{
		// Add new matrix
		m_vpMatrices.push_back(pNewMatrix);
		pNewMatrix->m_removed = false;
		m_numMatrices++;
	}
}

void QubicleBinary::RemoveQubicleMatrix(const char* matrixName)
{
	for(unsigned int i = 0; i < m_vpMatrices.size(); i++)
	{
		if(strcmp(m_vpMatrices[i]->m_name, matrixName) == 0)
		{
			m_vpMatrices[i]->m_removed = true;

			return;
		}
	}
}

void QubicleBinary::SetQubicleMatrixRender(const char* matrixName, bool render)
{
	for(unsigned int i = 0; i < m_vpMatrices.size(); i++)
	{
		if(strcmp(m_vpMatrices[i]->m_name, matrixName) == 0)
		{
			m_vpMatrices[i]->m_removed = (render == false);

			return;
		}
	}
}

// Sub selection
string QubicleBinary::GetSubSelectionName(int pickingId) const
{
	int index = pickingId - SUBSELECTION_NAMEPICKING_OFFSET;

	if(index >= 0 && index <= (int)m_numMatrices-1)
	{
		return m_vpMatrices[index]->m_name;
	}

	return "";
}

// Rendering modes
void QubicleBinary::SetWireFrameRender(bool wireframe)
{
	m_renderWireFrame = wireframe;
}

// Update
void QubicleBinary::Update(float dt)
{

}

//Rendering
void QubicleBinary::Render(bool renderOutline, bool reflection, bool silhouette, Colour OutlineColour)
{
	m_pRenderer->PushMatrix();
		for(unsigned int i = 0; i < m_numMatrices; i++)
		{
			if(m_vpMatrices[i]->m_removed == true)
			{
				continue;
			}

			m_pRenderer->PushMatrix();
				// Scale for external matrix scale value
				m_pRenderer->ScaleWorldMatrix(m_vpMatrices[i]->m_scale, m_vpMatrices[i]->m_scale, m_vpMatrices[i]->m_scale);

				// Translate for initial block offset
				m_pRenderer->TranslateWorldMatrix(0.5f, 0.5f, 0.5f);

				// Translate to center of model
				m_pRenderer->TranslateWorldMatrix(-(float)m_vpMatrices[i]->m_matrixSizeX*0.5f, -(float)m_vpMatrices[i]->m_matrixSizeY*0.5f, -(float)m_vpMatrices[i]->m_matrixSizeZ*0.5f);
				
				// Translate for external matrix offset value
				m_pRenderer->TranslateWorldMatrix(m_vpMatrices[i]->m_offsetX, m_vpMatrices[i]->m_offsetY, m_vpMatrices[i]->m_offsetZ);

				// Store cull mode
				CullMode cullMode = m_pRenderer->GetCullMode();

				if(renderOutline)
				{
					m_pRenderer->DisableDepthTest();
					m_pRenderer->SetLineWidth(3.0f);
					m_pRenderer->SetCullMode(CM_FRONT);
					m_pRenderer->SetRenderMode(RM_WIREFRAME);
					m_pRenderer->ImmediateColourAlpha(OutlineColour.GetRed(), OutlineColour.GetGreen(), OutlineColour.GetBlue(), OutlineColour.GetAlpha());
				}
				else if(silhouette)
				{
					m_pRenderer->DisableDepthTest();
					m_pRenderer->SetCullMode(CM_FRONT);
					m_pRenderer->SetRenderMode(RM_SOLID);
					m_pRenderer->ImmediateColourAlpha(OutlineColour.GetRed(), OutlineColour.GetGreen(), OutlineColour.GetBlue(), OutlineColour.GetAlpha());
				}
				else if(m_renderWireFrame)
				{
					m_pRenderer->SetLineWidth(1.0f);
					m_pRenderer->SetRenderMode(RM_WIREFRAME);
					m_pRenderer->SetCullMode(CM_NOCULL);
				}
				else
				{
					m_pRenderer->SetRenderMode(RM_SOLID);
				}

				// Store the model matrix
				if(reflection == false)
				{
					m_pRenderer->GetModelMatrix(&m_vpMatrices[i]->m_modelMatrix);
				}

				m_pRenderer->PushMatrix();
					m_pRenderer->StartMeshRender();

					// Texture manipulation (for shadow rendering)
					{
						Matrix4x4 worldMatrix;
						m_pRenderer->GetModelMatrix(&worldMatrix);

						m_pRenderer->PushTextureMatrix();
						m_pRenderer->MultiplyWorldMatrix(worldMatrix);
					}

					if(m_meshAlpha < 1.0f)
					{
						m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
					}
					m_pRenderer->EnableMaterial(m_materialID);

					if(renderOutline || silhouette)
					{
						m_pRenderer->EndMeshRender();
						m_pRenderer->RenderMesh_NoColour(m_vpMatrices[i]->m_pMesh);
					}
					else
					{
						m_pRenderer->MeshStaticBufferRender(m_vpMatrices[i]->m_pMesh);
					}

					if (m_meshAlpha < 1.0f)
					{
						m_pRenderer->DisableTransparency();
					}

					// Texture manipulation (for shadow rendering)
					{
						m_pRenderer->PopTextureMatrix();
					}
					m_pRenderer->EndMeshRender();
				m_pRenderer->PopMatrix();

				// Restore cull mode
				m_pRenderer->SetCullMode(cullMode);

				if(renderOutline || silhouette)
				{
					m_pRenderer->EnableDepthTest(DT_LESS);
				}
			m_pRenderer->PopMatrix();
		}
	m_pRenderer->PopMatrix();
}

void QubicleBinary::RenderWithAnimator(MS3DAnimator** pSkeleton, VoxelCharacter* pVoxelCharacter, bool renderOutline, bool reflection, bool silhouette, Colour OutlineColour, bool subSelectionNamePicking)
{
	if(pVoxelCharacter == NULL)
	{
		return;
	}

	m_pRenderer->PushMatrix();
		m_pRenderer->StartMeshRender();

		for(unsigned int i = 0; i < m_numMatrices; i++)
		{
			if(m_vpMatrices[i]->m_removed == true)
			{
				continue;
			}

			if(subSelectionNamePicking)
			{
				m_pRenderer->LoadNameOntoStack(SUBSELECTION_NAMEPICKING_OFFSET + i);
			}

			m_pRenderer->PushMatrix();
				MS3DAnimator* pSkeletonToUse = pSkeleton[AnimationSections_FullBody];			
				if(m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetHeadBoneIndex() ||
				   m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetBodyBoneIndex())
				{
					pSkeletonToUse = pSkeleton[AnimationSections_Head_Body];

					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
					m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation()*0.65f, 0.0f, 0.0f);
					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
				}
				else if(m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLeftShoulderBoneIndex() ||
						m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLeftHandBoneIndex())
				{
					pSkeletonToUse = pSkeleton[AnimationSections_Left_Arm_Hand];

					//m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
					m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation(), 0.0f, 0.0f);
					//m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
				}
				else if(m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetRightShoulderBoneIndex() ||
						m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetRightHandBoneIndex())
				{
					pSkeletonToUse = pSkeleton[AnimationSections_Right_Arm_Hand];

					//m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
					m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation(), 0.0f, 0.0f);
					//m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
				}
				else if(m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLegsBoneIndex() ||
						m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetRightFootBoneIndex() ||
						m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLeftFootBoneIndex())
				{
					pSkeletonToUse = pSkeleton[AnimationSections_Legs_Feet];

					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
					m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation()*0.3f, 0.0f, 0.0f);
					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
				}

				m_pRenderer->PushMatrix();
					// Breathing animation
					if(pVoxelCharacter->IsBreathingAnimationStarted())
					{
						float offsetAmount = 0.0f;
						if(m_vpMatrices[i]->m_boneIndex != -1)
						{
							offsetAmount = pVoxelCharacter->GetBreathingAnimationOffsetForBone(m_vpMatrices[i]->m_boneIndex);
						}

						m_pRenderer->TranslateWorldMatrix(0.0f, offsetAmount, 0.0f);
					}

					// Body and hands/shoulders looking direction
					if(m_vpMatrices[i]->m_boneIndex != -1)
					{
						if( m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetBodyBoneIndex() ||
							m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLeftShoulderBoneIndex() ||
							m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLeftHandBoneIndex() ||
							m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetRightShoulderBoneIndex() ||
							m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetRightHandBoneIndex() )
						{
							vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
							lForward.y = 0.0f;
							lForward = normalize(lForward);
							vec3 forwardDiff = lForward - vec3(0.0f, 0.0f, 1.0f);
							lForward = normalize(vec3(0.0f, 0.0f, 1.0f) + (forwardDiff*0.5f));

							vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
							vec3 lRight = normalize(cross(lUp, lForward));
							lUp = normalize(cross(lForward, lRight));

							float lMatrix[16] =
							{
								lRight.x, lRight.y, lRight.z, 0.0f,
								lUp.x, lUp.y, lUp.z, 0.0f,
								lForward.x, lForward.y, lForward.z, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f
							};
							Matrix4x4 lookingMat;
							lookingMat.SetValues(lMatrix);
							m_pRenderer->MultiplyWorldMatrix(lookingMat);
						}
					}

					// Translate by attached bone matrix
					if(m_vpMatrices[i]->m_boneIndex != -1)
					{
						Matrix4x4 boneMatrix = pSkeletonToUse->GetBoneMatrix(m_vpMatrices[i]->m_boneIndex);
						vec3 boneScale = pVoxelCharacter->GetBoneScale();
						m_pRenderer->ScaleWorldMatrix(boneScale.x, boneScale.y, boneScale.z);
						m_pRenderer->MultiplyWorldMatrix(boneMatrix);
						m_pRenderer->ScaleWorldMatrix(1.0f/boneScale.x, 1.0f/boneScale.y, 1.0f/boneScale.z);
					}

					// Rotation due to 3dsmax export affecting the bone rotations
					m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f);

					// Face looking direction
					if(m_vpMatrices[i]->m_boneIndex != -1)
					{
						if(m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetHeadBoneIndex())
						{
							vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
							vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
							vec3 lRight = normalize(cross(lUp, lForward));
							lUp = normalize(cross(lForward, lRight));

							float lMatrix[16] =
							{
								lRight.x, lRight.y, lRight.z, 0.0f,
								lUp.x, lUp.y, lUp.z, 0.0f,
								lForward.x, lForward.y, lForward.z, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f
							};
							Matrix4x4 lookingMat;
							lookingMat.SetValues(lMatrix);
							m_pRenderer->MultiplyWorldMatrix(lookingMat);
						}
					}

					// Scale for external matrix scale value
					m_pRenderer->ScaleWorldMatrix(m_vpMatrices[i]->m_scale, m_vpMatrices[i]->m_scale, m_vpMatrices[i]->m_scale);

					// Translate for initial block offset
					m_pRenderer->TranslateWorldMatrix(0.5f, 0.5f, 0.5f);

					// Translate to center of model
					m_pRenderer->TranslateWorldMatrix(-(float)m_vpMatrices[i]->m_matrixSizeX*0.5f, -(float)m_vpMatrices[i]->m_matrixSizeY*0.5f, -(float)m_vpMatrices[i]->m_matrixSizeZ*0.5f);

					// Translate for external matrix offset value
					m_pRenderer->TranslateWorldMatrix(m_vpMatrices[i]->m_offsetX, m_vpMatrices[i]->m_offsetY, m_vpMatrices[i]->m_offsetZ);

					// Store cull mode
					CullMode cullMode = m_pRenderer->GetCullMode();

					if(renderOutline)
					{
						m_pRenderer->DisableDepthTest();
						m_pRenderer->SetLineWidth(3.0f);
						m_pRenderer->SetCullMode(CM_FRONT);
						m_pRenderer->SetRenderMode(RM_WIREFRAME);
						m_pRenderer->ImmediateColourAlpha(OutlineColour.GetRed(), OutlineColour.GetGreen(), OutlineColour.GetBlue(), OutlineColour.GetAlpha());
					}
					else if(silhouette)
					{
						m_pRenderer->DisableDepthTest();
						m_pRenderer->SetCullMode(CM_FRONT);
						m_pRenderer->SetRenderMode(RM_SOLID);
						m_pRenderer->ImmediateColourAlpha(OutlineColour.GetRed(), OutlineColour.GetGreen(), OutlineColour.GetBlue(), OutlineColour.GetAlpha());
					}
					else if(m_renderWireFrame)
					{
						m_pRenderer->SetLineWidth(1.0f);
						m_pRenderer->SetRenderMode(RM_WIREFRAME);
						m_pRenderer->SetCullMode(CM_NOCULL);
					}
					else
					{
						m_pRenderer->SetRenderMode(RM_SOLID);
					}

					// Store the model matrix
					if(reflection == false)
					{
						m_pRenderer->GetModelMatrix(&m_vpMatrices[i]->m_modelMatrix);
					}

					// Texture manipulation (for shadow rendering)
					{
						Matrix4x4 worldMatrix;
						m_pRenderer->GetModelMatrix(&worldMatrix);

						m_pRenderer->PushTextureMatrix();
						m_pRenderer->MultiplyWorldMatrix(worldMatrix);
					}

					if(m_meshAlpha < 1.0f)
					{
						m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
					}
					m_pRenderer->EnableMaterial(m_materialID);

					if(renderOutline || silhouette)
					{
						m_pRenderer->EndMeshRender();
						m_pRenderer->RenderMesh_NoColour(m_vpMatrices[i]->m_pMesh);
					}
					else
					{
						m_pRenderer->MeshStaticBufferRender(m_vpMatrices[i]->m_pMesh);
					}

					if (m_meshAlpha < 1.0f)
					{
						m_pRenderer->DisableTransparency();
					}

					// Texture manipulation (for shadow rendering)
					{
						m_pRenderer->PopTextureMatrix();
					}

					// Restore cull mode
					m_pRenderer->SetCullMode(cullMode);

					if(renderOutline || silhouette)
					{
						m_pRenderer->EnableDepthTest(DT_LESS);
					}
				m_pRenderer->PopMatrix();

				if(subSelectionNamePicking)
				{
					m_pRenderer->EndNameStack();
				}
			m_pRenderer->PopMatrix();
		}

		m_pRenderer->EndMeshRender();
	m_pRenderer->PopMatrix();
}

void QubicleBinary::RenderSingleMatrix(MS3DAnimator** pSkeleton, VoxelCharacter* pVoxelCharacter, string matrixName, bool renderOutline, bool silhouette, Colour OutlineColour)
{
	if(pVoxelCharacter == NULL)
	{
		return;
	}

	m_pRenderer->PushMatrix();
		m_pRenderer->StartMeshRender();

		int matrixIndex = GetMatrixIndexForName(matrixName.c_str());

		if(matrixIndex != -1 && m_vpMatrices[matrixIndex]->m_removed == false)
		{
			m_pRenderer->PushMatrix();
				MS3DAnimator* pSkeletonToUse = pSkeleton[AnimationSections_FullBody];			
				if(m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetHeadBoneIndex() ||
					m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetBodyBoneIndex())
				{
					pSkeletonToUse = pSkeleton[AnimationSections_Head_Body];

					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
					m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation()*0.65f, 0.0f, 0.0f);
					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
				}
				else if(m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetLeftShoulderBoneIndex() ||
					m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetLeftHandBoneIndex())
				{
					pSkeletonToUse = pSkeleton[AnimationSections_Left_Arm_Hand];

					//m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
					m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation(), 0.0f, 0.0f);
					//m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
				}
				else if(m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetRightShoulderBoneIndex() ||
					m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetRightHandBoneIndex())
				{
					pSkeletonToUse = pSkeleton[AnimationSections_Right_Arm_Hand];

					//m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
					m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation(), 0.0f, 0.0f);
					//m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
				}
				else if(m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetLegsBoneIndex() ||
					m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetRightFootBoneIndex() ||
					m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetLeftFootBoneIndex())
				{
					pSkeletonToUse = pSkeleton[AnimationSections_Legs_Feet];

					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
					m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation()*0.3f, 0.0f, 0.0f);
					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
				}

				// Breathing animation
				if(pVoxelCharacter->IsBreathingAnimationStarted())
				{
					float offsetAmount = 0.0f;
					if(m_vpMatrices[matrixIndex]->m_boneIndex != -1)
					{
						offsetAmount = pVoxelCharacter->GetBreathingAnimationOffsetForBone(m_vpMatrices[matrixIndex]->m_boneIndex);
					}

					m_pRenderer->TranslateWorldMatrix(0.0f, offsetAmount, 0.0f);
				}

				// Body and hands/shoulders looking direction
				if(m_vpMatrices[matrixIndex]->m_boneIndex != -1)
				{
					if( m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetBodyBoneIndex() ||
						m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetLeftShoulderBoneIndex() ||
						m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetLeftHandBoneIndex() ||
						m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetRightShoulderBoneIndex() ||
						m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetRightHandBoneIndex() )
					{
						vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
						lForward.y = 0.0f;
						lForward = normalize(lForward);
						vec3 forwardDiff = lForward - vec3(0.0f, 0.0f, 1.0f);
						lForward = normalize(vec3(0.0f, 0.0f, 1.0f) + (forwardDiff*0.5f));

						vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
						vec3 lRight = normalize(cross(lUp, lForward));
						lUp = normalize(cross(lForward, lRight));

						float lMatrix[16] =
						{
							lRight.x, lRight.y, lRight.z, 0.0f,
							lUp.x, lUp.y, lUp.z, 0.0f,
							lForward.x, lForward.y, lForward.z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f
						};
						Matrix4x4 lookingMat;
						lookingMat.SetValues(lMatrix);
						m_pRenderer->MultiplyWorldMatrix(lookingMat);
					}
				}

				// Translate by attached bone matrix
				if(m_vpMatrices[matrixIndex]->m_boneIndex != -1)
				{
					Matrix4x4 boneMatrix = pSkeletonToUse->GetBoneMatrix(m_vpMatrices[matrixIndex]->m_boneIndex);
					vec3 boneScale = pVoxelCharacter->GetBoneScale();
					m_pRenderer->ScaleWorldMatrix(boneScale.x, boneScale.y, boneScale.z);
					m_pRenderer->MultiplyWorldMatrix(boneMatrix);
					m_pRenderer->ScaleWorldMatrix(1.0f/boneScale.x, 1.0f/boneScale.y, 1.0f/boneScale.z);

					// Rotation due to 3dsmax export affecting the bone rotations
					m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f);
				}

				// Face looking direction
				if(m_vpMatrices[matrixIndex]->m_boneIndex != -1)
				{
					if(m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetHeadBoneIndex())
					{
						vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
						vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
						vec3 lRight = normalize(cross(lUp, lForward));
						lUp = normalize(cross(lForward, lRight));

						float lMatrix[16] =
						{
							lRight.x, lRight.y, lRight.z, 0.0f,
							lUp.x, lUp.y, lUp.z, 0.0f,
							lForward.x, lForward.y, lForward.z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f
						};
						Matrix4x4 lookingMat;
						lookingMat.SetValues(lMatrix);
						m_pRenderer->MultiplyWorldMatrix(lookingMat);
					}
				}

				// Scale for external matrix scale value
				m_pRenderer->ScaleWorldMatrix(m_vpMatrices[matrixIndex]->m_scale, m_vpMatrices[matrixIndex]->m_scale, m_vpMatrices[matrixIndex]->m_scale);

				// Translate for initial block offset
				m_pRenderer->TranslateWorldMatrix(0.5f, 0.5f, 0.5f);

				// Translate to center of model
				m_pRenderer->TranslateWorldMatrix(-(float)m_vpMatrices[matrixIndex]->m_matrixSizeX*0.5f, -(float)m_vpMatrices[matrixIndex]->m_matrixSizeY*0.5f, -(float)m_vpMatrices[matrixIndex]->m_matrixSizeZ*0.5f);

				// Translate for external matrix offset value
				m_pRenderer->TranslateWorldMatrix(m_vpMatrices[matrixIndex]->m_offsetX, m_vpMatrices[matrixIndex]->m_offsetY, m_vpMatrices[matrixIndex]->m_offsetZ);

				// Store cull mode
				CullMode cullMode = m_pRenderer->GetCullMode();

				if(renderOutline)
				{
					m_pRenderer->DisableDepthTest();
					m_pRenderer->SetLineWidth(3.0f);
					m_pRenderer->SetCullMode(CM_FRONT);
					m_pRenderer->SetRenderMode(RM_WIREFRAME);
					m_pRenderer->ImmediateColourAlpha(OutlineColour.GetRed(), OutlineColour.GetGreen(), OutlineColour.GetBlue(), OutlineColour.GetAlpha());
				}
				else if(silhouette)
				{
					m_pRenderer->DisableDepthTest();
					m_pRenderer->SetCullMode(CM_FRONT);
					m_pRenderer->SetRenderMode(RM_SOLID);
					m_pRenderer->ImmediateColourAlpha(OutlineColour.GetRed(), OutlineColour.GetGreen(), OutlineColour.GetBlue(), OutlineColour.GetAlpha());
				}
				else if(m_renderWireFrame)
				{
					m_pRenderer->SetLineWidth(1.0f);
					m_pRenderer->SetRenderMode(RM_WIREFRAME);
					m_pRenderer->SetCullMode(CM_NOCULL);
				}
				else
				{
					m_pRenderer->SetRenderMode(RM_SOLID);
				}

				// Texture manipulation (for shadow rendering)
				{
					Matrix4x4 worldMatrix;
					m_pRenderer->GetModelMatrix(&worldMatrix);

					m_pRenderer->PushTextureMatrix();
					m_pRenderer->MultiplyWorldMatrix(worldMatrix);
				}

				if(m_meshAlpha < 1.0f)
				{
					m_pRenderer->EnableTransparency(BF_SRC_ALPHA, BF_ONE_MINUS_SRC_ALPHA);
				}
				m_pRenderer->EnableMaterial(m_materialID);

				if(renderOutline || silhouette)
				{
					m_pRenderer->EndMeshRender();
					m_pRenderer->RenderMesh_NoColour(m_vpMatrices[matrixIndex]->m_pMesh);
				}
				else
				{
					m_pRenderer->MeshStaticBufferRender(m_vpMatrices[matrixIndex]->m_pMesh);
				}

				if (m_meshAlpha < 1.0f)
				{
					m_pRenderer->DisableTransparency();
				}

				// Texture manipulation (for shadow rendering)
				{
					m_pRenderer->PopTextureMatrix();
				}

				// Restore cull mode
				m_pRenderer->SetCullMode(cullMode);

				if(renderOutline || silhouette)
				{
					m_pRenderer->EnableDepthTest(DT_LESS);
				}
			m_pRenderer->PopMatrix();
		}

		m_pRenderer->EndMeshRender();
	m_pRenderer->PopMatrix();
}

void QubicleBinary::RenderFace(MS3DAnimator* pSkeleton, VoxelCharacter* pVoxelCharacter, bool transparency, bool useScale, bool useTranslate)
{
	if(pVoxelCharacter == NULL)
	{
		return;
	}

	// Render eyes
	{
		int eyesBoneIndex = pVoxelCharacter->GetEyesBone();
		vec3 eyeOffset = pVoxelCharacter->GetEyesOffset();

		if(eyesBoneIndex != -1)
		{
			m_pRenderer->PushMatrix();
				m_pRenderer->EnableMaterial(m_materialID);

				if(useScale)
				{
					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
					m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation()*0.65f, 0.0f, 0.0f);
					m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());

					// Breathing animation
					if(pVoxelCharacter->IsBreathingAnimationStarted())
					{
						float offsetAmount = 0.0f;
						if(eyesBoneIndex != -1)
						{
							offsetAmount = pVoxelCharacter->GetBreathingAnimationOffsetForBone(eyesBoneIndex);
						}

						m_pRenderer->TranslateWorldMatrix(0.0f, offsetAmount, 0.0f);
					}
				}

				// Translate by attached bone matrix
				Matrix4x4 boneMatrix = pSkeleton->GetBoneMatrix(eyesBoneIndex);
				vec3 boneScale = pVoxelCharacter->GetBoneScale();
				m_pRenderer->ScaleWorldMatrix(boneScale.x, boneScale.y, boneScale.z);
				m_pRenderer->MultiplyWorldMatrix(boneMatrix);
				m_pRenderer->ScaleWorldMatrix(1.0f/boneScale.x, 1.0f/boneScale.y, 1.0f/boneScale.z);

				// Rotation due to 3dsmax export affecting the bone rotations
				m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f);

				// Face looking direction
				vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
				vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
				vec3 lRight = normalize(cross(lUp, lForward));
				lUp = normalize(cross(lForward, lRight));

				float lMatrix[16] =
				{
					lRight.x, lRight.y, lRight.z, 0.0f,
					lUp.x, lUp.y, lUp.z, 0.0f,
					lForward.x, lForward.y, lForward.z, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f
				};
				Matrix4x4 lookingMat;
				lookingMat.SetValues(lMatrix);
				m_pRenderer->MultiplyWorldMatrix(lookingMat);

				int eyesMatrixIndex = pVoxelCharacter->GetEyesMatrixIndex();

				if(useScale)
				{
					// Scale for external matrix scale value
					m_pRenderer->ScaleWorldMatrix(m_vpMatrices[eyesMatrixIndex]->m_scale, m_vpMatrices[eyesMatrixIndex]->m_scale, m_vpMatrices[eyesMatrixIndex]->m_scale);
				}
				if(useTranslate)
				{
					// Translate for external matrix offset value
					m_pRenderer->TranslateWorldMatrix(m_vpMatrices[eyesMatrixIndex]->m_offsetX, m_vpMatrices[eyesMatrixIndex]->m_offsetY, m_vpMatrices[eyesMatrixIndex]->m_offsetZ);
				}

				m_pRenderer->TranslateWorldMatrix(eyeOffset.x, eyeOffset.y, eyeOffset.z);
				pVoxelCharacter->RenderFaceTextures(true, m_renderWireFrame, transparency);
			m_pRenderer->PopMatrix();
		}
	}


	// Render mouth
	{
		int mouthBoneIndex = pVoxelCharacter->GetMouthBone();
		vec3 mouthOffset = pVoxelCharacter->GetMouthOffset();

		m_pRenderer->PushMatrix();
			m_pRenderer->EnableMaterial(m_materialID);

			if(useScale)
			{
				m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, -pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());
				m_pRenderer->RotateWorldMatrix(pVoxelCharacter->GetHeadAndUpperBodyLookRotation()*0.65f, 0.0f, 0.0f);
				m_pRenderer->TranslateWorldMatrix(0.0f, 0.0f, pVoxelCharacter->GetHeadAndUpperBodyLookzTranslate());

				// Breathing animation
				if(pVoxelCharacter->IsBreathingAnimationStarted())
				{
					float offsetAmount = 0.0f;
					if(mouthBoneIndex != -1)
					{
						offsetAmount = pVoxelCharacter->GetBreathingAnimationOffsetForBone(mouthBoneIndex);
					}

					m_pRenderer->TranslateWorldMatrix(0.0f, offsetAmount, 0.0f);
				}
			}

			// Translate by attached bone matrix
			Matrix4x4 boneMatrix = pSkeleton->GetBoneMatrix(mouthBoneIndex);
			vec3 boneScale = pVoxelCharacter->GetBoneScale();
			m_pRenderer->ScaleWorldMatrix(boneScale.x, boneScale.y, boneScale.z);
			m_pRenderer->MultiplyWorldMatrix(boneMatrix);
			m_pRenderer->ScaleWorldMatrix(1.0f/boneScale.x, 1.0f/boneScale.y, 1.0f/boneScale.z);

			// Rotation due to 3dsmax export affecting the bone rotations
			m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f);

			// Face looking direction
			vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
			vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
			vec3 lRight = normalize(cross(lUp, lForward));
			lUp = normalize(cross(lForward, lRight));

			float lMatrix[16] =
			{
				lRight.x, lRight.y, lRight.z, 0.0f,
				lUp.x, lUp.y, lUp.z, 0.0f,
				lForward.x, lForward.y, lForward.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};
			Matrix4x4 lookingMat;
			lookingMat.SetValues(lMatrix);
			m_pRenderer->MultiplyWorldMatrix(lookingMat);

			int mouthMatrixIndex = pVoxelCharacter->GetMouthMatrixIndex();

			if(useScale)
			{
				// Scale for external matrix scale value
				m_pRenderer->ScaleWorldMatrix(m_vpMatrices[mouthMatrixIndex]->m_scale, m_vpMatrices[mouthMatrixIndex]->m_scale, m_vpMatrices[mouthMatrixIndex]->m_scale);
			}
			if(useTranslate)
			{
				// Translate for external matrix offset value
				m_pRenderer->TranslateWorldMatrix(m_vpMatrices[mouthMatrixIndex]->m_offsetX, m_vpMatrices[mouthMatrixIndex]->m_offsetY, m_vpMatrices[mouthMatrixIndex]->m_offsetZ);
			}

			m_pRenderer->TranslateWorldMatrix(mouthOffset.x, mouthOffset.y, mouthOffset.z);
			pVoxelCharacter->RenderFaceTextures(false, m_renderWireFrame, transparency);
		m_pRenderer->PopMatrix();
	}
}

void QubicleBinary::RenderPaperdoll(MS3DAnimator* pSkeleton_Left, MS3DAnimator* pSkeleton_Right, VoxelCharacter* pVoxelCharacter)
{
	if(pVoxelCharacter == NULL)
	{
		return;
	}

	m_pRenderer->PushMatrix();
		m_pRenderer->StartMeshRender();

		for(unsigned int i = 0; i < m_numMatrices; i++)
		{
			if(m_vpMatrices[i]->m_removed == true)
			{
				continue;
			}

			MS3DAnimator* pSkeletonToUse = NULL;

			if (m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLeftShoulderBoneIndex() ||
				m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLeftHandBoneIndex())
			{
				pSkeletonToUse = pSkeleton_Left;
			}
			else
			{
				pSkeletonToUse = pSkeleton_Right;
			}

			m_pRenderer->PushMatrix();
				// Breathing animation
				if(pVoxelCharacter->IsBreathingAnimationStarted())
				{
					float offsetAmount = 0.0f;
					if(m_vpMatrices[i]->m_boneIndex != -1)
					{
						offsetAmount = pVoxelCharacter->GetBreathingAnimationOffsetForBone(m_vpMatrices[i]->m_boneIndex);
					}

					m_pRenderer->TranslateWorldMatrix(0.0f, offsetAmount, 0.0f);
				}

				// Body and hands/shoulders looking direction
				if(m_vpMatrices[i]->m_boneIndex != -1)
				{
					if( m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetBodyBoneIndex() ||
						m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLeftShoulderBoneIndex() ||
						m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetLeftHandBoneIndex() ||
						m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetRightShoulderBoneIndex() ||
						m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetRightHandBoneIndex() )
					{
						vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
						lForward.y = 0.0f;
						lForward = normalize(lForward);
						vec3 forwardDiff = lForward - vec3(0.0f, 0.0f, 1.0f);
						lForward = normalize(vec3(0.0f, 0.0f, 1.0f) + (forwardDiff*0.5f));

						vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
						vec3 lRight = normalize(cross(lUp, lForward));
						lUp = normalize(cross(lForward, lRight));

						float lMatrix[16] =
						{
							lRight.x, lRight.y, lRight.z, 0.0f,
							lUp.x, lUp.y, lUp.z, 0.0f,
							lForward.x, lForward.y, lForward.z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f
						};
						Matrix4x4 lookingMat;
						lookingMat.SetValues(lMatrix);
						m_pRenderer->MultiplyWorldMatrix(lookingMat);
					}
				}

				// Translate by attached bone matrix
				if(m_vpMatrices[i]->m_boneIndex != -1)
				{
					Matrix4x4 boneMatrix = pSkeletonToUse->GetBoneMatrix(m_vpMatrices[i]->m_boneIndex);
					vec3 boneScale = pVoxelCharacter->GetBoneScale();
					m_pRenderer->ScaleWorldMatrix(boneScale.x, boneScale.y, boneScale.z);
					m_pRenderer->MultiplyWorldMatrix(boneMatrix);
					m_pRenderer->ScaleWorldMatrix(1.0f/boneScale.x, 1.0f/boneScale.y, 1.0f/boneScale.z);

					// Rotation due to 3dsmax export affecting the bone rotations
					m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f);
				}

				// Face looking direction
				if(m_vpMatrices[i]->m_boneIndex != -1)
				{
					if(m_vpMatrices[i]->m_boneIndex == pVoxelCharacter->GetHeadBoneIndex())
					{
						vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
						vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
						vec3 lRight = normalize(cross(lUp, lForward));
						lUp = normalize(cross(lForward, lRight));

						float lMatrix[16] =
						{
							lRight.x, lRight.y, lRight.z, 0.0f,
							lUp.x, lUp.y, lUp.z, 0.0f,
							lForward.x, lForward.y, lForward.z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f
						};
						Matrix4x4 lookingMat;
						lookingMat.SetValues(lMatrix);
						m_pRenderer->MultiplyWorldMatrix(lookingMat);
					}
				}

				// Scale for external matrix scale value
				m_pRenderer->ScaleWorldMatrix(m_vpMatrices[i]->m_scale, m_vpMatrices[i]->m_scale, m_vpMatrices[i]->m_scale);

				// Translate for initial block offset
				m_pRenderer->TranslateWorldMatrix(0.5f, 0.5f, 0.5f);

				// Translate to center of model
				m_pRenderer->TranslateWorldMatrix(-(float)m_vpMatrices[i]->m_matrixSizeX*0.5f, -(float)m_vpMatrices[i]->m_matrixSizeY*0.5f, -(float)m_vpMatrices[i]->m_matrixSizeZ*0.5f);

				// Translate for external matrix offset value
				m_pRenderer->TranslateWorldMatrix(m_vpMatrices[i]->m_offsetX, m_vpMatrices[i]->m_offsetY, m_vpMatrices[i]->m_offsetZ);

				if (m_renderWireFrame)
				{
					m_pRenderer->SetLineWidth(1.0f);
					m_pRenderer->SetRenderMode(RM_WIREFRAME);
					m_pRenderer->SetCullMode(CM_NOCULL);
				}
				else
				{
					m_pRenderer->SetRenderMode(RM_SOLID);
				}

				// Texture manipulation (for shadow rendering)
				{
					Matrix4x4 worldMatrix;
					m_pRenderer->GetModelMatrix(&worldMatrix);

					m_pRenderer->PushTextureMatrix();
					m_pRenderer->MultiplyWorldMatrix(worldMatrix);
				}

				m_pRenderer->EnableMaterial(m_materialID);

				m_pRenderer->MeshStaticBufferRender(m_vpMatrices[i]->m_pMesh);

				// Texture manipulation (for shadow rendering)
				{
					m_pRenderer->PopTextureMatrix();
				}
			m_pRenderer->PopMatrix();
		}

		m_pRenderer->EndMeshRender();
	m_pRenderer->PopMatrix();
}

void QubicleBinary::RenderPortrait(MS3DAnimator* pSkeleton, VoxelCharacter* pVoxelCharacter, string matrixName)
{
	if(pVoxelCharacter == NULL)
	{
		return;
	}

	m_pRenderer->PushMatrix();
		m_pRenderer->StartMeshRender();

		int matrixIndex = GetMatrixIndexForName(matrixName.c_str());

		if(matrixIndex != -1 && m_vpMatrices[matrixIndex]->m_removed == false)
		{
			MS3DAnimator* pSkeletonToUse = pSkeleton;			

			m_pRenderer->PushMatrix();
				// Body and hands/shoulders looking direction
				if(m_vpMatrices[matrixIndex]->m_boneIndex != -1)
				{
					if( m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetBodyBoneIndex() ||
						m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetLeftShoulderBoneIndex() ||
						m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetLeftHandBoneIndex() ||
						m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetRightShoulderBoneIndex() ||
						m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetRightHandBoneIndex() )
					{
						vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
						lForward.y = 0.0f;
						lForward = normalize(lForward);
						vec3 forwardDiff = lForward - vec3(0.0f, 0.0f, 1.0f);
						lForward = normalize(vec3(0.0f, 0.0f, 1.0f) + (forwardDiff*0.5f));

						vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
						vec3 lRight = normalize(cross(lUp, lForward));
						lUp = normalize(cross(lForward, lRight));

						float lMatrix[16] =
						{
							lRight.x, lRight.y, lRight.z, 0.0f,
							lUp.x, lUp.y, lUp.z, 0.0f,
							lForward.x, lForward.y, lForward.z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f
						};
						Matrix4x4 lookingMat;
						lookingMat.SetValues(lMatrix);
						m_pRenderer->MultiplyWorldMatrix(lookingMat);
					}
				}

				// Translate by attached bone matrix
				if(m_vpMatrices[matrixIndex]->m_boneIndex != -1)
				{
					Matrix4x4 boneMatrix = pSkeletonToUse->GetBoneMatrix(m_vpMatrices[matrixIndex]->m_boneIndex);
					vec3 boneScale = pVoxelCharacter->GetBoneScale();
					m_pRenderer->ScaleWorldMatrix(boneScale.x, boneScale.y, boneScale.z);
					m_pRenderer->MultiplyWorldMatrix(boneMatrix);
					m_pRenderer->ScaleWorldMatrix(1.0f/boneScale.x, 1.0f/boneScale.y, 1.0f/boneScale.z);

					// Rotation due to 3dsmax export affecting the bone rotations
					m_pRenderer->RotateWorldMatrix(0.0f, 0.0f, -90.0f);
				}

				// Face looking direction
				if(m_vpMatrices[matrixIndex]->m_boneIndex != -1)
				{
					if(m_vpMatrices[matrixIndex]->m_boneIndex == pVoxelCharacter->GetHeadBoneIndex())
					{
						vec3 lForward = normalize(pVoxelCharacter->GetFaceLookingDirection());
						vec3 lUp = vec3(0.0f, 1.0f, 0.0f);
						vec3 lRight = normalize(cross(lUp, lForward));
						lUp = normalize(cross(lForward, lRight));

						float lMatrix[16] =
						{
							lRight.x, lRight.y, lRight.z, 0.0f,
							lUp.x, lUp.y, lUp.z, 0.0f,
							lForward.x, lForward.y, lForward.z, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f
						};
						Matrix4x4 lookingMat;
						lookingMat.SetValues(lMatrix);
						m_pRenderer->MultiplyWorldMatrix(lookingMat);
					}
				}

				// Scale for external matrix scale value
				if(matrixName == "Helm")
				{
					// NOTE : ONLY scale for portrait when we are rendering the helm, the head should stay a normal size.
					m_pRenderer->ScaleWorldMatrix(m_vpMatrices[matrixIndex]->m_scale, m_vpMatrices[matrixIndex]->m_scale, m_vpMatrices[matrixIndex]->m_scale);
				}

				// Translate for initial block offset
				m_pRenderer->TranslateWorldMatrix(0.5f, 0.5f, 0.5f);

				// Translate to center of model
				m_pRenderer->TranslateWorldMatrix(-(float)m_vpMatrices[matrixIndex]->m_matrixSizeX*0.5f, -(float)m_vpMatrices[matrixIndex]->m_matrixSizeY*0.5f, -(float)m_vpMatrices[matrixIndex]->m_matrixSizeZ*0.5f);

				// Translate for external matrix offset value
				m_pRenderer->TranslateWorldMatrix(m_vpMatrices[matrixIndex]->m_offsetX, m_vpMatrices[matrixIndex]->m_offsetY, m_vpMatrices[matrixIndex]->m_offsetZ);

				if (m_renderWireFrame)
				{
					m_pRenderer->SetLineWidth(1.0f);
					m_pRenderer->SetRenderMode(RM_WIREFRAME);
					m_pRenderer->SetCullMode(CM_NOCULL);
				}
				else
				{
					m_pRenderer->SetRenderMode(RM_SOLID);
				}

				// Texture manipulation (for shadow rendering)
				{
					Matrix4x4 worldMatrix;
					m_pRenderer->GetModelMatrix(&worldMatrix);

					m_pRenderer->PushTextureMatrix();
					m_pRenderer->MultiplyWorldMatrix(worldMatrix);
				}

				m_pRenderer->EnableMaterial(m_materialID);

				m_pRenderer->MeshStaticBufferRender(m_vpMatrices[matrixIndex]->m_pMesh);

				// Texture manipulation (for shadow rendering)
				{
					m_pRenderer->PopTextureMatrix();
				}
			m_pRenderer->PopMatrix();
		}

		m_pRenderer->EndMeshRender();
	m_pRenderer->PopMatrix();
}
