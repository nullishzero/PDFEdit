#ifndef __PDFEDIT_H__
#define __PDFEDIT_H__
#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qsplitter.h>
#include <qmainwindow.h>
#include <qsinterpreter.h>

/** PdfEditWindow - class handling main application window */
class PdfEditWindow : public QMainWindow {
 Q_OBJECT
public:
 PdfEditWindow(QWidget *parent = 0, const char *name = 0);
 ~PdfEditWindow();
 void saveWindowState();
 void restoreWindowState();
public slots: //These will be exported to scripting
 void createNewWindow(); 
 void exitApp();
 void closeWindow();
protected:
 void closeEvent(QCloseEvent *e);
 void runScript(QString script);
protected slots:
 void menuActivated(int id);
private:
 QSInterpreter *qs;
};

void createNewEditorWindow();

#endif
