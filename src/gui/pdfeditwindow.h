#ifndef __PDFEDITWINDOW_H__
#define __PDFEDITWINDOW_H__
#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qsplitter.h>
#include <qmainwindow.h>
#include <qsinterpreter.h>
#include "commandwindow.h"

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
 CommandWindow *cmdLine;
 QSInterpreter *qs;
 QSProject *qp;
};

void createNewEditorWindow();

#endif
