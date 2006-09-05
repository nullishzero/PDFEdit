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

#ifndef QSMEMBER_H
#define QSMEMBER_H

#include "qsglobal.h"
#include "../kernel/dlldefs.h"
#include <qstring.h>
#include <qmap.h>

class QSObject;
class QSList;
class QSClass;
class QSFunctionBodyNode;
class QSEnv;

typedef QSObject (*QSMemberFunctionPointer)( QSObject *, const QSEnv *env );
typedef QSObject (*QSFunctionPointer)( QSEnv *env );
typedef void (*QSVoidFunctionPointer)( QSEnv *env );

class QUICKCORE_EXPORT QSMember {
public:
    enum Type { Undefined,
		Variable,
		Object,
		Identifier,
		Class,
		Custom,
		NativeFunction,
		NativeMemberFunction,
		NativeVoidFunction,
		ScriptFunction
    };

    QSMember( Type t = Undefined, int a = AttributeNone )
	: typ( t ), attrs( a ), own( 0 ) { }

    QSMember( Type t, int i, int a )
	: typ( t ), attrs( a ), own( 0 ), idx( i ) { }

    QSMember( QSFunctionPointer fptr,
	      int a = AttributeNonWritable )
         : typ( NativeFunction ), attrs( a|AttributeExecutable ), own(0),
          nativeFunction( fptr ) { }

    QSMember( QSMemberFunctionPointer fptr, int a = AttributeNonWritable )
	 : typ( NativeMemberFunction ), attrs( a|AttributeExecutable ),
	   own( 0 ), nativeMemberFunction( fptr ) { }

    QSMember( QSVoidFunctionPointer fptr, int a = AttributeNonWritable )
	 : typ( NativeVoidFunction ), attrs( a|AttributeExecutable ),
	   own( 0 ), nativeVoidFunction( fptr ) { }

    QSMember( QSFunctionBodyNode * f, int a = AttributeNonWritable )
	 : typ( ScriptFunction ), attrs( a|AttributeExecutable ), own(0),
	  scriptFunction( f ) { }

    Type type() const { return typ; }
    void setType( Type t ) { typ = t; }
    QString typeName() const;

    int attributes() const { return attrs; }
    bool hasAttribute( QSAttribute a ) const { return attrs & a; }
    bool isStatic() const { return attrs&AttributeStatic; }

    bool isDefined() const { return typ != Undefined; }

    bool isExecutable() const { return attrs&AttributeExecutable; }
    bool isReadable() const { return !(attrs&AttributeNonReadable);  }
    bool isWritable() const { return !(attrs&AttributeNonWritable); }
    bool isEnumberable() const { return attrs&AttributeEnumerable; }
    bool isPrivate() const { return attrs&AttributePrivate; }

    bool isMemberOf( const QSClass * c ) const { return c==own; }

    void setExecutable( bool exec ) {
	attrs = exec ? attrs|AttributeExecutable : attrs&~AttributeExecutable;
    }
    void setReadable( bool read ) {
	attrs = read ? attrs&~AttributeNonReadable :
		attrs|AttributeNonReadable;
    }
    void setWritable( bool write ) {
	attrs = write ? attrs&~AttributeNonWritable :
		attrs|AttributeNonWritable;
    }
    void setPrivate( bool priv ) {
	attrs = priv ? attrs|AttributePrivate :
		attrs&~AttributePrivate;
    }
    void setStatic( bool stat ) {
	attrs = stat ? attrs | AttributeStatic :
		attrs&~AttributeStatic;
    }

    int index() const { return idx; }
    void setIndex( int i ) { idx = i; }

    QString name() const { return str; }
    void setName( const QString &n ) { str = n; }

    const QSClass *owner() const { return own; }
    void setOwner( const QSClass *cl ) { own = cl; }

private:
    Type typ;
    int attrs;
    const QSClass *own;
    QString str;

public:
    // ### make private
    union {
	int idx;
	QSFunctionBodyNode * scriptFunction;
	QSFunctionPointer nativeFunction;
	QSMemberFunctionPointer nativeMemberFunction;
	QSVoidFunctionPointer nativeVoidFunction;
	QSObject *obj;
    };
};

typedef QMap<QString, QSMember> QSMemberMap;

QString operator+( const QString &a, const QSMember &b );
bool operator==( const QSMember &a, const QSMember &b );

#endif
