// ******************************************************************************
// Filename:    Renderer.h
// Project:     Vox
// Author:      Steven Ball
//
// Revision History:
//   Initial Revision - 12/10/15
//
// Copyright (c) 2005-2016, Steven Ball
// ******************************************************************************

#include "../glew/include/GL/glew.h"

#include "Renderer.h"

// GL ERROR CHECK
int CheckGLErrors(const char *file, int line)
{
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		const GLubyte* sError = gluErrorString(glErr);

		if (sError)
			cout << "GL Error #" << glErr << "(" << gluErrorString(glErr) << ") " << " in File " << file << " at line: " << line << endl;
		else
			cout << "GL Error #" << glErr << " (no message available)" << " in File " << file << " at line: " << line << endl;

		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

Renderer::Renderer(int width, int height, int depthBits, int stencilBits)
{
	m_windowWidth = width;
	m_windowHeight = height;

	m_stencil = false;
	m_depth = false;

	// Default clipping planes
	m_clipNear = 0.1f;
	m_clipFar = 10000.0f;

	// Is depth buffer needed?
	if (depthBits > 0)
	{
		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0f);
		glDepthFunc(GL_LESS);

		m_depth = true;
	}

	// Is stencil buffer needed?
	if (stencilBits > 0)
	{
		glClearStencil(0);

		m_stencil = true;
	}

	// Set clear colour to black
	SetClearColour(0.0f, 0.0f, 0.0f, 1.0f);

	// Enable smooth shading
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Quadratic initialize
	m_Quadratic = gluNewQuadric();
	gluQuadricNormals(m_Quadratic, GLU_SMOOTH);
	gluQuadricTexture(m_Quadratic, GL_TRUE);

	// Initialize defaults
	m_cullMode = CM_NOCULL;
	m_primativeMode = PM_TRIANGLES;
	m_activeViewport = -1;

	// Rendered information
	m_numRenderedVertices = 0;
	m_numRenderedFaces = 0;

	InitOpenGLExtensions();
}

Renderer::~Renderer()
{
	unsigned int i;

	// Delete the vertex arrays
	m_vertexArraysMutex.lock();
	for (i = 0; i < m_vertexArrays.size(); i++)
	{
		delete m_vertexArrays[i];
		m_vertexArrays[i] = 0;
	}
	m_vertexArrays.clear();
	m_vertexArraysMutex.unlock();

	// Delete the viewports
	for (i = 0; i < m_viewports.size(); i++)
	{
		delete m_viewports[i];
		m_viewports[i] = 0;
	}
	m_viewports.clear();

	// Delete the frustums
	for (i = 0; i < m_frustums.size(); i++)
	{
		delete m_frustums[i];
		m_frustums[i] = 0;
	}
	m_frustums.clear();

	// Delete the materials
	for (i = 0; i < m_materials.size(); i++)
	{
		delete m_materials[i];
		m_materials[i] = 0;
	}
	m_materials.clear();

	// Delete the textures
	for (i = 0; i < m_textures.size(); i++)
	{
		delete m_textures[i];
		m_textures[i] = 0;
	}
	m_textures.clear();

	// Delete the lights
	for (i = 0; i < m_lights.size(); i++)
	{
		delete m_lights[i];
		m_lights[i] = 0;
	}
	m_lights.clear();

	// Delete the FreeType fonts
	for (i = 0; i < m_freetypeFonts.size(); i++)
	{
		delete m_freetypeFonts[i];
		m_freetypeFonts[i] = 0;
	}
	m_freetypeFonts.clear();

	// Delete the frame buffers
	for (i = 0; i < m_vFrameBuffers.size(); i++)
	{
		delete m_vFrameBuffers[i];
		m_vFrameBuffers[i] = 0;
	}
	m_vFrameBuffers.clear();

	// Delete the shaders
	for (i = 0; i < m_shaders.size(); i++)
	{
		//delete m_shaders[i];
		//m_shaders[i] = 0;
	}
	m_shaders.clear();

	// Delete the quadratic drawer
	gluDeleteQuadric(m_Quadratic);
}

// Resize
void Renderer::ResizeWindow(int newWidth, int newHeight)
{
	m_windowWidth = newWidth;
	m_windowHeight = newHeight;
}

// Viewport
bool Renderer::CreateViewport(int bottom, int left, int width, int height, float fov, unsigned int *pID)
{
	Viewport* pViewport = new Viewport();
	Frustum* pFrustum = new Frustum();

	pViewport->Bottom = bottom;
	pViewport->Left = left;
	pViewport->Width = width;
	pViewport->Height = height;
	pViewport->Fov = fov;
	pViewport->Aspect = (float)width / (float)height;

	// Create the perspective projection for the viewport
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(fov, pViewport->Aspect, m_clipNear, m_clipFar);
	float mat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, mat);
	pViewport->Perspective = mat;
	glPopMatrix();

	// Setup the frustum for this viewport
	pFrustum->SetFrustum(fov, pViewport->Aspect, m_clipNear, m_clipFar);

	// Push this frustum onto the list
	m_frustums.push_back(pFrustum);

	// Create the orthographic projection matrix for the viewport
	float coordright = 1.0f;
	float coordleft = -1.0f;
	float coordtop = 1.0f;
	float coordbottom = -1.0f;

	memset(&(pViewport->Orthographic), 0, sizeof(Matrix4x4));
	pViewport->Orthographic.m[0] = 2.0f / (coordright - coordleft);
	pViewport->Orthographic.m[5] = 2.0f / (coordtop - coordbottom);
	pViewport->Orthographic.m[10] = -2.0f / (m_clipFar - m_clipNear);
	pViewport->Orthographic.m[12] = -(coordright + coordleft) / (coordright - coordleft);
	pViewport->Orthographic.m[13] = -(coordtop + coordbottom) / (coordtop - coordbottom);
	pViewport->Orthographic.m[14] = -(m_clipFar + m_clipNear) / (m_clipFar - m_clipNear);
	pViewport->Orthographic.m[15] = 1.0f;

	// Create the 2d projection matrix for the viewport
	coordright = (float)m_windowWidth;
	coordleft = 0.0f;
	coordtop = (float)m_windowHeight;
	coordbottom = 0.0f;

	memset(&(pViewport->Projection2d), 0, sizeof(Matrix4x4));
	pViewport->Projection2d.m[0] = 2.0f / (coordright - coordleft);
	pViewport->Projection2d.m[5] = 2.0f / (coordtop - coordbottom);
	pViewport->Projection2d.m[10] = -2.0f / (m_clipFar - m_clipNear);
	pViewport->Projection2d.m[12] = -(coordright + coordleft) / (coordright - coordleft);
	pViewport->Projection2d.m[13] = -(coordtop + coordbottom) / (coordtop - coordbottom);
	pViewport->Projection2d.m[14] = -(m_clipFar + m_clipNear) / (m_clipFar - m_clipNear);
	pViewport->Projection2d.m[15] = 1.0f;

	// Push this viewport onto the list
	m_viewports.push_back(pViewport);

	// Return the viewport id
	*pID = (int)m_viewports.size() - 1;

	return true;
}


bool Renderer::ResizeViewport(unsigned int viewportid, int bottom, int left, int width, int height, float fov)
{
	Viewport* pViewport = m_viewports[viewportid];
	Frustum* pFrustum = m_frustums[viewportid];

	pViewport->Bottom = bottom;
	pViewport->Left = left;
	pViewport->Width = width;
	pViewport->Height = height;
	pViewport->Fov = fov;
	pViewport->Aspect = (float)width / (float)height;

	// Create the perspective projection for the viewport
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(pViewport->Fov, pViewport->Aspect, m_clipNear, m_clipFar);
	float mat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, mat);
	pViewport->Perspective = mat;
	glPopMatrix();

	// Resize the frustum
	pFrustum->SetFrustum(pViewport->Fov, pViewport->Aspect, m_clipNear, m_clipFar);

	// Create the orthographic projection matrix for the viewport
	float coordright = 1.0f;
	float coordleft = -1.0f;
	float coordtop = 1.0f;
	float coordbottom = -1.0f;

	memset(&(pViewport->Orthographic), 0, sizeof(Matrix4x4));
	pViewport->Orthographic.m[0] = 2.0f / (coordright - coordleft);
	pViewport->Orthographic.m[5] = 2.0f / (coordtop - coordbottom);
	pViewport->Orthographic.m[10] = -2.0f / (m_clipFar - m_clipNear);
	pViewport->Orthographic.m[12] = -(coordright + coordleft) / (coordright - coordleft);
	pViewport->Orthographic.m[13] = -(coordtop + coordbottom) / (coordtop - coordbottom);
	pViewport->Orthographic.m[14] = -(m_clipFar + m_clipNear) / (m_clipFar - m_clipNear);
	pViewport->Orthographic.m[15] = 1.0f;

	// Create the 2d projection matrix for the viewport
	coordright = (float)m_windowWidth;
	coordleft = 0.0f;
	coordtop = (float)m_windowHeight;
	coordbottom = 0.0f;

	memset(&(pViewport->Projection2d), 0, sizeof(Matrix4x4));
	pViewport->Projection2d.m[0] = 2.0f / (coordright - coordleft);
	pViewport->Projection2d.m[5] = 2.0f / (coordtop - coordbottom);
	pViewport->Projection2d.m[10] = -2.0f / (m_clipFar - m_clipNear);
	pViewport->Projection2d.m[12] = -(coordright + coordleft) / (coordright - coordleft);
	pViewport->Projection2d.m[13] = -(coordtop + coordbottom) / (coordtop - coordbottom);
	pViewport->Projection2d.m[14] = -(m_clipFar + m_clipNear) / (m_clipFar - m_clipNear);
	pViewport->Projection2d.m[15] = 1.0f;

	return true;
}

