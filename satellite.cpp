///////////////////////////////////////////////////////////////////////////////
// Sphere.cpp
// ==========
// Sphere for OpenGL with (radius, sectors, stacks)
// The min number of sectors is 3 and the min number of stacks are 2.
// The default up axis is +Z axis. You can change the up axis with setUpAxis():
// X=1, Y=2, Z=3.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2017-11-01
// UPDATED: 2023-03-11
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>    // include windows.h to avoid thousands of compile errors even though this class is not depending on Windows
#endif

#define _USE_MATH_DEFINES
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <iostream>
#include <iomanip>
#include <cmath>
#include "satellite.h"
#include <string>

#include "DateTime.h"
#include "CoordTopocentric.h"
#include "CoordGeodetic.h"
#include "Observer.h"
#include "SGP4.h"
#include "Tle.h"

using namespace std;
using namespace libsgp4;

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


Satellite::Satellite(float radius, int sectors, int stacks, bool smooth, int up) : interleavedStride(32)
{
    set(radius, sectors, stacks, smooth, up);
}

void Satellite::set(float radius, int sectors, int stacks, bool smooth, int up)
{
    if (radius > 0)
        this->radius = radius;
    this->sectorCount = sectors;
    
}

int prevSecond = 0;


void Satellite::draw(std::vector<float> centerPoint, float radius, string TLE) const {
    
    Sphere sphere1(0.2f, 36, 18, false, 2);
    sphere1.draw();
    

    
    vector<int> currentTimeV = getCurrentTimeV();

    if (prevSecond == (int)(currentTimeV[6] / 500)) {
        return;
    }
    prevSecond = (int)(currentTimeV[6] / 500);
    
    std::string line1 = "1 25544U 98067A   23146.29199340  .00013557  00000+0  24348-3 0  9997";
    std::string line2 = "2 25544  51.6416  77.0709 0005558  20.0880 112.3558 15.50182998398391";
    
    


    // Create a TLE object from the two lines of text
    Tle tle("ISS (ZARYA)", line1, line2);

    // Create an SGP4 propagator object
    SGP4 sgp4(tle);

    // Create a UTC date and time object for the desired time
    
    /*cout << prevSecond << endl;
    for (int i : currentTimeV) {
        std::cout << i << " ";
    }*/
    DateTime dt(currentTimeV[0], currentTimeV[1], currentTimeV[2], currentTimeV[3], currentTimeV[4], currentTimeV[5], currentTimeV[6]);

    // Propagate the satellite's orbit to the desired time
    Eci eci = sgp4.FindPosition(dt);

    // Convert the satellite's ECI coordinates to geodetic coordinates
    CoordGeodetic geo = eci.ToGeodetic();

    /*std::cout << eci.Position().x << endl;
    std::cout << eci.Position().y << endl;
    std::cout << eci.Position().z << endl;*/

    //std::cout << geo.latitude << endl;
    //std::cout << geo.longitude << endl;
    //std::cout << geo.altitude << endl;
    
    //translateX = eci.Position().x / 100;
    //translateY = eci.Position().y / 100;
    //translateZ = eci.Position().z / 100;

    std::cout << geo.latitude * 180 / M_PI << endl;
    std::cout << geo.longitude * 180 / M_PI << endl;
    std::cout << geo.altitude << endl << endl;




    
}