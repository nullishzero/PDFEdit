#include "globalfunctions.h"
#include "kernel/displayparams.h"

float toRadians(int angle)
{
	return PI* angle/180;
}
int toDegree(float angle)
{
	return 180*angle/PI;
}
void rotate(int angle, double& x, double& y)
{
	float an = toRadians(angle);
	double xx = x*cos(an) + y*sin(an);  //po smere hodinych ruciciek
	double yy = -x*sin(an) + y*cos(an);
	x = xx;
	y = yy;
}
using namespace pdfobjects;
void rotatePdf( DisplayParams displayparams, double& x,double& y, bool toUpPosition) //dostanem pozcie ake su v pdfku
{//proti smeru hodinovych ruciciek!
	if (!toUpPosition)
		displayparams.rotate*=-1;
	int angle = (displayparams.rotate + 360)%360;
	double par[] = { displayparams.vDpi/72.0f, displayparams.hDpi/72.0f};
	double resx = displayparams.pageRect.xright *par[0];
	double resy = displayparams.pageRect.yright *par[1];

	::rotate(angle,resx,resy);
	if (!toUpPosition)
		::rotate(angle,x,y);
	switch (angle/90)
	{
	case 1: //staci vymena x,y osi
		x = fabs(resx) - fabs(x);
		break;
	case 2:
		x = fabs(resx) - fabs(x);
		y = fabs(resy) - fabs(y);
		break;
	case 3:
		y = fabs(resy) - fabs(y);
		break;
	default:
		break;
	}
	if (toUpPosition)
		::rotate(angle,x,y);
	x = fabs(x);
	y = fabs(y);
}