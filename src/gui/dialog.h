#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <qstring.h>
#include <qwidget.h>

QString openFileDialog(QWidget* parent=NULL);
QString saveFileDialog(QWidget* parent=NULL,const QString &oldname=QString::null);
QString readStringDialog(QWidget* parent,const QString &message, const QString &def="");

#endif
