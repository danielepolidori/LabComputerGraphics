#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>   // Sistema particellare

static unsigned int programId;
#define PI 3.14159265358979323846

unsigned int VAO_CIELO, VAO_SOLE, VAO_SISTEMAPARTICELLARE;
unsigned int VBO_C, VBO_S, VBO_SP, MatProj, MatModel;

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


// Sole

int nTriangles_sole = 30;
int vertices_sole = 3 * 2 * nTriangles_sole;
Point* Sole = new Point[vertices_sole];
int intorno = 30;   // Facilita il riconoscere dove si trova il sole

// Posizione del sole
int posSole_x = 0;   // [0, width]
int posSole_y = 0;   // [0, height]


// Sistema particellare

typedef struct {
	float r;
	float g;
	float b;
} Color;

typedef struct {
	float x;
	float y;
	float alpha;
	float xFactor;
	float yFactor;
	float drag;
	Color color;
} PARTICLE;

vector <PARTICLE> particles;

int nPoint = 5000;
Point* Punti = new Point[nPoint];


// Aloni nel cielo

int num_aloni = 10;
bool alone_inglobato[10] = {};	// Indica se l'alone e' stato inglobato (quindi non e' piu' visibile)

// Posizione degli aloni
//  - An empty initializer can be used to initialize an array
int posAlone_x[10] = {};   // ~ posAlone_x[num_aloni];
int posAlone_y[10] = {};   // ~ posAlone_y[num_aloni];



void keyboardPressedEvent(unsigned char key, int x, int y) {

	switch (key) {

		case 27:		// esc
			exit(0);
			break;

		default:
			break;
	}
}

// Sistema particellare
Color computeRainbow() {

	static float rgb[3] = { 1.0, 0.0, 0.0 };
	static int fase = 0, counter = 0;
	const float step = 0.1;
	Color paint;

	 switch (fase) {
		case 0: rgb[1] += step;
		break;
		case 1: rgb[0] -= step;
		break;
		case 2: rgb[2] += step;
		break;
		case 3: rgb[1] -= step;
		break;
		case 4: rgb[0] += step;
		break;
		case 5: rgb[2] -= step;
		break;
		default:
		break;
	}
	//fprintf(stdout, "Rosso e verde e blu: %f,%f,%f, counter= %i\n", rgb[0], rgb[1], rgb[2], counter);

	 counter++;
	if (counter > 1.0 / step) {
		counter = 0;
		fase < 5 ? fase++ : fase = 0;
	}

	 paint.r = rgb[0];
	paint.g = rgb[1];
	paint.b = rgb[2];
	return paint;
}

void mouseMotionEvent(int x, int y) {

	int xPos_ = x;
	int yPos_ = height - y;   // L'asse y e' invertito (posSole_y=0 quando y=height)


	// Sole
	posSole_x = xPos_;
	posSole_y = yPos_;

	// Sistema particellare
	Color rgb = computeRainbow();
	for (int i = 0; i < 10; i++) {

		PARTICLE p;
		p.x = xPos_;
		p.y = yPos_;
		p.alpha = 1.0;
		p.drag = 1.05;
		p.xFactor = (rand() % 1000 + 1) / 300 * (rand() % 2 == 0 ? -1 : 1);
		p.yFactor = (rand() % 1000 + 1) / 300 * (rand() % 2 == 0 ? -1 : 1);
		p.color.r = rgb.r;
		p.color.g = rgb.g;
		p.color.b = rgb.b;
		particles.push_back(p);
	}

	// Controllo se il sole sta toccando un alone
	for (int i = 0; i < num_aloni; i++) {   // Scorro gli aloni

		if (xPos_ < posAlone_x[i] + intorno &&
		    xPos_ > posAlone_x[i] - intorno &&
		    yPos_ < posAlone_y[i] + intorno &&
		    yPos_ > posAlone_y[i] - intorno) {

			alone_inglobato[i] = true;
		}
	}

	// Controllo se il sole sta toccando il bordo dello schermo
	if (xPos_ < 0 + intorno ||
	    xPos_ > width - intorno ||
	    yPos_ < 0 + intorno ||
	    yPos_ > height - intorno) {

		for (int i = 0; i < num_aloni; i++) {

			alone_inglobato[i] = false;
		}
	}


	glutPostRedisplay();
}

// Cielo
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

