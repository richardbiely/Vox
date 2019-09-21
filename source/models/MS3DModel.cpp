#include "MS3DModel.h"

#include <assert.h>

#include <fstream>
using namespace std;


MS3DModel::MS3DModel(Renderer *lpRenderer)
{
	mpRenderer = lpRenderer;

	numVertices = 0;
	pVertices = nullptr;

	numTriangles = 0;
	pTriangles = nullptr;

	numMaterials = 0;
	pMaterials = nullptr;

	numMeshes = 0;
	pMeshes = nullptr;

	numJoints = 0;
	pJoints = nullptr;

	mbStatic = false;

	mAnimationFPS = 0.0f;
	mStaticRenderBuffers = nullptr;
}

MS3DModel::~MS3DModel()
{
	int i;
	for(i = 0; i < numMeshes; i++)
		delete[] pMeshes[i].pTriangleIndices;

	for(i = 0; i < numMaterials; i++)
		delete[] pMaterials[i].pTextureFilename;

	numMeshes = 0;
	if(pMeshes != nullptr)
	{
		delete[] pMeshes;
		pMeshes = nullptr;
	}

	numMaterials = 0;
	if(pMaterials != nullptr)
	{
		delete[] pMaterials;
		pMaterials = nullptr;
	}

	numTriangles = 0;
	if(pTriangles != nullptr)
	{
		delete[] pTriangles;
		pTriangles = nullptr;
	}

	numVertices = 0;
	if(pVertices != nullptr)
	{
		delete[] pVertices;
		pVertices = nullptr;
	}

	for(i = 0; i < numJoints; i++)
	{
		delete[] pJoints[i].pRotationKeyframes;
		delete[] pJoints[i].pTranslationKeyframes;
	}

	numJoints = 0;
	if(pJoints != nullptr)
	{
		delete[] pJoints;
		pJoints = nullptr;
	}
}