int Renderer::GetActiveViewPort()
{
	return m_activeViewport;
}

// Render modes
void Renderer::SetRenderMode(RenderMode mode)
{
	switch (mode)
	{
	case RM_WIREFRAME:
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
		break;
	case RM_SOLID:
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
		break;
	case RM_SHADED:
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
		break;
	case RM_TEXTURED:
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
		break;
	case RM_TEXTURED_LIGHTING:
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
		break;
	};
}

void Renderer::SetPrimativeMode(PrimativeMode mode)
{
	switch (mode)
	{
	case PM_POINTS:
		m_primativeMode = GL_POINTS;
		break;
	case PM_LINES:
		m_primativeMode = GL_LINES;
		break;
	case PM_LINELIST:
		m_primativeMode = GL_LINE_STRIP;
		break;
	case PM_TRIANGLES:
		m_primativeMode = GL_TRIANGLES;
		break;
	case PM_TRIANGLESTRIPS:
		m_primativeMode = GL_TRIANGLE_STRIP;
		break;
	case PM_TRIANGLEFANS:
		m_primativeMode = GL_TRIANGLE_FAN;
		break;
	case PM_QUADS:
		m_primativeMode = GL_QUADS;
		break;
	}
}

void Renderer::SetCullMode(CullMode mode)
{
	m_cullMode = mode;

	switch (mode)
	{
	case CM_NOCULL:
		glDisable(GL_CULL_FACE);
		break;
	case CM_FRONT:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case CM_BACK:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	}
}

CullMode Renderer::GetCullMode()
{
	return m_cullMode;
}

void Renderer::SetLineWidth(float width)
{
	glLineWidth(width);
}

void Renderer::SetPointSize(float width)
{
	glPointSize(width);
}

void Renderer::SetFrontFaceDirection(FrontFaceDirection direction)
{
	if(direction == FrontFaceDirection_CW)
	{
		glFrontFace(GL_CW);
	}
	else if (direction == FrontFaceDirection_CCW)
	{
		glFrontFace(GL_CCW);
	}
}

// Projection
bool Renderer::SetProjectionMode(ProjectionMode mode, int viewPort)
{
	Viewport* pVeiwport = m_viewports[viewPort];
	glViewport(pVeiwport->Left, pVeiwport->Bottom, pVeiwport->Width, pVeiwport->Height);

	m_activeViewport = viewPort;

	if (mode == PM_PERSPECTIVE) {
		m_projection = &(pVeiwport->Perspective);
	}
	else if (mode == PM_ORTHOGRAPHIC) {
		m_projection = &(pVeiwport->Orthographic);
	}
	else if (mode == PM_2D) {
		m_projection = &(pVeiwport->Projection2d);
	}
	else {
		return false;
	}

	SetViewProjection();

	return true;
}

void Renderer::SetViewProjection()
{
	glMatrixMode(GL_PROJECTION);
	MultViewProjection();
	glMatrixMode(GL_MODELVIEW);
}

void Renderer::MultViewProjection()
{
	SetWorldMatrix(m_view * (*m_projection));
}

void Renderer::SetupOrthographicProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Scene
bool Renderer::ClearScene(bool pixel, bool depth, bool stencil)
{
	GLbitfield clear(0);

	if (pixel)
		clear |= GL_COLOR_BUFFER_BIT;
	if (depth && m_depth)
		clear |= GL_DEPTH_BUFFER_BIT;
	if (stencil && m_stencil)
		clear |= GL_STENCIL_BUFFER_BIT;

	glClear(clear);

	return true;
}

bool Renderer::BeginScene(bool pixel, bool depth, bool stencil)
{
	ClearScene(pixel, depth, stencil);

	// Reset the renderer stat counters
	ResetRenderedStats();

	// Reset the projection and modelview matrices to be identity
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	IdentityWorldMatrix();

	// Start off with lighting and texturing disabled. If these are required, they need to be set explicitly
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	return true;
}

void Renderer::EndScene()
{
	// Swap buffers
}

void Renderer::SetColourMask(bool red, bool green, bool blue, bool alpha)
{
	glColorMask(red, green, blue, alpha);
}

void Renderer::SetClearColour(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
}

void Renderer::SetClearColour(Colour col)
{
	glClearColor(col.GetRed(), col.GetGreen(), col.GetBlue(), col.GetAlpha());
}

// Push / Pop matrix stack
void Renderer::PushMatrix()
{
	glPushMatrix();

	m_modelStack.push_back(m_model);
}

void Renderer::PopMatrix()
{
	glPopMatrix();

	m_model = m_modelStack.back();
	m_modelStack.pop_back();
}

// Matrix manipulations
void Renderer::SetWorldMatrix(const Matrix4x4& mat)
{
	float m[16];
	mat.GetMatrix(m);
	glLoadMatrixf(m);
}

void Renderer::GetModelViewMatrix(Matrix4x4 *pMat)
{
	float mat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mat);
	memcpy(pMat->m, mat, 16 * sizeof(float));
}

void Renderer::GetModelMatrix(Matrix4x4 *pMat)
{
	memcpy(pMat->m, m_model.m, 16 * sizeof(float));
}

void Renderer::GetViewMatrix(Matrix4x4 *pMat)
{
	memcpy(pMat->m, m_view.m, 16 * sizeof(float));
}

void Renderer::GetProjectionMatrix(Matrix4x4 *pMat)
{
	float mat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, mat);
	memcpy(pMat->m, mat, 16 * sizeof(float));
	//memcpy(pMat->m, m_projection->m, 16 * sizeof(float));
}

void Renderer::IdentityWorldMatrix()
{
	glLoadIdentity();

	m_model.LoadIdentity();
}

void Renderer::MultiplyWorldMatrix(const Matrix4x4 &mat)
{
	float m[16];
	mat.GetMatrix(m);
	glMultMatrixf(m);

	Matrix4x4 world(mat);
	m_model = world * m_model;
}

void Renderer::TranslateWorldMatrix(float x, float y, float z)
{
	glTranslatef(x, y, z);

	Matrix4x4 translate;
	translate.SetTranslation(vec3(x, y, z));
	m_model = translate * m_model;
}

void Renderer::RotateWorldMatrix(float x, float y, float z)
{
	// Posible gimbal lock?
	glRotatef(z, 0.0f, 0.0f, 1.0f);
	glRotatef(y, 0.0f, 1.0f, 0.0f);
	glRotatef(x, 1.0f, 0.0f, 0.0f);

	Matrix4x4 rotX;
	Matrix4x4 rotY;
	Matrix4x4 rotZ;
	rotX.SetXRotation(DegToRad(x));
	rotY.SetYRotation(DegToRad(y));
	rotZ.SetZRotation(DegToRad(z));

	m_model = rotZ * rotY * rotX * m_model;
}

void Renderer::ScaleWorldMatrix(float x, float y, float z)
{
	glScalef(x, y, z);

	Matrix4x4 scale;
	scale.SetScale(vec3(x, y, z));
	m_model = scale * m_model;
}

// Texture matrix manipulations
void Renderer::SetTextureMatrix()
{
	static double modelView[16];
	static double projection[16];

	// This is matrix transform every coordinate x,y,z
	// x = x* 0.5 + 0.5 
	// y = y* 0.5 + 0.5 
	// z = z* 0.5 + 0.5 
	// Moving from unit cube [-1,1] to [0,1]  
	const GLdouble bias[16] = {
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0 };

	// Grab modelview and transformation matrices
	glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);


	glMatrixMode(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE7);

	glLoadIdentity();
	glLoadMatrixd(bias);

	// concatating all matrice into one.
	glMultMatrixd(projection);
	glMultMatrixd(modelView);

	// Go back to normal matrix mode
	glMatrixMode(GL_MODELVIEW);
}

void Renderer::PushTextureMatrix()
{
	glMatrixMode(GL_TEXTURE);
	glActiveTextureARB(GL_TEXTURE7);
	glPushMatrix();
}

void Renderer::PopTextureMatrix()
{
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

// Scissor testing
void Renderer::EnableScissorTest(int x, int y, int width, int height)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y, width, height);
}

void Renderer::DisableScissorTest()
{
	glDisable(GL_SCISSOR_TEST);
}

// Screen projection
vec3 Renderer::GetWorldProjectionFromScreenCoordinates(int x, int y, float z)
{
	double mvmatrix[16];
	double projmatrix[16];
	int viewport[4];
	double dX, dY, dZ, dClickY;

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
	dClickY = double(m_windowHeight - y);

	// Get the z co-ordinate from the depth buffer
	//GLfloat winZ;
	//glReadPixels((double)x, dClickY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject((double)x, dClickY, z, mvmatrix, projmatrix, viewport, &dX, &dY, &dZ);
	return vec3((float)dX, (float)dY, (float)dZ);
}

void Renderer::GetScreenCoordinatesFromWorldPosition(vec3 pos, int *x, int *y)
{
	// NOTE : Projection and camera must be set before calling this function, else you wont get 'camera-relative' results...

	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble winx, winy, winz;
	gluProject(pos.x, pos.y, pos.z, model_view, projection, viewport, &winx, &winy, &winz);

	(*x) = (int)winx;
	(*y) = (int)winy;
}

// Clip planes
void Renderer::EnableClipPlane(unsigned int index, double eq1, double eq2, double eq3, double eq4)
{
	double plane[] = { eq1, eq2, eq3, eq4 };
	glClipPlane(GL_CLIP_PLANE0 + index, plane);
	glEnable(GL_CLIP_PLANE0 + index);
}

void Renderer::DisableClipPlane(unsigned int index)
{
	glDisable(GL_CLIP_PLANE0);
}

// Camera functionality
void Renderer::SetLookAtCamera(vec3 pos, vec3 target, vec3 up)
{
	gluLookAt(pos.x, pos.y, pos.z, target.x, target.y, target.z, up.x, up.y, up.z);
}

