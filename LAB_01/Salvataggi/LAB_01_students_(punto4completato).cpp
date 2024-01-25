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

unsigned int VAOp;    // Punti
unsigned int VBOp;
unsigned int VAOc;    // Curva
unsigned int VBOc;

// Tratto cubico (che verra' poi collegato alla curva di base)
unsigned int VAOp3;    // Punti
unsigned int VBOp3;
unsigned int VAOc3;    // Curva
unsigned int VBOc3;

using namespace glm;

// Curva di base
#define MaxNumPts 120
float PointArray[MaxNumPts][3];
float CurveArray[MaxNumPts][3];
int NumPts = 0;

// Tratto cubico
#define MaxNumPts3 4   // Puo' avere al massimo 4 punti essendo cubico
float PointArray3[MaxNumPts3][3];
float CurveArray3[MaxNumPts][3];   // Qui impostiamo MaxNumPts perche' vengono calcolati molti punti per definire la curva
int NumPts3 = 0;

int continuita = 0;   // Indica la continuita' selezionata: 0->C0 (default), 1->C1, 2->G1.
#define PI 3.1415926535897932384626433832795
//#include <glm/gtx/string_cast.hpp>   // Per la stampa di vettori

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

		case '0':			// Continuita' C0 (default)
			continuita = 0;
			cout << "\nSelezionata continuita' C0.\n\n";
			break;

		case '1':			// Continuita' C1
			continuita = 1;
			cout << "\nSelezionata continuita' C1.\n\n";
			break;

		case 'g':			// Continuita' G1
			continuita = 2;
			cout << "\nSelezionata continuita' G1.\n\n";
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

// Aggiunge un nuovo punto al tratto cubico.
//  Quando il tratto cubico e' completo viene attaccato alla curva di base (in attesa di un nuovo tratto cubico).
void addNewPoint(float x, float y) {

	if (NumPts==0 && NumPts3==0 && continuita==0) cout << "\nSelezionata continuita' C0 (default).\n\n";

	// Lo aggiungo al tratto cubico
	PointArray3[NumPts3][0] = x;
	PointArray3[NumPts3][1] = y;
	PointArray3[NumPts3][2] = 0;
	NumPts3++;

	if (NumPts3 == MaxNumPts3) {   // Se il tratto cubico e' completo

		// Attacco il tratto cubico alla curva di base

		if (NumPts == 0) {   // Se e' il primo tratto cubico (i.e. ancora non c'e' una curva di base) considero anche PointArray3[0]~v0

			PointArray[NumPts][0] = PointArray3[0][0];   // x
			PointArray[NumPts][1] = PointArray3[0][1];   // y
			PointArray[NumPts][2] = 0;

			NumPts++;
		}
		else {

			/* ... --> p2 --> p3=v0 --> v1 --> ...
			 *
			 * p2    ~ PointArray[NumPts-2]
			 * p3=v0 ~ PointArray[NumPts-1]
			 * v1    ~ PointArray3[1]
			 */

			// Modifico PointArray3[1]~v1 in base alla continuita' selezionata (con C0 rimane invariato)
			if (continuita == 1) {   // Continuita' C1

				// v1 viene trasformato in v1' (cosi' da rispettare la continuita' C1)

				/* delta = p3 - p2
				 * v1'  =  p3 + delta  =  p3 + (p3 - p2)  =  2*p3 - p2
				 */
				PointArray3[1][0] = 2*PointArray[NumPts-1][0] - PointArray[NumPts-2][0];   // x
				PointArray3[1][1] = 2*PointArray[NumPts-1][1] - PointArray[NumPts-2][1];   // y
			}
			else if (continuita == 2) {   // Continuita' G1

				// v1 viene trasformato in v1' (cosi' da rispettare la continuita' G1)
				//  * Il delta viene dimezzato: in questo modo ho la stessa direzione ma modulo differente.

				// v1'  = p3 + delta/2  =  p3 + (p3 - p2)/2
				PointArray3[1][0] = PointArray[NumPts-1][0] + (PointArray[NumPts-1][0] - PointArray[NumPts-2][0]) / 2;   // x
				PointArray3[1][1] = PointArray[NumPts-1][1] + (PointArray[NumPts-1][1] - PointArray[NumPts-2][1]) / 2;   // y
			}
		}

		for (int i = 1; i < NumPts3; i++) {   // Non considero i=0 perche' v0 scompare (sostituito da p3)

			PointArray[NumPts][0] = PointArray3[i][0];   // x
			PointArray[NumPts][1] = PointArray3[i][1];   // y
			PointArray[NumPts][2] = 0;

			NumPts++;
		}

		NumPts3 = 0;   // Azzero il tratto cubico (per crearne uno nuovo)
	}
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
	glGenVertexArrays(1, &VAOp);
	glBindVertexArray(VAOp);

	glGenBuffers(1, &VBOp);
	glBindBuffer(GL_ARRAY_BUFFER, VBOp);

	// Creo un secondo VBO e VAO
	glGenVertexArrays(1, &VAOc);
	glBindVertexArray(VAOc);
	glGenBuffers(1, &VBOc);
	glBindBuffer(GL_ARRAY_BUFFER, VBOc);


	// Tratto cubico - Punti
	glGenVertexArrays(1, &VAOp3);
	glBindVertexArray(VAOp3);
	glGenBuffers(1, &VBOp3);
	glBindBuffer(GL_ARRAY_BUFFER, VBOp3);

	// Tratto cubico - Curva
	glGenVertexArrays(1, &VAOc3);
	glBindVertexArray(VAOc3);
	glGenBuffers(1, &VBOc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBOc3);


	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, width, height);
}

