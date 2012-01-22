/*************************
* 
* this source code and its parts are distribuded under GPLv2 licence
*   
* */
#include "pdfgui.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	pdfGui w;
	w.show();
	bool ret = a.exec();
	return ret;
}
