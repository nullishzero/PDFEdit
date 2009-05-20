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
#ifndef __BASEGUI_H__
#define __BASEGUI_H__

#include "base.h"
#include "annotdialog.h"
#include <qmap.h>
#include <qobject.h>
#include <qstring.h>
#include <qvariant.h>

class QProgressBar;

namespace gui {

class ColorTool;
class ConsoleWriterGui;
class EditTool;
class NumberTool;
class PdfEditWindow;
class SelectTool;
class QSMenu;
class TreeItemAbstract;

//TODO: update documentation generator

/**
 Class that host scripts and contain static script functions<br>
 This class is also responsible for garbage collection of scripting
 objects and interaction of editor and scripts
 \brief Script hosting base class
*/
class BaseGUI : public Base {
 Q_OBJECT
public:
 BaseGUI(PdfEditWindow *parent);
 virtual ~BaseGUI();
 void runInitScript();
 void addColorTool(ColorTool *tool);
 void addEditTool(EditTool *tool);
 void addSelectTool(SelectTool *tool);
 void addNumberTool(NumberTool *tool);
 void treeItemDeleted(TreeItemAbstract* theItem);
public slots: //This will be all exported to scripting
 virtual boost::shared_ptr<pdfobjects::CPdf> getBasePdfInstance(const QString &filename, const QString &openMode=QString::null, bool askPassword=true);
 /*-
  Allow application to process its events, so the gui can redraw itself.
  Useful, it it is called periodically while doing some lenthy operation
 */
 void processEvents();
 /*-
  Shows dialog that will ask user for password.
  Parameter title specifies title of the dialog. If not specified, some default like "Enter password" is used.
  Return entered password or null if dialog is cancelled
 */
 QString askPassword(const QString &title=QString::null);
 /*-
  Set text of all menu and toolbar items with given name to specified string.
  The string will be transtated according to the translation file, so it is
  suggested to supply english text and add corresponding localized translation
  into the localization file.
 */
 void setItemText(const QString &name,const QString &newText);
 /*-
  Get text of menu or toolbar item with given name.
  Returns untranslated (english) string, even when the application is run
  using another language translation.
  When setting back the same string, it will be re-translated according to
  translation file, if different GUI language is selected
 */
 QString getItemText(const QString &name);
 /*- Invokes "About" dialog, showing information about this program and its authors */
 void about();
 /*-
  Creates dialog for annotations creation.
  Parameter page is page in which the annotation will be added
  (x1,y1) are coordinated of lower left point of annotation rectangle.
  W is the rectangle widht and h is its height
  Returns after annotation have been inserted into page, or dialog have been cancelled
  */
 void addAnnotation(QSPage * page,double x1,double y1,double w,double h);
 void addAnnotation(QObject * page,double x1,double y1,double w,double h);
 /*-
  Invokes dialog for adding additional objects to specified container (which must be <link linkend="type_Dict">Dictionary</link> or <link linkend="type_Array">Array</link>).
  After invoking dialog, this function returns immediately and the dialog is left for the user to use.
  If given object is not specified, curently selected object in property editor is used.
  If the object is not <link linkend="type_Dict">Dictionary</link> or <link linkend="type_Array">Array</link>, no dialog is shown.
 */
 void addObjectDialog(QSIProperty *container=NULL);
 void addObjectDialog(QObject *container);
 /*-
  Check (second parameter is true) or uncheck (false) item in toolbar and/or menu, given its name
  If you prefix name with slash ("/"), you will affect "class" of items
  - every item that belong to the specified class
  <note>
   Toolbuttons will automatically convert to Togglable toolbuttons this way
   and will start togling itself automatically on each succesive click
  </note>
 */
 void checkItem(const QString &name,bool check);
 /*- Clear the console output window */
 void clearConsole();
 /*- Closes all windows (and thus ends application) */
 void closeAll();
 /*-
  Closes file opened in this editor window.
  if askSave is true, user is asked to save the file if it is modified.
  if onlyAsk is true, file is not actually closed, only user is asked if he want to save work (if not specified, defaults to false).
  returns true in case of success, or false if user decide not to close current document and keep it open.
 */
 bool closeFile(bool askSave,bool onlyAsk=false);
 /*-
  Load one menu or toolbar item and insert it into parent menu or toolbar.
  If the name is "" or "-", separator is inserted on place of the item and rest of parameters are ignored.
  If inserting item into toolbar and the name will match one of the special items, the special item
  will be inserted in toolbar and rest of the parameters are ignored.
  Items that are loaded from settings have theyir name equal to the key in setting they are stored under.
  <informaltable frame="none">
   <tgroup cols="2"><tbody>
   <row><entry>parentName</entry><entry>Name of parent menu/toolbar. If NULL/empty, main menubar is the parent</entry></row>
   <row><entry>name	</entry><entry> Name of the item. Must be unique.</entry></row>
   <row><entry>caption	</entry><entry> Caption of item</entry></row>
   <row><entry>action	</entry><entry> Script to execute when the item is selected</entry></row>
   <row><entry>accel	</entry><entry> Keyboard accelerator</entry></row>
   <row><entry>icon	</entry><entry> Name of icon to use</entry></row>
   <row><entry>classes	</entry><entry> List of item classes</entry></row>
   </tbody></tgroup>
  </informaltable>
 */
 void createMenuItem(const QString &parentName,const QString &name,const QString &caption,const QString &action,const QString &accel=QString::null,const QString &icon=QString::null,const QStringList &classes=QStringList());
 /*- Creates new editor window with empty document in it. */
 void createNewWindow();
 /*-
  Enable (second parameter is true) or disable (false) item in toolbar and/or menu, given its name
  If you prefix name with slash ("/"), you will enable or disable "class" of items
  - every item that belong to the specified class
 */
 void enableItem(const QString &name,bool enable);
 /*-
  Return name of file loaded in editor window. If the file does not exist on disk
 (not loaded any file, or file was never saved, having no name), empty string is returned.
 */
 QString filename();
 /*- Invokes "open file" dialog and return selected filename, or NULL if dialog was cancelled */
 QString fileOpenDialog();
 /*-
  Invokes "save file" dialog and return selected filename, or NULL if dialog was cancelled.
  If file selected in dialog already exists, user is asked to confirm overwriting before returing its name.
 */
 QString fileSaveDialog(const QString &oldName=QString::null);
 /*-
  Show "save file" dialog and return file selected, or NULL if dialog was cancelled.
  If file selected in dialog already exists, user is asked to confirm overwriting before returing its name.
  Name must be different than origName.
 */
 QString fileSaveDialogDiffer(const QString &origName);
 /*-
  Invokes "save file" dialog for XML file and return selected filename, or NULL if dialog was cancelled.
  If file selected in dialog already exists, user is asked to confirm overwriting before returing its name.
 */
 QString fileSaveDialogXml(const QString &oldName=QString::null);
 /*-
  Get color from color picker with given name.
  Returns false if the color picker does not exist.
 */
 QVariant getColor(const QString &colorName);/*Variant=Color*/
 /*-
  Get text from editbox or text selection box in toolbar with given name.
  Returns false if the box does not exist.
 */
 QString getEditText(const QString &textName);
 /*-
  Get number from number editbox in toolbar with given name.
  Returns 0 if the number editbox does not exist.
 */
 double getNumber(const QString &name);

