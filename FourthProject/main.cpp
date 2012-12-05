
#include <GLMatrixStack.h>
#include <GLTools.h>            // OpenGL toolkit
#include <GL/glew.h> 
#include <GLFrustum.h>
#include <GLFrame.h>
#include <StopWatch.h>
#include <GLGeometryTransform.h>


#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif

GLfloat M_PI = 3.14f;
GLuint shader;
GLuint MVPMatrixLocation;
GLFrustum viewFrustum;
GLFrame cameraFrame;
M3DMatrix44f matrix;
M3DMatrix44f mCamera;
CStopWatch timer; 
GLMatrixStack matrixStack;
GLMatrixStack modelView;
GLMatrixStack projection;
GLGeometryTransform geometryPipeline;

GLuint MVMatrixLocation;
GLuint normalMatrixLocation;
GLuint ambientLightLocation;
GLuint shaderPositionLocation;
GLuint shaderColorLocation;
GLfloat shaderAngleLocation;
GLfloat shaderAttenuation0Location;
GLfloat shaderAttenuation1Location;
GLfloat shaderAttenuation2Location;
GLuint shaderAmbientColorLocation;
GLuint shaderDiffuseColorLocation;
GLuint shaderSpecularColorLocation;
GLuint shaderSpecularExponentLocation;

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.

void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(60.0f, w/h, 3.0f, 50.0f);
}

void SetUpFrame(GLFrame &frame,const M3DVector3f origin,
				const M3DVector3f forward,
				const M3DVector3f up) {
	frame.SetOrigin(origin);
	frame.SetForwardVector(forward);
	M3DVector3f side,oUp;
	m3dCrossProduct3(side,forward,up);
	m3dCrossProduct3(oUp,side,forward);
	frame.SetUpVector(oUp);
	frame.Normalize();
}

void LookAt(GLFrame &frame, const M3DVector3f eye,
        const M3DVector3f at,
        const M3DVector3f up) {
    M3DVector3f forward;
    m3dSubtractVectors3(forward, at, eye);
    SetUpFrame(frame, eye, forward, up);
}

void TriangleFace(M3DVector3f a, M3DVector3f b, M3DVector3f c) {
   M3DVector3f normal, bMa, cMa;
   m3dSubtractVectors3(bMa, b, a);
   m3dSubtractVectors3(cMa, c, a);
   m3dCrossProduct3(normal, bMa, cMa);
   m3dNormalizeVector3(normal);
   glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
   glVertex3fv(a);
   glVertex3fv(b);
   glVertex3fv(c);
}

///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context.
// This is the first opportunity to do any OpenGL related tasks.

void SetupRC() {
    glClearColor(0.6f, 0.6f, 0.6f, 1.0f);

    shader = gltLoadShaderPairWithAttributes("gouraud_phong.vp", "gouraud.fp",
            2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_NORMAL : %d \n",
            GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_NORMAL);

	MVPMatrixLocation = glGetUniformLocation(shader, "MVPMatrix");
	MVMatrixLocation = glGetUniformLocation(shader,"MVMatrix");

	normalMatrixLocation = glGetUniformLocation(shader, "normalMatrix");
	ambientLightLocation = glGetUniformLocation(shader, "ambientLight");

	shaderPositionLocation = glGetUniformLocation(shader, "light1.position");
	shaderColorLocation = glGetUniformLocation(shader, "light1.color");
	shaderAngleLocation = glGetUniformLocation(shader, "light1.angle");
	shaderAttenuation0Location = glGetUniformLocation(shader,"light1.attenuation0");
	shaderAttenuation1Location = glGetUniformLocation(shader, "light1.attenuation1");
	shaderAttenuation2Location = glGetUniformLocation(shader, "light1.attenuation2");

	shaderAmbientColorLocation = glGetUniformLocation(shader, "material.ambientColor");
	shaderDiffuseColorLocation = glGetUniformLocation(shader, "material.diffuseColor");

	if(MVPMatrixLocation==-1)
	{
		fprintf(stderr,"uniform MVPMatrix could not be found\n");
	}
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

void drawFloor()
{
	glBegin(GL_QUADS);
	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.5, 0.0);
		glVertex3f( -10.0f, -10.0f, 0.0f);
		glVertex3f( 10.0f, -10.0f, 0.0f);
		glVertex3f( 10.0f, 10.0f, 0.0f);
		glVertex3f( -10.0f, 10.0f, 0.0f);
	glEnd();

}

void drawPyramid() 
{
	M3DVector3f v1 = {0.0f, 0.0f, 2.0f};
	M3DVector3f v2 = {-1.0f,-1.0f, 0.0f};
	M3DVector3f v3 = {1.0f,-1.0f, 0.0f};
	M3DVector3f v4 = {1.0f, 1.0f, 0.0f};
	M3DVector3f v5 = {-1.0f, 1.0f, 0.0f};

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.5, 0.5, 0.5);
		TriangleFace(v2,v3,v4);
    glEnd();

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.5, 0.5, 0.5);
		TriangleFace(v4,v5,v2);
	glEnd();

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 1.0, 0.0);
		TriangleFace(v1,v2,v3);
    glEnd();

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.0, 0.0);
		TriangleFace(v1,v3,v4);	
    glEnd();

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
		TriangleFace(v1,v4,v5);
    glEnd();

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
		TriangleFace(v1,v5,v2);
    glEnd();
}

