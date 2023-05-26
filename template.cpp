#include <windows.h>
#include "glut.h"
#include "model3DS.h"
#include <time.h>
#include <direct.h>
#include <GL/glu.h>
#include "sphere.h"
#include "satellite.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>


//#include <GL/glaux.h>
//#define GLUTCHECKLOOP
	//{"info":{"satid":25544,"satname":"SPACE STATION","transactionscount":1},"tle":"1 25544U 98067A   23144.35992656  .00014977  00000-0  26854-3 0  9995\r\n2 25544  51.6416  86.6432 0005375  14.6417 128.6708 15.50136251398097"}
int windowWidth = 800;
int windowHeight = 600;
bool isFullScreen = false;
GLint windowLeft = 1, windowRight = 2;

// stereo projection options
int stereoMode = 0;
int stereoEyeSpacing = 5;				
int lookAtPointDistance = 150;
bool stereoFrameId = false;
bool timing100FPS = true;

// camera options
int mouseX;
int mouseY;
double cameraX;
double cameraY;
double cameraZ;
double cameraPointY;
double cameraPointYSpeed;
double cameraSpeed;
bool cameraMobility;
double cameraAngle;
double cameraSpinSpeed;
#define MIN_DISTANCE 0.5
double cameraArea = 0;

#define _DEFINITIONS
#include "definitions.cpp"


/** OBSTACLE REGISTER **/

struct obstacle{
	obstacle *next;
	double posX1;
	double posZ1;
	double posX2;
	double posZ2;
};
obstacle *obstacleArea = NULL;

void resetCamera(){
	cameraX = 0;
	cameraY = 4;
	cameraZ = 40;
	cameraAngle = -0.4;
	cameraPointY = -15;
	cameraPointYSpeed = 0;
	cameraSpeed = 0;
	cameraSpinSpeed = 0;
	cameraMobility = true;
}

void setupCameraArea (double X){
	cameraArea = X;	
}

bool inArea(double posX, double posZ){ 
	if ( posX*posX + posZ*posZ > (cameraArea-MIN_DISTANCE*2)*(cameraArea-MIN_DISTANCE*2) ) return false;
	obstacle * pom = obstacleArea;
	while (pom){
		if (pom->posX1 < posX && 
			pom->posX2 > posX && 
			pom->posZ1 < posZ && 
			pom->posZ2 > posZ ) return false;
		pom = pom -> next;
	}
	return true;
}

void drawObstacle(double X1, double Z1, double X2, double Z2){
	obstacle * pom = new obstacle;
	if (X1 > X2) {double tmp = X1; X1 = X2; X2 = tmp;}
	if (Z1 > Z2) {double tmp = Z1; Z1 = Z2 ;Z2 = tmp;}
	pom -> posX1 = X1;
	pom -> posZ1 = Z1;
	pom -> posX2 = X2;
	pom -> posZ2 = Z2;
	pom -> next = obstacleArea;
	obstacleArea = pom;
}

/** USER INTERACTION HANDLING **/

void DefaultOnMouseClick (int button, int state, int x, int y)
{
	switch (state)
	{
		case GLUT_UP:
			cameraSpeed = 0;
			cameraSpinSpeed = 0;
			cameraPointYSpeed = 0;
		break;
		case GLUT_DOWN:
				mouseX = x;
				mouseY = y;
			if (button == GLUT_LEFT_BUTTON)
				cameraMobility = true;
			else 
				cameraMobility = false;
		break;
	}
}

void DefaultOnMousePane (int x, int y)
{
	cameraSpinSpeed = -(mouseX - x) * 0.001;
	if (cameraMobility)
	{
		cameraSpeed = (mouseY - y) * 0.02;
		cameraPointYSpeed = 0;
	} else {
		cameraPointYSpeed = (mouseY - y) * 0.06;
		cameraSpeed = 0;
	}
}

void DefaultOnKeyPress (GLubyte key, int x, int y)
{
   switch (key) {
   case 27:    
      exit(1);
   break;
   case ' ':    
      if (stereoMode != 2) glutFullScreen();
      break;

   }

}

/** INITIALIZATION **/

#define _INTERACTION
#include "interaction.cpp"

void windowInit()
{
	glClearColor(0.2, 0.2, 1.0, 0.0);			
    glShadeModel(GL_SMOOTH);					
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST); 
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL); 
	glEnable(GL_LIGHTING);
	GLfloat  ambient[4] = {0.3,0.3,0.3,1};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient); 

	GLfloat  diffuse[4] = {0.9,0.9,0.9,1};
	GLfloat  specular[4] = {0.9,0.9,0.9,1};
	GLfloat	 position[4] = {30,30,-30,1};
	glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
	glLightfv(GL_LIGHT0,GL_SPECULAR,specular);
	glLightfv(GL_LIGHT0,GL_POSITION,position);
	glEnable(GL_LIGHT0);

	/******************* FOG **************************/

	float fogColor[4]= {0.9f, 0.9f, 0.9f, 0.1f};
	glFogi(GL_FOG_MODE,GL_EXP2); // [GL_EXP, GL_EXP2, GL_LINEAR ]
	glFogfv(GL_FOG_COLOR, fogColor); 
	glFogf(GL_FOG_DENSITY, 0.009f); 
	glFogf(GL_FOG_START, 0.0f); 
	glFogf(GL_FOG_END, 100.0f); 
	//glEnable(GL_FOG);  

}

