#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <math.h>			//	Biblioteca pentru calcule matematice;
#include <GL/glew.h>        //  Defineste prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>   
#include <cstdlib>


#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "SOIL.h"

//  Identificatorii obiectelor de tip OpenGL;
GLuint
	VaoId,
	VboId,
	EboId,
	ProgramId,
	myMatrixLocation,
	viewLocation,
	projLocation,
	codColLocation,
	matrUmbraLocation,
	texture;

GLint objectColorLoc, lightColorLoc, lightPosLoc, viewPosLoc;

float const PI = 3.141592f;

// Elemente pentru reprezentarea suprafetei
// (1) intervalele pentru parametrii considerati (u si v)
float const U_MIN = -PI / 2, U_MAX = PI / 2, V_MIN = 0, V_MAX = 2 * PI;
// (2) numarul de paralele/meridiane, de fapt numarul de valori ptr parametri
int const NR_PARR = 131, NR_MERID = 132;
// (3) pasul cu care vom incrementa u, respectiv v
float step_u = (U_MAX - U_MIN) / NR_PARR, step_v = (V_MAX - V_MIN) / NR_MERID;

// alte variabile
int codCol;
float radius = 50;
int index, index_aux;

// variabile pentru matricea de vizualizare
float Refx = 0.0f, Refy = 1000.0f, Refz = 0.0f; 
float alpha = PI / 8, beta = 0.0f, dist = 2500.0f;
float Obsx, Obsy, Obsz;
float Vx = 0.0f, Vy = 0.0f, Vz = 1.0f;

// variabile pentru matricea de proiectie
float width, height, znear = 0.1, fov = 45; 

// pentru fereastra de vizualizare 
GLint winWidth = 1000, winHeight = 600;

// matrice utilizate
glm::mat4 view, projection;
glm::mat4 myMatrix, matrTrans, matrScale;

// sursa de lumina
float xL = 0.0f, yL = 100.0f, zL = 250.0f;

// matricea umbrei
float matrUmbra[4][4];

// variabile planete
int total_triangles = NR_MERID * NR_PARR * 2;
int numIndices = total_triangles * 3;


void processNormalKeys(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'l':
		Vx -= 0.1;
		break;
	case 'r':
		Vx += 0.1;
		break;
	case '+':
		dist += 5;
		break;
	case '-':
		dist -= 5;
		break;
	}
	if (key == 27)
		exit(0);
}


void processSpecialKeys(int key, int xx, int yy)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		beta -= 0.01;
		break;
	case GLUT_KEY_RIGHT:
		beta += 0.01;
		break;
	case GLUT_KEY_UP:
		alpha += 0.01;
		break;
	case GLUT_KEY_DOWN:
		alpha -= 0.01;
		break;
	}
}


void CreateVBO(void)
{
	glm::vec4 Vertices1[(NR_PARR + 1) * NR_MERID];
	GLushort Indices1[2 * (NR_PARR + 1) * NR_MERID + 4 * (NR_PARR + 1) * NR_MERID];
	for (int merid = 0; merid < NR_MERID; merid++)
	{
		for (int parr = 0; parr < NR_PARR + 1; parr++)
		{
			float u = U_MIN + parr * step_u;
			float v = V_MIN + merid * step_v;
			float x_vf = radius * cosf(u) * cosf(v);
			float y_vf = radius * cosf(u) * sinf(v);
			float z_vf = radius * sinf(u);

			index = merid * (NR_PARR + 1) + parr;
			Vertices1[index] = glm::vec4(x_vf, y_vf, z_vf, 1.0);
			Indices1[index] = index;

			index_aux = parr * (NR_MERID)+merid;
			Indices1[(NR_PARR + 1) * NR_MERID + index_aux] = index;

			if ((parr + 1) % (NR_PARR + 1) != 0)
			{
				int AUX = 2 * (NR_PARR + 1) * NR_MERID;
				int index1 = index;
				int index2 = index + (NR_PARR + 1);
				int index3 = index2 + 1;
				int index4 = index + 1;
				if (merid == NR_MERID - 1)
				{
					index2 = index2 % (NR_PARR + 1);
					index3 = index3 % (NR_PARR + 1);
				}
				Indices1[AUX + 4 * index] = index1;
				Indices1[AUX + 4 * index + 1] = index2;
				Indices1[AUX + 4 * index + 2] = index3;
				Indices1[AUX + 4 * index + 3] = index4;
			}
		}
	}
	glGenVertexArrays(1, &VaoId); 
	glGenBuffers(1, &VboId);
	glGenBuffers(1, &EboId);

	glBindVertexArray(VaoId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices1), Indices1, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
}