bool MS3DModel::LoadModel(const char *modelFileName, bool lStatic)
{
	ifstream inputFile;

	// Open the file
	inputFile.open(modelFileName, ios::in | ios::binary);
	if (inputFile.fail())
	{
		//cerr << "Couldn't open the model file." << endl;
		return false;
	}

	char pathTemp[PATH_MAX + 1];
	int pathLength;
	for (pathLength = (int)strlen( modelFileName ); --pathLength;)
	{
		if ( modelFileName[pathLength] == '/' || modelFileName[pathLength] == '\\' )
		{
			break;
		}
	}

	strncpy(pathTemp, modelFileName, pathLength);

	int i;

	if ( pathLength > 0 )
	{
		pathTemp[pathLength++] = '/';
	}

	inputFile.seekg( 0, ios::end );
	long fileSize = (long)inputFile.tellg();
	inputFile.seekg( 0, ios::beg );

	byte *pBuffer = new byte[fileSize];

	//Read the whole file into pBuffer
	inputFile.read( (char *)pBuffer, fileSize );
	inputFile.close();

	//Now go through each byte of the file with *pPtr
	const byte *pPtr = pBuffer;

	//Load the Header
	MS3DHeader *pHeader = ( MS3DHeader* )pPtr;
	pPtr += sizeof( MS3DHeader );

	if ( strncmp( pHeader->ID, "MS3D000000", 10 ) != 0 )
	{
		//cerr << "Not an MS3D file." << endl;
		return false;
	}

	if ( pHeader->version < 3 )
	{
		//cerr << "I know nothing about MS3D v1.2" << endl;
		return false;
	}


	//Load the Vertices
	int nVertices = *( word* )pPtr; 
	numVertices = nVertices;
	pVertices = new Vertex[nVertices];
	pPtr += sizeof( word );

	for ( i = 0; i < nVertices; i++ )
	{
		MS3DVertex *pVertex = ( MS3DVertex* )pPtr;
		pVertices[i].boneID = pVertex->boneID;
		memcpy( pVertices[i].location, pVertex->vertex, sizeof( float )*3 );
		pPtr += sizeof( MS3DVertex );
	}


	//Load the Triangles
	int nTriangles = *( word* )pPtr;
	numTriangles = nTriangles;
	pTriangles = new Triangle[nTriangles];
	pPtr += sizeof( word );

	for ( i = 0; i < nTriangles; i++ )
	{
		MS3DTriangle *pTriangle = ( MS3DTriangle* )pPtr;
		int vertexIndices[3] = { pTriangle->vertexIndices[0], pTriangle->vertexIndices[1], pTriangle->vertexIndices[2] };
		float t[3] = { 1.0f-pTriangle->t[0], 1.0f-pTriangle->t[1], 1.0f-pTriangle->t[2] };
		memcpy( pTriangles[i].vertexNormals, pTriangle->vertexNormals, sizeof( float )*3*3 );
		memcpy( pTriangles[i].s, pTriangle->s, sizeof( float )*3 );
		memcpy( pTriangles[i].t, t, sizeof( float )*3 );
		memcpy( pTriangles[i].vertexIndices, vertexIndices, sizeof( int )*3 );
		pPtr += sizeof( MS3DTriangle );
	}


	//Load the Meshes
	int nGroups = *( word* )pPtr;
	numMeshes = nGroups;
	pMeshes = new Mesh[nGroups];
	pPtr += sizeof( word );

	for ( i = 0; i < nGroups; i++ )
	{
		pPtr += sizeof( byte );		//flags
		pPtr += 32;					//name

		word nTriangles = *( word* )pPtr;
		pPtr += sizeof( word );
		int *pTriangleIndices = new int[nTriangles];
		for ( int j = 0; j < nTriangles; j++ )
		{
			pTriangleIndices[j] = *( word* )pPtr;
			pPtr += sizeof( word );
		}

		char materialIndex = *( char* )pPtr;
		pPtr += sizeof( char );

		pMeshes[i].materialIndex = materialIndex;
		pMeshes[i].numTriangles = nTriangles;
		pMeshes[i].pTriangleIndices = pTriangleIndices;
	}


	//Load the Materials
	int nMaterials = *( word* )pPtr;
	numMaterials = nMaterials;
	pMaterials = new Material_Model[nMaterials];
	pPtr += sizeof( word );

	for ( i = 0; i < nMaterials; i++ )
	{
		MS3DMaterial *pMaterial = ( MS3DMaterial* )pPtr;
		memcpy( pMaterials[i].ambient, pMaterial->ambient, sizeof( float )*4 );
		memcpy( pMaterials[i].diffuse, pMaterial->diffuse, sizeof( float )*4 );
		memcpy( pMaterials[i].specular, pMaterial->specular, sizeof( float )*4 );
		memcpy( pMaterials[i].emissive, pMaterial->emissive, sizeof( float )*4 );
		pMaterials[i].shininess = pMaterial->shininess;

		if ( strncmp( pMaterial->texture, ".\\", 2 ) == 0 ) {
			//MS3D 1.5.x relative path
			strcpy( pathTemp + pathLength, pMaterial->texture + 2 );
			pMaterials[i].pTextureFilename = new char[strlen( pathTemp )+1];
			strcpy( pMaterials[i].pTextureFilename, pathTemp );
		}
		else {
			//MS3D 1.4.x or earlier - absolute path
			strcpy( pathTemp + pathLength, pMaterial->texture );
			pMaterials[i].pTextureFilename = new char[strlen( pathTemp )+1];
			strcpy( pMaterials[i].pTextureFilename, pathTemp );
		}
		pPtr += sizeof( MS3DMaterial );
	}

	//Get the Animation Speed
	mAnimationFPS = *( float* )pPtr;
	pPtr += sizeof( float );

	//Skip currentTime
	pPtr += sizeof( float );

	//Get the total frames for the animation
	int totalFrames = *( int* )pPtr;
	pPtr += sizeof( int );

	//Work out the total time for the animation
	//totalTime = totalFrames * 1000.0/mAnimationFPS;

	//Get the number of joints
	numJoints = *( word* )pPtr;
	pPtr += sizeof( word );

	pJoints = new Joint[numJoints];

	struct JointNameListRec
	{
		int jointIndex;
		const char *pName;
	};

	const byte *pTempPtr = pPtr;

	JointNameListRec *pNameList = new JointNameListRec[numJoints];
	for ( i = 0; i < numJoints; i++ )
	{
		MS3DJoint *pJoint = ( MS3DJoint* )pTempPtr;
		pTempPtr += sizeof( MS3DJoint );
		pTempPtr += sizeof( MS3DKeyframe )*( pJoint->numRotationKeyframes + pJoint->numTranslationKeyframes );

		pNameList[i].jointIndex = i;
		pNameList[i].pName = pJoint->name;
		
		sprintf(pJoints[i].name, "%s", pJoint->name);
	}


	//Load the Joints
	for ( i = 0; i < numJoints; i++ )
	{
		MS3DJoint *pJoint = ( MS3DJoint* )pPtr;
		pPtr += sizeof( MS3DJoint );

		int j, parentIndex = -1;
		if ( strlen( pJoint->parentName ) > 0 )
		{
			for ( j = 0; j < numJoints; j++ )
			{
#ifdef _WIN32
				if (_strcmpi(pNameList[j].pName, pJoint->parentName) == 0)
#else
				if (strcasecmp(pNameList[j].pName, pJoint->parentName) == 0)
#endif //_WIN32
				{
					parentIndex = pNameList[j].jointIndex;
					break;
				}
			}
			if ( parentIndex == -1 ) {
				//cerr << "Unable to find parent bone in MS3D file" << endl;
				return false;
			}
		}

		memcpy( pJoints[i].localRotation, pJoint->rotation, sizeof( float )*3 );
		memcpy( pJoints[i].localTranslation, pJoint->translation, sizeof( float )*3 );
		pJoints[i].parent = parentIndex;
		pJoints[i].numRotationKeyframes = pJoint->numRotationKeyframes;
		pJoints[i].pRotationKeyframes = new Keyframe[pJoint->numRotationKeyframes];
		pJoints[i].numTranslationKeyframes = pJoint->numTranslationKeyframes;
		pJoints[i].pTranslationKeyframes = new Keyframe[pJoint->numTranslationKeyframes];

		//Load the Rotation keyframes
		for ( j = 0; j < pJoint->numRotationKeyframes; j++ )
		{
			MS3DKeyframe *pKeyframe = ( MS3DKeyframe* )pPtr;
			pPtr += sizeof( MS3DKeyframe );

			SetJointKeyframe( i, j, pKeyframe->time*1000.0f, pKeyframe->parameter, true );
		}

		//Load the Translation keyframes
		for ( j = 0; j < pJoint->numTranslationKeyframes; j++ )
		{
			MS3DKeyframe *pKeyframe = ( MS3DKeyframe* )pPtr;
			pPtr += sizeof( MS3DKeyframe );

			SetJointKeyframe( i, j, pKeyframe->time*1000.0f, pKeyframe->parameter, false );
		}
	}

	// Setup the joints
	SetupJoints();

	// Load the textures
	if(!LoadTextures())
	{
		return false;
	}


	// Delete the temporary arrays
	delete[] pNameList;
	delete[] pBuffer;

	// Calculate the bounding box
	CalculateBoundingBox();

	mbStatic = lStatic;

	if(mbStatic)
	{
		SetupStaticBuffer();
	}

	return true;
}

