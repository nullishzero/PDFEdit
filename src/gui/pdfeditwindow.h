/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __PDFEDITWINDOW_H__
#define __PDFEDITWINDOW_H__

#include <kernel/cobject.h>
#include <kernel/cpdf.h>
#include <kernel/cannotation.h>
#include <kernel/pdfoperators.h>
#include <kernel/pdfwriter.h>
#include <qmainwindow.h>
#include <qprogressbar.h>

class QFont;
class QListViewItem;
class QSplitter;
class QString;

namespace gui {

class BaseGUI;
class ColorTool;
class CommandWindow;
class EditTool;
class Menu;
class MultiTreeWindow;
class NumberTool;
class PageSpace;
class ProgressBar;
class PropertyEditor;
class QSPage;
class SelectTool;
class StatusBar;
class TreeItem;
class TreeItemAbstract;

using namespace pdfobjects;

/**
 PdfEditWindow - class handling main application window
 \brief Main window of application
 */
class PdfEditWindow : public QMainWindow {
 Q_OBJECT
public:
 static PdfEditWindow* create(const QString &fName=QString::null);
 PdfEditWindow(const QString &fName=QString::null,QWidget *parent=0,const char *name=0);
 ~PdfEditWindow();
 void addColorTool(ColorTool *tool);
 void addEditTool(EditTool *tool);
 void addNumberTool(NumberTool *tool);
 void addSelectTool(SelectTool *tool);
 bool modified();
 QString filename();
 void saveVisibility(QWidget *w,const QString &name);
 void loadVisibility(QWidget *w,const QString &name);
 void saveWindowState();
 void restoreWindowState();
 bool save(bool newRevision=false);
 bool saveCopy(const QString &name);
 bool closeFile(bool askSave,bool onlyAsk=false);
 bool openFile(const QString &name, bool askPassword=true);
 void exitApp();
 void closeWindow();
 int pageNumber();
 void bigPixmap();
 PageSpace* getPageSpace() const;
 void run(const QString &scriptName);
 void runFile(const QString &scriptName);
 void eval(const QString &code);
 QStringList allUnits();
 QStringList allUnitIds();
 boost::shared_ptr<pdfobjects::utils::ProgressObserver> getProgressObserver();
 QProgressBar * getProgressBar();
public slots:
 void pageDeleteSelection();
 void setSelection(std::vector<boost::shared_ptr<PdfOperator> > vec);
 void setSelection(std::vector<boost::shared_ptr<CAnnotation> > vec);
 void changeRevision(int revision);
 void receiveHelpText(const QString &message);
signals:
 /**
  Signal emitted whenever any IProperty object is deleted
  @param it Property that was deleted
 */
 void itemDeleted(boost::shared_ptr<IProperty> it);
 /**
  Signal emitted when closing a file or editor window. All helper editor widgets opened from
  this window that are subclasses of SelfDestructiveWidget will close themselves
 */
 void selfDestruct();
 /**
  Signal emitted when active document is changed
  Reference to newly opened document is sent.
  If document is closed without opening a new file, NULL is sent instead.
  @param newDocument Pointer to new document
 */
 void documentChanged(boost::shared_ptr<pdfobjects::CPdf> newDocument);
 /**
  Signal emitted when user changes the current revision in any way (script, revision tool ...)
  @param revision number of selected revision
 */
 void revisionChanged(int revision);
protected:
 virtual void closeEvent(QCloseEvent *e);
protected slots:
 void pageChange(const QSPage &pg, int numberOfPage);
 void treeClicked(int button,QListViewItem *item);
 void menuActivated(int id);
 void setObject();
private slots:
 void unsetObjectIf(TreeItem *ipItem);
 void unsetObjectIf(TreeItemAbstract *theItem);
 void pagePopup(const QPoint &globalPos);
 void settingUpdate(QString key);
 void runScript(QString script);
private:
 void setTitle(int revision=0);
 void addObjectDialogI(boost::shared_ptr<IProperty> ip);
 void setFileName(const QString &name);
 void destroyFile();
 void emptyFile();
 /** Progress observer which holds progress bar.
  * Value is initialized in constructor. Wrapped qt progress bar
  * instance is allocated in constructor but deallocating by
  * qt.
  */
 boost::shared_ptr<pdfobjects::utils::ProgressObserver> progressObserver;
 /** Progressbar widget */
 QProgressBar * progressBar;
 /** Commandline window */
 CommandWindow *cmdLine;
 /** Object treeview */
 MultiTreeWindow *tree;
 /** Property editor */
 PropertyEditor *prop;
 /** Edited PDF document */
 boost::shared_ptr<CPdf> document;
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
 BaseGUI *base;
 /** Status bar on bottmo of application */
 StatusBar * status;
 /** Base should be allowed to access everything in PdfEditWindow */
 friend class BaseGUI;
 //TODO: maybe remove this later
};

} // namespace gui

#endif