 /*- Invokes program help. Optional parameter is topic - if invalid or not defined, help title page will be invoked */
 void help(const QString &topic=QString::null);
 /*-
  Check if part of the window is visible (returns true) or hidden (returns false)
  widgetName specifies which part:
  <informaltable frame="none">
   <tgroup cols="2"><tbody>
    <row><entry>commandline	</entry><entry>Command line</entry></row>
    <row><entry>rightside	</entry><entry>Right side tools (Tree Window and property editor)</entry></row>
    <row><entry>propertyeditor	</entry><entry>Property editor</entry></row>
    <row><entry>tree		</entry><entry>Tree window</entry></row>
    <row><entry>statusbar	</entry><entry>Statusbar</entry></row>
   </tbody></tgroup>
  </informaltable>
 */
 bool isVisible(const QString &widgetName);
 /*- Show simple messagebox with specified message and wait until user dismiss it */
 void message(const QString &msg);
 /*-
  Bring up "merge pages from another PDF in this document" dialog.
  Returns result of merge or NULL if dialog was cancelled

  Result is array of three elements:
  <itemizedlist>
   <listitem><para>First element is array with page numbers</para></listitem>
   <listitem><para>Second element is array with page positions</para></listitem>
   <listitem><para>Third is filename of the document to be merged in</para></listitem>
  </itemizedlist>
 */
 QVariant mergeDialog();
 /*-
  Bring up "select pages" dialog.
  Returns result of selection or NULL if dialog was cancelled.

  Result is array of three elements:
  <itemizedlist>
   <listitem><para>First element is array with page numbers</para></listitem>
   <listitem><para>Third is filename of the document to be merged in</para></listitem>
  </itemizedlist>
 */
 QVariant selectPagesDialog (const QString& filename) const;

