#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include <qstring.h>
#include <qlabel.h>
#include <qwidget.h>
#include <iproperty.h>

using namespace pdfobjects;

/** Property flags (hidden, readonly, unset ...) */
typedef int PropertyFlags;

/** ancestor of all property items */
class Property : public QWidget {
public:
 Property(const QString &_name=0,QWidget *parent=0, PropertyFlags _flags=0);
 virtual ~Property();
 QString getName();
 PropertyFlags getFlags();
 void setFlags(PropertyFlags flag);
 bool getReadOnly();
 void setReadOnly(bool _readonly);
 /** write internal value to given PDF object */
 virtual void writeValue(IProperty *pdfObject) = 0; //virtual
 /** read internal value from given PDF object */
 virtual void readValue(IProperty *pdfObject) = 0; //virtual
protected:
 /** Name of this property */
 QString name;
 /** flags of the property (from PDF object) */
 PropertyFlags flags;
 /** is this property readonly? Independent from flags */
 bool readonly;
 /** is this property hidden? Independent from flags */
 bool hidden;
};

#endif
