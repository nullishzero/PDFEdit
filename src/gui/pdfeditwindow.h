#ifndef __PDFEDITWINDOW_H__
#define __PDFEDITWINDOW_H__

#include <cobject.h>
#include <cpdf.h>
#include <qmainwindow.h>

class QListViewItem;
class QFont;
class QPushButton;
class QSInterpreter;
class QSProject;
class QSplitter;
class QString;

namespace gui {

class PageSpace;
class CommandWindow;
class QSImporter;
class QSCObject;
class QSPdf;
class TreeWindow;
class TreeItemAbstract;
class PropertyEditor;
class Menu;
class QSMenu;
class QSDict;
class QSArray;
class QSIProperty;

using namespace pdfobjects;

/** PdfEditWindow - class handling main application window */
class PdfEditWindow : public QMainWindow {
 Q_OBJECT
public:
 PdfEditWindow(const QString &fName=QString::null,QWidget *parent=0,const char *name=0);
 ~PdfEditWindow();
 int question_ync(const QString &msg);
 void call(const QString &name);
public slots: //These will be exported to scripting
 void addObjectDialog(QSDict &container);
 void addObjectDialog(QSArray &container);
 void addObjectDialog(QSArray *container);
 void addObjectDialog(QSDict *container);
 void addObjectDialog(QSIProperty *container=NULL);
 QSMenu* popupMenu(const QString &menuName=QString::null);
 void run(QString scriptName);
 bool save();
 bool saveAs(const QString &name);
 QString fileOpenDialog();
 QString fileSaveDialog(const QString &oldName=QString::null);
 bool modified();
 bool exists(const QString &chkFileName);
 void options();
 void message(const QString &msg);
 bool question(const QString &msg);
 bool closeFile(bool askSave,bool onlyAsk=false);
 void openFileNew(const QString &name);
 void openFile(const QString &name);
 void saveWindowState();
 void restoreWindowState();
 void about();
 void help(const QString &topic=QString::null);
 void createNewWindow(); 
 void exitApp();
 void closeWindow();
 void print(const QString &str);
 void objects();
 void functions();
 void variables();
 QString filename();
 QString version();
protected:
 void closeEvent(QCloseEvent *e);
protected slots:
 void treeClicked(int button,QListViewItem *item);
 void setObject(const QString &name,boost::shared_ptr<IProperty> obj);
 void runScript(QString script);
 void menuActivated(int id);
 void setObject();
private slots:
 void settingUpdate(QString key);
private:
 void addObjectDialogI(boost::shared_ptr<IProperty> ip);
 void setFileName(const QString &name);
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
 /** Currently selected page (for scripting) */
 boost::shared_ptr<CPage> selectedPage;
 /** Currently selected IProperty (in editor) */
 boost::shared_ptr<IProperty> selectedProperty;
 /** Currently selected tree item (for scripting) */
 TreeItemAbstract *selectedTreeItem;
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
 /** Name of file loaded in editor without path */
 QString baseName;
 /** Menus and toolbars */
 Menu *menuSystem;
 /** Page space - page view Widget*/
 PageSpace *pagespc;
};

void createNewEditorWindow(const QString &fName=QString::null);

} // namespace gui

#endif