// Sole
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
	//vec4 col_top_sole = { 0.5, 0.5, 0.5, 0.5 };   // PER MODIFICARE LA STRUTTURA DEL SOLE
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
	disegna_piano(0, 0, 1, 1, col_bottom, col_top, Cielo);
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

	// Sistema particellare
	glGenVertexArrays(1, &VAO_SISTEMAPARTICELLARE);
	glBindVertexArray(VAO_SISTEMAPARTICELLARE);
	glGenBuffers(1, &VBO_SP);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_SP);
	glBindVertexArray(0);

	// Aloni nel cielo
	for (int i = 0; i < num_aloni; i++) {

		// Imposto la posizione degli aloni (con valori random)
		posAlone_x[i] = rand() % width;
		posAlone_y[i] = rand() % height;
	}


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
	Model = translate(Model, vec3(0.0, 0.0, 0.0));
	Model = scale(Model, vec3(float(width), float(height), 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_CIELO);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, vertices_Cielo);
	glBindVertexArray(0);


	// Sistema particellare

	int P_size = 0; // particles.size();

	for (int i = 0; i < particles.size(); i++) {

		particles.at(i).xFactor /= particles.at(i).drag;
		particles.at(i).yFactor /= particles.at(i).drag;

		particles.at(i).x += particles.at(i).xFactor;
		particles.at(i).y += particles.at(i).yFactor;

		particles.at(i).alpha -= 0.05;

		float xPos = (float)particles.at(i).x / width;
		float yPos = (float)particles.at(i).y / height;

		if (particles.at(i).alpha <= 0.0) {
			particles.erase(particles.begin() + i);
		}
		else {
			Punti[i].x = xPos;
			Punti[i].y = yPos;
			Punti[i].z = 0.0;
			Punti[i].r = particles.at(i).color.r;
			Punti[i].g = particles.at(i).color.g;
			Punti[i].b = particles.at(i).color.b;
			Punti[i].a = particles.at(i).alpha;
			P_size += 1;
		}
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(VAO_SISTEMAPARTICELLARE);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_SP);
	glBufferData(GL_ARRAY_BUFFER, P_size * sizeof(Point), &Punti[0], GL_STATIC_DRAW);
	// Configura l'attributo posizione
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Configura l'attributo colore
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glPointSize(3.0);
	glDrawArrays(GL_POINTS, 0, P_size);
	glBindVertexArray(0);


	// Disegna sole
	Model = mat4(1.0);
	Model = translate(Model, vec3(float(posSole_x), float(posSole_y), 0.0));
	Model = scale(Model, vec3(30.0, 30.0, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_SOLE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, vertices_sole / 2);
	glBindVertexArray(0);

	//Disegna Alone del sole
	Model = mat4(1.0);
	Model = translate(Model, vec3(float(posSole_x), float(posSole_y), 0.0));
	Model = scale(Model, vec3(80.0, 80.0, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_SOLE);
	glDrawArrays(GL_TRIANGLES, vertices_sole / 2, vertices_sole / 2);
	//glBindVertexArray(0);   // Il VAO non va scollegato, perche' lo uso anche per gli aloni nel cielo


	// Aloni luminosi nel cielo
	//  - Uso lo stesso VAO del sole e del suo alone (VAO_SOLE)
	for (int i = 0; i < num_aloni; i++) {   // Creo piu' copie, ognuna nella propria posizione

		if (!alone_inglobato[i]) {   // Se l'alone non e' stato inglobato, allora lo mostro

			Model = mat4(1.0);
			Model = translate(Model, vec3(float(posAlone_x[i]), float(posAlone_y[i]), 0.0));
			Model = scale(Model, vec3(20.0, 20.0, 1.0));
			glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
			glDrawArrays(GL_TRIANGLES, vertices_sole / 2, vertices_sole / 2);
		}
	}
	glBindVertexArray(0);   // Alla fine delle copie, scollego il VAO


	glutSwapBuffers();
}

// Sistema particellare
void update(int value) {

	glutPostRedisplay();
	glutTimerFunc(10, update, 0);	// Con 20 avremo la coda dei punti piu' lunga
}

int main(int argc, char* argv[]) {

	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	//glutInitContextProfile(GLUT_CORE_PROFILE);   // Sistema particellare
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("2D Animation");
	glutDisplayFunc(drawScene);

	// Sistema particellare
	glutTimerFunc(20, update, 0);

	glutKeyboardFunc(keyboardPressedEvent);		// Evento tastiera tasto premuto
	glutPassiveMotionFunc(mouseMotionEvent);	// Cattura il movimento del mouse

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutMainLoop();
}