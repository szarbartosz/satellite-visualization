/*
    Vector class
    © Keith O'Conor 2004
    keith.oconor @ {cs.tcd.ie, gmail.com}
*/

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <iostream>

class Vector{
public:
	float x;
	float y;
	float z;

	/* 
	Vector();
	Vector(float a, float b, float c);
	Vector(Vector& copy);

	Vector& operator=(const Vector &other);
	int		operator== (const Vector &other);
	int		operator!= (const Vector &other);
	Vector	operator+(const Vector &other);
	Vector	operator-(const Vector &other);
	Vector	operator*(const float &value);
	Vector	operator/(const float &value);
	Vector& operator+=(const Vector &other);
	Vector& operator-=(const Vector &other);
	Vector& operator*=(const float& other);
	Vector& operator/=(const float& other);
	float	operator[](unsigned i);

	float	length();
	float	lengthSq();
	float	dotProduct(const Vector &other);
	Vector	crossProduct(const Vector &other);
	void	normalize();
	float	distance(const Vector &other);
	float	distanceSq(const Vector &other);
	void	set(float newX, float newY, float newZ);
	void	zero();
	*/

	  /////////////////
	 // Constructors
	/////////////////

	inline Vector():x(0),y(0),z(0){}
	inline Vector(const float a, const float b, const float c):x(a),y(b),z(c){}
	inline Vector(const Vector& copy):x(copy.x),y(copy.y),z(copy.z){}

	  //////////////
	 // Operators
	//////////////

	inline Vector& operator= (const Vector &other){
		x=other.x;y=other.y;z=other.z;
		return *this;
	}

	inline int operator== (const Vector &other) const{
		return (x==other.x && y==other.y && z==other.z);
	}
	inline int operator!= (const Vector &other) const{
		return (x!=other.x || y!=other.y || z!=other.z);
	}

	inline Vector operator+ (const Vector &other) const{
		return Vector(x+other.x, y+other.y, z+other.z);
	}

	inline Vector operator- (const Vector &other) const{
		return Vector(x-other.x, y-other.y,	z-other.z);
	}

	inline Vector operator* (const float &value) const{
		return Vector(x*value, y*value,	z*value);
	}

	inline Vector operator/ (const float &value) const{
		return Vector(x/value, y/value,	z/value);
	}

	inline Vector& operator+= (const Vector &other){
		x+=other.x;
		y+=other.y;
		z+=other.z;
		return *this;
	}
	inline Vector& operator-= (const Vector &other){
		x-=other.x;
		y-=other.y;
		z-=other.z;
		return *this;
	}

	inline Vector& operator*= (const float &value){
		x*=value;
		y*=value;
		z*=value;
		return *this;
	}

	inline Vector& operator/= (const float &value){
		x/=value;
		y/=value;
		z/=value;
		return *this;
	}

	inline float operator[] (unsigned i) const{
		switch(i){
		case 0:return x;
		case 1:return y;
		case 2:return z;
		}
	}

	  /////////////////////
	 // Other operations 
	/////////////////////

	inline float length() const{
		float len=(x*x)+(y*y)+(z*z);
		return (float)sqrt(len);
	}

	inline float lengthSq() const{
		return (x*x)+(y*y)+(z*z);
	}

	inline float dotProduct(const Vector &other) const{
		//this[dot]other
		return (x*other.x) + (y*other.y) + (z*other.z);
	}

	inline Vector crossProduct(const Vector &other) const{
		//(x1,y1,z1)×(x2,y2,z2) = (y1z2-y2z1, x2z1-x1z2, x1y2-x2y1). 
		return Vector(
			(y*other.z) - (z*other.y),
			(z*other.x) - (x*other.z),
			(x*other.y) - (y*other.x)
		);
	}

	inline void normalize(){
		float len=length();
		if(len==0)return;
		len=1.0f/len;
		x*=len;
		y*=len;
		z*=len;
	}

	inline float distance(const Vector &other) const{
		return (Vector(other.x-x,other.y-y,other.z-z)).length();
	}

	inline float distanceSq(const Vector &other) const{
		return (Vector(other.x-x,other.y-y,other.z-z)).lengthSq();
	}

	inline void set(float newX, float newY, float newZ){
		x=newX;y=newY;z=newZ;
	}

	inline void zero(){
		x=y=z=0;
	}

};

typedef Vector Vertex;

const Vector vZero=Vector(0,0,0);

  /////////////////////////////
 // Global stream operators 
//////////////////////////////
inline std::ostream& operator<<(std::ostream &str, const Vector &v){
	str<<v.x<<", "<<v.y<<", "<<v.z;
	return str;
}

#endif //VECTOR_H