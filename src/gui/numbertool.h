#ifndef __NUMBERTOOL_H__
#define __NUMBERTOOL_H__

#include <qwidget.h>
#include <qcombobox.h>
#include <qstring.h>

class QDoubleValidator;

namespace gui {

class ToolButton;

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
 void setPredefs(const QString &predefs,const QString &separator=",");
 QSize sizeHint() const;
 QString getName() const;
 double getNum() const;
 void setNum(double newNum);
signals:
 /**
  Signal emitted when user changes the number
  @param name Name of the tool
 */
 void clicked(const QString &name);
protected:
 void resizeEvent (QResizeEvent *e);
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
};

} // namespace gui

#endif