// Transparency
void Renderer::EnableTransparency(BlendFunction source, BlendFunction destination)
{
	//glDisable(GL_DEPTH_WRITEMASK);
	glEnable(GL_BLEND);
	glBlendFunc(GetBlendEnum(source), GetBlendEnum(destination));
}

void Renderer::DisableTransparency()
{
	glDisable(GL_BLEND);
	//glEnable(GL_DEPTH_WRITEMASK);
}

GLenum Renderer::GetBlendEnum(BlendFunction flag)
{
	GLenum glFlag;
	switch (flag)
	{
	case BF_ONE:
		glFlag = GL_ONE;
		break;
	case BF_ZERO:
		glFlag = GL_ZERO;
		break;
	case BF_SRC_ALPHA:
		glFlag = GL_SRC_ALPHA;
		break;
	case BF_ONE_MINUS_SRC_ALPHA:
		glFlag = GL_ONE_MINUS_SRC_ALPHA;
		break;
	default: assert(false);
	}

	return glFlag;
}

// Sampling
void Renderer::EnableMultiSampling()
{
	glEnable(GL_MULTISAMPLE_ARB);
}

void Renderer::DisableMultiSampling()
{
	glDisable(GL_MULTISAMPLE_ARB);
}

// Vector normalize
void Renderer::EnableVectorNormalize()
{
	glEnable(GL_NORMALIZE);
}

void Renderer::DisableVectorNormalize()
{
	glDisable(GL_NORMALIZE);
}

// Depth testing
void Renderer::EnableDepthTest(DepthTest lTestFunction)
{
	glEnable(GL_DEPTH_TEST);

	glDepthFunc(GetDepthTest(lTestFunction));
}

void Renderer::DisableDepthTest()
{
	glDisable(GL_DEPTH_TEST);
}

GLenum Renderer::GetDepthTest(DepthTest lTest)
{
	GLenum glFlag;
	switch (lTest)
	{
	case DT_NEVER:
		glFlag = GL_NEVER;
		break;
	case DT_ALWAYS:
		glFlag = GL_ALWAYS;
		break;
	case DT_LESS:
		glFlag = GL_LESS;
		break;
	case DT_LEQUAL:
		glFlag = GL_LEQUAL;
		break;
	case DT_EQUAL:
		glFlag = GL_EQUAL;
		break;
	case DT_GEQUAL:
		glFlag = GL_GEQUAL;
		break;
	case DT_GREATER:
		glFlag = GL_GREATER;
		break;
	case DT_NOTEQUAL:
		glFlag = GL_NOTEQUAL;
		break;
	default: assert(false);
	}

	return glFlag;
}

void Renderer::EnableDepthWrite()
{
	glDepthMask(GL_TRUE);
}

void Renderer::DisableDepthWrite()
{
	glDepthMask(GL_FALSE);
}

// Colour material
void Renderer::EnableColourMaterial()
{
	glEnable(GL_COLOR_MATERIAL);
}

void Renderer::DisableColourMaterial()
{
	glDisable(GL_COLOR_MATERIAL);
}

// Immediate mode
void Renderer::EnableImmediateMode(ImmediateModePrimitive mode)
{
	GLenum glMode;
	switch (mode)
	{
	case IM_POINTS:
		glMode = GL_POINTS;
		break;
	case IM_LINES:
		glMode = GL_LINES;
		break;
	case IM_LINE_LOOP:
		glMode = GL_LINE_LOOP;
		break;
	case IM_LINE_STRIP:
		glMode = GL_LINE_STRIP;
		break;
	case IM_TRIANGLES:
		glMode = GL_TRIANGLES;
		break;
	case IM_TRIANGLE_STRIP:
		glMode = GL_TRIANGLE_STRIP;
		break;
	case IM_TRIANGLE_FAN:
		glMode = GL_TRIANGLE_FAN;
		break;
	case IM_QUADS:
		glMode = GL_QUADS;
		break;
	case IM_QUAD_STRIP:
		glMode = GL_QUAD_STRIP;
		break;
	case IM_POLYGON:
		glMode = GL_POLYGON;
		break;
	default:
		assert(false);
	}

	glBegin(glMode);
}

void Renderer::ImmediateVertex(float x, float y, float z)
{
	glVertex3f(x, y, z);
}

void Renderer::ImmediateVertex(int x, int y, int z)
{
	glVertex3i(x, y, z);
}

void Renderer::ImmediateNormal(float x, float y, float z)
{
	glNormal3f(x, y, z);
}

void Renderer::ImmediateNormal(int x, int y, int z)
{
	glNormal3i(x, y, z);
}

void Renderer::ImmediateTextureCoordinate(float s, float t)
{
	glTexCoord2f(s, t);
}

void Renderer::ImmediateColourAlpha(float r, float g, float b, float a)
{
	glColor4f(r, g, b, a);
}

void Renderer::DisableImmediateMode()
{
	glEnd();
}

// Drawing helpers
void Renderer::DrawLineCircle(float lRadius, int lPoints)
{
	glBegin(GL_LINE_LOOP);

	float lAngleRatio = DegToRad(360.0f / lPoints);
	for (float i = 0.0f; i < lPoints; i += 1.0f)
	{
		float angle = i * lAngleRatio;
		glVertex3f(cos(angle) * lRadius, 0.0f, sin(angle) * lRadius);
	}

	glEnd();
}

void Renderer::DrawSphere(float lRadius, int lSlices, int lStacks)
{
	gluSphere(m_Quadratic, lRadius, lSlices, lStacks);
}

void Renderer::DrawBezier(Bezier3 curve, int lPoints)
{
	glBegin(GL_LINE_STRIP);

	float ratio = 1.0f / (float)lPoints;
	for (float i = 0.0f; i <= 1.0f; i += ratio)
	{
		vec3 point = curve.GetInterpolatedPoint(i);

		glVertex3f(point.x, point.y, point.z);
	}

	vec3 point = curve.GetInterpolatedPoint(1.0f);
	glVertex3f(point.x, point.y, point.z);

	glEnd();
}

void Renderer::DrawBezier(Bezier4 curve, int lPoints)
{
	glBegin(GL_LINE_STRIP);

	float ratio = 1.0f / (float)lPoints;
	for (float i = 0.0f; i <= 1.0f; i += ratio)
	{
		vec3 point = curve.GetInterpolatedPoint(i);

		glVertex3f(point.x, point.y, point.z);
	}

	vec3 point = curve.GetInterpolatedPoint(1.0f);
	glVertex3f(point.x, point.y, point.z);

	glEnd();
}

void Renderer::DrawCircleSector(float lRadius, float angle, int lPoints)
{
	glBegin(GL_LINE_LOOP);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(cos(angle) * lRadius, 0.0f, sin(angle) * lRadius);

	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(cos(-angle) * lRadius, 0.0f, sin(-angle) * lRadius);

	float lAngleRatio = DegToRad(RadToDeg(angle*2.0f) / lPoints);
	for (float i = 0.0f; i <= lPoints; i += 1.0f)
	{
		float newAngle = -angle + i * lAngleRatio;
		glVertex3f(cos(newAngle) * lRadius, 0.0f, sin(newAngle) * lRadius);
	}

	glEnd();
}

void Renderer::DrawSphericalSector(float lRadius, float angle, int lSectors, int lPoints)
{
	float lAngleRatio = 360.0f / lSectors;
	for (float i = 0.0f; i <= lSectors; i += 1.0f)
	{
		float rotateAngle = i * lAngleRatio;
		PushMatrix();
		RotateWorldMatrix(rotateAngle, 0.0f, 0.0f);
		DrawCircleSector(lRadius, angle, lPoints);
		PopMatrix();
	}
}

// Text rendering
bool Renderer::CreateFreeTypeFont(const char *fontName, int fontSize, unsigned int *pID, bool noAutoHint)
{
	FreeTypeFont* font = new FreeTypeFont();

	// Build the new freetype font
	font->BuildFont(fontName, fontSize, noAutoHint);

	// Push this font onto the list of fonts and return the id
	m_freetypeFonts.push_back(font);
	*pID = (unsigned int)m_freetypeFonts.size() - 1;

	return true;
}

bool Renderer::RenderFreeTypeText(unsigned int fontID, float x, float y, float z, Colour colour, float scale, const char *inText, ...)
{
	char		outText[8192];
	va_list		ap;  // Pointer to list of arguments

	if (inText == nullptr)
		return false;  // Return fail if there is no text

	// Loop through variable argument list and add them to the string
	va_start(ap, inText);
		vsprintf(outText, inText, ap);
	va_end(ap);

	glColor4fv(colour.GetRGBA());

	// Add on the descent value, so we don't draw letters with underhang out of bounds. (e.g - g, y, q and p)
	y -= GetFreeTypeTextDescent(fontID);

	// HACK : The descent has rounding errors and is usually off by about 1 pixel
	y -= 1;

	glPushMatrix();
		glTranslatef(x, y, 0);
		m_freetypeFonts[fontID]->DrawString(outText, scale);
	glPopMatrix();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	return true;
}

int Renderer::GetFreeTypeTextWidth(unsigned int fontID, const char *inText, ...)
{
	char outText[8192];
	va_list ap;

	if (inText == nullptr)
		return 0;

	// Loop through variable argument list and add them to the string
	va_start(ap, inText);
		vsprintf(outText, inText, ap);
	va_end(ap);

	return m_freetypeFonts[fontID]->GetTextWidth(outText);
}

int Renderer::GetFreeTypeTextHeight(unsigned int fontID, const char *inText, ...)
{
	return m_freetypeFonts[fontID]->GetCharHeight('a');
}

int Renderer::GetFreeTypeTextAscent(unsigned int fontID)
{
	return m_freetypeFonts[fontID]->GetAscent();
}

