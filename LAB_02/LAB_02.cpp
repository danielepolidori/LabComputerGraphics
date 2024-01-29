#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

static unsigned int programId;
#define PI 3.14159265358979323846

unsigned int VAO_CIELO, VAO_SOLE;
unsigned int VBO_C, VBO_S, MatProj, MatModel;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

mat4 Projection;  //Matrice di proiezione
mat4 Model; //Matrice per il cambiamento di sistema di riferimento: 
			// da Sistema di riferimento dell'oggetto a sistema di riferimento nel Mondo

// viewport size
int width = 1280;
int height = 720;


float angolo = 0.0;
typedef struct { float x, y, z, r, g, b, a; } Point;

// Cielo
int vertices_Cielo = 6;
Point* Cielo = new Point[vertices_Cielo];

//Sole
int nTriangles_sole = 30;
int vertices_sole = 3 * 2 * nTriangles_sole;
Point* Sole = new Point[vertices_sole];



void keyboardPressedEvent(unsigned char key, int x, int y) {

	switch (key) {

		case 27:		// esc
			exit(0);
			break;

		default:
			break;
	}
}

void disegna_piano(float x, float y, float width, float height, vec4 color_top, vec4 color_bot, Point* piano) {

	piano[0].x = x;	piano[0].y = y; piano[0].z = 0;
	piano[0].r = color_bot.r; piano[0].g = color_bot.g; piano[0].b = color_bot.b; piano[0].a = color_bot.a;
	piano[1].x = x + width;	piano[1].y = y;	piano[1].z = 0;
	piano[1].r = color_top.r; piano[1].g = color_top.g; piano[1].b = color_top.b; piano[1].a = color_top.a;
	piano[2].x = x + width;	piano[2].y = y + height; piano[2].z = 0;
	piano[2].r = color_bot.r; piano[2].g = color_bot.g; piano[2].b = color_bot.b; piano[2].a = color_bot.a;

	piano[3].x = x + width;	piano[3].y = y + height; piano[3].z = 0;
	piano[3].r = color_bot.r; piano[3].g = color_bot.g; piano[3].b = color_bot.b; piano[3].a = color_bot.a;
	piano[4].x = x;	piano[4].y = y + height; piano[4].z = 0;
	piano[4].r = color_top.r; piano[4].g = color_top.g; piano[4].b = color_top.b; piano[4].a = color_top.a;
	piano[5].x = x;	piano[5].y = y; piano[5].z = 0;
	piano[5].r = color_bot.r; piano[5].g = color_bot.g; piano[5].b = color_bot.b; piano[5].a = color_bot.a;	
}

void disegna_cerchio(int nTriangles, int step, vec4 color_top, vec4 color_bot, Point* Cerchio) {

	int i;
	float stepA = (2 * PI) / nTriangles;

	int comp = 0;
	// step = 1 -> triangoli adiacenti, step = n -> triangoli distanti step l'uno dall'altro
	for (i = 0; i < nTriangles; i += step) {

		Cerchio[comp].x = cos((double)i * stepA);
		Cerchio[comp].y = sin((double)i * stepA);
		Cerchio[comp].z = 0.0;
		Cerchio[comp].r = color_top.r;
		Cerchio[comp].g = color_top.g;
		Cerchio[comp].b = color_top.b;
		Cerchio[comp].a = color_top.a;

		Cerchio[comp + 1].x = cos((double)(i + 1) * stepA);
		Cerchio[comp + 1].y = sin((double)(i + 1) * stepA);
		Cerchio[comp + 1].z = 0.0;
		Cerchio[comp + 1].r = color_top.r;
		Cerchio[comp + 1].g = color_top.g;
		Cerchio[comp + 1].b = color_top.b;
		Cerchio[comp + 1].a = color_top.a;

		Cerchio[comp + 2].x = 0.0;
		Cerchio[comp + 2].y = 0.0;
		Cerchio[comp + 2].z = 0.0;
		Cerchio[comp + 2].r = color_bot.r;
		Cerchio[comp + 2].g = color_bot.g;
		Cerchio[comp + 2].b = color_bot.b;
		Cerchio[comp + 2].a = color_bot.a;
		
		comp += 3;
	}
}

