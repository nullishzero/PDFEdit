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
/** @file
 Dialog - class with various static dialogs:<br>
 openFileDialog - pick a filename for opening it<br>
 saveFileDialog - pick a filename for saving as<br>
 openFileDialogPdf, saveFileDialogPdf - specialization of above for PDF files<br>
 colorDialog - dialog for selecting color<br>
 readStringDialog - Ask user a question end expect him to enter some string as answer<br>
 @author Martin Petricek
*/
#include "qtcompat.h"
#include "dialog.h"
#include "settings.h"
#include "version.h"
#include <qdir.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcolordialog.h>
#include <qfileinfo.h>
#include <utils/debug.h>
#include <qstring.h>
#include "util.h"

namespace gui {

/**
 Get current directory from File Dialog
 @param fd File Dialog
 @return current directory
*/
QString getDir(QFileDialog &fd) {
#ifdef QT3
 const QDir *d=fd.dir();
 QString name=d->absPath();
 delete d;
#else
 QDir d=fd.directory();
 QString name=d.absPath();
#endif
 return name;
}

/**
 Generic "Open file" dialog.
 Wait for user to select one existing file and return its name.
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @param caption Caption used in dialog title
 @param settingName Key in settings used to save/load dialog window position
 @param savePath Key in settings used to identify where to save last used directory in the dialog
 @param filters Filters available to user to restrict shown filetypes in dialog
 @return selected Filename (or NULL)
*/
QString openFileDialog(QWidget* parent,const QString &caption/*=QString::null*/,const QString &settingName/*=QString::null*/,const QString &filters/*=QString::null*/,const QString &savePath/*=QString::null*/) {
 guiPrintDbg(debug::DBG_DBG,"openFileDialog invoked");
#ifdef QT3
 QFileDialog fd(parent,"openfiledialog",TRUE);
#else
 QFileDialog fd(parent);
#endif
 if (!filters.isNull()) {
  //Set filters if filters specified
  fd.setFilter(filters);
 }
#ifdef QT3
 fd.setShowHiddenFiles(TRUE);
#endif
 if (!caption.isNull()) fd.setCaption(caption);
 if (savePath.isNull()) {
  //No save path specified -> start in current directory
  fd.setDir(".");
 } else {
  //Try to set last used saved path, if it exists
  fd.setDir(globalSettings->read("history/path/"+savePath,"."));
 }
 fd.setMode(QFileDialog::ExistingFile);

 // "Infinite" loop, to restart the dialog if necessary (invalid file selected, etc ... )
 for(;;) {
  if (!settingName.isNull()) {
   // Restore window position from settings if applicable
   globalSettings->restoreWindow(&fd,settingName);
  }
  if (fd.exec()==QDialog::Accepted) {	//Dialog accepted
   if (!settingName.isNull()) {
    // Save window position to settings if applicable
    globalSettings->saveWindow(&fd,settingName);
   }
   if (!savePath.isNull()) {
    //Save the path if desired
    if (globalSettings->readBool("history/save_filePath",true)) {
     //Note that there is only one central setting "save paths in dialog" for all dialog types
     globalSettings->write("history/path/"+savePath,getDir(fd));
    }
   }
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

/**
 Invoke "open file" dialog for PDF files. Specialization of openFileDialog
 \see openFileDialog
 @param parent Parent widget - will be disabled during the dialog.
 @return selected Filename (or NULL)
*/
QString openFileDialogPdf(QWidget* parent) {
 return openFileDialog(parent,QObject::tr("Open file ..."),"file_dialog",QObject::tr("PDF files (*.pdf)"),"filePath");
}

/**
 Invoke generic "save file" dialog. Wait for user to select or type a single file and return its name.
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @param oldname Name of file to be saved - if specified, this name will be pre-selected.
 @param askOverwrite If true and selected file exists, user will be asked to confirm overwriting it
 @param caption Caption used in dialog title
 @param settingName Key in settings used to save/load dialog window position
 @param savePath Key in settings used to identify where to save last used directory in the dialog
 @param filters Filters available to user to restrict shown filetypes in dialog
 @return selected Filename (or NULL)
*/
QString saveFileDialog(QWidget* parent,const QString &oldname,bool askOverwrite/*=true*/,const QString &caption/*=QString::null*/,const QString &settingName/*=QString::null*/,const QString &filters/*=QString::null*/,const QString &savePath/*=QString::null*/) {
 guiPrintDbg(debug::DBG_DBG,"saveFileDialog invoked");
#ifdef QT3
 QFileDialog fd(parent,"savefiledialog",TRUE);
#else
 QFileDialog fd(parent);
#endif
 if (!filters.isNull()) {
  //Set filters if filters specified
  fd.setFilter(filters);
 }
#ifdef QT3
 fd.setShowHiddenFiles(TRUE);
#endif
 if (!caption.isNull()) fd.setCaption(caption);
 if (savePath.isNull()) {
  //No save path specified -> start in current directory
  fd.setDir(".");
 } else {
  //Try to set last used saved path, if it exists
  fd.setDir(globalSettings->read("history/path/"+savePath,"."));
 }
 if (!oldname.isNull()) {
#ifdef QT3
  fd.setSelection(oldname);
#else
  fd.selectFile(oldname);
#endif
 }
 fd.setMode(QFileDialog::AnyFile);
 //Name that will hold the file (if some is picked)
 QString name;
 // "Infinite" loop, to restart the dialog if necessary (invalid file selected, etc ... )
 for(;;) {
  if (!settingName.isNull()) {
   // Restore window position from settings if applicable
   globalSettings->restoreWindow(&fd,settingName);
  }
  if (fd.exec()==QDialog::Accepted) {
   if (!settingName.isNull()) {
    // Save window position to settings if applicable
    globalSettings->saveWindow(&fd,settingName);
   }
   name=fd.selectedFile();
   //TODO: check if not directory
   if (askOverwrite && QFile::exists(name)) {
    //File exists : ask if it should be overwritten
    int answer=QMessageBox::question(parent,APP_NAME,
     QObject::tr("File called \"%1\" already exists. Do you want to overwrite it?").arg(name),
     QObject::tr("&Yes"),QObject::tr("&No"),QObject::tr("&Cancel"),1,2);
    if (answer==0) {				 //Yes, overwrite is ok
     //Break from the cycle mean valid file was selected
     break;
    }
    if (answer==1) continue;			//No, restart dialog and ask for another file
    if (answer==2) return QString::null;	//Cancel, do not overwrite and exit
   }
   //Not asking about overwrite
   //Break from the cycle mean valid file was selected
   break;
  }
  //Dialog cancelled
  if (!settingName.isNull()) {
   // Save window position to settings if applicable
   globalSettings->saveWindow(&fd,settingName);
  }
  return QString::null;
 } //End of not-so-infinite for cycle

 if (!savePath.isNull()) {
  //Save the path if desired
  if (globalSettings->readBool("history/save_filePath",true)) {
   //Note that there is only one central setting "save paths in dialog" for all dialog types
   globalSettings->write("history/path/"+savePath,getDir(fd));
  }
 }
 return name;
}

/**
 Invoke "save file" dialog for PDF files. Specialization of saveFileDialog
 \see saveFileDialog
 @param parent Parent widget - will be disabled during the dialog.
 @param oldname Name of file to be saved - if specified, this name will be pre-selected.
 @param askOverwrite If true and selected file exists, user will be asked to confirm overwriting it
 @return selected Filename (or NULL)
*/
QString saveFileDialogPdf(QWidget* parent,const QString &oldname,bool askOverwrite/*=true*/) {
 return saveFileDialog(parent,oldname,askOverwrite,QObject::tr("Save file as ..."),"file_dialog",QObject::tr("PDF files (*.pdf)"),"filePath");
}

/**
 Invoke "save file" dialog for xml files. Specialization of saveFileDialog
 \see saveFileDialog
 @param parent Parent widget - will be disabled during the dialog.
 @param oldname Name of file to be saved - if specified, this name will be pre-selected.
 @param askOverwrite If true and selected file exists, user will be asked to confirm overwriting it
 @return selected Filename (or NULL)
*/
QString saveFileDialogXml(QWidget* parent,const QString &oldname,bool askOverwrite/*=true*/) {
 return saveFileDialog(parent,oldname,askOverwrite,QObject::tr("Save file as ..."),"file_dialog",QObject::tr("xml files (*.xml)"),"filePath");
}


/**
 Invoke "read string" dialog. Show message and wait for user to type any string.
 Will return NULL if user cancels the dialog.
 @param parent Parent widget - will be disabled during the dialog.
 @param message Message to show in the dialog
 @param def Default text that will be pre-typed in the dialog
 @return typed text, or NULL if dialog cancelled
 */
QString readStringDialog(QWidget* parent,const QString &message, const QString &def) {
 bool ok=FALSE;
 QString res=QInputDialog::getText(APP_NAME,message,QLineEdit::Normal,def,&ok,parent,"read_string");
 if (ok) return res;
 return QString::null;
}

/**
 Invoke dialog to select color.
 Last selected color is remembered and offered as default next time.
 The 'initial default color' is red
 @param parent Parent widget - will be disabled during the dialog.
 @return selected color. If dialog is cancelled, 'invalid' color is returned
*/
QColor colorDialog(QWidget* parent) {
 static QColor defaultColor=Qt::red;
 QColor ret=QColorDialog::getColor(defaultColor,parent,"std_color_dialog");
 if (ret.isValid()) defaultColor=ret;
 return ret;
}

/**
 Asks question with Yes/No answer. "Yes" is default.
 Return true if user selected "yes", false if user selected "no"
 @param msg Question to display
 @param parent Parent widget - will be disabled during the dialog.
 @return True if yes, false if no
*/
bool questionDialog(QWidget* parent,const QString &msg) {
 int answer=QMessageBox::question(parent,APP_NAME,msg,QObject::tr("&Yes"),QObject::tr("&No"),QString::null,0,1);
 return (answer==0);
}

} // namespace gui