int Renderer::GetFreeTypeTextDescent(unsigned int fontID)
{
	return m_freetypeFonts[fontID]->GetDescent();
}

// Lighting
bool Renderer::CreateLight(const Colour &ambient, const Colour &diffuse, const Colour &specular, vec3 &position, vec3 &direction, float exponent, float cutoff, float cAtten, float lAtten, float qAtten, bool point, bool spot, unsigned int *pID)
{
	Light *pLight = new Light();

	pLight->Ambient(ambient);
	pLight->Diffuse(diffuse);
	pLight->Specular(specular);
	pLight->Position(position);
	pLight->Direction(direction);
	pLight->Exponent(exponent);
	pLight->Cutoff(cutoff);
	pLight->ConstantAttenuation(cAtten);
	pLight->LinearAttenuation(lAtten);
	pLight->QuadraticAttenuation(qAtten);
	pLight->Point(point);
	pLight->Spotlight(spot);

	// Push the light onto the list
	m_lights.push_back(pLight);

	// Return the light id
	*pID = (int)m_lights.size() - 1;

	return true;
}

bool Renderer::EditLight(unsigned int id, const Colour &ambient, const Colour &diffuse, const Colour &specular, vec3 &position, vec3 &direction, float exponent, float cutoff, float cAtten, float lAtten, float qAtten, bool point, bool spot)
{
	Light *pLight = m_lights[id];

	pLight->Ambient(ambient);
	pLight->Diffuse(diffuse);
	pLight->Specular(specular);
	pLight->Position(position);
	pLight->Direction(direction);
	pLight->Exponent(exponent);
	pLight->Cutoff(cutoff);
	pLight->ConstantAttenuation(cAtten);
	pLight->LinearAttenuation(lAtten);
	pLight->QuadraticAttenuation(qAtten);
	pLight->Point(point);
	pLight->Spotlight(spot);

	return true;
}

bool Renderer::EditLightPosition(unsigned int id, vec3 &position)
{
	Light *pLight = m_lights[id];

	pLight->Position(position);

	return true;
}

void Renderer::DeleteLight(unsigned int id)
{
	if (m_lights[id]) {
		delete m_lights[id];
		m_lights[id] = 0;
	}
}

void Renderer::EnableLight(unsigned int id, unsigned int lightNumber)
{
	if (m_lights[id])
	{
		m_lights[id]->Apply(lightNumber);
	}
}

void Renderer::DisableLight(unsigned int lightNumber)
{
	glDisable(GL_LIGHT0 + lightNumber);
}

void Renderer::RenderLight(unsigned int id)
{
	m_lights[id]->Render();
}

Colour Renderer::GetLightAmbient(unsigned int id)
{
	return m_lights[id]->Ambient();
}

Colour Renderer::GetLightDiffuse(unsigned int id)
{
	return m_lights[id]->Diffuse();
}

Colour Renderer::GetLightSpecular(unsigned int id)
{
	return m_lights[id]->Specular();
}

vec3 Renderer::GetLightPosition(unsigned int id)
{
	return m_lights[id]->Position();
}

float Renderer::GetConstantAttenuation(unsigned int id)
{
	return m_lights[id]->ConstantAttenuation();
}

float Renderer::GetLinearAttenuation(unsigned int id)
{
	return m_lights[id]->LinearAttenuation();
}

float Renderer::GetQuadraticAttenuation(unsigned int id)
{
	return m_lights[id]->QuadraticAttenuation();
}

// Materials
bool Renderer::CreateMaterial(const Colour &ambient, const Colour &diffuse, const Colour &specular, const Colour &emmisive, float specularPower, unsigned int *pID)
{
	Material *pMaterial = new Material();

	pMaterial->Ambient(ambient);
	pMaterial->Diffuse(diffuse);
	pMaterial->Specular(specular);
	pMaterial->Emission(emmisive);
	pMaterial->Shininess(specularPower);

	// Push the material onto the list
	m_materials.push_back(pMaterial);

	// Return the material id
	*pID = (int)m_materials.size() - 1;

	return true;
}

bool Renderer::EditMaterial(unsigned int id, const Colour &ambient, const Colour &diffuse, const Colour &specular, const Colour &emmisive, float specularPower)
{
	Material *pMaterial = m_materials[id];

	pMaterial->Ambient(ambient);
	pMaterial->Diffuse(diffuse);
	pMaterial->Specular(specular);
	pMaterial->Emission(emmisive);
	pMaterial->Shininess(specularPower);

	return true;
}

void Renderer::EnableMaterial(unsigned int id)
{
	m_materials[id]->Apply();
}

void Renderer::DeleteMaterial(unsigned int id)
{
	if (m_materials[id])
	{
		delete m_materials[id];
		m_materials[id] = 0;
	}
}

// Textures
bool Renderer::LoadTexture(string fileName, int *width, int *height, int *width_power2, int *height_power2, unsigned int *pID)
{
	// Check that this texture hasn't already been loaded
	for (unsigned int i = 0; i < m_textures.size(); i++)
	{
		if (m_textures[i]->GetFileName() == fileName)
		{
			*width = m_textures[i]->GetWidth();
			*height = m_textures[i]->GetHeight();
			*width_power2 = m_textures[i]->GetWidthPower2();
			*height_power2 = m_textures[i]->GetHeightPower2();
			*pID = i;

			return true;
		}
	}

	// Texture hasn't already been loaded, create and load it!
	Texture *pTexture = new Texture();
	pTexture->Load(fileName, width, height, width_power2, height_power2, false);

	// Push the vertex array onto the list
	m_textures.push_back(pTexture);

	// Return the vertex array id
	*pID = (int)m_textures.size() - 1;

	return true;
}

bool Renderer::RefreshTexture(unsigned int id)
{
	Texture *pTexture = m_textures[id];

	int width;
	int height;
	int width_power2;
	int height_power2;
	pTexture->Load(pTexture->GetFileName(), &width, &height, &width_power2, &height_power2, true);

	return true;
}

bool Renderer::RefreshTexture(string filename)
{
	for (unsigned int i = 0; i < m_textures.size(); i++)
	{
		if (m_textures[i]->GetFileName() == filename)
		{
			return RefreshTexture(i);
		}
	}

	return false;
}

void Renderer::BindTexture(unsigned int id)
{
	glEnable(GL_TEXTURE_2D);
	m_textures[id]->Bind();
}

void Renderer::PrepareShaderTexture(unsigned int textureIndex, unsigned int textureId)
{
	glActiveTextureARB(GL_TEXTURE0_ARB + textureIndex);
	glUniform1iARB(textureId, textureIndex);
}

void Renderer::EmptyTextureIndex(unsigned int textureIndex)
{
	glActiveTextureARB(GL_TEXTURE0_ARB + textureIndex);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureIndex);
}

void Renderer::DisableTexture()
{
	glDisable(GL_TEXTURE_2D);
}

Texture* Renderer::GetTexture(unsigned int id)
{
	return m_textures[id];
}

void Renderer::BindRawTextureId(unsigned int textureId)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureId);
}

void Renderer::GenerateEmptyTexture(unsigned int *pID)
{
	Texture *pTexture = new Texture();
	pTexture->GenerateEmptyTexture();

	// Push the vertex array onto the list
	m_textures.push_back(pTexture);

	// Return the vertex array id
	*pID = (int)m_textures.size() - 1;
}

void Renderer::SetTextureData(unsigned int id, int width, int height, unsigned char *texdata)
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_textures[id]->GetId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);
	glDisable(GL_TEXTURE_2D);
}

// Cube textures
bool Renderer::LoadCubeTexture(int *width, int *height, string front, string back, string top, string bottom, string left, string right, unsigned int *pID)
{
	bool loaded = false;
	unsigned char *texdataFront = 0;
	unsigned char *texdataBack = 0;
	unsigned char *texdataTop = 0;
	unsigned char *texdataBottom = 0;
	unsigned char *texdataLeft = 0;
	unsigned char *texdataRight = 0;

	// Only support TGA cubemaps for now!
	loaded = LoadFileTGA(front.c_str(), &texdataFront, width, height, true) == 1;
	loaded = LoadFileTGA(back.c_str(), &texdataBack, width, height, true) == 1;
	loaded = LoadFileTGA(top.c_str(), &texdataTop, width, height, true) == 1;
	loaded = LoadFileTGA(bottom.c_str(), &texdataBottom, width, height, true) == 1;
	loaded = LoadFileTGA(left.c_str(), &texdataLeft, width, height, true) == 1;
	loaded = LoadFileTGA(right.c_str(), &texdataRight, width, height, true) == 1;

	GLuint id;
	glEnable(GL_TEXTURE_CUBE_MAP);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdataFront);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdataBack);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdataTop);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdataBottom);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdataLeft);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdataRight);

	glDisable(GL_TEXTURE_CUBE_MAP);

	*pID = id;

	return true;
}

