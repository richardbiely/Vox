#include "OBJModel.h"
#include "../utils/FileUtils.h"

#pragma warning(disable: 4473)  // 'fscanf_s' : not enough arguments passed for format string


OBJModel::OBJModel(Renderer *lpRenderer)
{
	mpRenderer = lpRenderer;

	m_numVertices = 0;
	m_numTexCoordinates = 0;
	m_numNormals = 0;
	m_numFaces = 0;

	m_pVertices = nullptr;
	m_pTextureCoordinates = nullptr;
	m_pNormals = nullptr;
	m_pFaces = nullptr;

	m_texture = -1;
}

OBJModel::~OBJModel()
{
	// Delete the vertices
	delete[] m_pVertices;
	m_pVertices = nullptr;

	// Delete the texture coordinates
	delete[] m_pTextureCoordinates;
	m_pTextureCoordinates = nullptr;

	// Delete the normals
	delete[] m_pNormals;
	m_pNormals = nullptr;

	// Delete the indices from the polygon list
	for(int i = 0; i < m_numFaces; i++) {
		delete[] m_pFaces[i].pIndices;
		m_pFaces[i].pIndices = nullptr;
	}

	// Delete the faces
	delete[] m_pFaces;
	m_pFaces = nullptr;
}


bool OBJModel::Load(const char *modelFileName, const char *textureFileName)
{
	m_ModelFilename = modelFileName;

	FILE *pFile;
	char buffer[256] = {0};

	// Make sure that we have passed a filename
	if(modelFileName == nullptr)
		return false;

	// Open the file
	if( fopen_s(&pFile, modelFileName, "r") )
		return false;

	// Go through the obj file and count the number of vertices and faces
	while( !feof(pFile) ) {
		fgets(buffer, sizeof(buffer), pFile);

		if(buffer[0] == 'v' && buffer[1] == ' ') {
			m_numVertices++;
		}
		else if(buffer[0] == 'v' && buffer[1] == 't') {
			m_numTexCoordinates++;
		}
		else if(buffer[0] == 'v' && buffer[1] == 'n') {
			m_numNormals++;
		}
		else if(buffer[0] == 'f' && buffer[1] == ' ') {
			m_numFaces++;
		}
	}

	// Allocate the arrays
	m_pVertices = new OBJ_Vertex[m_numVertices];
	m_pTextureCoordinates = new OBJ_TextureCoordinate[m_numTexCoordinates];
	m_pNormals = new vec3[m_numNormals];
	m_pFaces = new OBJ_Face[m_numFaces];

	memset(m_pVertices, 0, sizeof(OBJ_Vertex) * m_numVertices);
	memset(m_pTextureCoordinates, 0, sizeof(OBJ_TextureCoordinate) * m_numTexCoordinates);
	memset(m_pNormals, 0, sizeof(vec3) * m_numNormals);
	memset(m_pFaces, 0, sizeof(OBJ_Face) * m_numFaces);

	// Reset the buffer
	memset(buffer, '\0', sizeof(buffer));

	// Get back to the beggining of the file
	rewind(pFile);

	int vertexIndex = 0;
	int textureCoordinateIndex = 0;
	int normalIndex = 0;
	int faceIndex = 0;
	char *token;
	char tempBuffer[256];
	char tempChar;
	//char indexValue[50];
	//int offset;

	while( !feof(pFile) ) {
		// Read in the charater of the line
		fscanf(pFile, "%c", &tempChar);

		// If it is a v then we are reading either a vertex point, texture coordinate or a vertex normal
		if(tempChar == 'v') {
			fscanf(pFile, "%c", &tempChar);

			if(tempChar == ' ') {
				// Vertex point
				// Read in the x, y, z values of this vertex point
				fscanf(pFile, "%f", &m_pVertices[vertexIndex].position.x);
				fscanf(pFile, "%f", &m_pVertices[vertexIndex].position.y);
				fscanf(pFile, "%f", &m_pVertices[vertexIndex].position.z);

				// Set all the vertices to initially not be assigned to a mass
				m_pVertices[vertexIndex].massID = -1;

				// Read in the newline
				fscanf(pFile, "%c", &tempChar);

				// Increment the index
				vertexIndex++;
			}
			else if(tempChar == 't') {
				// Texture coordinate
				// Read in the u, v, values of this texture coordinate
				fscanf(pFile, "%f", &m_pTextureCoordinates[textureCoordinateIndex].u);
				fscanf(pFile, "%f", &m_pTextureCoordinates[textureCoordinateIndex].v);

				// Read in the newline
				fscanf(pFile, "%c", &tempChar);

				// Increment the index
				textureCoordinateIndex++;
			}
			else if(tempChar == 'n') {
				// Vertex normal
				// Read in the x, y, z values of this vertex normal
				fscanf(pFile, "%f", &m_pNormals[normalIndex].x);
				fscanf(pFile, "%f", &m_pNormals[normalIndex].y);
				fscanf(pFile, "%f", &m_pNormals[normalIndex].z);

				// Read in the newline
				fscanf(pFile, "%c", &tempChar);

				// Increment the index
				normalIndex++;
			}
		}
		else if(tempChar == 'f') {
			// If we read in a f check that the next char is a space
			fscanf(pFile, "%c", &tempChar);

			if(tempChar == ' ') {
				// Read in the rest of the line
				fgets(buffer, sizeof(buffer), pFile);

				// Copy to a temp buffer so we can mess with it.
				buffer[strlen(buffer)] = '\0';
				strcpy(tempBuffer, buffer);

				// Set the search token. Used to distinguish between each polygon
				token = strtok(tempBuffer, " ");

				// Loop through the line and get the number of times we see the search token
				int count = 0;
				for(count = 0; token != nullptr; count++)
				{
					token = strtok(nullptr, " ");
				}

				m_pFaces[faceIndex].pIndices = new int[count * 3];
				m_pFaces[faceIndex].numPoints = count;

				// HACK : This is a *very* bad way to do this!
				if(m_numNormals == 0 && m_numTexCoordinates == 0)
				{
					if(count == 1)
					{
						sscanf(buffer, "%i", &m_pFaces[faceIndex].pIndices[0]);
					}
					else if(count == 2)
					{
						sscanf(buffer, "%i %i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[3]);
					}
					else if(count == 3)
					{
						sscanf(buffer, "%i %i %i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[6]);
					}
					else if(count == 4)
					{
						sscanf(buffer, "%i %i %i %i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[9]);
					}
					else if(count == 5)
					{
						sscanf(buffer, "%i %i %i %i %i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[12]);
					}
					else if(count == 6)
					{
						sscanf(buffer, "%i %i %i %i %i %i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[12], &m_pFaces[faceIndex].pIndices[15]);
					}
				}
				else if (m_numNormals == 0)
				{
					if(count == 1)
					{
						sscanf(buffer, "%i/%i/", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1]);
					}
					else if(count == 2)
					{
						sscanf(buffer, "%i/%i %i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4]);
					}
					else if(count == 3)
					{
						sscanf(buffer, "%i/%i %i/%i %i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[7]);
					}
					else if(count == 4)
					{
						sscanf(buffer, "%i/%i %i/%i %i/%i %i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[7],
						&m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[10]);
					}
					else if(count == 5)
					{
						sscanf(buffer, "%i/%i %i/%i %i/%i %i/%i %i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[7],
						&m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[10],
						&m_pFaces[faceIndex].pIndices[12], &m_pFaces[faceIndex].pIndices[13]);
					}
					else if(count == 6)
					{
						sscanf(buffer, "%i/%i %i/%i %i/%i %i/%i %i/%i %i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[7],
						&m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[10],
						&m_pFaces[faceIndex].pIndices[12], &m_pFaces[faceIndex].pIndices[13],
						&m_pFaces[faceIndex].pIndices[15], &m_pFaces[faceIndex].pIndices[16]);
					}
				}
				else if(m_numTexCoordinates == 0)
				{
					if(count == 1)
					{
						sscanf(buffer, "%i//%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[2]);
					}
					else if(count == 2)
					{
						sscanf(buffer, "%i//%i %i//%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[5]);
					}
					else if(count == 3)
					{
						sscanf(buffer, "%i//%i %i//%i %i//%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[5],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[8]);
					}
					else if(count == 4)
					{
						sscanf(buffer, "%i//%i %i//%i %i//%i %i//%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[5],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[8],
						&m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[11]);
					}
					else if(count == 5)
					{
						sscanf(buffer, "%i//%i %i//%i %i//%i %i//%i %i//%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[5],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[8],
						&m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[11],
						&m_pFaces[faceIndex].pIndices[12], &m_pFaces[faceIndex].pIndices[14]);
					}
					else if(count == 6)
					{
						sscanf(buffer, "%i//%i %i//%i %i//%i %i//%i %i//%i %i//%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[5],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[8],
						&m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[11],
						&m_pFaces[faceIndex].pIndices[12], &m_pFaces[faceIndex].pIndices[14],
						&m_pFaces[faceIndex].pIndices[15], &m_pFaces[faceIndex].pIndices[17]);
					}
				}
				else
				{
					if(count == 1)
					{
						sscanf(buffer, "%i/%i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1], &m_pFaces[faceIndex].pIndices[2]);
					}
					else if(count == 2)
					{
						sscanf(buffer, "%i/%i/%i %i/%i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4], &m_pFaces[faceIndex].pIndices[5]);
					}
					else if(count == 3)
					{
						sscanf(buffer, "%i/%i/%i %i/%i/%i %i/%i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4], &m_pFaces[faceIndex].pIndices[5],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[7], &m_pFaces[faceIndex].pIndices[8]);
					}
					else if(count == 4)
					{
						sscanf(buffer, "%i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4], &m_pFaces[faceIndex].pIndices[5],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[7], &m_pFaces[faceIndex].pIndices[8],
						&m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[10], &m_pFaces[faceIndex].pIndices[11]);
					}
					else if(count == 5)
					{
						sscanf(buffer, "%i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4], &m_pFaces[faceIndex].pIndices[5],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[7], &m_pFaces[faceIndex].pIndices[8],
						&m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[10], &m_pFaces[faceIndex].pIndices[11],
						&m_pFaces[faceIndex].pIndices[12], &m_pFaces[faceIndex].pIndices[13], &m_pFaces[faceIndex].pIndices[14]);
					}
					else if(count == 6)
					{
						sscanf(buffer, "%i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i", &m_pFaces[faceIndex].pIndices[0], &m_pFaces[faceIndex].pIndices[1], &m_pFaces[faceIndex].pIndices[2],
						&m_pFaces[faceIndex].pIndices[3], &m_pFaces[faceIndex].pIndices[4], &m_pFaces[faceIndex].pIndices[5],
						&m_pFaces[faceIndex].pIndices[6], &m_pFaces[faceIndex].pIndices[7], &m_pFaces[faceIndex].pIndices[8],
						&m_pFaces[faceIndex].pIndices[9], &m_pFaces[faceIndex].pIndices[10], &m_pFaces[faceIndex].pIndices[11],
						&m_pFaces[faceIndex].pIndices[12], &m_pFaces[faceIndex].pIndices[13], &m_pFaces[faceIndex].pIndices[14],
						&m_pFaces[faceIndex].pIndices[15], &m_pFaces[faceIndex].pIndices[16], &m_pFaces[faceIndex].pIndices[17]);
					}
				}
				// HACK : End of hack!

				faceIndex++;
			}
		}
		else {
			// Read the rest of the line
			if(tempChar != '\n')
				fgets(buffer, sizeof(buffer), pFile);
		}

		memset(buffer, '0', sizeof (buffer));
	}

	// Close the file
	fclose(pFile);

	// Also load a texture for this model
	int lTextureWidth;
	int lTextureHeight;
	int lTextureWidthPower2;
	int lTextureHeightPower2;

	if(textureFileName)
	{
		m_TextureFilename = textureFileName;
		mpRenderer->LoadTexture(textureFileName, &lTextureWidth, &lTextureHeight, &lTextureWidthPower2, &lTextureHeightPower2, &m_texture);
	}

	// Calculate the bounding box
	CalculateBoundingBox();

	return true;
}

void OBJModel::CalculateBoundingBox()
{
	for(int i = 0; i < m_numVertices; i++)
	{
		// Always set the first vertex values
		if(i == 0)
		{
			m_BoundingBox.mMinX = m_pVertices[i].position.x;
			m_BoundingBox.mMinY = m_pVertices[i].position.y;
			m_BoundingBox.mMinZ = m_pVertices[i].position.z;

			m_BoundingBox.mMaxX = m_pVertices[i].position.x;
			m_BoundingBox.mMaxY = m_pVertices[i].position.y;
			m_BoundingBox.mMaxZ = m_pVertices[i].position.z;
		}
		else
		{
			if(m_pVertices[i].position.x < m_BoundingBox.mMinX)
			{
				m_BoundingBox.mMinX = m_pVertices[i].position.x;
			}

			if(m_pVertices[i].position.y < m_BoundingBox.mMinY)
			{
				m_BoundingBox.mMinY = m_pVertices[i].position.y;
			}

			if(m_pVertices[i].position.z < m_BoundingBox.mMinZ)
			{
				m_BoundingBox.mMinZ = m_pVertices[i].position.z;
			}

			if(m_pVertices[i].position.x > m_BoundingBox.mMaxX)
			{
				m_BoundingBox.mMaxX = m_pVertices[i].position.x;
			}

			if(m_pVertices[i].position.y > m_BoundingBox.mMaxY)
			{
				m_BoundingBox.mMaxY = m_pVertices[i].position.y;
			}

			if(m_pVertices[i].position.z > m_BoundingBox.mMaxZ)
			{
				m_BoundingBox.mMaxZ = m_pVertices[i].position.z;
			}
		}
	}
}

BoundingBox* OBJModel::GetBoundingBox()
{
	return &m_BoundingBox;
}

void OBJModel::Update(float dt)
{

}

void OBJModel::Render()
{
	RenderMesh();
	RenderWireFrame();

	RenderBoundingBox();
}

void OBJModel::RenderMesh()
{
	mpRenderer->PushMatrix();

	// NOTE : Lighting isnt supported yet!

		if(m_texture != -1)
		{
			mpRenderer->BindTexture(m_texture);

			mpRenderer->SetRenderMode(RM_TEXTURED);
		}
		else
		{
			mpRenderer->SetRenderMode(RM_SOLID);
		}

		// Draw each face
		for(int i = 0; i < m_numFaces; i++)
		{
			// Decide on what primitive type to use, depending on how many points there are
			if(m_pFaces[i].numPoints == 3)
				mpRenderer->EnableImmediateMode(IM_TRIANGLES);
			else if(m_pFaces[i].numPoints == 4)
				mpRenderer->EnableImmediateMode(IM_QUADS);
			else
				mpRenderer->EnableImmediateMode(IM_POLYGON);

			mpRenderer->ImmediateColourAlpha(1.0f, 1.0f, 1.0f, 1.0f);

			/*
			// Normal for each face
			if(m_numNormals != 0)
			{
				int vertexIndex1 = m_pFaces[i].pIndices[0] - 1;
				int vertexIndex2 = m_pFaces[i].pIndices[3] - 1;
				int vertexIndex3 = m_pFaces[i].pIndices[6] - 1;

				// Calculate the normal for the triangle, since flat shading only requires a normal for each face
				vec3 triangle[3];
				triangle[0] = m_pVertices[vertexIndex1].position;
				triangle[1] = m_pVertices[vertexIndex2].position;
				triangle[2] = m_pVertices[vertexIndex3].position;

				vec3 normal = PolygonNormal(triangle);
				mpRenderer->ImmediateNormal(normal.x, normal.y, normal.z);
			}
			*/

			// Draw the primitive
			for(int j = 0; j < m_pFaces[i].numPoints; j++)
			{
				int vertexIndex = m_pFaces[i].pIndices[(3 * j)] - 1;
				int textureIndex = m_pFaces[i].pIndices[(3 * j) + 1] - 1;
				int normalIndex = m_pFaces[i].pIndices[(3 * j) + 2] - 1;

				/*
				if(m_numNormals != 0)
				{
					// Normals from each vertex
					vec3 normal(m_pNormals[normalIndex].x, m_pNormals[normalIndex].y, m_pNormals[normalIndex].z);
					normal = normalize(normal);
					mpRenderer->ImmediateNormal(normal.x, normal.y, normal.z);
				}
				*/

				if(m_numTexCoordinates != 0)
				{
					mpRenderer->ImmediateTextureCoordinate(m_pTextureCoordinates[textureIndex].u, 1 - m_pTextureCoordinates[textureIndex].v);
				}

				mpRenderer->ImmediateVertex(m_pVertices[vertexIndex].position.x, m_pVertices[vertexIndex].position.y, m_pVertices[vertexIndex].position.z);
			}

			mpRenderer->DisableImmediateMode();
		}
	
		if(m_texture != -1)
		{
			mpRenderer->DisableTexture();
		}

	mpRenderer->PopMatrix();
}

void OBJModel::RenderWireFrame()
{
	mpRenderer->PushMatrix();

		// Store cull mode
		CullMode cullMode = mpRenderer->GetCullMode();

		mpRenderer->SetRenderMode(RM_WIREFRAME);
		mpRenderer->SetCullMode(CM_NOCULL);

		// Draw each face
		for(int i = 0; i < m_numFaces; i++)
		{
			// Decide on what primitive type to use, depending on how many points there are
			if(m_pFaces[i].numPoints == 3)
				mpRenderer->EnableImmediateMode(IM_TRIANGLES);
			else if(m_pFaces[i].numPoints == 4)
				mpRenderer->EnableImmediateMode(IM_QUADS);
			else
				mpRenderer->EnableImmediateMode(IM_POLYGON);

			mpRenderer->ImmediateColourAlpha(0.0f, 0.0f, 0.0f, 1.0f);

			// Draw the primitive
			for(int j = 0; j < m_pFaces[i].numPoints; j++)
			{
				int vertexIndex = m_pFaces[i].pIndices[(3 * j)] - 1;

				mpRenderer->ImmediateVertex(m_pVertices[vertexIndex].position.x, m_pVertices[vertexIndex].position.y, m_pVertices[vertexIndex].position.z);
			}

			mpRenderer->DisableImmediateMode();
		}

		// Restore cull mode
		mpRenderer->SetCullMode(cullMode);

	mpRenderer->PopMatrix();
}

void OBJModel::RenderBoundingBox()
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