//	Schimba inaltimea/latimea scenei in functie de modificarile facute de utilizator ferestrei (redimensionari);
void ReshapeFunction(GLint newWidth, GLint newHeight)
{
	glViewport(0, 0, newWidth, newHeight);
	winWidth = newWidth;
	winHeight = newHeight;
	width = winWidth / 10, height = winHeight / 10;
}

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

//  Crearea si compilarea obiectelor de tip shader;
void CreateShaders(void)
{
	ProgramId = LoadShaders("shader.vert", "shader.frag");
	glUseProgram(ProgramId);
}

// Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

//	Functia de incarcare a texturilor in program;
void LoadTexture(const char* texturePath)
{
	// Generarea unui obiect textura si legarea acestuia;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//	Desfasurarea imaginii pe orizontala/verticala in functie de parametrii de texturare;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Modul in care structura de texeli este aplicata pe cea de pixeli;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Incarcarea texturii si transferul datelor in obiectul textura; 
	int width, height;
	unsigned char* image = SOIL_load_image(texturePath, &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Eliberarea resurselor
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	glClearColor(0.08f, 0.01f, 0.15f, 1.0f); // culoarea de fond a ecranului

	//	Incarcarea texturii;
	LoadTexture("sun.jpg");

	// Crearea VBO / shadere-lor
	CreateVBO();
	CreateShaders();

	// Locatii ptr shader
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
	matrUmbraLocation = glGetUniformLocation(ProgramId, "matrUmbra");
	viewLocation = glGetUniformLocation(ProgramId, "view");
	projLocation = glGetUniformLocation(ProgramId, "projection");
	objectColorLoc = glGetUniformLocation(ProgramId, "objectColor"); 
	lightColorLoc = glGetUniformLocation(ProgramId, "lightColor");
	lightPosLoc = glGetUniformLocation(ProgramId, "lightPos");
	viewPosLoc = glGetUniformLocation(ProgramId, "viewPos");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");

	numIndices = NR_MERID * NR_PARR * 2 * 3;
}

void DrawPlanet(
	glm::vec3 position,
	glm::vec3 scale,
	glm::vec3 color,
	glm::vec3 lightColor,
	glm::vec3 lightPosition,
	glm::vec3 observerPosition,
	int codCol = 0)
{
	// Configurare uniform pentru shader
	glUniform1i(codColLocation, codCol);
	glUniform3f(objectColorLoc, color.x, color.y, color.z);
	glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(lightPosLoc, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(viewPosLoc, observerPosition.x, observerPosition.y, observerPosition.z);

	// Matrice de transformare
	glm::mat4 matrTrans = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 matrScale = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 myMatrix = matrTrans * matrScale;

	// Trimit matricea catre shader
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

	// Desenare planeta
	glBindVertexArray(VaoId);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
}


void DrawShadow(glm::vec3 position, glm::vec3 scale)
{
	glm::vec3 shadowPosition = position + glm::vec3(0.0f, -150.0f, 0.0f);

	// Creare matrice de transformare pt umbra
	glm::mat4 matrTrans = glm::translate(glm::mat4(1.0f), shadowPosition);
	glm::mat4 matrScale = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 shadowMatrix = matrTrans * matrScale;

	// Transmiterea matricei pt umbra la shader
	glUniform1i(codColLocation, 1r); // culoare neagra
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &shadowMatrix[0][0]);

	// Desenare umbra
	glBindVertexArray(VaoId);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
}

void DrawOrbit(glm::vec3 center, float radius) {
    const int numSegments = 100; // nr de segmente pentru aproximarea cercului
    float angleStep = 2.0f * PI / numSegments;

    glLineWidth(2.0f);
    glUniform1i(codColLocation, 0);
    glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f);

    // creare si trimitere matrice de transformare pt centru
    glm::mat4 matrTrans = glm::translate(glm::mat4(1.0f), center);
    glm::mat4 myMatrix = matrTrans;
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    glBindVertexArray(0);

    // desenez orbita
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < numSegments; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();

    glLineWidth(1.0f); 
}


