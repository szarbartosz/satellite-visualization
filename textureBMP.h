/*
    BMP texture loader
    © Keith O'Conor 2005
    keith.oconor @ {cs.tcd.ie, gmail.com}
*/

#ifndef TEXTUREBMP_H
#define TEXTUREBMP_H

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include "GL/gl.h"
#include "GL/glu.h"

typedef unsigned char byte;

class textureBMP{
private:
    int m_width;
    int m_height;
    int m_bpp;
    int m_format;
    
public:
    textureBMP(const char *filename, const int textureId, const bool stereoMode);
	textureBMP(std::string filename, const int textureId, const bool stereoMode){textureBMP(filename.c_str(),textureId,stereoMode);}
    
    int getWidth(){return m_width;}
    int getHeight(){return m_width;}
    int getBpp(){return m_bpp;}
};

#endif //TEXTUREBMP_H