void size (int width, int height)
{
	if (width==0) width++;
	if (width==0) width++;
	if (stereoMode != 2) {
		windowWidth=width;
		windowHeight=height; 
	}
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glViewport(0,0,windowWidth,windowHeight+24); 
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity(); 
    gluPerspective(45.0f,(GLfloat)windowWidth/(GLfloat)windowHeight,1.0f,1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void sizeLeft (int width, int height)
{
	glutSetWindow(windowLeft);
	size(width, height);
}

void sizeRight (int width, int height)
{
	glutSetWindow(windowRight);
	size(width, height);
}

/** 3DS MODELS MANAGING **/

struct model_in_composition {
	char * filename;
	model3DS * model;
	struct model_in_composition *wsk;
};
struct model_in_composition* models_composition = NULL;

void dodajModel (model3DS * _model, char* file_name)
{
    struct model_in_composition* tmp;
    tmp = (struct model_in_composition *) malloc (sizeof(struct model_in_composition));
    tmp -> filename  = (char *) malloc(strlen(file_name)+1);
    strcpy( tmp -> filename, file_name);
    tmp -> model = _model;
    tmp->wsk = models_composition;
    models_composition = tmp;
}

model3DS * downloadModel (char* file_name)
{
	struct model_in_composition* composition_tmp = models_composition;
	while (composition_tmp){
		if (!_stricmp(composition_tmp->filename,file_name)) return composition_tmp->model;
		char file_name_full[_MAX_PATH];
			strcpy (file_name_full,file_name);
			strcat (file_name_full,".3ds");
		if (!_stricmp(composition_tmp->filename,file_name_full)) return composition_tmp->model;

		composition_tmp = composition_tmp->wsk;
	}
	return NULL;
}

void drawModel(char * file_name, int tex_num = -1 )
{
	model3DS * model_tmp;	
	if (model_tmp = downloadModel(file_name))
		if (tex_num == -1) 
			model_tmp -> draw();
		else
			model_tmp -> draw(tex_num, false);
		
}

void activateSpecialModelRender(char * file_name, int spec_id = 0)
{
	model3DS * model_tmp;	
	if (model_tmp = downloadModel (file_name))
		model_tmp->setSpecialTransform(spec_id);
}

void loadModels()
{
	WIN32_FIND_DATA *fd;
	HANDLE fh;
	model3DS * model_tmp;
	char directory[_MAX_PATH];
	if( _getcwd( directory, _MAX_PATH ) == NULL ) return;
	strcat (directory,"\\data\\*.3ds");
	
	fd = (WIN32_FIND_DATA *)malloc(sizeof(WIN32_FIND_DATA));
	fh = FindFirstFile((LPCSTR) directory,fd);
	if(fh != INVALID_HANDLE_VALUE)
		do {
			if(fd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
				if (FindNextFile(fh,fd)) continue; else break;
			}
			char filename[_MAX_PATH];
			strcpy (filename,"data\\");
			strcat (filename,fd->cFileName);
			model_tmp = new model3DS (filename,1,stereoMode == 2);
			dodajModel (model_tmp,fd->cFileName);
			printf("[3DS] Model '%s' stored\n",fd->cFileName);
		} while(FindNextFile(fh,fd));
}

/****************** DRAWING FRAME CONTENT ******************/



float translateX = 0;
float translateY = 0;
float translateZ = 0;

void drawFrame(bool right)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	switch (stereoMode){
		case 0: // mono
			 gluLookAt (cameraX, cameraY, cameraZ, cameraX + 100*sin(cameraAngle), 3 + cameraPointY, cameraZ - 100*cos(cameraAngle), 0, 1, 0); // camera
		break;
		case 1: // 3D-ready
		case 2: // stereo
			if (right){
				float newcameraX = cameraX - stereoEyeSpacing/2*cos(cameraAngle);
				float newcameraZ = cameraZ - stereoEyeSpacing/2*sin(cameraAngle);
				gluLookAt (newcameraX, cameraY, newcameraZ, cameraX+0.2 + lookAtPointDistance*sin(cameraAngle), 3 + cameraPointY, cameraZ - lookAtPointDistance*cos(cameraAngle), 0, 1, 0); // camera
			}
			else {
				float newcameraX = cameraX + stereoEyeSpacing/2*cos(cameraAngle);
				float newcameraZ = cameraZ + stereoEyeSpacing/2*sin(cameraAngle);
				gluLookAt (newcameraX, cameraY, newcameraZ, cameraX+0.2 + lookAtPointDistance*sin(cameraAngle), 3 + cameraPointY, cameraZ - lookAtPointDistance*cos(cameraAngle), 0, 1, 0); // camera
			}
		break;
	}

	#define _DRAWING
	#include "drawing.cpp"

	glTranslatef(translateX, translateY, translateZ);

	Satellite satellite(1.0f, 36, 18, false, 2);
	std::vector<float> a{ 2,2,2 };
	satellite.draw(a, 5, "my string");


	glFlush(); 
    glPopMatrix();
}

void draw()
{
	switch (stereoMode){
		case 0: // mono
			 drawFrame (false);
			 glutSwapBuffers(); 
		break;
		case 1: // 3D-ready
			 stereoFrameId = !stereoFrameId;
			 drawFrame (stereoFrameId);
			 glutSwapBuffers(); 
		break;
		case 2: // stereo
			glutSetWindow(windowLeft);
			drawFrame (false);
			glutSetWindow(windowRight);
			drawFrame  (true);
			glutSetWindow(windowLeft);
	 		glutSwapBuffers();
			glutSetWindow(windowRight);
	 		glutSwapBuffers(); 
		break;
	}
}

void timer()
{
	double cameraXTmp = cameraX+cameraSpeed*sin(cameraAngle);
    double cameraZTmp = cameraZ-cameraSpeed*cos(cameraAngle);
	cameraAngle = cameraAngle + cameraSpinSpeed;
	cameraPointY = cameraPointY + cameraPointYSpeed;
	if (inArea(cameraXTmp,cameraZTmp))
	{
		cameraX = cameraXTmp;
		cameraZ = cameraZTmp;
	} else 
		cameraSpeed = 0;
	draw();		
}

void syncTimer (int ID)
{
	timer();
	glutTimerFunc(1,syncTimer,10);
}

/*** HANDLING API KY FOR SATELITE DATA FETCHING ***/

std::string readApiKeyFromFile(const std::string& filePath) {
	std::ifstream file(filePath); // Open the file

	if (!file) {
		std::cerr << "Failed to open the file: " << filePath << std::endl;
		return ""; // Return an empty string or handle the error appropriately
	}

	std::string apiKey;
	std::getline(file, apiKey); // Read the API key from the file

	file.close(); // Close the file

	return apiKey;
}

int main(int argc, char **argv)
{
	#define _CONFIGURATION
	#include "configuration.cpp"
	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 's')
	{
		stereoMode = 2;
		windowWidth = 800;
		windowHeight = 600;
	}
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	if (stereoMode == 2) {
		glutInitWindowSize(windowWidth - 8, windowHeight);
		glutInitWindowPosition(0,0);
		windowLeft = glutCreateWindow("Left");
		HWND hwnd = FindWindow(NULL, "Left");
		SetWindowLong(hwnd, GWL_STYLE, WS_BORDER | WS_MAXIMIZE);
		glutSetWindow(windowLeft);
		windowInit();
		glutReshapeFunc (sizeLeft);
		glutKeyboardFunc (OnKeyPress);
		glutSpecialFunc (OnSpecialKeyPress);
		glutMouseFunc (OnMouseClick);
		glutMotionFunc (OnMousePane);
		glutDisplayFunc(draw);
		glutInitWindowSize(windowWidth - 8, windowHeight);
		glutInitWindowPosition(windowWidth + 4, 0);
		windowRight = glutCreateWindow("Right");
		glutSetWindow(windowRight);
		windowInit();
		hwnd = FindWindow(NULL, "Right");
		SetWindowLong(hwnd, GWL_STYLE, WS_BORDER | WS_MAXIMIZE);
		glutReshapeFunc (sizeRight);
		glutKeyboardFunc (OnKeyPress);
		glutSpecialFunc (OnSpecialKeyPress);
		glutMouseFunc (OnMouseClick);
		glutMotionFunc (OnMousePane);
		glutDisplayFunc(draw);
	} else {
		glutInitWindowSize(windowWidth, windowHeight);
		glutInitWindowPosition(0,0);
		windowLeft = glutCreateWindow("Template");  
		windowInit();
		glutReshapeFunc (size);
		glutKeyboardFunc (OnKeyPress);
		glutSpecialFunc (OnSpecialKeyPress);
		glutMouseFunc (OnMouseClick);
		glutMotionFunc (OnMousePane);
		glutDisplayFunc(draw);
	}
		if (stereoMode == 1 || !timing100FPS)
			glutIdleFunc(timer);				
		else 
			glutTimerFunc(10,syncTimer,10);
		resetCamera();
		//srand( (unsigned)time( NULL ) );
	    loadModels();
		activateSpecialModelRender("woda",1);
		activateSpecialModelRender("most",2);
		if (isFullScreen && stereoMode != 2) glutFullScreen();
		glutMainLoop();        
	return(0);    
}