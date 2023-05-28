#define NOMINMAX
#define _USE_MATH_DEFINES

#include <windows.h>
#include "glut.h"
#include "model3DS.h"
#include <time.h>
#include <direct.h>
#include <GL/glu.h>
#include "sphere.h"
#include "Bmp.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <json/json.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include "DateTime.h"
#include "CoordTopocentric.h"
#include "CoordGeodetic.h"
#include "Observer.h"
#include "SGP4.h"
#include <map>
#include "Tle.h"
#include <json/json.h>


using namespace std;
using namespace libsgp4;

// window config
int windowWidth = 800;
int windowHeight = 600;
bool isFullScreen = false;
GLint windowLeft = 1, windowRight = 2;
const int TEXT_WIDTH = 8;
const int TEXT_HEIGHT = 21;
void* font = GLUT_BITMAP_8_BY_13;

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
double cameraVerticalSpeed;
double cameraSpeed;
bool cameraMobility;
double cameraAngle;
double cameraSpinSpeed;
#define MIN_DISTANCE 0.5
double cameraArea = 0;

//earth sphere and satellite setup
struct SatellitePos{
	float x;
	float y;
	float z;
};

Sphere earthSphere(1.0f, 36, 18, true, 2);
GLuint earthTexId;
vector<string> satnames;
vector<SatellitePos> satellitePositions;
vector<vector<SatellitePos>> satellitePositionsFrom2Sec;
vector<string> TLEs;
float interpolationIncrement = 0;
int prevSecond = 0;
int numberOfSatellites = 0;

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


/*** HANDLING API KY FOR SATELITE DATA FETCHING ***/

std::string readFromFile(const std::string& filePath) {
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

vector<string> splitString(string str, char splitChar) {
	std::vector<std::string> splitVector;
	std::stringstream ss(str);
	std::string singleStr;

	while (std::getline(ss, singleStr, splitChar)) {
		// Remove leading and trailing whitespaces from each part
		singleStr.erase(0, singleStr.find_first_not_of(" \t"));
		singleStr.erase(singleStr.find_last_not_of(" \t") + 1);

		splitVector.push_back(singleStr);
	}

	return splitVector;
}

vector<string> getTLEsFromAPI(int numberOfSatellites) {
	vector<string> TLEsVector;
	std::string apiKey = readFromFile("apikey.txt");
	std::string response;

	std::cout << "Fetching data from API...\n";

	std::string noradIDStr = readFromFile("noradIDs.txt");
	vector<string> noradIDs = splitString(noradIDStr, ',');

	int numberOfSatellitesToDisplay = numberOfSatellites < noradIDs.size() ? numberOfSatellites : noradIDs.size();

	for (int i = 0; i < numberOfSatellitesToDisplay; i++) {
		std::string apiURL = "https://api.n2yo.com/rest/v1/satellite//tle/" + noradIDs[i] + "&apiKey=" + apiKey;
		try {
			curlpp::Cleanup cleaner;
			curlpp::Easy request;

			request.setOpt(new curlpp::options::Url(apiURL));
			std::ostringstream stream;
			curlpp::options::WriteStream ws(&stream);
			request.setOpt(ws);

			// Perform the request
			request.perform();

			// Get the response from the stream
			response = stream.str();
		}
		catch (curlpp::LogicError& e) {
			std::cout << e.what() << std::endl;
		}
		catch (curlpp::RuntimeError& e) {
			std::cout << e.what() << std::endl;
		}

		Json::Value root;
		Json::Reader reader;
		reader.parse(response, root);
		if (root["tle"].asString() != "") {
			TLEsVector.push_back(root["tle"].asString());
			std::string satname = root["info"]["satname"].asString();
			satnames.push_back(satname);
		}
	}
	for (string tle : TLEsVector) {
		std::cout << tle << std::endl;
	}
	return TLEsVector;
}


SatellitePos interpolate(const SatellitePos& p1, const SatellitePos& p2, double t) {
	SatellitePos result;
	result.x = p1.x + (p2.x - p1.x) * t;
	result.y = p1.y + (p2.y - p1.y) * t;
	result.z = p1.z + (p2.z - p1.z) * t;
	return result;
}

SatellitePos createSatellitePos(float x, float y, float z) {
	SatellitePos satellitePos = { x, y, z };
	return satellitePos;
}

vector<int> getCurrentTimeV() {
	auto now = std::chrono::system_clock::now();
	auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
	auto value = now_ms.time_since_epoch().count();

	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm* timeinfo = std::gmtime(&now_c);

	std::ostringstream oss;
	oss << std::put_time(timeinfo, "%Y %m %d %H %M %S ");

	std::string milliseconds = std::to_string(value % 1000);
	oss << std::setw(3) << std::setfill('0') << milliseconds;

	std::string currentTime = oss.str();

	vector<int> dateTimeV;
	std::istringstream iss(currentTime);
	std::string temp;

	while (std::getline(iss, temp, ' ')) {
		dateTimeV.push_back(stoi(temp));
	}

	// Displaying the vector elements
	return dateTimeV;
}



void resetCamera(){
	cameraX = 0;
	cameraY = 0;
	cameraZ = 260;
	cameraAngle = 0;
	cameraPointY = 0;
	cameraPointYSpeed = 0;
	cameraVerticalSpeed = 0;
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

GLuint loadTextureBruh(const char* fileName, bool wrap)
{
	Image::Bmp bmp;
	if (!bmp.read(fileName))
		return 0;     // exit if failed load image

	// get bmp info
	int width = bmp.getWidth();
	int height = bmp.getHeight();
	const unsigned char* data = bmp.getDataRGB();
	GLenum type = GL_UNSIGNED_BYTE;    // only allow BMP with 8-bit per channel

	// We assume the image is 8-bit, 24-bit or 32-bit BMP
	GLenum format;
	int bpp = bmp.getBitCount();
	if (bpp == 8)
		format = GL_LUMINANCE;
	else if (bpp == 24)
		format = GL_RGB;
	else if (bpp == 32)
		format = GL_RGBA;
	else
		return 0;               // NOT supported, exit

	// gen texture ID
	GLuint texture;
	glGenTextures(1, &texture);

	// set active texture and configure it
	glBindTexture(GL_TEXTURE_2D, texture);

	// select modulate to mix texture with color for shading
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// if wrap is true, the texture wraps over at the edges (repeat)
	//       ... false, the texture ends at the edges (clamp)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// copy texture data
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data);
	//glGenerateMipmap(GL_TEXTURE_2D);

	// build our texture mipmaps
	switch (bpp)
	{
	case 8:
		gluBuild2DMipmaps(GL_TEXTURE_2D, 1, width, height, GL_LUMINANCE, type, data);
		break;
	case 24:
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, type, data);
		break;
	case 32:
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, type, data);
		break;
	}

	bmp.printSelf();
	return texture;
}