void Renderer::BindCubeTexture(unsigned int id)
{
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

void Renderer::EmptyCubeTextureIndex(unsigned int textureIndex)
{
	glActiveTextureARB(GL_TEXTURE0_ARB + textureIndex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureIndex);
	glDisable(GL_TEXTURE_CUBE_MAP);
}

void Renderer::DisableCubeTexture()
{
	glDisable(GL_TEXTURE_CUBE_MAP);
}

// Vertex buffers
bool Renderer::CreateStaticBuffer(VertexType type, unsigned int materialID, unsigned int textureID, int nVerts, int nTextureCoordinates, int nIndices, const void *pVerts, const void *pTextureCoordinates, const unsigned int *pIndices, unsigned int *pID)
{
	VertexArray *pVertexArray = new VertexArray();

	pVertexArray->nIndices = nIndices;
	pVertexArray->nVerts = nVerts;
	pVertexArray->nTextureCoordinates = nTextureCoordinates;
	pVertexArray->materialID = materialID;
	pVertexArray->textureID = textureID;
	pVertexArray->type = type;

	// Get the correct vertex size and construct the vertex array to hold the vertices
	if (nVerts)
	{
		switch (type)
		{
		case VT_POSITION:
			pVertexArray->vertexSize = sizeof(OGLPositionVertex);
			pVertexArray->pVA = new float[nVerts * 3];
			break;
		case VT_POSITION_DIFFUSE:
			pVertexArray->vertexSize = sizeof(OGLPositionDiffuseVertex);
			pVertexArray->pVA = new float[nVerts * 6];
			break;
		case VT_POSITION_DIFFUSE_ALPHA:
			pVertexArray->vertexSize = sizeof(OGLPositionDiffuseAlphaVertex);
			pVertexArray->pVA = new float[nVerts * 7];
			break;
		case VT_POSITION_NORMAL:
			pVertexArray->vertexSize = sizeof(OGLPositionNormalVertex);
			pVertexArray->pVA = new float[nVerts * 6];
			break;
		case VT_POSITION_NORMAL_COLOUR:
			pVertexArray->vertexSize = sizeof(OGLPositionNormalColourVertex);
			pVertexArray->pVA = new float[nVerts * 10];
			break;
		case VT_POSITION_NORMAL_UV:
			pVertexArray->vertexSize = sizeof(OGLPositionNormalVertex);
			pVertexArray->pVA = new float[nVerts * 6];
			pVertexArray->textureCoordinateSize = sizeof(OGLUVCoordinate);
			pVertexArray->pTextureCoordinates = new float[nTextureCoordinates * 2];
			break;
		case VT_POSITION_NORMAL_UV_COLOUR:
			pVertexArray->vertexSize = sizeof(OGLPositionNormalColourVertex);
			pVertexArray->pVA = new float[nVerts * 10];
			pVertexArray->textureCoordinateSize = sizeof(OGLUVCoordinate);
			pVertexArray->pTextureCoordinates = new float[nTextureCoordinates * 2];
			break;
		}
	}


	// If we have indices, create the indices array to hold the information
	if (nIndices)
	{
		pVertexArray->pIndices = new unsigned int[nIndices];
	}

	// Copy the vertices into the vertex array
	memcpy(pVertexArray->pVA, pVerts, pVertexArray->vertexSize*nVerts);

	// Copt the texture coordinates into the texture array
	memcpy(pVertexArray->pTextureCoordinates, pTextureCoordinates, pVertexArray->textureCoordinateSize*nTextureCoordinates);

	// Copy the indices into the vertex array
	memcpy(pVertexArray->pIndices, pIndices, sizeof(unsigned int)*nIndices);

	// Push the vertex array onto the list
	m_vertexArraysMutex.lock();
	m_vertexArrays.push_back(pVertexArray);

	// Return the vertex array id
	*pID = (int)m_vertexArrays.size() - 1;
	m_vertexArraysMutex.unlock();

	return true;
}

bool Renderer::RecreateStaticBuffer(unsigned int ID, VertexType type, unsigned int materialID, unsigned int textureID, int nVerts, int nTextureCoordinates, int nIndices, const void *pVerts, const void *pTextureCoordinates, const unsigned int *pIndices)
{
	m_vertexArraysMutex.lock();

	// Create a new vertex array
	m_vertexArrays[ID] = new VertexArray();

	// Get this already existing array pointer from the list
	VertexArray *pVertexArray = m_vertexArrays[ID];

	pVertexArray->nIndices = nIndices;
	pVertexArray->nVerts = nVerts;
	pVertexArray->nTextureCoordinates = nTextureCoordinates;
	pVertexArray->materialID = materialID;
	pVertexArray->textureID = textureID;
	pVertexArray->type = type;

	// Get the correct vertex size and construct the vertex array to hold the vertices
	if (nVerts)
	{
		switch (type)
		{
		case VT_POSITION:
			pVertexArray->vertexSize = sizeof(OGLPositionVertex);
			pVertexArray->pVA = new float[nVerts * 3];
			break;
		case VT_POSITION_DIFFUSE:
			pVertexArray->vertexSize = sizeof(OGLPositionDiffuseVertex);
			pVertexArray->pVA = new float[nVerts * 6];
			break;
		case VT_POSITION_DIFFUSE_ALPHA:
			pVertexArray->vertexSize = sizeof(OGLPositionDiffuseAlphaVertex);
			pVertexArray->pVA = new float[nVerts * 7];
			break;
		case VT_POSITION_NORMAL:
			pVertexArray->vertexSize = sizeof(OGLPositionNormalVertex);
			pVertexArray->pVA = new float[nVerts * 6];
			break;
		case VT_POSITION_NORMAL_COLOUR:
			pVertexArray->vertexSize = sizeof(OGLPositionNormalColourVertex);
			pVertexArray->pVA = new float[nVerts * 10];
			break;
		case VT_POSITION_NORMAL_UV:
			pVertexArray->vertexSize = sizeof(OGLPositionNormalVertex);
			pVertexArray->pVA = new float[nVerts * 6];
			pVertexArray->textureCoordinateSize = sizeof(OGLUVCoordinate);
			pVertexArray->pTextureCoordinates = new float[nTextureCoordinates * 2];
			break;
		case VT_POSITION_NORMAL_UV_COLOUR:
			pVertexArray->vertexSize = sizeof(OGLPositionNormalColourVertex);
			pVertexArray->pVA = new float[nVerts * 10];
			pVertexArray->textureCoordinateSize = sizeof(OGLUVCoordinate);
			pVertexArray->pTextureCoordinates = new float[nTextureCoordinates * 2];
			break;
		}
	}

	// If we have indices, create the indices array to hold the information
	if (nIndices)
	{
		pVertexArray->pIndices = new unsigned int[nIndices];
	}

	// Copy the vertices into the vertex array
	memcpy(pVertexArray->pVA, pVerts, pVertexArray->vertexSize*nVerts);

	// Copt the texture coordinates into the texture array
	memcpy(pVertexArray->pTextureCoordinates, pTextureCoordinates, pVertexArray->textureCoordinateSize*nTextureCoordinates);

	// Copy the indices into the vertex array
	memcpy(pVertexArray->pIndices, pIndices, sizeof(unsigned int)*nIndices);
	
	m_vertexArraysMutex.unlock();

	return true;
}

void Renderer::DeleteStaticBuffer(unsigned int id)
{
	m_vertexArraysMutex.lock();
	if (m_vertexArrays[id])
	{
		delete m_vertexArrays[id];
		m_vertexArrays[id] = 0;
	}
	m_vertexArraysMutex.unlock();
}

bool Renderer::RenderStaticBuffer(unsigned int id)
{
	m_vertexArraysMutex.lock();

	if (id >= m_vertexArrays.size())
	{
		m_vertexArraysMutex.unlock();
		return false;  // We have supplied an invalid id		
	}

	// Find the vertex array from the list
	VertexArray *pVertexArray = m_vertexArrays[id];

	bool rendered = false;
	if (pVertexArray != nullptr)
	{
		m_numRenderedVertices += pVertexArray->nVerts;
		switch (m_primativeMode)
		{
		case GL_POINTS:
		case GL_LINES:
			m_numRenderedFaces += 0;
			break;
		case GL_TRIANGLES:
			m_numRenderedFaces += (pVertexArray->nIndices / 3);
			break;
		case GL_TRIANGLE_STRIP:
			m_numRenderedFaces += (pVertexArray->nIndices - 2);
			break;
		case GL_TRIANGLE_FAN:
			m_numRenderedFaces += (pVertexArray->nIndices - 2);
			break;
		case GL_QUADS:
			m_numRenderedFaces += (pVertexArray->nIndices / 4);
			break;
		}

		if ((pVertexArray->type != VT_POSITION_DIFFUSE_ALPHA) && (pVertexArray->type != VT_POSITION_DIFFUSE))
		{
			if (pVertexArray->materialID != -1)
			{
				m_materials[pVertexArray->materialID]->Apply();
			}
		}

		if (pVertexArray->type == VT_POSITION_NORMAL_UV || pVertexArray->type == VT_POSITION_NORMAL_UV_COLOUR)
		{
			if (pVertexArray->textureID != -1)
			{
				BindTexture(pVertexArray->textureID);
			}
		}

		// Calculate the stride
		GLsizei totalStride = GetStride(pVertexArray->type);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, totalStride, pVertexArray->pVA);

		if (pVertexArray->type == VT_POSITION_NORMAL || pVertexArray->type == VT_POSITION_NORMAL_UV || pVertexArray->type == VT_POSITION_NORMAL_UV_COLOUR || pVertexArray->type == VT_POSITION_NORMAL_COLOUR)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, totalStride, &pVertexArray->pVA[3]);
		}

		if (pVertexArray->type == VT_POSITION_NORMAL_UV || pVertexArray->type == VT_POSITION_NORMAL_UV_COLOUR)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, pVertexArray->pTextureCoordinates);
		}

		if (pVertexArray->type == VT_POSITION_DIFFUSE_ALPHA)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, totalStride, &pVertexArray->pVA[3]);
		}

		if (pVertexArray->type == VT_POSITION_DIFFUSE)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(3, GL_FLOAT, totalStride, &pVertexArray->pVA[3]);
		}

		if (pVertexArray->type == VT_POSITION_NORMAL_UV_COLOUR || pVertexArray->type == VT_POSITION_NORMAL_COLOUR)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, totalStride, &pVertexArray->pVA[6]);
		}

		if (pVertexArray->nIndices != 0)
		{
			glDrawElements(m_primativeMode, pVertexArray->nIndices, GL_UNSIGNED_INT, pVertexArray->pIndices);
		}
		else
		{
			glDrawArrays(m_primativeMode, 0, pVertexArray->nVerts);
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		rendered =  true;
	}

	m_vertexArraysMutex.unlock();

	return rendered;
}