 /*- Return true if the document was modified since it was opened or last saved, false otherwise. */
 bool modified();
 /*-
  Opens file with given name in this editor window.
  Opens without any questions, does not ask user to save changes to current file, etc ...
  If askPassword is true (default), user is asked for password to the
   document and loading fails if user does not enter correct password.
  If askPassword is false, script have to handle entering the correct
   password and possible closing of the document on failure by itself.
  Return true on success, false on failure to load file.
 */
 bool openFile(const QString &name, bool askPassword=true);
 /*- Opens file with given name in new editor window */
 void openFileNew(const QString &name);
 /*- Invokes options dialog. Does not wait for dialog to be closed by user and return immediately. */
 void options();
 /*- Return currently shown page */
 QSPage* page();
 /*- Return page number of currently shown page */
 int pageNumber();
 /*-
  Invoke dialog to select color and return the color that user have picked,
  or false if user cancelled the dialog.
 */
 QVariant pickColor();/*Variant=Color*/
 /*-
  Create and return a <link linkend="type_Menu">Menu</link> object. The menuName parameter specifies
  name of item or list from configuration, that will be used to initially fill the menu with items.
  If this parameter is not specified or invalid, the menu will be initially empty.
  See documentation for <link linkend="type_Menu">Menu</link> object type for more info about
  using popup menus.
 */
 QSMenu* popupMenu(const QString &menuName=QString::null);
 /*- Show Yes/No question and wait for answer. Return true if user selected "yes", or false if user selected "no" */
 bool question(const QString &msg);
 /*-
  Show Yes/No/Cancel question and wait for answer.
  Return 1 if user selected "yes", 0 if user selected "no", -1 if user cancelled dialog (selecting Cancel)
 */
 int question_ync(const QString &msg);
 /*-
  Restore state of current editor window (size and position of window and elements inside it)
  State is restored from state saved in editor's configuration file.
 */
 void restoreWindowState();
 /*-
  Save currently edited document.
  If current document have no name, user will be asked for name.
  If it is not possible to ask user or user presses cancel in file chooser,
  document is not saved.
  Return true if document was saved, false if it was not saved for any reason (file write error, user refused to give filename on new file ...)
 */
 bool save();
 /*-
  Save currently edited document (with currently active revision) under different name.
  Return true if document was saved, false if it failed to save for any reason
 */
 bool saveCopy(const QString &name);
 /*-
  Save currently edited document, while creating new revision in the process.
  Return true if document was saved, false if it was not saved for any reason.
 */
 bool saveRevision();
 /*-
  Save state of current editor window (size and position of window and elements inside it)
  State is saved to editor's configuration file.
 */
 void saveWindowState();
 /*-
  Set color of color picker with given name
 */
 void setColor(const QString &colorName,const QVariant &newColor);
 /*-
  Set text in toolbar editbox with given name or set selection in text selection box.
  If you specify string that is not one of the items in the selection box,
   the current item will remain selected.
 */
 void setEditText(const QString &textName,const QString &newText);
 /*-
  Set number in toolbar number editbox with given name
 */
 void setNumber(const QString &name,double number);
 /*-
  Set list of predefined values for number edit box or select text box with given name.
  The values in the list must be separated by commas.
  For number edit box, the user is still able to type in any value not in the list.
 */
 void setPredefs(const QString &name,const QString &predefs);
 /*-
  Set tooltip text for specified number edit box or select text box with given name.
  Tooltip text will be shown when hovering with mouse over the tool.
 */
 void setTooltip(const QString &name,const QString &tip);
 /*-
  Set list of predefined values for number edit box or select text box with given name.
  For number edit box, the user is still able to type in any value not in the list.
 */
 void setPredefs(const QString &name,const QStringList &predefs);
 /*-
  Change active revision in current PDF document
 */
 void setRevision(int revision);
 /*-
  Set part of the window to be either visible or invisible,
  widgetName specifies which part, see isVisible for list
  of possible names.
 */
 void setVisible(const QString &widgetName, bool visible);
 /*-
  Show (second parameter is true) or hide (false) item in toolbar, given its name
  If you prefix name with slash ("/"), you will affect "class" of items
  - every item that belong to the specified class
 */
 void showItem(const QString &name,bool show);
 /*- Return root item of currently selected tree */
 QSTreeItem* treeRoot();
 /*- Return root item of main tree */
 QSTreeItem* treeRootMain();
 /*-
  Outputs given warning string to command window, followed by newline,
  and show this string in a messagebox to alert user.
 */
 void warn(const QString &str);