bool MS3DModel::LoadTextures()
{
	for(int i = 0; i < numMaterials; i++)
	{
		if(strlen( pMaterials[i].pTextureFilename ) > 0)
		{
			int lTextureWidth, lTextureHeight, lTextureWidth2, lTextureHeight2;
			if(!mpRenderer->LoadTexture(pMaterials[i].pTextureFilename, &lTextureWidth, &lTextureHeight, &lTextureWidth2, &lTextureHeight2, &pMaterials[i].texture))
			{
				return false;
			}
		}
		else
		{
			pMaterials[i].texture = -1;
		}
	}

	return true;
}

void MS3DModel::SetupStaticBuffer()
{
	mStaticRenderBuffers = new unsigned int[numMeshes];
	for(int i = 0; i < numMeshes; i++)
	{
		// Create the buffer used for drawing the image
		OGLPositionNormalVertex* vertices;
		vertices = new OGLPositionNormalVertex[pMeshes[i].numTriangles*3];
		
		OGLUVCoordinate* textureCoordinates;
		textureCoordinates = new OGLUVCoordinate[pMeshes[i].numTriangles*3];

		int lIndexCounter = 0;
		for(int j = 0; j < pMeshes[i].numTriangles; j++)
		{
			int triangleIndex = pMeshes[i].pTriangleIndices[j];
			const Triangle* pTri = &pTriangles[triangleIndex];

			for ( int k = 0; k < 3; k++ )
			{
				int index = pTri->vertexIndices[k];

				Matrix4x4& final = pJoints[pVertices[index].boneID].absolute;

				vec3 newVertex( pVertices[index].location[0], pVertices[index].location[1], pVertices[index].location[2]);
				newVertex = final * newVertex;

				vertices[lIndexCounter].x = newVertex.x;
				vertices[lIndexCounter].y = newVertex.y;
				vertices[lIndexCounter].z = newVertex.z;

				vec3 newNormal( pTri->vertexNormals[k][0], pTri->vertexNormals[k][1], pTri->vertexNormals[k][2]);
				newNormal = final * newNormal;

				vertices[lIndexCounter].nx = pTri->vertexNormals[k][0];
				vertices[lIndexCounter].ny = pTri->vertexNormals[k][1];
				vertices[lIndexCounter].nz = pTri->vertexNormals[k][2];

				//vertices[lIndexCounter].u = pTri->s[k];
				//vertices[lIndexCounter].v = pTri->t[k];
				textureCoordinates[lIndexCounter].u = pTri->s[k];
				textureCoordinates[lIndexCounter].v = pTri->t[k];

				lIndexCounter++;
			}
		}

		int materialIndex = pMeshes[i].materialIndex;

		mpRenderer->CreateStaticBuffer(VT_POSITION_NORMAL_UV, -1, pMaterials[materialIndex].texture, pMeshes[i].numTriangles*3, pMeshes[i].numTriangles*3, 0, vertices, textureCoordinates, nullptr, &mStaticRenderBuffers[i]);
	}
}