// `result`: e' un puntatore.
// `cub`: indica se si sta applicando sul tratto cubico.
void deCasteljau(float t, float* result, bool cub) {

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

// Quando viene applicato sul tratto cubico
void deCasteljau3(float t, float* result, bool cub) {

	int i, j;
	float coordX[MaxNumPts3], coordY[MaxNumPts3];

	// Inizialmente inserisco le coordinate dei CP
	for (i = 0; i < NumPts3; i++) {

		coordX[i] = PointArray3[i][0];	// 0 --> x
		coordY[i] = PointArray3[i][1];	// 1 --> y
	}

	// A partire da queste, calcolo ricorsivamente il Lerp
	for (i = 1; i < NumPts3; i++) {

		for (j = 0; j < NumPts3 - i; j++) {

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
	

	if (NumPts > 0) {   // Se c'e' almeno un punto li disegno

		glBindVertexArray(VAOp);
		glBindBuffer(GL_ARRAY_BUFFER, VBOp);
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


		if (NumPts > 1) {   // Se ci sono almeno 2 punti disegno la curva

			int i;
			float result[3];

			for (i = 0; i <= 100; i++) {

				deCasteljau((GLfloat)i / 100, result, false);

				CurveArray[i][0] = result[0];
				CurveArray[i][1] = result[1];
				CurveArray[i][2] = 0;
			}

			glBindVertexArray(VAOc);	// Collego il nuovo VAO (corrispondente alla curva)
			glBindBuffer(GL_ARRAY_BUFFER, VBOc);
			glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glLineWidth(0.5);
			glDrawArrays(GL_LINE_STRIP, 0, 101);   // Il for ha creato 101 elementi in CurveArray
			glBindVertexArray(0);   // Scollego il VAO
		}
	}


	// ** Tratto cubico **
	
	if (NumPts3 > 0) {   // Se c'e' almeno un punto li disegno

		glBindVertexArray(VAOp3);
		glBindBuffer(GL_ARRAY_BUFFER, VBOp3);
		glBufferData(GL_ARRAY_BUFFER, sizeof(PointArray3), &PointArray3[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		
		glLineWidth(2.5);
		glDrawArrays(GL_LINE_STRIP, 0, NumPts3);
		glPointSize(8.0);
		glDrawArrays(GL_POINTS, 0, NumPts3);
		glBindVertexArray(0);


		if (NumPts3 > 1) {   // Se ci sono almeno 2 punti disegno la curva

			int i;
			float result3[3];

			for (i = 0; i <= 100; i++) {

				deCasteljau3((GLfloat)i / 100, result3, true);

				CurveArray3[i][0] = result3[0];
				CurveArray3[i][1] = result3[1];
				CurveArray3[i][2] = 0;
			}

			glBindVertexArray(VAOc3);	// Collego il nuovo VAO (corrispondente alla curva)
			glBindBuffer(GL_ARRAY_BUFFER, VBOc3);
			glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray3), &CurveArray3[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glLineWidth(0.5);
			glDrawArrays(GL_LINE_STRIP, 0, 101);   // Il for ha creato 101 elementi in CurveArray
			glBindVertexArray(0);   // Scollego il VAO
		}
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