bool Renderer::RenderStaticBuffer_NoColour(unsigned int id)
{
	m_vertexArraysMutex.lock();

	if (id >= m_vertexArrays.size())
	{
		m_vertexArraysMutex.unlock();
		return false;  // We have supplied an invalid id		
	}

	// Find the vertex array from the list
	VertexArray *pVertexArray = m_vertexArrays[id];

	bool rendered = false;
	if (pVertexArray != nullptr)
	{
		m_numRenderedVertices += pVertexArray->nVerts;
		switch (m_primativeMode)
		{
		case GL_POINTS:
		case GL_LINES:
			m_numRenderedFaces += 0;
			break;
		case GL_TRIANGLES:
			m_numRenderedFaces += (pVertexArray->nIndices / 3);
			break;
		case GL_TRIANGLE_STRIP:
			m_numRenderedFaces += (pVertexArray->nIndices - 2);
			break;
		case GL_TRIANGLE_FAN:
			m_numRenderedFaces += (pVertexArray->nIndices - 2);
			break;
		case GL_QUADS:
			m_numRenderedFaces += (pVertexArray->nIndices / 4);
			break;
		}

		if ((pVertexArray->type != VT_POSITION_DIFFUSE_ALPHA) && (pVertexArray->type != VT_POSITION_DIFFUSE))
		{
			if (pVertexArray->materialID != -1)
			{
				m_materials[pVertexArray->materialID]->Apply();
			}
		}

		if (pVertexArray->type == VT_POSITION_NORMAL_UV || pVertexArray->type == VT_POSITION_NORMAL_UV_COLOUR)
		{
			if (pVertexArray->textureID != -1)
			{
				BindTexture(pVertexArray->textureID);
			}
		}

		// Calculate the stride
		GLsizei totalStride = GetStride(pVertexArray->type);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, totalStride, pVertexArray->pVA);

		if (pVertexArray->type == VT_POSITION_NORMAL || pVertexArray->type == VT_POSITION_NORMAL_UV || pVertexArray->type == VT_POSITION_NORMAL_UV_COLOUR || pVertexArray->type == VT_POSITION_NORMAL_COLOUR)
		{
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, totalStride, &pVertexArray->pVA[3]);
		}

		if (pVertexArray->type == VT_POSITION_NORMAL_UV || pVertexArray->type == VT_POSITION_NORMAL_UV_COLOUR)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, pVertexArray->pTextureCoordinates);
		}

		if (pVertexArray->type == VT_POSITION_DIFFUSE_ALPHA)
		{
			//glEnableClientState(GL_COLOR_ARRAY);
			//glColorPointer(4, GL_FLOAT, totalStride, &pVertexArray->pVA[3]);
		}

		if (pVertexArray->type == VT_POSITION_DIFFUSE)
		{
			//glEnableClientState(GL_COLOR_ARRAY);
			//glColorPointer(3, GL_FLOAT, totalStride, &pVertexArray->pVA[3]);
		}

		if (pVertexArray->type == VT_POSITION_NORMAL_UV_COLOUR || pVertexArray->type == VT_POSITION_NORMAL_COLOUR)
		{
			//glEnableClientState(GL_COLOR_ARRAY);
			//glColorPointer(4, GL_FLOAT, totalStride, &pVertexArray->pVA[6]);
		}

		if (pVertexArray->nIndices != 0)
		{
			glDrawElements(m_primativeMode, pVertexArray->nIndices, GL_UNSIGNED_INT, pVertexArray->pIndices);
		}
		else
		{
			glDrawArrays(m_primativeMode, 0, pVertexArray->nVerts);
		}

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		//glDisableClientState(GL_COLOR_ARRAY);

		rendered = true;
	}

	m_vertexArraysMutex.unlock();

	return rendered;
}

bool Renderer::RenderFromArray(VertexType type, unsigned int materialID, unsigned int textureID, int nVerts, int nTextureCoordinates, int nIndices, const void *pVerts, const void *pTextureCoordinates, const unsigned int *pIndices)
{
	if ((type != VT_POSITION_DIFFUSE_ALPHA) && (type != VT_POSITION_DIFFUSE))
	{
		if (materialID != -1)
		{
			m_materials[materialID]->Apply();
		}
	}

	if (type == VT_POSITION_NORMAL_UV || type == VT_POSITION_NORMAL_UV_COLOUR)
	{
		if (textureID != -1)
		{
			BindTexture(textureID);
		}
	}

	m_numRenderedVertices += nVerts;
	switch (m_primativeMode)
	{
	case GL_POINTS:
	case GL_LINES:
		m_numRenderedFaces += 0;
		break;
	case GL_TRIANGLES:
		m_numRenderedFaces += (nIndices / 3);
		break;
	case GL_TRIANGLE_STRIP:
		m_numRenderedFaces += (nIndices - 2);
		break;
	case GL_TRIANGLE_FAN:
		m_numRenderedFaces += (nIndices - 2);
		break;
	case GL_QUADS:
		m_numRenderedFaces += (nIndices / 4);
		break;
	}

	// Calculate the stride
	GLsizei totalStride = GetStride(type);

	float* pVertsf = (float*)pVerts;
	float* pTexturesf = (float*)pTextureCoordinates;

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, totalStride, pVerts);

	if (type == VT_POSITION_NORMAL || type == VT_POSITION_NORMAL_UV || type == VT_POSITION_NORMAL_UV_COLOUR || type == VT_POSITION_NORMAL_COLOUR)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, totalStride, &pVertsf[3]);
	}

	if (type == VT_POSITION_NORMAL_UV || type == VT_POSITION_NORMAL_UV_COLOUR)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, &pTexturesf);
	}

	if (type == VT_POSITION_DIFFUSE_ALPHA)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, totalStride, &pVertsf[3]);
	}

	if (type == VT_POSITION_DIFFUSE)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_FLOAT, totalStride, &pVertsf[3]);
	}

	if (type == VT_POSITION_NORMAL_UV_COLOUR || type == VT_POSITION_NORMAL_COLOUR)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, totalStride, &pVertsf[6]);
	}

	if (nIndices != 0)
	{
		glDrawElements(m_primativeMode, nIndices, GL_UNSIGNED_INT, pIndices);
	}
	else
	{
		glDrawArrays(m_primativeMode, 0, nVerts);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	return true;
}

unsigned int Renderer::GetStride(VertexType type)
{
	// Add xyz stride
	unsigned int totalStride = sizeof(float) * 3;

	// Add normals stride
	if (type == VT_POSITION_NORMAL || type == VT_POSITION_NORMAL_UV || type == VT_POSITION_NORMAL_UV_COLOUR || type == VT_POSITION_NORMAL_COLOUR)
		totalStride += sizeof(float) * 3;

	// Add colour  stride
	if (type == VT_POSITION_DIFFUSE)
		totalStride += sizeof(float) * 3;

	// Add colour and alpha stride
	if (type == VT_POSITION_DIFFUSE_ALPHA || type == VT_POSITION_NORMAL_UV_COLOUR || type == VT_POSITION_NORMAL_COLOUR)
		totalStride += sizeof(float) * 4;

	return totalStride;
}

// Mesh
OpenGLTriangleMesh* Renderer::CreateMesh(OGLMeshType meshType)
{
	OpenGLTriangleMesh* pNewMesh = new OpenGLTriangleMesh();

	pNewMesh->m_meshType = meshType;

	// Return the mesh pointer
	return pNewMesh;
}

void Renderer::ClearMesh(OpenGLTriangleMesh* pMesh)
{
	pMesh->m_textureId = -1;
	pMesh->m_materialId = -1;
	//pMesh->m_staticMeshId = -1; // DON'T reset this! Else we end up create more and more and more static buffers and data

	// Delete the vertices
	for (unsigned int i = 0; i < pMesh->m_vertices.size(); i++)
	{
		delete pMesh->m_vertices[i];
		pMesh->m_vertices[i] = 0;
	}

	// Delete the texture coordinates
	for (unsigned int i = 0; i < pMesh->m_textureCoordinates.size(); i++)
	{
		delete pMesh->m_textureCoordinates[i];
		pMesh->m_textureCoordinates[i] = 0;
	}

	// Delete the triangles
	for (unsigned int i = 0; i < pMesh->m_triangles.size(); i++)
	{
		delete pMesh->m_triangles[i];
		pMesh->m_triangles[i] = 0;
	}

	pMesh->m_vertices.clear();
	pMesh->m_textureCoordinates.clear();
	pMesh->m_triangles.clear();

	if (pMesh->m_staticMeshId != -1)
	{
		DeleteStaticBuffer(pMesh->m_staticMeshId);
	}
	pMesh->m_staticMeshId = -1;

	delete pMesh;
	pMesh = nullptr;
}

unsigned int Renderer::AddVertexToMesh(vec3 p, vec3 n, float r, float g, float b, float a, OpenGLTriangleMesh* pMesh)
{
	OpenGLMesh_Vertex* pNewVertex = new OpenGLMesh_Vertex();
	pNewVertex->vertexPosition[0] = p.x;
	pNewVertex->vertexPosition[1] = p.y;
	pNewVertex->vertexPosition[2] = p.z;

	pNewVertex->vertexNormals[0] = n.x;
	pNewVertex->vertexNormals[1] = n.y;
	pNewVertex->vertexNormals[2] = n.z;

	pNewVertex->vertexColour[0] = r;
	pNewVertex->vertexColour[1] = g;
	pNewVertex->vertexColour[2] = b;
	pNewVertex->vertexColour[3] = a;

	if (pMesh != nullptr)
	{
		pMesh->m_vertices.push_back(pNewVertex);

		unsigned int vertex_id = (int)pMesh->m_vertices.size() - 1;

		return vertex_id;
	}
	else
	{
		return -1;
	}
}

