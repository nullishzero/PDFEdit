#ifndef __SELECTTOOL_H__
#define __SELECTTOOL_H__

#include <qwidget.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qstringlist.h>

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
signals:
 /**
  Signal emitted when user changes the text
  @param name Name of the text tool
 */
 void clicked(const QString &name);
protected:
 void resizeEvent (QResizeEvent *e);
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
};

} // namespace gui

#endif
