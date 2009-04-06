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
#ifndef __SELECTTOOL_H__
#define __SELECTTOOL_H__

#include <qwidget.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qstringlist.h>
class QResizeEvent;

namespace gui {

/**
 Toolbutton allowing to select text from list of predefined values
 Scripts can read/write the text when needed
 Can be placed in toolbar in place of ordinary button
 \brief Toolbar widget for selecting arbitrary text
*/
class SelectTool : public QWidget {
Q_OBJECT
public:
 SelectTool(const QString &cName,QWidget *parent=0,const char *name=NULL);
 ~SelectTool();
 QSize sizeHint() const;
 QString getName() const;
 void setPredefs(const QString &predefs,const QString &separator=",");
 void setPredefs(const QStringList &predefs);
 QString getText() const;
 void setText(const QString &newText);
 void setTooltip(const QString &t);
signals:
 /**
  Signal emitted when user changes the text
  @param name Name of the text tool
 */
 void clicked(const QString &name);
 /**
  Send help message when mouse cursor enters/leaves the button.
  Help message is sent on enter and QString::null on leave.
  @param message Help message
 */
 void helpText(const QString &message);
protected:
 virtual void resizeEvent (QResizeEvent *e);
 virtual void enterEvent(QEvent *e);
 virtual void leaveEvent(QEvent *e);
protected slots:
 void textEntered(const QString &str);
protected:
 /** combo box or line edit for the text */
 QComboBox *ed;
 /** Text typed in the tool */
 QString text;
 /** Name of text in this widget */
 QString textName;
 /** Allowed choices */
 QStringList choices;
 /** Tooltip for this selectbox */
 QString tip;
};

} // namespace gui

#endif
