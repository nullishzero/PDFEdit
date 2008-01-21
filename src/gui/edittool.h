/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __EDITTOOL_H__
#define __EDITTOOL_H__

#include <qwidget.h>
#include <qlineedit.h>
#include <qstring.h>

class QResizeEvent;

namespace gui {

/**
 Toolbutton allowing to change text in the line edit control
 Scripts can read/write the text when needed
 Can be placed in toolbar in place of ordinary button
 \brief Toolbar widget for editing arbitrary text
*/
class EditTool : public QWidget {
Q_OBJECT
public:
 EditTool(const QString &cName,QWidget *parent=0,const char *name=NULL);
 ~EditTool();
 QSize sizeHint() const;
 QString getName() const;
 QString getText() const;
 void setText(const QString &newText);
signals:
 /**
  Signal emitted when user changes the text
  @param name Name of the text tool
 */
 void clicked(const QString &name);
protected:
 void resizeEvent (QResizeEvent *e);
protected slots:
 void textEntered();
protected:
 /** combo box or line edit for the text */
 QLineEdit *ed;
 /** Text typed in the tool */
 QString text;
 /** Name of text in this widget */
 QString textName;
};

} // namespace gui

#endif
