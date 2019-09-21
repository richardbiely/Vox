#pragma once

#include "../Renderer/Renderer.h"
#include "BoundingBox.h"

// byte-align structures
#if defined( _MSC_VER ) || defined( __BORLANDC__ )
#	include <pshpack1.h>
#	define PACK_STRUCT
#	define PATH_MAX _MAX_PATH
#elif defined( __GNUC__ )
#	define PACK_STRUCT	__attribute__((packed))
#	include <limits.h>
#else
#	error you must byte-align these structures with the appropriate compiler directives
#endif

/*
// byte-align structures
#ifdef _MSC_VER
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	define PACK_STRUCT
#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif
#elif defined( __GNUC__ )
#	define PACK_STRUCT	__attribute__((packed))
#	include <limits.h>
#else
#	error you must byte-align these structures with the appropriate compiler directives
#endif
*/


typedef unsigned char byte;
typedef unsigned short word;


// ---------------------------------------- //

// MS3D File structure
struct MS3DHeader
{
	char ID[10];
	int version;
} PACK_STRUCT;


// MS3D Vertex structure
struct MS3DVertex
{
	byte flags;
	float vertex[3];
	char boneID;
	byte refCount;
} PACK_STRUCT;


// MS3D Triangle structure
struct MS3DTriangle
{
	word flags;
	word vertexIndices[3];
	float vertexNormals[3][3];
	float s[3], t[3];
	byte smoothingGroup;
	byte groupIndex;
} PACK_STRUCT;


// MS3D Material structure
struct MS3DMaterial
{
	char name[32];
	float ambient[4];
	float diffuse[4];
	float specular[4];
	float emissive[4];
	float shininess;		//0.0f - 128.0f
	float transparency;		//0.0f - 1.0f
	byte mode;				//0, 1, 2 is unused now
	char texture[128];
	char alphamap[128];
} PACK_STRUCT;


// MS3D Joint structure
struct MS3DJoint
{
	byte flags;
	char name[32];
	char parentName[32];
	float rotation[3];
	float translation[3];
	word numRotationKeyframes;
	word numTranslationKeyframes;
} PACK_STRUCT;


// MS3D Keyframe structure
struct MS3DKeyframe
{
	float time;
	float parameter[3];
} PACK_STRUCT;

#if defined( _MSC_VER ) || defined( __BORLANDC__ )
#include <poppack.h> // End byte-align structures
#endif

// ---------------------------------------- //


// Vertex structure
typedef struct Vertex
{
	char boneID;
	float location[3];
} Vertex;


// Triangle structure
typedef struct Triangle
{
	float vertexNormals[3][3];
	int vertexIndices[3];
	float s[3], t[3];
} Triangle;


// Material_Model structure
typedef struct Material_Model
{
	float ambient[4], diffuse[4], specular[4], emissive[4];
	float shininess;
	unsigned int texture;
	char *pTextureFilename;
} Material_Model;


// Mesh structure
typedef struct Mesh
{
	int materialIndex;
	int numTriangles;
	int *pTriangleIndices;
} Mesh;


// Keyframe structure
typedef struct Keyframe
{
	int jointIndex;
	float time;				//In milliseconds
	float parameter[3];
} Keyframe;


// Joint structure
typedef struct Joint
{
	float localRotation[3];
	float localTranslation[3];
	Matrix4x4 absolute, relative;

	int numRotationKeyframes, numTranslationKeyframes;
	Keyframe *pTranslationKeyframes;
	Keyframe *pRotationKeyframes;

	int parent;

	char name[32];
} Joint;


class MS3DModel
{
public:
	MS3DModel(Renderer *lpRenderer);
	~MS3DModel();

	bool LoadModel(const char *modelFileName, bool lStatic = false);
	bool LoadTextures();

	void SetupStaticBuffer();

	void SetJointKeyframe( int jointIndex, int keyframeIndex, float time, float *parameter, bool isRotation );
	void SetupJoints();

	void CalculateBoundingBox();
	const BoundingBox* GetBoundingBox() const;

	int GetBoneIndex(const char* boneName) const;
	const char* GetNameFromBoneIndex(int boneIndex) const;

	int GetNumJoints() const;
	const Joint* GetJoint(const char* jointName) const;
	const Joint* GetJoint(int index) const;

	void Update(float dt);

	void Render(bool lMesh, bool lNormals, bool lBones, bool lBoundingBox);
	void RenderMesh();
	void RenderMesh_Static();
	void RenderNormals();
	void RenderBones();
	void RenderBoundingBox();

private:
	Renderer *mpRenderer;

	// Vertices
	int numVertices;
	Vertex *pVertices;

	// Triangles
	int numTriangles;
	Triangle *pTriangles;

	// Materials
	int numMaterials;
	Material_Model *pMaterials;

	// Meshes
	int numMeshes;
	Mesh *pMeshes;

	// Joints
	int numJoints;
	Joint *pJoints;

	// Animation FPS
	float mAnimationFPS;

	// Total animation time
	//double totalTime;

	// Static mesh?
	bool mbStatic;
	unsigned int* mStaticRenderBuffers;

	// Bounding box
	BoundingBox m_BoundingBox;

	friend class MS3DAnimator;
};