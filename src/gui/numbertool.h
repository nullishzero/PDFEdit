/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __NUMBERTOOL_H__
#define __NUMBERTOOL_H__

#include <qwidget.h>
#include <qcombobox.h>
#include <qstring.h>

class QResizeEvent;
class QDoubleValidator;

namespace gui {

/**
 Toolbutton allowing to change number in the line edit control with simple drop down box
 Scripts can read/write the number when needed
 Can be placed in toolbar in place of ordinary button
 \brief Toolbar widget for editing arbitrary text
*/
class NumberTool : public QWidget {
Q_OBJECT
public:
 NumberTool(const QString &cName,QWidget *parent=0,const char *name=NULL);
 ~NumberTool();
 void setPredefs(const QStringList &predefs);
 void setPredefs(const QString &predefs,const QString &separator=",");
 QSize sizeHint() const;
 QString getName() const;
 void setTooltip(const QString &t);
 double getNum() const;
 void setNum(double newNum);
signals:
 /**
  Signal emitted when user changes the number
  @param name Name of the tool
 */
 void clicked(const QString &name);
 /**
  Send help message when mouse cursor enters/leaves the button.
  Help message is sent on enter and QString::null on leave.
  @param message Help message
 */
 void helpText(const QString &message);
protected:
 virtual void resizeEvent(QResizeEvent *e);
 virtual void enterEvent(QEvent *e);
 virtual void leaveEvent(QEvent *e);
protected slots:
 void textEntered(const QString &str);
protected:
 /** Validator for validating the number */
 QDoubleValidator *val;
 /** Widget for editing the number */
 QComboBox *ed;
 /** Number selected in the tool */
 double num;
 /** Name of text in this widget */
 QString textName;
 /** Tooltip for this number select box */
 QString tip;
};

} // namespace gui

#endif