void MS3DModel::SetJointKeyframe( int jointIndex, int keyframeIndex, float time, float *parameter, bool isRotation )
{
	Keyframe& keyframe = isRotation ? pJoints[jointIndex].pRotationKeyframes[keyframeIndex] : pJoints[jointIndex].pTranslationKeyframes[keyframeIndex];

	keyframe.jointIndex = jointIndex;
	keyframe.time = time;
	memcpy( keyframe.parameter, parameter, sizeof( float )*3 );
}

void MS3DModel::SetupJoints()
{
	int i;
	for( i = 0; i < numJoints; i++ )
	{
		Joint& joint = pJoints[i];

		joint.relative.AddRotationRadians( joint.localRotation );
		joint.relative.AddTranslation( joint.localTranslation );

		if ( joint.parent != -1 )
		{
			joint.absolute = pJoints[joint.parent].absolute;

			joint.absolute.PostMultiply( joint.relative );
		}
		else
		{
			joint.absolute = joint.relative;
		}
	}

	for( i = 0; i < numVertices; i++ )
	{
		Vertex& vertex = pVertices[i];

		if ( vertex.boneID != -1 )
		{
			Matrix4x4& matrix = pJoints[vertex.boneID].absolute;

			matrix.InverseTranslateVector( vertex.location );
			matrix.InverseRotateVector( vertex.location );
		}
	}

	for( i = 0; i < numTriangles; i++ )
	{
		Triangle& triangle = pTriangles[i];
		for ( int j = 0; j < 3; j++ )
		{
			const Vertex& vertex = pVertices[triangle.vertexIndices[j]];
			if ( vertex.boneID != -1 )
			{
				Matrix4x4& matrix = pJoints[vertex.boneID].absolute;
				matrix.InverseRotateVector( triangle.vertexNormals[j] );
			}
		}
	}
}

