#ifndef __PDFEDIT_H__
#define __PDFEDIT_H__
#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qsplitter.h>
#include <qmainwindow.h>
#include <qsinterpreter.h>

/** pdfEditWidget - class handling main application window */
class pdfEditWidget : public QMainWindow {
 Q_OBJECT
private:
 QSInterpreter *qs;
public:
 pdfEditWidget(QWidget *parent = 0, const char *name = 0);
 void saveWindowState();
 void restoreWindowState();
protected:
 void closeEvent(QCloseEvent *e);
 void runScript(QString script);
public slots: //These will be exported to scripting
 void createNewWindow(); 
 void exitApp();
 void closeWindow();
protected slots:
 void menuActivated(int id);
};

void createNewEditorWindow();

#endif
