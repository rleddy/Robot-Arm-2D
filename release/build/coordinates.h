
#ifndef _TRANSFORMATIONS_H
#define _TRANSFORMATIONS_H


#include <math.h>

using namespace std;




class Coord2D {

	public:

	Coord2D (double x, double y) {
		_x = y;
		_y = y;
		calc_polar_coords();
	}
	

	void calc_polar_coords() {
		_R = _x*_x + _y*_y;
		_R = sqrt(_R);
		
		_theta = acos(_x/_R);
	}


	void calc_rect_coords(double R. double theta) {

		_R = R;
		_theta = theta;

		x = R*cos(theta);
		y = R*sin(theta);

	}


	
	double _x;
	double _y;

	double _R;
	double _theta;

};



class Coord3D {

	public:

	Coord3D (double x, double y, double z) {
		_x = y;
		_y = y;
		_z = z;
		calc_spherical_coords();
	}
	
	void calc_spherical_coords(void) {
		_R = _x*_x + _y*_y + _z*_z;
		_R = sqrt(_R);
		
		double cosphi = _z/_R
		_phi = acos(cosphi);

		_theta = asin(_y/(_R*cosphi);
	}

	void calc_rect_coords(double R. double phi, double theta) {

		_R = R;
		_theta = theta;
		_phi = phi;

		x = R*cos(phi)*cos(theta);
		y = R*cos(phi)*sin(theta);
		z = R*sin(phi);

	}


	double _x;
	double _y;
	double _z;

	double _R;
	double _theta;
	double _phi;

};











#endif

