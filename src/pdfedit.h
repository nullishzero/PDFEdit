#ifndef __PDFEDIT_H__
#define __PDFEDIT_H__
#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qsplitter.h>
#include <qmainwindow.h>

/** pdfEditWidget - class handling main application window */
class pdfEditWidget : public QMainWindow {
 Q_OBJECT
public:
 pdfEditWidget(QWidget *parent = 0, const char *name = 0);
public slots:
 void exiting();
 void createNewWindow(); 
 void menuActivated(int id);
};

#endif
