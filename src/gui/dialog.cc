/** @file
 Dialog - class with various static dialogs:
OpenFileDialog
SaveFileDialog
*/
#include "dialog.h"
#include "version.h"
#include <qinputdialog.h> 
#include <qfiledialog.h> 
#include <utils/debug.h>

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
 fd.setMode(QFileDialog::ExistingFile);
 if (fd.exec()==QDialog::Accepted) return fd.selectedFile();
 return QString::null;
}

/** Invoke "save file" dialog. Wait for user to select or type a single file and return its name.
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @param oldName Name of file to be saved - if specified, this name will be pre-selected.
 @return selected Filename*/
QString saveFileDialog(QWidget* parent,const QString &oldname) {
 printDbg(debug::DBG_DBG,"saveFileDialog invoked");
 QFileDialog fd(parent,"savefiledialog",TRUE);
 fd.setFilter(QObject::tr("PDF files (*.pdf)"));
 fd.setShowHiddenFiles(TRUE);
 fd.setCaption(QObject::tr("Save file as ..."));
 if (!oldname.isNull()) fd.setSelection(oldname);
 fd.setMode(QFileDialog::AnyFile);
 if (fd.exec()==QDialog::Accepted) return fd.selectedFile();
 return QString::null;
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