unsigned int Renderer::AddTextureCoordinatesToMesh(float s, float t, OpenGLTriangleMesh* pMesh)
{
	OpenGLMesh_TextureCoordinate* pNewTextureCoordinate = new OpenGLMesh_TextureCoordinate();
	pNewTextureCoordinate->s = s;
	pNewTextureCoordinate->t = t;

	if (pMesh != nullptr)
	{
		pMesh->m_textureCoordinates.push_back(pNewTextureCoordinate);

		unsigned int textureCoordinate_id = (int)pMesh->m_textureCoordinates.size() - 1;

		return textureCoordinate_id;
	}
	else
	{
		return -1;
	}
}

unsigned int Renderer::AddTriangleToMesh(unsigned int vertexId1, unsigned int vertexId2, unsigned int vertexId3, OpenGLTriangleMesh* pMesh)
{
	// Create the triangle
	OpenGLMesh_Triangle* pTri = new OpenGLMesh_Triangle();
	pTri->vertexIndices[0] = vertexId1;
	pTri->vertexIndices[1] = vertexId2;
	pTri->vertexIndices[2] = vertexId3;

	if (pMesh != nullptr)
	{
		pMesh->m_triangles.push_back(pTri);

		unsigned int tri_id = (int)pMesh->m_triangles.size() - 1;

		return tri_id;
	}
	else
	{
		return -1;
	}
}

void Renderer::ModifyMeshAlpha(float alpha, OpenGLTriangleMesh* pMesh)
{
	m_vertexArraysMutex.lock();
	VertexArray* pArray = m_vertexArrays[pMesh->m_staticMeshId];

	GLsizei totalStride = GetStride(pArray->type) / 4;
	int alphaIndex = totalStride - 1;

	for (int i = 0; i < pArray->nVerts; i++)
	{
		pArray->pVA[alphaIndex] = alpha;

		alphaIndex += totalStride;
	}
	m_vertexArraysMutex.unlock();
}

void Renderer::ModifyMeshColour(float r, float g, float b, OpenGLTriangleMesh* pMesh)
{
	m_vertexArraysMutex.lock();
	VertexArray* pArray = m_vertexArrays[pMesh->m_staticMeshId];

	GLsizei totalStride = GetStride(pArray->type) / 4;
	int rIndex = totalStride - 4;
	int gIndex = totalStride - 3;
	int bIndex = totalStride - 2;

	for (int i = 0; i < pArray->nVerts; i++)
	{
		pArray->pVA[rIndex] = r;
		pArray->pVA[gIndex] = g;
		pArray->pVA[bIndex] = b;

		rIndex += totalStride;
		gIndex += totalStride;
		bIndex += totalStride;
	}
	m_vertexArraysMutex.unlock();
}

void Renderer::ConvertMeshColour(float r, float g, float b, float matchR, float matchG, float matchB, OpenGLTriangleMesh* pMesh)
{
	m_vertexArraysMutex.lock();
	VertexArray* pArray = m_vertexArrays[pMesh->m_staticMeshId];

	GLsizei totalStride = GetStride(pArray->type) / 4;
	int rIndex = totalStride - 4;
	int gIndex = totalStride - 3;
	int bIndex = totalStride - 2;

	for (int i = 0; i < pArray->nVerts; i++)
	{
		float diffR = fabs(pArray->pVA[rIndex] - matchR);
		float diffG = fabs(pArray->pVA[gIndex] - matchG);
		float diffB = fabs(pArray->pVA[bIndex] - matchB);
		if (diffR < 0.005f && diffG < 0.005f && diffB < 0.005f)
		{
			pArray->pVA[rIndex] = r;
			pArray->pVA[gIndex] = g;
			pArray->pVA[bIndex] = b;
		}

		rIndex += totalStride;
		gIndex += totalStride;
		bIndex += totalStride;
	}
	m_vertexArraysMutex.unlock();
}

void Renderer::FinishMesh(unsigned int textureID, unsigned int materialID, OpenGLTriangleMesh* pMesh)
{
	unsigned int numTriangles = (int)pMesh->m_triangles.size();
	unsigned int numVertices = (int)pMesh->m_vertices.size();
	unsigned int numTextureCoordinates = (int)pMesh->m_textureCoordinates.size();
	unsigned int numIndices = (int)pMesh->m_triangles.size() * 3;

	pMesh->m_materialId = materialID;
	pMesh->m_textureId = textureID;

	// Vertices
	OGLPositionNormalColourVertex* meshBuffer;
	meshBuffer = new OGLPositionNormalColourVertex[numVertices];
	for (unsigned int i = 0; i < numVertices; i++)
	{
		meshBuffer[i].x = pMesh->m_vertices[i]->vertexPosition[0];
		meshBuffer[i].y = pMesh->m_vertices[i]->vertexPosition[1];
		meshBuffer[i].z = pMesh->m_vertices[i]->vertexPosition[2];

		meshBuffer[i].nx = pMesh->m_vertices[i]->vertexNormals[0];
		meshBuffer[i].ny = pMesh->m_vertices[i]->vertexNormals[1];
		meshBuffer[i].nz = pMesh->m_vertices[i]->vertexNormals[2];

		meshBuffer[i].r = pMesh->m_vertices[i]->vertexColour[0];
		meshBuffer[i].g = pMesh->m_vertices[i]->vertexColour[1];
		meshBuffer[i].b = pMesh->m_vertices[i]->vertexColour[2];
		meshBuffer[i].a = pMesh->m_vertices[i]->vertexColour[3];
	}

	// Texture coordinates
	OGLUVCoordinate* textureCoordinatesBuffer;
	textureCoordinatesBuffer = new OGLUVCoordinate[numTextureCoordinates];
	for (unsigned int i = 0; i < numTextureCoordinates; i++)
	{
		textureCoordinatesBuffer[i].u = pMesh->m_textureCoordinates[i]->s;
		textureCoordinatesBuffer[i].v = pMesh->m_textureCoordinates[i]->t;
	}

	// Indices
	unsigned int* indicesBuffer;
	indicesBuffer = new unsigned int[numIndices];
	int lIndexCounter = 0;
	for (unsigned int i = 0; i < numTriangles; i++)
	{
		indicesBuffer[lIndexCounter] = pMesh->m_triangles[i]->vertexIndices[0];
		indicesBuffer[lIndexCounter + 1] = pMesh->m_triangles[i]->vertexIndices[1];
		indicesBuffer[lIndexCounter + 2] = pMesh->m_triangles[i]->vertexIndices[2];

		lIndexCounter += 3;
	}

	if (pMesh->m_meshType == OGLMeshType_Colour)
	{
		if (pMesh->m_staticMeshId == -1)
		{
			CreateStaticBuffer(VT_POSITION_NORMAL_COLOUR, pMesh->m_materialId, -1, numVertices, 0, numIndices, meshBuffer, nullptr, indicesBuffer, &pMesh->m_staticMeshId);
		}
		else
		{
			RecreateStaticBuffer(pMesh->m_staticMeshId, VT_POSITION_NORMAL_COLOUR, pMesh->m_materialId, -1, numVertices, 0, numIndices, meshBuffer, nullptr, indicesBuffer);
		}
	}
	else if (pMesh->m_meshType == OGLMeshType_Textured)
	{
		if (pMesh->m_staticMeshId == -1)
		{
			CreateStaticBuffer(VT_POSITION_NORMAL_UV_COLOUR, pMesh->m_materialId, pMesh->m_textureId, numVertices, numTextureCoordinates, numIndices, meshBuffer, textureCoordinatesBuffer, indicesBuffer, &pMesh->m_staticMeshId);
		}
		else
		{
			RecreateStaticBuffer(pMesh->m_staticMeshId, VT_POSITION_NORMAL_UV_COLOUR, pMesh->m_materialId, pMesh->m_textureId, numVertices, numTextureCoordinates, numIndices, meshBuffer, textureCoordinatesBuffer, indicesBuffer);
		}
	}

	// Delete temp data
	delete[] meshBuffer;
	delete[] textureCoordinatesBuffer;
	delete[] indicesBuffer;
}

void Renderer::RenderMesh(OpenGLTriangleMesh* pMesh)
{
	PushMatrix();
		//SetCullMode(CM_NOCULL);
		SetPrimativeMode(PM_TRIANGLES);
		//SetRenderMode(RM_SOLID);
		if (pMesh->m_staticMeshId != -1)
		{
			RenderStaticBuffer(pMesh->m_staticMeshId);
		}
	PopMatrix();
}

void Renderer::RenderMesh_NoColour(OpenGLTriangleMesh* pMesh)
{
	PushMatrix();
		//SetCullMode(CM_NOCULL);
		SetPrimativeMode(PM_TRIANGLES);
		//SetRenderMode(RM_SOLID);
		if (pMesh->m_staticMeshId != -1)
		{
			RenderStaticBuffer_NoColour(pMesh->m_staticMeshId);
		}
	PopMatrix();
}

void Renderer::GetMeshInformation(int *numVerts, int *numTris, OpenGLTriangleMesh* pMesh)
{
	*numVerts = (int)pMesh->m_vertices.size();
	*numTris = (int)pMesh->m_triangles.size();
}

void Renderer::StartMeshRender()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

