/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __QSIPROPERTYARRAY_H__
#define __QSIPROPERTYARRAY_H__

#include "qscobject.h"
#include <cobject.h>
#include <stdlib.h>
#include <pdfoperators.h>

namespace gui {

class QSIProperty;

using namespace pdfobjects;

/*=
 This type represents arbitrary array of IProperty items
 It hold zero or more values indexed by positive integer, starting from zero.
 Values can be of any type, either simple types (int, bool, float, string)
 or complex types (Dict, Array)
*/
/** \brief QObject wrapper around array of arbitrary IProperty items */
class QSIPropertyArray : public QSCObject {
 Q_OBJECT
public:
 QSIPropertyArray(BaseCore *_base);
 virtual ~QSIPropertyArray();
 boost::shared_ptr<IProperty> get(int index);
 void append(boost::shared_ptr<IProperty> prop);
 void copyTo(PdfOperator::Operands &oper);
public slots:
 /*- append one IProperty element to end of array. */
 void append(QSIProperty *prop);
 void append(QObject *obj);
 /*- Delete all elements in the array */
 void clear();
 /*- Return size of this array (number of elements) */
 int count();
 /*- Get IProperty element with given index from this array */
 QSCObject* property(int index);
private:
 /** Vector holding parameters */
 std::vector<boost::shared_ptr<IProperty> > obj;
};

} // namespace gui 

#endif
