/*
 * Lab-01_students.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks.
 *
 * Usage:
 *   Left click to place a control point.
 *		Maximum number of control points allowed is currently set at 64.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
 */


#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>

 // Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

static unsigned int programId;

unsigned int VAO;    // Punti
unsigned int VBO;
unsigned int VAO2;   // Curva
unsigned int VBO2;

using namespace glm;

#define MaxNumPts 120
float PointArray[MaxNumPts][3];
float CurveArray[MaxNumPts][3];
int NumPts = 0;

// Window size in pixels
int width = 500;
int height = 500;

/* Prototypes */
void addNewPoint(float x, float y);
void removeFirstPoint();
void removeLastPoint();


void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'f':
		removeFirstPoint();
		glutPostRedisplay();
		break;
	case 'l':
		removeLastPoint();
		glutPostRedisplay();
		break;
	case 27:			// Escape key
		exit(0);
		break;
	}
}

void removeFirstPoint() {
	int i;
	if (NumPts > 0) {
		// Remove the first point, slide the rest down
		NumPts--;
		for (i = 0; i < NumPts; i++) {
			PointArray[i][0] = PointArray[i + 1][0];
			PointArray[i][1] = PointArray[i + 1][1];
			PointArray[i][2] = 0;
		}
	}
}
void resizeWindow(int w, int h)
{
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

// Left button presses place a new control point.
void myMouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		// (x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
		float xPos = -1.0f + ((float)x)*2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y))*2 / ((float)(height));
		
		printf("new point Xpos %f Ypos %f \n", xPos, yPos);
		printf("new pixel x %d y %d \n", x,y);
		addNewPoint(xPos, yPos);
		glutPostRedisplay();
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void removeLastPoint() {
	if (NumPts > 0) {
		NumPts--;
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void addNewPoint(float x, float y) {
	if (NumPts >= MaxNumPts) {
		removeFirstPoint();
	}
	PointArray[NumPts][0] = x;
	PointArray[NumPts][1] = y;
	PointArray[NumPts][2] = 0;
	NumPts++;
}
void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}

void init(void)
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Creo un secondo VBO e VAO
	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);
	glGenBuffers(1, &VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);

	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, width, height);

}


void deCasteljau(float t, float* result) {	// Il secondo input e' un puntatore

	int i, j;
	float coordX[MaxNumPts], coordY[MaxNumPts];

	// Inizialmente inserisco le coordinate dei CP
	for (i = 0; i < NumPts; i++) {

		coordX[i] = PointArray[i][0];	// 0 --> x
		coordY[i] = PointArray[i][1];	// 1 --> y
	}

	// A partire da queste, calcolo ricorsivamente il Lerp
	for (i = 1; i < NumPts; i++) {

		for (j = 0; j < NumPts - i; j++) {

			coordX[j] = (1 - t) * coordX[j] + (t)*coordX[j + 1];
			coordY[j] = (1 - t) * coordY[j] + (t)*coordY[j + 1];
		}
	}

	result[0] = coordX[0];
	result[1] = coordY[0];
	result[2] = 0.0;
}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointArray), &PointArray[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	// Draw the line segments
	glLineWidth(2.5);
	glDrawArrays(GL_LINE_STRIP, 0, NumPts);

	// Draw the points
	glPointSize(8.0);
	glDrawArrays(GL_POINTS, 0, NumPts);

	// Scollego il VAO
	glBindVertexArray(0);


	if (NumPts > 1) {

		int i;
		float result[3];

		for (i = 0; i <= 100; i++) {

			deCasteljau((GLfloat)i / 100, result);
			CurveArray[i][0] = result[0];
			CurveArray[i][1] = result[1];
			CurveArray[i][2] = 0;
		}

		glBindVertexArray(VAO2);	// Collego il nuovo VAO (corrispondente alla curva)
		glBindBuffer(GL_ARRAY_BUFFER, VBO2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glLineWidth(0.5);
		glDrawArrays(GL_LINE_STRIP, 0, 101);   // Il for ha creato 101 elementi in CurveArray
		glBindVertexArray(0);   // Scollego il VAO
	}

	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Draw curves 2D");
	
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resizeWindow);
	glutKeyboardFunc(myKeyboardFunc);
	glutMouseFunc(myMouseFunc);


	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glutMainLoop();
}
