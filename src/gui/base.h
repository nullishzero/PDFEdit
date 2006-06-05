#ifndef __BASE_H__
#define __BASE_H__

#include <qobject.h>
#include <qstring.h>
#include <qptrdict.h>
#include <qvariant.h>

class QSProject;
class QSInterpreter;

namespace gui {

class BaseData;
class PdfEditWindow;
class QSArray;
class QSCObject;
class QSTreeItem;
class QSDict;
class QSIProperty;
class QSIPropertyArray;
class QSImporter;
class QSMenu;
class QSPage;
class QSPdf;
class QSPdfOperator;
class QSPdfOperatorStack;
class TreeItemAbstract;

/**
 Class that host scripts and contain static script functions<br>
 This class is also responsible for garbage collection of scripting
 objects and interaction of editor and scripts
*/
class Base : public QObject {
 Q_OBJECT
public:
 Base(PdfEditWindow *parent);
 QSPdf* getQSPdf() const;
 void stopScript();
 void importDocument();
 void destroyDocument();
 void call(const QString &name);
 void runInitScript();
 void runScript(QString script);
 void addGC(QSCObject *o);
 void removeGC(QSCObject *o);
 void cleanup();
 void treeNeedReload();
 QSInterpreter* interpreter();
 ~Base();
public slots: //This will be all exported to scripting
 /*- Invokes "About" dialog, showing information about this program and its authors */
 void about();
 /*- Return revision number of active revision in current PDF document */
 int activeRevision();
 /*-
  Invokes dialog for adding additional objects to specified container (which must be <link linkend="type_Dict">Dictionary</link> or <link linkend="type_Array">Array</link>).
  After invoking dialog, this function returns immediately and the dialog is left for the user to use.
  If given object is not specified, curently selected object in property editor is used.
  If the object is not <link linkend="type_Dict">Dictionary</link> or <link linkend="type_Array">Array</link>, no dialog is shown.
 */
 void addObjectDialog(QSIProperty *container=NULL);
 void addObjectDialog(QObject *container);
 /*-
  Closes file opened in this editor window.
  if askSave is true, user is asked to save the file if it is modified.
  if onlyAsk is true, file is not actually closed, only user is asked if he want to save work (if not specified, defaults to false).
  returns true in case of success, or false if user decide not to close current document and keep it open.
 */
 bool closeFile(bool askSave,bool onlyAsk=false);
 /*-
  Closes current editor window.
  If the file is not saved, user is asked to save changes to current file.
 */
 void closeWindow();
 /*- Creates new editor window with empty document in it. */
 void createNewWindow();
 /*- Create and return new IProperty of type Array - empty array */
 QSIProperty* createArray();
 /*- Create and return new IProperty of type Bool */
 QSIProperty* createBool(bool value);
 /*- Create new composite PDF Operator with specified starting and ending operator text*/
 QSPdfOperator* createCompositeOperator(const QString &beginText,const QString &endText);
 /*- Create and return new IProperty of type Dict - empty dictionary */
 QSIProperty* createDict();
 /*- Create new empty (NULL) PDF Operator*/
 QSPdfOperator* createEmptyOperator();
 /*- Create and return new IProperty of type Int */
 QSIProperty* createInt(int value);
 /*-
  Create new array of IProperty items.
  This array can be used for example as operator parameters
 */
 QSIPropertyArray* createIPropertyArray();
 /*- Create and return new IProperty of type String */
 QSIProperty* createName(const QString &value);
 /*- Create new PDF Operator with specified parameters and operator text*/
 QSPdfOperator* createOperator(QSIPropertyArray* parameters,const QString &text);
 QSPdfOperator* createOperator(QObject* parameters,const QString &text);
 /*- Create new empty PDF Operator stack */
 QSPdfOperatorStack* createPdfOperatorStack();
 /*- Create and return new IProperty of type Real */
 QSIProperty* createReal(double value);
 /*-
  Create and return new IProperty of type Ref.
  Does not check fo validity of the values
  (if the reference target exists, etc ...)
 */
 QSIProperty* createRef(int valueNum,int valueGen);
 /*- Create and return new IProperty of type String */
 QSIProperty* createString(const QString &value);
 /*-
  Enable (second parameter is true) or disable (false) item in toolbar and/or menu, given its name
  If you prefix name with slash ("/"), you will enable or disable "class" of items
  - every item that belong to the specified class
 */
 void enableItem(const QString &name,bool enableItem);
 /*-
  Return last error message (localized) from some operations
  (openFile, save, saveRevision, saveCopy)
 */
 QString error();
 /*- Return true if given file exists on disk, false otherwise */
 bool exists(const QString &chkFileName);
 /*- Terminates application */
 void exitApp();
 /*-
  Return name of file loaded in editor window. If the file does not exist on disk
 (not loaded any file, or file was never saved, having no name), empty string is returned.
 */
 QString filename();
 /*- Invokes program help. Optional parameter is topic - if invalid or not defined, help title page will be invoked */
 void help(const QString &topic=QString::null);
 /*- Invokes "open file" dialog and return selected filename, or NULL if dialog was cancelled */
 QString fileOpenDialog();
 /*-
  Invokes "save file" dialog and return selected filename, or NULL if dialog was cancelled.
  If file selected in dialog already exists, user is asked to confirm overwriting before returing its name.
 */
 QString fileSaveDialog(const QString &oldName=QString::null);
 /*-
  Debugging function usable by script developers.
  Print all functions that are present in current script interpreter to command window
 */
 void functions();
 /*-
  Check if part of the window is visible (returns true) or hidden (returns false)
  widgetName specifies which part:
  <informaltable frame="none">
   <tgroup cols="2"><tbody>
    <row><entry>commandline	</entry><entry>Command line</entry></row>
    <row><entry>rightside	</entry><entry>Right side tools (Tree Window and property editor)</entry></row>
    <row><entry>propertyeditor	</entry><entry>Property editor</entry></row>
    <row><entry>tree		</entry><entry>Tree window</entry></row>
   </tbody></tgroup>
  </informaltable>
 */
 bool isVisible(const QString &widgetName);
 /*-
  Load a PDF file without replacing currently opened file in GUI.
  Script can manipulate the PDF file as necessary, but should close it with unloadPdf() method
  after it does not need to use it anymore.
  If advancedMode is set to true, document is opened in "advanced mode"
  (more advanced, but also more dangerous changes to it are possible)
 */
 QSPdf* loadPdf(const QString &name,bool advancedMode=false);
 /*- Show simple messagebox with specified message and wait until user dismiss it */
 void message(const QString &msg);
 /*- Return true if the document was modified since it was opened or last saved, false otherwise. */
 bool modified();
 /*-
  Debugging function usable by script developers.
  Print all objects that are present in current script interpreter to command window
 */
 void objects();
 /*-
  Opens file with given name in this editor window.
  Opens without any questions, does not ask user to save changes to current file, etc ...
  Return true on success, false on failure to load file.
 */
 bool openFile(const QString &name);
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
 /*-
  Outputs given string to command window, followed by newline.
  Useful to output various debugging or status messages
 */
 void print(const QString &str);
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
 /*- Return number of revisions in current PDF document */
 int revisions();
 /*-
  Loads and runs script from given filename.
  File is looked for in the script path, unless absolute filename is given.
  If the file is not found in script path, it is looked for in current directory.
 */
 void run(QString scriptName);
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
  Save state of current editor window (size and position of window and elements inside it)
  State is saved to editor's configuration file.
 */
 void saveWindowState();
 /*-
  Save currently edited document, while creating new revision in the process.
  Return true if document was saved, false if it was not saved for any reason.
 */
 bool saveRevision();
 /*-
  Set new debugging verbosity level. Accept same parameter as -d option on commandline
  (either number or symbolic constant)
 */
 void setDebugLevel(const QString &param);
 /*- Change active revision in current PDF document */
 void setRevision(int revision);
 /*-
  Set part of the window to be either visible or invisible,
  widgetName specifies which part, see isVisible for list
  of possible names.
 */
 void setVisible(const QString &widgetName, bool visible);
 /*-
  Translate given text to current locale. Optional parameter context can specify context
  of localized text. Return translated text
 */
 QString tr(const QString &text,const QString &context=QString::null);
 /*- Return root item of currently selected tree */
 QSTreeItem* treeRoot();
 /*- Return root item of main tree */
 QSTreeItem* treeRootMain();
 /*-
  Debugging function usable by script developers.
  Print all variables that are in current script interpreter to command window
 */
 void variables();
 /*- Return version of editor (in format 'major.minor.release') */
 QString version();
 /*-
  Outputs given warning string to command window, followed by newline,
  and show this string in a messagebox to alert user.
 */
 void warn(const QString &str);

 // These are internal slots, should not available to scripting,
 // but it is not possible to do that. But at least they are uncallable
 void _dragDrop(TreeItemAbstract *source,TreeItemAbstract *target);
 void _dragDropOther(TreeItemAbstract *source,TreeItemAbstract *target);
private:
 QWidget* getWidgetByName(const QString &widgetName);
 void deleteVariable(const QString &varName);
 void addDocumentObjects();
 void removeDocumentObjects();
 void scriptCleanup();
 void runFile(QString scriptName);
private:
 /** QObject wrapper around CPdf (document) that is exposed to scripting. Lifetime of this class is the same as lifetime of document */
 QSPdf *qpdf;
 /** Editor window in which this class exist */
 PdfEditWindow* w;
 /** QSA Scripting Project */
 QSProject *qp;
 /** QSA Interpreter - taken from project */
 QSInterpreter *qs;
 /** QSObject Importer */
 QSImporter *import;
 /** All Scripting objects created under this base. Will be used for purpose of garbage collection */
 QPtrDict<QSCObject> baseObjects;
 /** Flag specifying if the tree have changed while running script to the degree it need to be reloaded */
 bool treeReloadFlag;
};

} // namespace gui

#endif