void MS3DModel::CalculateBoundingBox()
{
	for(int i = 0; i < numVertices; i++)
	{
		if(pVertices[i].boneID == -1)
		{
			// Don't use this vertex if it doesnt have a valid boneID, since this will mess up the matrix math
			continue;
		}

		Matrix4x4& final = pJoints[pVertices[i].boneID].absolute;
		vec3 newVertex( pVertices[i].location[0], pVertices[i].location[1], pVertices[i].location[2]);

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

const BoundingBox* MS3DModel::GetBoundingBox() const
{
	return &m_BoundingBox;
}

int MS3DModel::GetBoneIndex(const char* boneName) const
{
	for ( int i = 0; i < numJoints; i++ )
	{
		if(strcmp(pJoints[i].name, boneName) == 0)
		{
			return i;
		}
	}

	return -1;
}

const char* MS3DModel::GetNameFromBoneIndex(int boneIndex) const
{
	return pJoints[boneIndex].name;
}

int MS3DModel::GetNumJoints() const
{
	return numJoints;
}

const Joint* MS3DModel::GetJoint(const char* jointName) const
{
	for(int i = 0; i < numJoints; i++)
	{
		if(strcmp(pJoints[i].name, jointName) == 0)
		{
			return GetJoint(i);
		}
	}

	return nullptr;
}

const Joint* MS3DModel::GetJoint(int index) const
{
	return &pJoints[index];
}

void MS3DModel::Update(float dt)
{
}

void MS3DModel::Render(bool lMesh, bool lNormals, bool lBones, bool lBoundingBox)
{
	if(lMesh)
	{
		if(mbStatic)
		{
			RenderMesh_Static();
		}
		else
		{
			RenderMesh();
		}
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

void MS3DModel::RenderMesh()
{
	//Draw by group
	for ( int i = 0; i < numMeshes; i++ )
	{
		int materialIndex = pMeshes[i].materialIndex;
		mpRenderer->BindTexture(pMaterials[materialIndex].texture);

		glBegin( GL_TRIANGLES );
		{
			for ( int j = 0; j < pMeshes[i].numTriangles; j++ )
			{
				int triangleIndex = pMeshes[i].pTriangleIndices[j];
				const Triangle* pTri = &pTriangles[triangleIndex];

				for ( int k = 0; k < 3; k++ )
				{
					int index = pTri->vertexIndices[k];

					if ( pVertices[index].boneID == -1 )
					{
						//Dont draw the foot steps!
					}
					else
					{
						//Rotate according to transformation matrix
						Matrix4x4& final = pJoints[pVertices[index].boneID].absolute;

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
						vec3 newVertex( pVertices[index].location[0], pVertices[index].location[1], pVertices[index].location[2]);

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

void MS3DModel::RenderMesh_Static()
{
	mpRenderer->PushMatrix();
		mpRenderer->SetRenderMode(RM_TEXTURED_LIGHTING);
		mpRenderer->SetPrimativeMode(PM_TRIANGLES);
		for(int i = 0; i < numMeshes; i++)
		{
			mpRenderer->RenderStaticBuffer(mStaticRenderBuffers[i]);
		}
	mpRenderer->PopMatrix();
}

void MS3DModel::RenderNormals()
{
	//Make the colour cyan
	glColor3ub(0, 255, 255);

	glDisable(GL_LIGHTING);
	mpRenderer->SetRenderMode(RM_SOLID);

	for ( int i = 0; i < numMeshes; i++ )
	{
		glBegin( GL_LINES );
		{
			for ( int j = 0; j < pMeshes[i].numTriangles; j++ )
			{
				int triangleIndex = pMeshes[i].pTriangleIndices[j];
				const Triangle* pTri = &pTriangles[triangleIndex];

				for ( int k = 0; k < 3; k++ )
				{
					int index = pTri->vertexIndices[k];

					if ( pVertices[index].boneID == -1 )
					{
						//Dont draw the foot steps!
					}
					else
					{
						//Rotate according to transformation matrix
						Matrix4x4& final = pJoints[pVertices[index].boneID].absolute;

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
						vec3 newVertex( pVertices[index].location[0], pVertices[index].location[1], pVertices[index].location[2]);

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

void MS3DModel::RenderBones()
{
	//Make the colour white
	glColor3ub(255, 255, 255);

	for ( int i = 0; i < numJoints; i++ )
	{
		glBegin( GL_LINES );
		{
			vec3 newVertex;

			newVertex = pJoints[i].absolute * newVertex;

			float tempVertex[3];
			tempVertex[0] = newVertex.x;
			tempVertex[1] = newVertex.y;
			tempVertex[2] = newVertex.z;
			glVertex3fv( tempVertex);

			if ( pJoints[i].parent != -1 )
			{
				Matrix4x4& final = pJoints[pJoints[i].parent].absolute;
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

void MS3DModel::RenderBoundingBox()
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