 //Tree selection related functions

 /*-
  Return first selected tree item.
  Set internal selected item pointer to first selected item
  For getting other selected tree items (in case more than one is selected, repeat calling nextSelectedItem() until NULL is returned (no more items)
  <example id="getting_through_list_of_selected_tree_items">
   <title>Getting through list of selected tree items</title>
   <programlisting>
    treeItem=firstSelectedItem();<br/>
    while (treeItem) {<br/>
    &nbsp;doSomething(treeItem);<br/>
    &nbsp;treeItem=nextSelectedItem();<br/>
    }
   </programlisting>
  </example>
  You can optionally specify different tree as parameter (by default, selected items from currently shown tree are taken)
  Specify "main" for main tree, "select" for selected operators or use number to specify number for Nth tree (indexed from zero).
  Omitting the parameter or specifying "current" will use currently shown tree
  If specified tree does not exist or its name is invalid, NULL is returned
 */
 QSTreeItem* firstSelectedItem(const QString &name=QString::null);
 /*-
  Return next selected tree item.
  Move internal selected item pointer to next selected item (or invalidate it if no more selected items is found)
 */
 QSTreeItem* nextSelectedItem();
 /*-
  Return object held in first selected tree item.
  Set internal selected item pointer to first selected item
  For getting other selected tree items (in case more than one is selected, repeat calling nextSelected() until NULL is returned (no more items)
  <example id="getting_through_list_of_selected_items">
   <title>Getting through list of selected items</title>
   <programlisting>
    treeObject=firstSelected();<br/>
    while (treeObject) {<br/>
    &nbsp;doSomething(treeObject);<br/>
    &nbsp;treeObject=nextSelected();<br/>
    }
   </programlisting>
  </example>
  You can optionally specify different tree as parameter (by default, selected items from currently shown tree are taken)
  Specify "main" for main tree, "select" for selected operators or use number to specify number for Nth tree (indexed from zero).
  Omitting the parameter or specifying "current" will use currently shown tree
  If specified tree does not exist or its name is invalid, NULL is returned
 */
 QSCObject* firstSelected(const QString &name=QString::null);
 /*-
  Return object held in next selected tree item.
  Move internal selected item pointer to next selected item (or invalidate it if no more selected items is found)
 */
 QSCObject* nextSelected();
 /*-
  Returns progress bar which can be used to provide visualization of progress.
  Uses common progress bar from PdfEditWindow class. User should keep in mind,
  that also someone alse can use this progress bar in same time and he should
  set total steps before each setProgress method.
  */
 QProgressBar * progressBar();