void disegna_sole(int nTriangles, Point* Sole) {

	int i, cont;
	Point* OutSide;
	int vertici = 3 * nTriangles;
	OutSide = new Point[vertici];

	vec4 col_top_sole = { 1.0, 1.0, 1.0, 1.0 };
	vec4 col_bottom_sole = { 1.0, 0.8627, 0.0, 1.0 };
	disegna_cerchio(nTriangles, 1, col_top_sole, col_bottom_sole, Sole);
	
	col_top_sole = { 1.0, 1.0, 1.0, 0.0 };
	col_bottom_sole = { 1.0, 0.8627, 0.0, 1.0 };
	disegna_cerchio(nTriangles, 1, col_top_sole, col_bottom_sole, OutSide);

	cont = 3 * nTriangles;
	for (i = 0; i < 3 * nTriangles; i++) {

		Sole[cont + i].x = OutSide[i].x;
		Sole[cont + i].y = OutSide[i].y;
		Sole[cont + i].z = OutSide[i].z;
		Sole[cont + i].r = OutSide[i].r;
		Sole[cont + i].g = OutSide[i].g;
		Sole[cont + i].b = OutSide[i].b;
		Sole[cont + i].a = OutSide[i].a;
	}
}

void initShader(void) {

	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader_C_M.glsl";
	char* fragmentShader = (char*)"fragmentShader_C_M.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);
}

void init(void) {

	Projection = ortho(0.0f, float(width), 0.0f, float(height));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");
	

	//Costruzione geometria e colori del CIELO
	vec4 col_top =	{ 0.3,0.6,1.0,1.0 };
	vec4 col_bottom = { 0.0 , 0.1, 1.0, 1.0 };
	//disegna_piano(0, 0, 1, 1, col_bottom, col_top, Cielo);
	disegna_piano(0, -1, 1, 2, col_bottom, col_top, Cielo);
	//Generazione del VAO del Cielo
	glGenVertexArrays(1, &VAO_CIELO);
	glBindVertexArray(VAO_CIELO);
	glGenBuffers(1, &VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
	glBufferData(GL_ARRAY_BUFFER, vertices_Cielo * sizeof(Point), &Cielo[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//Scollego il VAO
	glBindVertexArray(0);

	//Costruzione geometria e colori del SOLE
	//Genero il VAO del SOLE
    disegna_sole(nTriangles_sole, Sole);
	glGenVertexArrays(1, &VAO_SOLE);
	glBindVertexArray(VAO_SOLE);
	glGenBuffers(1, &VBO_S);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
	glBufferData(GL_ARRAY_BUFFER, vertices_sole * sizeof(Point), &Sole[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//Scollego il VAO
	glBindVertexArray(0);


	//Definisco il colore che verrà assegnato allo schermo
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	glutSwapBuffers();
}

void drawScene(void) {

	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	glClear(GL_COLOR_BUFFER_BIT);


	//Disegna cielo
	//Matrice per il cambiamento del sistema di riferimento del cielo
	Model = mat4(1.0);
	Model = translate(Model, vec3(0.0, float(height) / 2, 0.0));
	Model = scale(Model, vec3(float(width), float(height) / 2, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_CIELO);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, vertices_Cielo);
	glBindVertexArray(0);

	// Disegna sole
	Model = mat4(1.0);
	Model = translate(Model, vec3(float(width) * 0.5, float(height) * 0.8, 0.0));
	Model = scale(Model, vec3(30.0, 30.0, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_SOLE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, vertices_sole / 2);
	glBindVertexArray(0);
	//Disegna Alone del sole
	Model = mat4(1.0);
	Model = translate(Model, vec3(float(width) * 0.5, float(height) * 0.8, 0.0));
	Model = scale(Model, vec3(80.0, 80.0, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_SOLE);
	glDrawArrays(GL_TRIANGLES, vertices_sole / 2, vertices_sole / 2);
	glBindVertexArray(0); // IL VAO NON VA SCOLLEGATO ???


	glutSwapBuffers();
}

int main(int argc, char* argv[]) {

	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("2D Animation");
	glutDisplayFunc(drawScene);
	glutKeyboardFunc(keyboardPressedEvent);   // Evento tastiera tasto premuto

	//gestione animazione
	//glutTimerFunc(66, update, 0);
	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutMainLoop();
}