void loadEarthTexture() {
	earthTexId = loadTextureBruh("earth2048.bmp", true);
	std::cout << "Loaded earth texture - ID: " << earthTexId << "\n" << std::endl;
}

void drawString(const char* str, int x, int y, float color[4], void* font)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
	glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
	glDisable(GL_TEXTURE_2D);

	glColor4fv(color);          // set text color
	glRasterPos2i(x, y);        // place text position

	// loop all characters in the string
	while (*str)
	{
		glutBitmapCharacter(font, *str);
		++str;
	}

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glPopAttrib();
}

void showInfo()
{
	// backup current model-view matrix
	glPushMatrix();                     // save current modelview matrix
	glLoadIdentity();                   // reset modelview matrix

	// set to 2D orthogonal projection
	glMatrixMode(GL_PROJECTION);        // switch to projection matrix
	glPushMatrix();                     // save current projection matrix
	glLoadIdentity();                   // reset projection matrix
	//gluOrtho2D(0, screenWidth, 0, screenHeight); // set to orthogonal projection
	glOrtho(0, windowWidth, 0, windowHeight + TEXT_HEIGHT, -1, 1); // set to orthogonal projection

	float color[4] = { 1, 1, 1, 1 };

	std::stringstream ss_satnames;
	for (size_t i = 0; i < satnames.size(); ++i) {
		ss_satnames << satnames[i];
		if (i != satnames.size() - 1) {
			ss_satnames << ", ";
		}
	}
    std::string satnamesStr = ss_satnames.str();

	std::stringstream ss;
	ss << std::fixed << std::setprecision(3);

	ss << "Project: SATELLITE VISUALIZATION" << std::ends;
	drawString(ss.str().c_str(), 1, windowHeight - TEXT_HEIGHT, color, font);
	ss.str("");

	ss << "Shown satellites: " << satnamesStr << std::ends;
	drawString(ss.str().c_str(), 1, windowHeight - 2*TEXT_HEIGHT, color, font);
	ss.str("");

	// unset floating format
	ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);

	// restore projection matrix
	glPopMatrix();                   // restore to previous projection matrix

	// restore modelview matrix
	glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
	glPopMatrix();                   // restore to previous modelview matrix
}

