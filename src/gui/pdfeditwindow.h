#ifndef __PDFEDITWINDOW_H__
#define __PDFEDITWINDOW_H__

#include <cobject.h>
#include <cpdf.h>
#include <pdfoperators.h>
#include <qmainwindow.h>

class QListViewItem;
class QFont;
class QPushButton;
class QSInterpreter;
class QSProject;
class QSplitter;
class QString;

namespace gui {

class Base;
class PageSpace;
class CommandWindow;
class QSImporter;
class QSCObject;
class QSPdf;
class QSPage;
class MultiTreeWindow;
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
 static void create(const QString &fName=QString::null);
 PdfEditWindow(const QString &fName=QString::null,QWidget *parent=0,const char *name=0);
 ~PdfEditWindow();
 bool modified();
 QString filename();
 void saveWindowState();
 void restoreWindowState();
 bool save(bool newRevision=false);
 bool saveCopy(const QString &name);
 bool closeFile(bool askSave,bool onlyAsk=false);
 bool openFile(const QString &name);
 void exitApp();
 void closeWindow();
 int pageNumber();
 void bigPixmap();
 PageSpace* getPageSpace() const;
public slots:
 void changeRevision(int revision);
signals:
 /**
  Signal emitted when closing a file or editor window. All helper editor widgets opened from
  this window that are subclasses of SelfDestructiveWidget will close themselves
 */
 void selfDestruct();
 /** 
  Signal emitted when active document is changed 
  Reference to newly opened document is sent.
  If document is closed without opening a new file, NULL is sent instead.
 */
 void documentChanged(CPdf *newDocument);
 /**
  Signal emitted when user changes the current revision in any way (script, revision tool ...)
  @param revision number of selected revision
 */
 void revisionChanged(int revision);
protected:
 void closeEvent(QCloseEvent *e);
protected slots:
 void pageChange(const QSPage &pg, int numberOfPage);
 void treeClicked(int button,QListViewItem *item);
 void menuActivated(int id);
 void setObject();
private slots:
 void pagePopup(const QPoint &globalPos);
 void settingUpdate(QString key);
 void runScript(QString script);
private:
 void setTitle(int revision=0);
 void addObjectDialogI(boost::shared_ptr<IProperty> ip);
 void setFileName(const QString &name);
 void destroyFile();
 void emptyFile();
 void addDocumentObjects();
 void removeDocumentObjects();
 /** Commandline window */
 CommandWindow *cmdLine;
 /** Object treeview */
 MultiTreeWindow *tree;
 /** Property editor */
 PropertyEditor *prop;
 /** Edited PDF document */
 CPdf *document;
 /** Currently selected page (for scripting) */
 boost::shared_ptr<CPage> selectedPage;
 /** Currently selected page number */
 int selectedPageNumber;
 /** Currently selected IProperty (in editor) */
 boost::shared_ptr<IProperty> selectedProperty;
 /** Currently selected PdfOperator (in editor) */
 boost::shared_ptr<PdfOperator> selectedOperator;
 /** Currently selected tree item (for scripting) */
 TreeItemAbstract *selectedTreeItem;
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
 /** Base used to host scripts */
 Base *base;
 /** Last error message from exception in load/save, etc ... */
 QString lastErrorMessage;
 /** Base should be access everything in PdfEditWidget */
 friend class Base;
};

} // namespace gui

#endif