 /** Function return X position in pdf page from position of viewed page on screen (unapply rotation, viewing dpi, ...)
  @param fromX	X position on viewed page.
  @param fromY	Y position on viewed page.

  @return Return X position in pdf page.

  @see convertPixmapPosToPdfPos_y
  @see convertPdfPosToPixmapPos_x
  @see convertPdfPosToPixmapPos_y
 */
double convertPixmapPosToPdfPos_x ( double fromX, double fromY );
 /** Function return Y position in pdf page from position of viewed page on screen (unapply rotation, viewing dpi, ...)
  @param fromX	X position on viewed page.
  @param fromY	Y position on viewed page.

  @return Return Y position in pdf page.

  @see convertPixmapPosToPdfPos_x
  @see convertPdfPosToPixmapPos_x
  @see convertPdfPosToPixmapPos_y
 */
double convertPixmapPosToPdfPos_y ( double fromX, double fromY );
 /** Function return X position on viewed page from position in pdf page (apply rotation, viewing dpi, ...)
  @param fromX	X position in pdf page.
  @param fromY	Y position in pdf page.

  @return Return X position in pdf page.

  @see convertPdfPosToPixmapPos_y
  @see convertPixmapPosToPdfPos_x
  @see convertPixmapPosToPdfPos_y
 */
double convertPdfPosToPixmapPos_x ( double fromX, double fromY );
 /** Function return Y position on viewed page from position in pdf page (apply rotation, viewing dpi, ...)
  @param fromX	X position in pdf page.
  @param fromY	Y position in pdf page.

  @return Return Y position in pdf page.

  @see convertPdfPosToPixmapPos_x
  @see convertPixmapPosToPdfPos_x
  @see convertPixmapPosToPdfPos_y
 */
double convertPdfPosToPixmapPos_y ( double fromX, double fromY );
private slots:
 void toolChangeValue(const QString &toolName);
#ifndef DRAGDROP
private://This is workaround because of bug in MOC - it tries to include methods that are ifdef'ed out
#else
 // These are internal slots, should not available to scripting,
 // but it is not possible to do that. But at least they are uncallable
 void _dragDrop(TreeItemAbstract *source,TreeItemAbstract *target);
 void _dragDropOther(TreeItemAbstract *source,TreeItemAbstract *target);
#endif
private:
 QWidget* getWidgetByName(const QString &widgetName);
protected:
 //TODO: separate to BaseGuiCore
 virtual void addScriptingObjects();
 virtual void removeScriptingObjects();
 virtual void preRun(const QString &script,bool callback=false);
 virtual void postRun();
private:
 /** Console writer class writing to command window */
 ConsoleWriterGui* consoleWriter;
 /** Editor window in which this class exist */
 PdfEditWindow* w;
 /** Map with color picker tools */
 QMap<QString,ColorTool*> colorPickers;
 /** Map with edit tools */
 QMap<QString,EditTool*> editTools;
 /** Map with number tools */
 QMap<QString,NumberTool*> numberTools;
 /** Map with select tools */
 QMap<QString,SelectTool*> selectTools;
};

} // namespace gui

#endif