void output(int x, int y, float r, float g, float b, char* string)
{
	glColor3f(r, g, b);
	glRasterPos2f(x, y);
	int len, i;
	len = (int)strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, string[i]);
	}
}

void windowInit()
{
	glEnable(GL_NORMALIZE);						// normalize the normals after scaling

	glShadeModel(GL_SMOOTH);                    // shading method: GL_SMOOTH or GL_FLAT
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

	// enable /disable features
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);

	//// track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	//glEnable(GL_COLOR_MATERIAL);

	glClearColor(0, 0, 0, 0);                   // background color
	glClearStencil(0);                          // clear stencil buffer
	glClearDepth(1.0f);                         // 0 is near, 1 is far
	glDepthFunc(GL_LEQUAL);

	GLfloat lightKa[] = { .3f, .3f, .3f, 1.0f };  // ambient light
	GLfloat lightKd[] = { .7f, .7f, .7f, 1.0f };  // diffuse light
	GLfloat lightKs[] = { 1, 1, 1, 1 };           // specular light
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

	//// position the light
	float lightPos[4] = { 0, 10, 80, 0 }; // directional light
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	glEnable(GL_LIGHT0);

	/******************* FOG **************************/

	float fogColor[4]= {0.9f, 0.9f, 0.9f, 0.1f};
	glFogi(GL_FOG_MODE,GL_EXP2); // [GL_EXP, GL_EXP2, GL_LINEAR ]
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 0.009f);
	glFogf(GL_FOG_START, 0.0f);
	glFogf(GL_FOG_END, 100.0f);
	//glEnable(GL_FOG);

	loadEarthTexture();
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

void addModel(model3DS * _model, char* file_name)
{
    struct model_in_composition* tmp;
    tmp = (struct model_in_composition *) malloc (sizeof(struct model_in_composition));
    tmp -> filename  = (char *) malloc(strlen(file_name)+1);
    strcpy( tmp -> filename, file_name);
    tmp -> model = _model;
    tmp->wsk = models_composition;
    models_composition = tmp;
}

model3DS * downloadModel(char* file_name)
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
	if (model_tmp = downloadModel(file_name))
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
			addModel(model_tmp,fd->cFileName);
			printf("[3DS] Model '%s' stored\n",fd->cFileName);
		} while(FindNextFile(fh,fd));
}

/****************** DRAWING FRAME CONTENT ******************/

void drawFrame(bool right)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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


	#define _SATELLITE_PROPAGATION
	#include "satellitePropagation.cpp"

	#define _DRAWING
	#include "drawing.cpp"
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
	// double cameraYTmp = cameraY + sin(cameraVerticalSpeed);

	cameraAngle = cameraAngle + cameraSpinSpeed;
	cameraPointY = cameraPointY + cameraPointYSpeed;
	if (inArea(cameraXTmp,cameraZTmp))
	{
		cameraX = cameraXTmp;
		cameraZ = cameraZTmp;
		// cameraY = cameraYTmp;
	} else
		cameraSpeed = 0;
	draw();
}

void syncTimer (int ID)
{
	timer();
	glutTimerFunc(1,syncTimer,10);
}


int main(int argc, char **argv)
{
	#define _CONFIGURATION
	#include "configuration.cpp"

	std::cout << "Please provide the number of satellites that you want to visualize: ";
	std::cin >> numberOfSatellites;
	TLEs = getTLEsFromAPI(numberOfSatellites);

	if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 's')
	{
		stereoMode = 2;
		windowWidth = 800;
		windowHeight = 600;
	}
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
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
		glutKeyboardUpFunc (OnKeyRelease);
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
		glutKeyboardUpFunc(OnKeyRelease);
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
		glutKeyboardUpFunc(OnKeyRelease);
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