void RenderFunction(void)
{
	// Initializare ecran + test de adancime
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	//	Pozitia observatorului - coordonatele lui
	Obsx = Refx + dist * cos(alpha) * cos(beta);
	Obsy = Refy + dist * cos(alpha) * sin(beta);
	Obsz = Refz + dist * sin(alpha);

	//	Vectori pentru matricea de vizualizare;
	glm::vec3 Obs = glm::vec3(Obsx, Obsy, Obsz);
	glm::vec3 PctRef = glm::vec3(Refx, Refy, Refz);
	glm::vec3 Vert = glm::vec3(Vx, Vy, Vz);

	// Definirea pozitiei observatorului
	glm::vec3 observerPosition = glm::vec3(Obsx, Obsy, Obsz);
	glm::vec3 lightPosition = glm::vec3(0.f, 1000.f, 0.f);
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	// Matricea de vizualizare, transmitere catre shader
	view = glm::lookAt(Obs, PctRef, Vert);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	//	Proiectie;
	projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), znear);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

	// matricea pentru umbra
	float D = 100.0f;
	matrUmbra[0][0] = zL + D; matrUmbra[0][1] = 0; matrUmbra[0][2] = 0; matrUmbra[0][3] = 0;
	matrUmbra[1][0] = 0; matrUmbra[1][1] = zL + D; matrUmbra[1][2] = 0; matrUmbra[1][3] = 0;
	matrUmbra[2][0] = -xL; matrUmbra[2][1] = -yL; matrUmbra[2][2] = D; matrUmbra[2][3] = -1;
	matrUmbra[3][0] = -D * xL; matrUmbra[3][1] = -D * yL; matrUmbra[3][2] = -D * zL; matrUmbra[3][3] = zL;
	glUniformMatrix4fv(matrUmbraLocation, 1, GL_FALSE, &matrUmbra[0][0]);
	glUseProgram(ProgramId);

	// variabila pt soare
	glm::vec3 sunCenter = glm::vec3(0.0f, 1000.0f, 0.0f);

	// Soare
	DrawPlanet(sunCenter, glm::vec3(6.f, 6.f, 6.f), glm::vec3(1.0f, 0.4f, 0.2f), lightColor, lightPosition, observerPosition, 2);

	// Mercur
	DrawOrbit(sunCenter, 1250.f);
	DrawPlanet(glm::vec3(300.f, -200.f, 0.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.45f, 0.57f, 0.7f), lightColor, lightPosition, observerPosition);
	DrawShadow(glm::vec3(300.f, -200.f, 0.f), glm::vec3(1.f, 1.f, 1.f));
	
	// Venus
	DrawOrbit(sunCenter, 1625.f);
	DrawPlanet(glm::vec3(-300.f, -600.f, 0.f), glm::vec3(1.2f, 1.2f, 1.2f), glm::vec3(0.8f, 0.33f, 0.0f), lightColor, lightPosition, observerPosition);
	DrawShadow(glm::vec3(-300.f, -600.f, 0.f), glm::vec3(1.2f, 1.2f, 1.2f));

	// Pamant
	DrawOrbit(sunCenter, 2000.f);
	DrawPlanet(glm::vec3(100.f, -1000.f, 0.f), glm::vec3(1.4f, 1.4f, 1.4f), glm::vec3(0.0f, 0.0f, 0.63f), lightColor, lightPosition, observerPosition);
	DrawShadow(glm::vec3(100.f, -1000.f, 0.f), glm::vec3(1.4f, 1.4f, 1.4f));

	// Luna
	DrawPlanet(glm::vec3(200.f, -1100.f, 0.f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(0.7f, 0.7f, 0.7f), lightColor, lightPosition, observerPosition);
	DrawShadow(glm::vec3(200.f, -1100.f, 0.f), glm::vec3(0.4f, 0.4f, 0.4f));

	// Marte
	DrawOrbit(sunCenter, 2550.f);
	DrawPlanet(glm::vec3(500.f, -1500.f, 0.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(0.64f, 0.16f, 0.16f), lightColor, lightPosition, observerPosition);
	DrawShadow(glm::vec3(500.f, -1500.f, 0.f), glm::vec3(1.f, 1.f, 1.f));

	// Jupiter
	DrawOrbit(sunCenter, 3050.f);
	DrawPlanet(glm::vec3(-800.f, -2000.f, 0.f), glm::vec3(2.5f, 2.5f, 2.5f), glm::vec3(0.9f, 0.7f, 0.6f), lightColor, lightPosition, observerPosition);
	DrawShadow(glm::vec3(-800.f, -2000.f, 0.f), glm::vec3(2.5f, 2.5f, 2.5f));

	// Saturn
	DrawOrbit(sunCenter, 3500.f);
	DrawPlanet(glm::vec3(100.f, -2500.f, 0.f), glm::vec3(2.2f, 2.2f, 2.2f), glm::vec3(0.8f, 0.7f, 0.5f), lightColor, lightPosition, observerPosition);
	DrawShadow(glm::vec3(100.f, -2500.f, 0.f), glm::vec3(2.2f, 2.2f, 2.2f));

	// Inelele lui Saturn
	DrawPlanet(glm::vec3(100.f, -2500.f, 0.f), glm::vec3(4.0f, 4.0f, 0.2f), glm::vec3(0.9f, 0.8f, 0.6f), lightColor, lightPosition, observerPosition);

	// Uranus
	DrawOrbit(sunCenter, 4150.f);
	DrawPlanet(glm::vec3(1200.f, -3000.f, 0.f), glm::vec3(1.6f, 1.6f, 1.6f), glm::vec3(0.6f, 0.8f, 0.9f), lightColor, lightPosition, observerPosition);
	DrawShadow(glm::vec3(1200.f, -3000.f, 0.f), glm::vec3(1.6f, 1.6f, 1.6f));

	// Neptun
	DrawOrbit(sunCenter, 4800.f);
	DrawPlanet(glm::vec3(-1800.f, -3500.f, 0.f), glm::vec3(1.8f, 1.8f, 1.8f), glm::vec3(0.3f, 0.3f, 0.9f), lightColor, lightPosition, observerPosition);
	DrawShadow(glm::vec3(-1800.f, -3500.f, 0.f), glm::vec3(1.8f, 1.8f, 1.8f));


	// TEXTURARE
    // Activarea / legarea texturii active
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	//	Transmiterea variabilei uniforme pentru texturare spre shaderul de fragmente;
	glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

	glutSwapBuffers();
	glFlush();
}


//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("Sistem Solar 3D");

	glewInit();

	Initialize();
	glutReshapeFunc(ReshapeFunction);
	glutIdleFunc(RenderFunction);
	glutDisplayFunc(RenderFunction);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);
	glutCloseFunc(Cleanup);

	glutMainLoop();
}