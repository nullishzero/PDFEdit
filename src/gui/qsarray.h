/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __QSARRAY_H__
#define __QSARRAY_H__

#include "qsiproperty.h"
#include <kernel/cobject.h>

namespace gui {

using namespace pdfobjects;

/*=
 This type represents an array in PDF document.
 It hold zero or more values indexed by positive integer, starting from zero.
 Values can be of any type, either simple types (int, bool, float, string)
 or complex types (Dict, Array)
*/
/** \brief QObject wrapper around CArray */
class QSArray : public QSIProperty {
 Q_OBJECT
public:
 QSArray(QSArray &source);
 QSArray(boost::shared_ptr<CArray> _array,BaseCore *_base);
 virtual ~QSArray();
 boost::shared_ptr<CArray> get();
public slots:
 /*- Insert element at given index in array */
 void add(int index,QSIProperty *ip);
 void add(int index,QObject *ip);
 /*- Insert String element at given index in array */
 void add(int index,const QString &ip);
 /*- Insert Int element at given index in array */
 void add(int index,int ip);
 /*- Append element at end of array */
 void add(QSIProperty *ip);
 void add(QObject *ip);
 /*- Append String element at end of array */
 void add(const QString &ip);
 /*- Append Int element at end of array */
 void add(int ip);
 /*-
  Get Dict/Array property recursively
  Will take the name as slash-separated list of childs to traverse to get to target property,
  going through Dicts and Arrays.
  Any references on the way are automatically dereferenced
 */
 QSCObject* child(const QString &name);
 /*- Return size of this array (number of elements) */
 int count();
 /*- Delete element with given index from this array. Elements with higher index (if any) are shifted to occupy the space */
 void delProperty(int index);
 /*- Return string representation of this array */
 QString getText();
 /*- Get element with given index from this array */
 QSCObject* property(int index);
};

} // namespace gui

#endif
