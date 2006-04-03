#ifndef __PDFEDITWINDOW_H__
#define __PDFEDITWINDOW_H__

#include <cobject.h>
#include <cpdf.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qfont.h>
#include <qsplitter.h>
#include <qstring.h>
#include <qmainwindow.h>
#include <qsinterpreter.h>
#include "commandwindow.h"
#include "propertyeditor.h"
#include "treewindow.h"
#include "qspdf.h"
#include "qsimporter.h"

using namespace pdfobjects;

/** PdfEditWindow - class handling main application window */
class PdfEditWindow : public QMainWindow {
 Q_OBJECT
public:
 PdfEditWindow(const QString &fName=QString::null,QWidget *parent=0,const char *name=0);
 ~PdfEditWindow();
public slots: //These will be exported to scripting
 void closeFile();
 void openFileNew(const QString &name);
 void openFile(const QString &name);
 void saveWindowState();
 void restoreWindowState();
 void about();
 void createNewWindow(); 
 void exitApp();
 void closeWindow();
 void print(QString str);
 void objects();
 void functions();
 void variables();
 QString filename();
 QString version();
protected:
 void closeEvent(QCloseEvent *e);
protected slots:
 void runScript(QString script);
 void menuActivated(int id);
private:
 void destroyFile();
 void emptyFile();
 void addDocumentObjects();
 void removeDocumentObjects();
 /** Commandline window */
 CommandWindow *cmdLine;
 /** Object treeview */
 TreeWindow *tree;
 /** Property editor */
 PropertyEditor *prop;
 /** QSA Scripting Project */
 QSProject *qp;
 /** QSA Interpreter - taken from project */
 QSInterpreter *qs;
 /** Edited PDF document */
 CPdf *document;
 /** Currently selected page (for scripting)*/
 boost::shared_ptr<CPage> page;
 /** Currently selected object (for scripting)*/
 IProperty *item;
 //TODO: highlevel objects (Page, Anotation...) can be selected too
 /** QObject wrapper around CPdf (document) that is exposed to scripting. Lifetime of this class is the same as lifetime of document */
 QSPdf *qpdf;
 /** QSObject Importer */
 QSImporter *import;
 /** Horizontal splitter between (Preview + Commandline) and (Treeview + Property editor) */
 QSplitter *spl;
 /** Vertical splitter between command line and preview window */
 QSplitter *splCmd;
 /** Vertical splitter between treeview and property editor */
 QSplitter *splProp;
 /** Name of file loaded in editor */
 QString fileName;
};

void createNewEditorWindow(const QString &fName=QString::null);

#endif
