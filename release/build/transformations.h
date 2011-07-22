
#ifndef _TRANSFORMATIONS_H
#define _TRANSFORMATIONS_H


#include <math.h>

using namespace std;



class Matrix2D {
	public:

		Matrix2D(void) {
			_x11 = 0.0, _x12 = 0.0, _x21 = 0.0, _x22 = 0.0;
		}

		virtual ~Matrix2D(void) {
		}

		void set_values(double a1,double a2,double a3,double a4) {
			_x11 = a1;
			_x12 = a2;
			_x21 = a3;
			_x22 = a4;
		}

		void copy(Matrix2D *first) {
			this->set_values(first->a11(),first->a12(),first->a21(),first->a22());
		}

		void copy(Matrix2D &first) {
			this->set_values(first.a11(),first.a12(),first.a21(),first.a22());
		}

	protected:

		double _x11, _x12, _x21, _x22;

	public:

		double a11(void) { return(_x11); }
		double a12(void) { return(_x12); }
		double a21(void) { return(_x21); }
		double a22(void) { return(_x22); }

		void matrix_transform(double &x,double &y) {
			double xM = (this->a11()*x) + (this->a12()*y);
			double yM = (this->a21()*x) + (this->a22()*y);
			x = xM;
			y = yM;
		}

		Matrix2D & operator*(Matrix2D &m2) {
			double x = 0.0, y = 0.0;
				///---------------->>
			x = m2.a11();
			y = m2.a21();
			this->matrix_transform(x,y);
			_x11 = x;
			_x21 = y;
				///---------------->>
				//
			x = m2.a21();
			y = m2.a22();
			this->matrix_transform(x,y);
			_x21 = x;
			_x22 = y;
				//
			return(*this);
		}

		Matrix2D & operator =(Matrix2D &source) {
			this->copy(&source);
			return(*this);
		}

		Matrix2D & operator*(double scale) {
				//
			_x11 = scale*this->a11();
			_x12 = scale*this->a12();
				//
			_x21 = scale*this->a21();
			_x22 = scale*this->a22();
				//
			return(*this);
		}

};





class JointTransformation {

public:

	JointTransformation(void) {
	}

	virtual ~JointTransformation(void) {
	}

	void matrix_transform(Matrix2D &m1,double &x,double &y) {
		m1.matrix_transform(x,y);
	}


	void matrix_2d_transform(double M[4],double &x,double &y) {
		double xM = x*M[0] + y*M[1];
		double yM = x*M[2] + y*M[3];
		x = xM;
		y = yM;
	}

	void rotate_2d(double theta, double &x, double &y) {
		double M[4];
		M[0] = cos(theta);
		M[1] = -sin(theta);
		M[2] = sin(theta);
		M[3] = cos(theta);
		matrix_2d_transform(M,x,y);
	}

	void rotate_2d_sign(double theta, int sign, double &x, double &y) {
		double M[4];
		M[0] = cos(theta);
		M[1] = -sin(theta)*sign;
		M[2] = sin(theta)*sign;
		M[3] = cos(theta);
		matrix_2d_transform(M,x,y);
	}

	void rotate_90_CCK(double &x,double &y) {
		double xM = -y;
		double yM = x;
		x = xM;
		y = yM;
	}

	void rotate_180_CCK(double &x,double &y) {
		double xM = -x;
		double yM = -y;
		x = xM;
		y = yM;
	}

	void rotate_270_CCK(double &x,double &y) {
		double xM = y;
		double yM = -x;
		x = xM;
		y = yM;
	}

	void scaling_transform(double scale,double &x,double &y) {
		double xM = scale*x;
		double yM = scale*y;
		x = xM;
		y = yM;
	}

	void translation_transform(double translator[2],double &x,double &y) {
		double xM = translator[0] + x;
		double yM = translator[1] + y;
		x = xM;
		y = yM;
	}

	void translation_point_to_point_transform(double end_point[2], double start_point[2], double &x, double &y) {
		double translator[2];
		translator[0] = (end_point[0] - start_point[0]);
		translator[1] = (end_point[1] - start_point[1]);
		translation_transform(translator,x,y);
	}

	void y_axis_translation_transform(double translator,double &x,double &y) {
		/// don't change x
		y = translator + y;
	}

	Matrix2D *compile_transforms(list<Matrix2D *> *composition) {
			//----------->>
		if ( composition == NULL ) return(NULL);
		if ( composition->empty() ) return(NULL);
			///----------------------->>>
		Matrix2D *result = new Matrix2D();

		composition->reverse();

		list<Matrix2D *>::iterator lmit;
		lmit = composition->begin();

		Matrix2D *prev = *lmit++;
		result->copy(prev);

		Matrix2D holder = *prev;
		for ( ; lmit != composition->end(); lmit-- ) {
				///------------------->>
			Matrix2D *next = *lmit;
			Matrix2D operat = *next;
			
			holder = operat*holder;
		}

		*result = holder;

		return(result);
	}


};








#endif


