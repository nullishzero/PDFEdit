#ifndef __PROPERTY_H__
#define __PROPERTY_H__

#include <qwidget.h>
class QString;
namespace pdfobjects {
 class IProperty;
}

namespace gui {

using namespace pdfobjects;

/** Property flags (hidden, readonly, unset ...) */
typedef int PropertyFlags;

/** ancestor of all property items */
class Property : public QWidget {
Q_OBJECT
public:
 Property(const QString &_name=0,QWidget *parent=0, PropertyFlags _flags=0);
 virtual ~Property();
 QString getName();
 PropertyFlags getFlags();
 void setFlags(PropertyFlags flag);
 bool getReadOnly();
 virtual void setReadOnly(bool _readonly);
 /**
  write internal value to given PDF object
  @param pdfObject Objet to write to
 */
 virtual void writeValue(IProperty *pdfObject) = 0; //virtual
 /**
  read internal value from given PDF object
  @param pdfObject Objet to read from
 */
 virtual void readValue(IProperty *pdfObject) = 0; //virtual
 /** 
  Check if edited property is currently valid.
  @return true if valid, false if not
 */
 virtual bool isValid()=0;
signals:
 /** Signal emitted when property is edited.
  Send property pointer with the signal */
 void propertyChanged(Property *prop);
protected:
 void emitChanged();
protected:
 /** Name of this property */
 QString name;
 /** flags of the property (from PDF object) */
 PropertyFlags flags;
 /** is this property readonly? Independent from flags */
 bool readonly;
 /** is this property hidden? Independent from flags */
 bool hidden;
 /** was the property edited since last readValue or writeValue? */
 bool changed;
};

} // namespace gui

#endif
