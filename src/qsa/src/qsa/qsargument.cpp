/****************************************************************************
** $Id$
**
** Copyright (C) 2001-2006 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Script for Applications framework (QSA).
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding a valid Qt Script for Applications license may use
** this file in accordance with the Qt Script for Applications License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about QSA Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
*****************************************************************************/

#include "qsargument.h"
#include <qobject.h>

/*!
  \class QSArgument qsargument.h

  \brief The QSArgument class provides storage for various datatypes
  used in QSObjectFactory::create() calls.

  When the user writes Qt Script code like this:
  \code
    var x = new AnotherCppObject( arg1, arg2 ); // Qt Script
  \endcode
  the arguments are passed to the QSObjectFactory::create() call as a
  QValueList of QSArguments. A QSArgument can hold a QVariant, a
  QObject pointer or a void pointer. The type held by a QSArgument is
  returned by type() and the value is returned by variant(), qobject(), or ptr(), depending on the QSArgument type.

*/

/*! \enum QSArgument::Type

   This enum describes which datatype is held by this QSArgument.

   \value Invalid Invalid datatype
   \value Variant Holds a QVariant
   \value QObjectPtr Holds a QObject pointer
   \value VoidPointer Holds an untyped pointer (void*)
*/

/*! Creates an invalid QSArgument */

QSArgument::QSArgument()
{
    qobj = 0;
    pt = 0;
    typ = Invalid;
}

/*!
  Creates a QSArgument which holds the variant \a v as its value and
  is of type Variant.
*/

QSArgument::QSArgument( const QVariant &v )
{
    var = v;
    qobj = 0;
    pt = 0;
    typ = Variant;
}

/*!
  Creates a QSArgument which holds the QObject pointer \a o as its value
  and is of type QObjectPtr.
*/

QSArgument::QSArgument( QObject *o )
{
    qobj = o;
    pt = 0;
    typ = QObjectPtr;
}

/*!
  Creates a QSArgument which holds the pointer \a p as its value and
  is of type VoidPointer.
*/

QSArgument::QSArgument( void *p )
{
    pt = p;
    qobj = 0;
    typ = VoidPointer;
}

/*!
    If this QSArgument's type() is Variant, it returns the variant value;
    otherwise returns an invalid variant.
*/

QVariant QSArgument::variant() const
{
    if ( typ == Variant )
	return var;
    return QVariant();
}

/*!
    If this QSArgument's type() is QObjectPtr, it returns the object
    pointer; otherwise returns 0.
*/

QObject *QSArgument::qobject() const
{
    if ( typ == QObjectPtr )
	return qobj;
    return 0;
}

/*!
    If this QSArgument's type() is VoidPointer, it returns the void pointer;
    otherwise returns 0.
*/
void *QSArgument::ptr() const
{
    if ( typ == VoidPointer )
	return pt;
    return 0;
}

/*!
  Returns the type of this QSArgument. This can be \c Invalid, \c
  Variant, \c QObjectPtr, or \c VoidPointer

  \sa typeName()
*/

QSArgument::Type QSArgument::type() const
{
    return typ;
}

/*! Returns the type name of this argument. */

QString QSArgument::typeName() const
{
    switch ( typ ) {
    case Invalid:
	return QString::fromLatin1("Invalid");
    case Variant:
	return QString::fromLatin1(variant().typeName());
    case QObjectPtr:
	return QString::fromLatin1(qobject()->className());
    case VoidPointer:
	return QString::fromLatin1("Pointer");
    }
    return QString::fromLatin1("Unknown");
}




/*!
  \class QSArgumentList qsargument.h

  \brief The QSArgumentList class provides a QValueList of QSArguments
  with some convenience constructors.

  This class inherits QValueList<QSArgument>, so the API of QValueList
  applies for this class. In addition, this class offers convenience
  constructors which make working with this class more comfortable.

*/

/*! \fn QSArgumentList::QSArgumentList()

Creates an empty QSArgumentList.
*/

/*! \fn QSArgumentList::QSArgumentList( const QValueList<QVariant> &l )

  Converts the list \a l of QVariants into a QSArgumentList.
*/

/*! \fn QSArgumentList::QSArgumentList( const QVariant &v )

  Creates a QSArgumentList and appends \a v to it.
*/

/*! \fn QSArgumentList::QSArgumentList( QObject *o )

  Creates a QSArgumentList and appends \a o to it.
*/

/*! \fn QSArgumentList::QSArgumentList( void *p )

  Creates a QSArgumentList and appends \a p to it.
*/


bool operator==( const QSArgument &a, const QSArgument &b )
{
    if( a.typ != b.typ )
	return FALSE;
    switch( a.typ ) {
    case QSArgument::Variant: return a.var == b.var;
    case QSArgument::QObjectPtr: return a.qobj == b.qobj;
    case QSArgument::VoidPointer: return a.pt == b.pt;
    case QSArgument::Invalid: return TRUE;
    }
    return FALSE;
}