void Renderer::EndMeshRender()
{
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

bool Renderer::MeshStaticBufferRender(OpenGLTriangleMesh* pMesh)
{
	SetPrimativeMode(PM_TRIANGLES);

	return RenderStaticBuffer(pMesh->m_staticMeshId);
}

// Name rendering and name picking
void Renderer::InitNameStack()
{
	glInitNames();
}

void Renderer::LoadNameOntoStack(int lName)
{
	glPushName(lName);
	//glLoadName(lName);
}

void Renderer::EndNameStack()
{
	glPopName();
}

void Renderer::StartNamePicking(unsigned int lViewportid, int lX, int lY)
{
	ClearScene(true, true, true);

	glSelectBuffer(NAME_PICKING_BUFFER, m_SelectBuffer);

	glRenderMode(GL_SELECT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	int	viewportCoords[4] = { 0, 0, 0, 0 };
	glGetIntegerv(GL_VIEWPORT, viewportCoords);
	gluPickMatrix(lX, lY, 3, 3, viewportCoords);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	MultViewProjection();
}

int Renderer::GetPickedObject()
{
	int objectsFound = 0;
	objectsFound = glRenderMode(GL_RENDER);

	if (objectsFound > 0)
	{
		unsigned int lowestDepth = m_SelectBuffer[1];

		int selectedObject = m_SelectBuffer[3];

		// Any time that we find more than one named object, we choose the one with the lowest depth. i.e Closest to the screen
		for (int i = 1; i < objectsFound; i++)
		{
			unsigned int lDepth = m_SelectBuffer[(i * 4) + 1];
			int lname = m_SelectBuffer[(i * 4) + 3];

			// Also make sure that a name of -1 doesnt ever take priority
			if (lDepth < lowestDepth && lname != -1)
			{
				lowestDepth = m_SelectBuffer[(i * 4) + 1];

				selectedObject = m_SelectBuffer[(i * 4) + 3];
			}
		}

		return selectedObject;
	}

	//No objects found
	return -1;
}

// Frustum
Frustum* Renderer::GetFrustum(unsigned int frustumid)
{
	Frustum* pFrustum = m_frustums[frustumid];

	return pFrustum;
}

int Renderer::PointInFrustum(unsigned int frustumid, const vec3 &point)
{
	Frustum* pFrustum = m_frustums[frustumid];

	return pFrustum->PointInFrustum(point);
}

int Renderer::SphereInFrustum(unsigned int frustumid, const vec3 &point, float radius)
{
	Frustum* pFrustum = m_frustums[frustumid];

	return pFrustum->SphereInFrustum(point, radius);
}

int Renderer::CubeInFrustum(unsigned int frustumid, const vec3 &center, float x, float y, float z)
{
	Frustum* pFrustum = m_frustums[frustumid];

	return pFrustum->CubeInFrustum(center, x, y, z);
}

// Frame buffers
bool Renderer::CreateFrameBuffer(int idToResetup, bool diffuse, bool position, bool normal, bool depth, int width, int height, float viewportScale, string name, unsigned int *pId)
{
	FrameBuffer* pNewFrameBuffer = nullptr;
	if (idToResetup == -1)
	{
		pNewFrameBuffer = new FrameBuffer();
	}
	else
	{
		pNewFrameBuffer = m_vFrameBuffers[idToResetup];

		glDeleteFramebuffersEXT(1, &pNewFrameBuffer->m_fbo);

		glDeleteTextures(1, &pNewFrameBuffer->m_diffuseTexture);
		glDeleteTextures(1, &pNewFrameBuffer->m_positionTexture);
		glDeleteTextures(1, &pNewFrameBuffer->m_normalTexture);
		glDeleteTextures(1, &pNewFrameBuffer->m_depthTexture);
	}

	pNewFrameBuffer->m_name = name;
	pNewFrameBuffer->m_diffuseTexture = -1;
	pNewFrameBuffer->m_positionTexture = -1;
	pNewFrameBuffer->m_normalTexture = -1;
	pNewFrameBuffer->m_depthTexture = -1;

	pNewFrameBuffer->m_width = width;
	pNewFrameBuffer->m_height = height;
	pNewFrameBuffer->m_viewportScale = viewportScale;

	glGenFramebuffersEXT(1, &pNewFrameBuffer->m_fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, pNewFrameBuffer->m_fbo);

	if (diffuse)
	{
		glGenTextures(1, &pNewFrameBuffer->m_diffuseTexture);
		glBindTexture(GL_TEXTURE_2D, pNewFrameBuffer->m_diffuseTexture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, (int)(width*viewportScale), (int)(height*viewportScale), 0, GL_RGBA, GL_FLOAT, nullptr);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, pNewFrameBuffer->m_diffuseTexture, 0);
	}

	if (position)
	{
		glGenTextures(1, &pNewFrameBuffer->m_positionTexture);
		glBindTexture(GL_TEXTURE_2D, pNewFrameBuffer->m_positionTexture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, (int)(width*viewportScale), (int)(height*viewportScale), 0, GL_RGBA, GL_FLOAT, nullptr);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, pNewFrameBuffer->m_positionTexture, 0);
	}

	if (normal)
	{
		glGenTextures(1, &pNewFrameBuffer->m_normalTexture);
		glBindTexture(GL_TEXTURE_2D, pNewFrameBuffer->m_normalTexture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, (int)(width*viewportScale), (int)(height*viewportScale), 0, GL_RGBA, GL_FLOAT, nullptr);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, pNewFrameBuffer->m_normalTexture, 0);
	}

	if (depth)
	{
		glGenTextures(1, &pNewFrameBuffer->m_depthTexture);
		glBindTexture(GL_TEXTURE_2D, pNewFrameBuffer->m_depthTexture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glTexParameterf(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, (int)(width*viewportScale), (int)(height*viewportScale), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

		// Instruct openGL that we won't bind a color texture with the currently binded FBO
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, pNewFrameBuffer->m_depthTexture, 0);
	}

	// Check if all worked fine and unbind the FBO
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		//throw new std::exception("Can't initialize an FBO render texture. FBO initialization failed.");
		return false;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	if (idToResetup == -1)
	{
		// Push the frame buffer onto the list
		m_vFrameBuffers.push_back(pNewFrameBuffer);

		// Return the frame buffer id
		*pId = (int)m_vFrameBuffers.size() - 1;
	}
	else
	{
		*pId = idToResetup;
	}

	return true;
}

int Renderer::GetNumFrameBuffers()
{
	return (int)m_vFrameBuffers.size();
}

FrameBuffer* Renderer::GetFrameBuffer(string name)
{
	int foundIndex = -1;
	for (int i = 0; i < (int)m_vFrameBuffers.size(); i++)
	{
		if (m_vFrameBuffers[i]->m_name == name)
		{
			foundIndex = i;
		}
	}

	if (foundIndex == -1)
		return nullptr;

	return GetFrameBuffer(foundIndex);
}

FrameBuffer* Renderer::GetFrameBuffer(int index)
{
	return m_vFrameBuffers[index];
}

int Renderer::GetFrameBufferIndex(string name)
{
	int foundIndex = -1;
	for (int i = 0; i < (int)m_vFrameBuffers.size(); i++)
	{
		if (m_vFrameBuffers[i]->m_name == name)
		{
			foundIndex = i;
		}
	}

	return foundIndex;
}

void Renderer::StartRenderingToFrameBuffer(unsigned int frameBufferId)
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_vFrameBuffers[frameBufferId]->m_fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, (int)(m_vFrameBuffers[frameBufferId]->m_width*m_vFrameBuffers[frameBufferId]->m_viewportScale), (int)(m_vFrameBuffers[frameBufferId]->m_height*m_vFrameBuffers[frameBufferId]->m_viewportScale));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Clear the render targets
	GLbitfield clear(0);

	if (m_vFrameBuffers[frameBufferId]->m_diffuseTexture != -1)
		clear |= GL_COLOR_BUFFER_BIT;
	if (m_vFrameBuffers[frameBufferId]->m_depthTexture != -1)
		clear |= GL_DEPTH_BUFFER_BIT;
	glClear(clear);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);

	// Specify what to render an start acquiring
	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT };
	glDrawBuffers(3, buffers);
}

void Renderer::StopRenderingToFrameBuffer(unsigned int frameBufferId)
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glPopAttrib();
}

unsigned int Renderer::GetDiffuseTextureFromFrameBuffer(unsigned int frameBufferId)
{
	return m_vFrameBuffers[frameBufferId]->m_diffuseTexture;
}

unsigned int Renderer::GetPositionTextureFromFrameBuffer(unsigned int frameBufferId)
{
	return m_vFrameBuffers[frameBufferId]->m_positionTexture;
}

unsigned int Renderer::GetNormalTextureFromFrameBuffer(unsigned int frameBufferId)
{
	return m_vFrameBuffers[frameBufferId]->m_normalTexture;
}

unsigned int Renderer::GetDepthTextureFromFrameBuffer(unsigned int frameBufferId)
{
	return m_vFrameBuffers[frameBufferId]->m_depthTexture;
}

// Rendered information
void Renderer::ResetRenderedStats()
{
	m_numRenderedVertices = 0;
	m_numRenderedFaces = 0;
}

int Renderer::GetNumRenderedVertices()
{
	return m_numRenderedVertices;
}

int Renderer::GetNumRenderedFaces()
{
	return m_numRenderedFaces;
}

// Shaders
bool Renderer::LoadGLSLShader(const char* vertexFile, const char* fragmentFile, unsigned int *pID)
{
	glShader* lpShader = nullptr;

	// Load the shader
	lpShader = ShaderManager.loadfromFile(vertexFile, fragmentFile);  // load (and compile, link) from file

	if (lpShader != nullptr)
	{
		// Push the vertex array onto the list
		m_shaders.push_back(lpShader);

		// Return the vertex array id
		*pID = (int)m_shaders.size() - 1;

		return true;
	}

	cout << "ERROR: Could not load GLSL shaders: " << vertexFile << ", " << fragmentFile << endl << flush;

	return false;
}

void Renderer::BeginGLSLShader(unsigned int shaderID)
{
	m_shaders[shaderID]->begin();
}

void Renderer::EndGLSLShader(unsigned int shaderID)
{
	m_shaders[shaderID]->end();
}

glShader* Renderer::GetShader(unsigned int shaderID)
{
	return m_shaders[shaderID];
}