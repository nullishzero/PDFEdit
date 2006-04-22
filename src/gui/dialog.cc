/** @file
 Dialog - class with various static dialogs:<br>
 OpenFileDialog - pick a filename for opening it<br>
 SaveFileDialog - pick a filename for saving as<br>
 readStringDialog - Ask user a question end expect him to enter some string as answer<br>
 @author Martin Petricek
*/
#include "dialog.h"
#include "settings.h"
#include "version.h"
#include <qdir.h> 
#include <qinputdialog.h> 
#include <qmessagebox.h> 
#include <qfiledialog.h> 
#include <qfileinfo.h> 
#include <utils/debug.h>
#include <qstring.h>

namespace gui {

/** Get current directory from File Dialog
 @param fd File Dialog
 @return current directory
 */
QString getDir(QFileDialog &fd) {
 const QDir *d=fd.dir();
 QString name=d->absPath();
 delete d;
 return name;
}

/** Invoke "open file" dialog. Wait for user to select one existing file and return its name.
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @return selected Filename*/
QString openFileDialog(QWidget* parent) {
 printDbg(debug::DBG_DBG,"openFileDialog invoked");
 QFileDialog fd(parent,"openfiledialog",TRUE);
 fd.setFilter(QObject::tr("PDF files (*.pdf)"));
 fd.setShowHiddenFiles(TRUE);
 fd.setCaption(QObject::tr("Open file ..."));
 fd.setDir(globalSettings->read("history/filePath","."));
 fd.setMode(QFileDialog::ExistingFile);
 for(;;) {
  globalSettings->restoreWindow(&fd,"file_dialog");
  if (fd.exec()==QDialog::Accepted) {
   globalSettings->saveWindow(&fd,"file_dialog");
   if (globalSettings->readBool("history/save_filePath",true)) globalSettings->write("history/filePath",getDir(fd));
   QString name=fd.selectedFile();
    if (QFileInfo(name).isDir()) { //directory was selected
     //TODO: test this !
     fd.setDir(name);
     continue;//restart dialog
    }
   return name;
  }
  return QString::null;
 }
}

/** Invoke "save file" dialog. Wait for user to select or type a single file and return its name.
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @param oldname Name of file to be saved - if specified, this name will be pre-selected.
 @param askOverwrite If true and selected file exists, user will be asked to confirm overwriting it
 @return selected Filename*/
QString saveFileDialog(QWidget* parent,const QString &oldname,bool askOverwrite/*=true*/) {
 printDbg(debug::DBG_DBG,"saveFileDialog invoked");
 QFileDialog fd(parent,"savefiledialog",TRUE);
 fd.setFilter(QObject::tr("PDF files (*.pdf)"));
 fd.setShowHiddenFiles(TRUE);
 fd.setCaption(QObject::tr("Save file as ..."));
 fd.setDir(globalSettings->read("history/filePath","."));
 if (!oldname.isNull()) fd.setSelection(oldname);
 fd.setMode(QFileDialog::AnyFile);
 for(;;) {
  globalSettings->restoreWindow(&fd,"file_dialog");
  if (fd.exec()==QDialog::Accepted) {
   globalSettings->saveWindow(&fd,"file_dialog");
   QString name=fd.selectedFile();
   //TODO: check if not directory
   if (askOverwrite && QFile::exists(name)) { //File exists : ask if it should be overwritten
    int answer=QMessageBox::question(parent,APP_NAME,QObject::tr("File \"")+name+QObject::tr("\" already exists. Overwrite?"),
                                     QObject::tr("&Yes"),QObject::tr("&No"),QObject::tr("&Cancel"),1,2);
    if (answer==0) { //Yes, overwrite is ok
     if (globalSettings->readBool("history/save_filePath",true)) globalSettings->write("history/filePath",getDir(fd));
     return name;		  
    }
    if (answer==1) continue;		  //No, restart dialog
    if (answer==2) return QString::null;//Cancel, do not overwrite and exit
   }
   if (globalSettings->readBool("history/save_filePath",true)) globalSettings->write("history/filePath",getDir(fd));
   //Not asking about overwrite
   return name;
  }
  //Dialog cancelled
  globalSettings->saveWindow(&fd,"file_dialog");
  return QString::null;
 }
}

/** Invoke "read string" dialog. Show message and wait for user to type any string, 
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @param message Message to show in the dialog
 @param def Default text that will be pre-typed in the dialog
 @return typed text, or NULL if dialog cancelled */
QString readStringDialog(QWidget* parent,const QString &message, const QString &def) {
 bool ok=FALSE;
 QString res=QInputDialog::getText(APP_NAME,message,QLineEdit::Normal,def,&ok,parent,"read_string");
 if (ok) return res;
 return QString::null;
}

} // namespace gui
