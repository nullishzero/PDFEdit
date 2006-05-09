#ifndef __BASE_H__
#define __BASE_H__

#include <qobject.h>
#include <qstring.h>
#include <qptrdict.h>

class QSProject;
class QSInterpreter;

namespace gui {

class PdfEditWindow;
class QSMenu;
class QSImporter;
class QSPdf;
class QSDict;
class QSArray;
class QSIProperty;
class QSCObject;

class Base : public QObject {
 Q_OBJECT
public:
 Base(PdfEditWindow *parent);
 void stopScript();
 void importDocument();
 void destroyDocument();
 void addDocumentObjects();
 void call(const QString &name);
 void removeDocumentObjects();
 void runInitScript();
 void runScript(QString script);
 void addGC(QSCObject *o);
 void removeGC(QSCObject *o);
 void cleanup();
 ~Base();
public slots: //This will be all exported to scripting
 /*- Invokes "About" dialog, showing information about this program and its authors */
 void about();
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
 */
 void openFile(const QString &name);
 /*- Opens file with given name in new editor window */
 void openFileNew(const QString &name);
 /*- Invokes options dialog. Does not wait for dialog to be closed by user and return immediately. */
 void options();
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
 /*- Loads and runs script from given filename. */
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
  Save currently edited document under different name.
  Also updates the name of file in editor to the one specified.
  Return true if document was saved, false if it failed to save for any reason
 */
 bool saveAs(const QString &name);
 /*-
  Save state of current editor window (size and position of window and elements inside it)
  State is saved to editor's configuration file.
 */
 void saveWindowState();
 /*-
  Set new debugging verbosity level. Accept same parameter as -d option on commandline
  (either number or symbolic constant)
 */
 void setDebugLevel(const QString &param);
 /*-
  Translate given text to current locale. Optional parameter context can specify context
  of localized text. Return translated text
 */
 QString tr(const QString &text,const QString &context=QString::null);
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
};

} // namespace gui

#endif