float ico_vertices[3 * 12] = {
      0., 0., -0.9510565162951536,
      0., 0., 0.9510565162951536,
      -0.85065080835204, 0., -0.42532540417601994,
      0.85065080835204, 0., 0.42532540417601994,
      0.6881909602355868, -0.5, -0.42532540417601994,
      0.6881909602355868, 0.5, -0.42532540417601994,
      -0.6881909602355868, -0.5, 0.42532540417601994,
      -0.6881909602355868, 0.5, 0.42532540417601994,
      -0.2628655560595668, -0.8090169943749475, -0.42532540417601994,
      -0.2628655560595668, 0.8090169943749475, -0.42532540417601994,
      0.2628655560595668, -0.8090169943749475, 0.42532540417601994,
      0.2628655560595668, 0.8090169943749475, 0.42532540417601994
      };
int ico_faces[3*20]={
      1 ,			 11 ,			 7 ,
      1 ,			 7 ,			 6 ,
      1 ,			 6 ,			 10 ,
      1 ,			 10 ,			 3 ,
      1 ,			 3 ,			 11 ,
      4 ,			 8 ,			 0 ,
      5 ,			 4 ,			 0 ,
      9 ,			 5 ,			 0 ,
      2 ,			 9 ,			 0 ,
      8 ,			 2 ,			 0 ,
      11 ,			 9 ,			 7 ,
      7 ,			 2 ,			 6 ,
      6 ,			 8 ,			 10 ,
      10 ,			 4 ,			 3 ,
      3 ,			 5 ,			 11 ,
      4 ,			 10 ,			 8 ,
      5 ,			 3 ,			 4 ,
      9 ,			 11 ,			 5 ,
      2 ,			 7 ,			 9 ,
      8 ,			 6 ,			 2 };
 
void drawTriangles(int n_faces, float *vertices, int *faces) {
	for (int i = 0; i < n_faces; i++) {
		glBegin(GL_TRIANGLES);
			TriangleFace(vertices + 3 * faces[3 * i], vertices + 3 * faces[3 * i + 1], vertices + 3 * faces[3 * i + 2]);
		glEnd();
	}
}
 
void drawSmoothTriangles(int n_faces, float *vertices, int *faces) {
	M3DVector3f normal;
    for (int i = 0; i < n_faces; i++) {
    glBegin(GL_TRIANGLES);
		for(int j=0;j<3;++j) {
			m3dCopyVector3(normal,vertices+3*faces[i*3+j]);
			m3dNormalizeVector3(normal);
			glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
			glVertex3fv(vertices+3*faces[i*3+j]);
		}
    glEnd();
    }
}
///////////////////////////////////////////////////////////////////////////////
// Called to draw scene

void uniformLightLoad(M3DVector3f color){
	
	M3DVector3f ambientLight = {1.0f, 1.0f, 1.0f};
	M3DVector3f position = {8.0f, 8.0f, 4.0f};

	M3DVector3f ambientColor = {0.8f, 0.8f, 0.8f};
	M3DVector3f diffuseColor = {0.0f, 0.8f, 0.0f};
	float lightAngle = 90.0f;
	float attenuation0 = 0.5f;
	float attenuation1 = 0.5f;
	float attenuation2 = 0.5f;

	glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
	glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
	glUniform3fv(ambientLightLocation, 1, ambientLight);
	glUniform3fv(shaderPositionLocation, 1, position);
	glUniform3fv(shaderColorLocation, 1, color);
	glUniform1f(shaderAngleLocation, lightAngle);
	glUniform1f(shaderAttenuation0Location, attenuation0);
	glUniform1f(shaderAttenuation1Location, attenuation1);
	glUniform1f(shaderAttenuation2Location, attenuation2);
	glUniform3fv(shaderAmbientColorLocation, 1, ambientColor);
	glUniform3fv(shaderDiffuseColorLocation, 1, diffuseColor);
}

void RenderScene(void) {
    // Clear the window with current clearing color
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

	geometryPipeline.SetMatrixStacks(modelView,projection);

	projection.LoadMatrix(viewFrustum.GetProjectionMatrix());
	modelView.PushMatrix();

	cameraFrame.GetCameraMatrix(mCamera);
	modelView.LoadMatrix(mCamera);
	modelView.PushMatrix();

	M3DVector3f color = {0.0f, 0.2f, 0.2f};
	uniformLightLoad(color);

		glPolygonOffset(1.0f, 1.0f);
		drawLines();
		glEnable(GL_POLYGON_OFFSET_FILL);
		drawLines();
		glDisable(GL_POLYGON_OFFSET_FILL);
		drawPyramid();
		
	modelView.PopMatrix();
	modelView.PushMatrix();
	modelView.Translate(1.0f,2.0f,2.0f);
	modelView.Rotate(180.0f,0.0f,0.0f,1.0f);
	M3DVector3f color2 = {0.6f, 0.0f, 0.0f};
	uniformLightLoad(color2);
    
		drawPyramid();

	modelView.PopMatrix();
	modelView.PushMatrix();
	modelView.Translate(6.0f,-4.0f,1.0f);
	modelView.Rotate(60.0f,1.0f,1.0f,1.5f);
	M3DVector3f color3 = {0.5f, 0.5f, 0.0f};
	uniformLightLoad(color3);
    
		drawTriangles(20, ico_vertices, ico_faces);

	modelView.PopMatrix();
	modelView.PushMatrix();
	modelView.Translate(-2.0f,-4.0f,1.0f);
	modelView.Rotate(80.0f,1.0f,0.0f,1.0f);
	modelView.Scale(1.8f,0.4f,1.0f);
	M3DVector3f color4 = {0.0f, 0.0f, 0.3f};
	uniformLightLoad(color4);

		drawSmoothTriangles(20, ico_vertices, ico_faces);

	modelView.PopMatrix();
	modelView.PopMatrix();

    // Perform the buffer swap to display back buffer
    glutSwapBuffers();
	glutPostRedisplay();
}


///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs

int main(int argc, char* argv[]) {
  

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Triangle");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
	glEnable(GL_DEPTH_TEST);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();

    glutMainLoop();
    return 0;
}