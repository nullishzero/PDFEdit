#ifndef __PDFEDITWINDOW_H__
#define __PDFEDITWINDOW_H__

#include <cobject.h>
#include <cpdf.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qsplitter.h>
#include <qmainwindow.h>
#include <qsinterpreter.h>
#include "commandwindow.h"

using namespace pdfobjects;

/** PdfEditWindow - class handling main application window */
class PdfEditWindow : public QMainWindow {
 Q_OBJECT
public:
 PdfEditWindow(QWidget *parent=0,const char *name=0);
 ~PdfEditWindow();
 void saveWindowState();
 void restoreWindowState();
public slots: //These will be exported to scripting
 void about();
 void createNewWindow(); 
 void exitApp();
 void closeWindow();
 void print(QString str);
protected:
 void closeEvent(QCloseEvent *e);
protected slots:
 void runScript(QString script);
 void menuActivated(int id);
private:
 /** Commandline window */
 CommandWindow *cmdLine;
 /** QSA Scripting Project */
 QSProject *qp;
 /** QSA Interpreter - taken from project */
 QSInterpreter *qs;
 /** Edited PDF document */
 IProperty *document;
};

void createNewEditorWindow();

#endif
