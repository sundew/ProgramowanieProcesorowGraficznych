#include <stdio.h>
#include <GLTools.h>
#include <GL/glut.h>
#include <GLFrustum.h>
#include <GLFrame.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <math3d.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <StopWatch.h>

GLfloat M_PI = 3.14f;
GLuint shader;
GLMatrixStack modelView;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLFrustum viewFrustum;
GLFrame cameraFrame;
M3DMatrix44f mCamera;
GLint MVPMatrixLocation;
GLint MVMatrixLocation;
GLint normalMatrixLocation;
GLint textureLocation;
GLint lightPositionLocation;
GLint diffuseColorLocation;
GLint ambientColorLocation;
GLint specularColorLocation;
GLint alphaLocation;
CStopWatch timer;
GLuint textureID[3];


void SetUpFrame(GLFrame &frame,const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up) {
	frame.SetOrigin(origin);
	frame.SetForwardVector(forward);
	M3DVector3f side,oUp;
	m3dCrossProduct3(side,forward,up);
	m3dCrossProduct3(oUp,side,forward);
	frame.SetUpVector(oUp);
	frame.Normalize();
}

void LookAt(GLFrame &frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up) {
	M3DVector3f forward;
	m3dSubtractVectors3(forward, at, eye);
	SetUpFrame(frame, eye, forward, up);
}

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode) {
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;

	// Read the texture bits
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if (pBits == NULL)
		return false;

	fprintf(stderr, "read texture from %s %dx%d\n", szFileName, nWidth, nHeight);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);

	free(pBits);

	if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
		minFilter == GL_LINEAR_MIPMAP_NEAREST ||
		minFilter == GL_NEAREST_MIPMAP_LINEAR ||
		minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);

	return true;
}

void Texture2f(float s, float t) {
	glVertexAttrib2f(GLT_ATTRIBUTE_TEXTURE0, s, t);
}

void Color3f(float r, float g, float b) {
	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, r, g, b);
}

GLint GetUniformLocation(GLuint shader_a, const char *name) {
	GLint location = glGetUniformLocation(shader_a, name);
	if (location == -1) {
		fprintf(stderr, "uniform %s could not be found\n", name);
	}
	return location;
};


void ChangeSize(int w, int h) {
	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(50.0f, float(w) / float(h), 1.0, 200.0);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeline.SetMatrixStacks(modelView, projectionMatrix);
}

void SetupRC() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	shader = gltLoadShaderPairWithAttributes("shader.vp", "shader.fp",
		4,
		GLT_ATTRIBUTE_VERTEX, "vVertex",
		GLT_ATTRIBUTE_COLOR, "vColor",
		GLT_ATTRIBUTE_TEXTURE0, "texCoord0",
		GLT_ATTRIBUTE_NORMAL, "vNormal");

	fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \nGLT_ATTRIBUTE_TEXTURE0 : %d\n",
		GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR, GLT_ATTRIBUTE_TEXTURE0);

	glGenTextures(3, textureID);
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	if (!LoadTGATexture("blue.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE)) {
		fprintf(stderr, "error loading texture\n");
	}

	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	if (!LoadTGATexture("gray.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE)) {
		fprintf(stderr, "error loading texture\n");
	}

	glBindTexture(GL_TEXTURE_2D, textureID[2]);
	if (!LoadTGATexture("green.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE)) {
		fprintf(stderr, "error loading texture\n");
	}

	MVPMatrixLocation = glGetUniformLocation(shader, "MVPMatrix");
	textureLocation = glGetUniformLocation(shader, "texture2D");
	lightPositionLocation = glGetUniformLocation(shader, "lightPosition");
	diffuseColorLocation = glGetUniformLocation(shader, "diffuseColor");
	ambientColorLocation = glGetUniformLocation(shader, "ambientColor");
	specularColorLocation = glGetUniformLocation(shader, "specularColor");
	MVMatrixLocation = glGetUniformLocation(shader, "MVMatrix");
	normalMatrixLocation = glGetUniformLocation(shader, "normalMatrix");
	alphaLocation = glGetUniformLocation(shader, "alpha");

	M3DVector3f eye = {3.0f, 3.0f, 20.0f};
	M3DVector3f at = {0.0f, 0.0f, 0.0f};
	M3DVector3f up = {0.0f, 0.0f, 1.0f};
	LookAt(cameraFrame, eye, at, up);

	cameraFrame.GetCameraMatrix(mCamera);
}

void UpdateMatrixUniforms() {
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, transformPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, transformPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, transformPipeline.GetNormalMatrix());
}

void drawLines()
{
	glBegin(GL_LINES);
	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 0.0, 0.0);
	for(GLfloat i = -10; i <= 10; i+=1)
	{
		glVertex3f( i, -10.0f, 0.0f);
		glVertex3f( i, 10.0f, 0.0f);
		glVertex3f( -10, i, 0.0f);
		glVertex3f( 10, i, 0.0f);
	}
	glEnd();

}

