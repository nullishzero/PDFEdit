/** \file globalfunctions.h defines hlper functions */
#ifndef __GLOBAL_F__
#define __GLOBAL_F__

#include <cmath>
#include "kernel/displayparams.h"
//TODO asi by t mohlo byt vacsie, na viac desatinnych miest
#define PI 3.14

#define min(a,b) (a) < (b)? (a):(b)
#define max(a,b) (a) > (b)? (a):(b) 

/** \brief converts degres to radians */
float toRadians(int angle);

/** \brief converts degrees to radians */
int toDegree(float angle);

/** rotates the position*/
/** rotates the positin around origin (0,0) ny angle */
void rotate(int angle, double& x, double& y);

template<typename T>
void rotatePosition( T xin, T yin, T& xout, T &yout, int angle )
{
	float s = sqrt(static_cast<double>(xin*xin+yin*yin));
	xout = s*cos(toRadians(angle));
	yout = s*sin(toRadians(angle));
};
void rotatePdf( pdfobjects::DisplayParams displayparams, double& x,double& y, bool toUpPosition);

#define ANNOTS(XX) \
	XX("Text", Text) \
	XX("Link", Link) \
	XX("Highlight", Highlight) \
	XX("", Supported) \

#define AENUMS(a,b) A##b,
#define CREATE_ARRAY(a,b)	a,

/** \brief enum for supported annotations */
/** annotation not define here will not be handled */

enum SupportedAnnotation
{
	ANNOTS(AENUMS)
};

#endif // __GLOBAL_F__
