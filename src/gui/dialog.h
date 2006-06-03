#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <qstring.h>
#include <qcolor.h>

class QWidget;

/**
 GUI namespace<br>
 This namespace contain all user-interface related classes
 (widgets, tree items, scripting wrappers and others)
*/
namespace gui {

QString openFileDialog(QWidget* parent,const QString &caption=QString::null,const QString &settingName=QString::null,const QString &filters=QString::null,const QString &savePath=QString::null);
QString openFileDialogPdf(QWidget* parent=NULL);
QString saveFileDialog(QWidget* parent,const QString &oldname,bool askOverwrite=true,const QString &caption=QString::null,const QString &settingName=QString::null,const QString &filters=QString::null,const QString &savePath=QString::null);
QString saveFileDialogPdf(QWidget* parent=NULL,const QString &oldname=QString::null,bool askOverwrite=true);
QString readStringDialog(QWidget* parent,const QString &message, const QString &def="");
QColor colorDialog(QWidget* parent);
bool questionDialog(QWidget* parent,const QString &msg);

} // namespace gui

#endif
