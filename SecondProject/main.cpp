// Triangle.cpp
// Our first OpenGL program that will just draw a triangle on the screen.

#include <GLTools.h>            // OpenGL toolkit
#include <GL/glew.h> 
#include <GLFrustum.h>
#include <GLFrame.h>
#include <StopWatch.h>

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
CStopWatch timer; 

///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.

void ChangeSize(int w, int h) {
    glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(60.0f, w/h, 5.0f, 50.0f);
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

///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context.
// This is the first opportunity to do any OpenGL related tasks.

void SetupRC() {
    glClearColor(0.6f, 0.6f, 0.6f, 1.0f);

    shader = gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp",
            2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");
    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \n",
            GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);

	MVPMatrixLocation=glGetUniformLocation(shader,"MVPMatrix");
	if(MVPMatrixLocation==-1)
	{
		fprintf(stderr,"uniform MVPMatrix could not be found\n");
	}
}

///////////////////////////////////////////////////////////////////////////////
// Called to draw scene

void RenderScene(void) {
    // Clear the window with current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glUseProgram(shader);

	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);

    m3dMatrixMultiply44(matrix, viewFrustum.GetProjectionMatrix(), mCamera);
    glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,matrix);

    glBegin(GL_QUADS);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.5, 0.5, 0.5);
		glVertex3f(-1.0f,-1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f, 1.0f, 0.0f);
    glEnd();

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 1.0, 0.0);
		glVertex3f( 0.0f, 0.0f, 2.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
    glEnd();

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.0, 0.0);
		glVertex3f( 0.0f, 0.0f, 2.0f);
		glVertex3f( 1.0f,-1.0f, 0.0f);
		glVertex3f( 1.0f, 1.0f, 0.0f);	
    glEnd();

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
		glVertex3f( 0.0f, 0.0f, 2.0f);
		glVertex3f( 1.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f, 1.0f, 0.0f);	
    glEnd();

	glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
		glVertex3f( 0.0f, 0.0f, 2.0f);
		glVertex3f(-1.0f, 1.0f, 0.0f);
		glVertex3f(-1.0f,-1.0f, 0.0f);
    glEnd();

	M3DVector3f at={0,0,0};
	M3DVector3f up={0,0,1};
	M3DVector3f eye;
	float angle = timer.GetElapsedSeconds()*M_PI;

	eye[0]=6.8f*cos(angle);
	eye[1]=6.0f*sin(angle);
	eye[2]=5.0f; 
	LookAt(cameraFrame,eye,at,up);


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