void DrawCube() {
	modelView.PushMatrix();
	UpdateMatrixUniforms();
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	glBegin(GL_QUADS);
		Texture2f (0.0,1.0);
		glVertex3f(  0.5, -0.5, -0.5 );
		Texture2f (1.0,1.0);
		glVertex3f(  0.5,  0.5, -0.5 );
		Texture2f (1.0,0.0);
		glVertex3f( -0.5,  0.5, -0.5 );
		Texture2f (0.0,0.0);
		glVertex3f( -0.5, -0.5, -0.5 );
	glEnd();

	glBegin(GL_QUADS);
		Texture2f (0.0,1.0);
		glVertex3f(  0.5, -0.5, 0.5 );
		Texture2f (1.0,1.0);
		glVertex3f(  0.5,  0.5, 0.5 );
		Texture2f (1.0,0.0);
		glVertex3f( -0.5,  0.5, 0.5 );
		Texture2f (0.0,0.0);
		glVertex3f( -0.5, -0.5, 0.5 );
	glEnd();

	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	glBegin(GL_QUADS);
		Texture2f (0.0,1.0);
		glVertex3f( 0.5, -0.5, -0.5 );
		Texture2f (1.0,1.0);
		glVertex3f( 0.5,  0.5, -0.5 );
		Texture2f (1.0,0.0);
		glVertex3f( 0.5,  0.5,  0.5 );
		Texture2f (0.0,0.0);
		glVertex3f( 0.5, -0.5,  0.5 );
	glEnd();


	glBegin(GL_QUADS);
		Texture2f (0.0,1.0);
		glVertex3f( -0.5, -0.5,  0.5 );
		Texture2f (1.0,1.0);
		glVertex3f( -0.5,  0.5,  0.5 );
		Texture2f (1.0,0.0);
		glVertex3f( -0.5,  0.5, -0.5 );
		Texture2f (0.0,0.0);
		glVertex3f( -0.5, -0.5, -0.5 );
	glEnd();

	glBindTexture(GL_TEXTURE_2D, textureID[2]);
	glBegin(GL_POLYGON);
		Texture2f (0.0,1.0);
		glVertex3f(  0.5,  0.5,  0.5 );
		Texture2f (1.0,1.0);
		glVertex3f(  0.5,  0.5, -0.5 );
		Texture2f (1.0,0.0);
		glVertex3f( -0.5,  0.5, -0.5 );
		Texture2f (0.0,0.0);
		glVertex3f( -0.5,  0.5,  0.5 );
	glEnd();


	glBegin(GL_QUADS);
		Texture2f (0.0,1.0);
		glVertex3f(  0.5, -0.5, -0.5 );
		Texture2f (1.0,1.0);
		glVertex3f(  0.5, -0.5,  0.5 );
		Texture2f (1.0,0.0);
		glVertex3f( -0.5, -0.5,  0.5 );
		Texture2f (0.0,0.0);
		glVertex3f( -0.5, -0.5, -0.5 );
	glEnd();
	modelView.PopMatrix();
}

void RenderScene() {
	modelView.PushMatrix();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glUseProgram(shader);
	glFrontFace(GL_CW);

	M3DVector3f at={0,0,0};
	M3DVector3f up={0,0,1};
	M3DVector3f eye;
	float angle = timer.GetElapsedSeconds()*M_PI/3;

	eye[0]=6.8f*cos(angle);
	eye[1]=6.0f*sin(angle);
	eye[2]=5.0f; 
	LookAt(cameraFrame,eye,at,up);

	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelView.PushMatrix(mCamera);

	glUniform4f(diffuseColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform4f(ambientColorLocation, .3f, .30f, 0.30f, 1.0f);
	glUniform4f(specularColorLocation, .4f, .4f, 0.4f, 1.0f);

	M3DVector4f lightPosition={-10.0f,20.0f, 50.0f,1.0f};
	M3DVector4f eyeLightPosition;
	m3dTransformVector4(eyeLightPosition,lightPosition,mCamera);
	glUniform3fv(lightPositionLocation,1,&eyeLightPosition[0] );

	glUniform1i(textureLocation, 0);

	modelView.PushMatrix();

	cameraFrame.GetCameraMatrix(mCamera);
	modelView.LoadMatrix(mCamera);
	modelView.PushMatrix();
	UpdateMatrixUniforms();

	glPolygonOffset(1.0f, 1.0f);
	drawLines();
	glEnable(GL_POLYGON_OFFSET_FILL);
	drawLines();
	glDisable(GL_POLYGON_OFFSET_FILL);

	modelView.PopMatrix();
	modelView.PushMatrix();
	DrawCube();

	modelView.Translate(-3.0f,0.0f,0.0f);
	DrawCube();

	modelView.Translate(2.0f,2.0f,2.0f);
	modelView.Rotate(60.0f,0.0f,1.0f,1.0f);
	modelView.Scale(1.0f,1.0f,-1.0f);
	DrawCube();
	modelView.PopMatrix();

	
	modelView.Translate(-2.0f,0.0f,1.0f);
	modelView.Rotate(60.0f,0.0f,1.0f,0.0f);
	UpdateMatrixUniforms();
	glUniform1f(alphaLocation,0.5f);
	glUniform4f(specularColorLocation,0.0,0.0,0.0,0.0);
	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);

	Texture2f (0.0,1.0);
	glVertex3f(-2.0f,-2.0f,0.0f);
	Texture2f (1.0,1.0);
	glVertex3f(-2.0f,2.0f,0.0f);
	Texture2f (1.0,0.0);
	glVertex3f(2.0f,2.0f,0.0f);
	Texture2f (0.0,0.0);
	glVertex3f(2.0f,-2.0f,0.0f);
	glEnd();
	glDisable(GL_BLEND);

	modelView.PopMatrix();
	modelView.PopMatrix();

	glutSwapBuffers();
}

int main(int argc, char * argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(2400, 1800);
	glutCreateWindow("Triangle");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutIdleFunc(RenderScene);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
	}

	SetupRC();
	glutMainLoop();
	return 0;
}