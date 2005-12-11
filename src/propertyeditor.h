#ifndef __PROPERTYEDITOR_H__
#define __PROPERTYEDITOR_H__

#include <qstringlist.h>
#include <qobjectlist.h>
#include <qscrollview.h>
#include <qdict.h>
#include <qwidget.h>
#include <qgrid.h>
#include <qlayout.h>
#include "property.h"

/** property editor widget */
class PropertyEditor : public QWidget {
 Q_OBJECT
public:
 PropertyEditor(QWidget *parent=0, const char *name=0);
 virtual ~PropertyEditor();
 void unsetObject();
 void setObject(void *pdfObject);
 void commitProperties();
 void resizeEvent (QResizeEvent *e);
public slots:
 void update(Property &p);
private:
 void clear();
 void addProperty(Property *prop);
 void addProperty(QString name);
private:
// QGrid *grid;
 int nObjects;
 QFrame *grid;
 QGridLayout *gridl;
 QScrollView *scroll;
 QStringList *list;
 QDict<Property> *items;
 QDict<QLabel> *labels;
};

#endif
