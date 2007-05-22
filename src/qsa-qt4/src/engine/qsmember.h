/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech AS. All rights reserved.
**
** This file is part of the QSA of the Qt Toolkit.
**
** For QSA Commercial License Holders (non-open source):
** 
** Licensees holding a valid Qt Script for Applications (QSA) License Agreement
** may use this file in accordance with the rights, responsibilities and
** obligations contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of the Licensing Agreement are
** not clear to you.
** 
** Further information about QSA licensing is available at:
** http://www.trolltech.com/products/qsa/licensing.html or by contacting
** info@trolltech.com.
** 
** 
** For Open Source Edition:  
** 
** This file may be used under the terms of the GNU General Public License
** version 2 as published by the Free Software Foundation and appearing in the
** file LICENSE.GPL included in the packaging of this file.  Please review the
** following information to ensure GNU General Public Licensing requirements
** will be met:  http://www.trolltech.com/products/qt/opensource.html 
** 
** If you are unsure which license is appropriate for your use, please review
** the following information:
** http://www.trolltech.com/products/qsa/licensing.html or contact the 
** sales department at sales@trolltech.com.

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSMEMBER_H
#define QSMEMBER_H

#include "qsglobal.h"
#include <QString>
#include <QMap>

class QSObject;
class QSList;
class QSClass;
class QSFunctionBodyNode;
class QSEnv;

typedef QSObject (*QSMemberFunctionPointer)(QSObject *, const QSEnv *env);
typedef QSObject (*QSFunctionPointer)(QSEnv *env);
typedef void (*QSVoidFunctionPointer)(QSEnv *env);

class QSMember {
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

    QSMember(Type t = Undefined, int a = AttributeNone)
	: typ(t), attrs(a), own(0) { }

    QSMember(Type t, int i, int a)
	: typ(t), attrs(a), own(0), idx(i) { }

    QSMember(QSFunctionPointer fptr,
	      int a = AttributeNonWritable)
         : typ(NativeFunction), attrs(a|AttributeExecutable), own(0),
          nativeFunction(fptr) { }

    QSMember(QSMemberFunctionPointer fptr, int a = AttributeNonWritable)
	 : typ(NativeMemberFunction), attrs(a|AttributeExecutable),
	   own(0), nativeMemberFunction(fptr) { }

    QSMember(QSVoidFunctionPointer fptr, int a = AttributeNonWritable)
	 : typ(NativeVoidFunction), attrs(a|AttributeExecutable),
	   own(0), nativeVoidFunction(fptr) { }

    QSMember(QSFunctionBodyNode * f, int a = AttributeNonWritable)
	 : typ(ScriptFunction), attrs(a|AttributeExecutable), own(0),
	  scriptFunction(f) { }

    Type type() const { return typ; }
    void setType(Type t) { typ = t; }
    QString typeName() const;

    int attributes() const { return attrs; }
    bool hasAttribute(QSAttribute a) const { return attrs & a; }
    bool isStatic() const { return attrs&AttributeStatic; }

    bool isDefined() const { return typ != Undefined; }

    bool isExecutable() const { return attrs&AttributeExecutable; }
    bool isReadable() const { return !(attrs&AttributeNonReadable);  }
    bool isWritable() const { return !(attrs&AttributeNonWritable); }
    bool isEnumberable() const { return attrs&AttributeEnumerable; }
    bool isPrivate() const { return attrs&AttributePrivate; }

    bool isMemberOf(const QSClass * c) const { return c==own; }

    void setExecutable(bool exec) {
	attrs = exec ? attrs|AttributeExecutable : attrs&~AttributeExecutable;
    }
    void setReadable(bool read) {
	attrs = read ? attrs&~AttributeNonReadable :
		attrs|AttributeNonReadable;
    }
    void setWritable(bool write) {
	attrs = write ? attrs&~AttributeNonWritable :
		attrs|AttributeNonWritable;
    }
    void setPrivate(bool priv) {
	attrs = priv ? attrs|AttributePrivate :
		attrs&~AttributePrivate;
    }
    void setStatic(bool stat) {
	attrs = stat ? attrs | AttributeStatic :
		attrs&~AttributeStatic;
    }

    int index() const { return idx; }
    void setIndex(int i) { idx = i; }

    QString name() const { return str; }
    void setName(const QString &n) { str = n; }

    const QSClass *owner() const { return own; }
    void setOwner(const QSClass *cl) { own = cl; }

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

QString operator+(const QString &a, const QSMember &b);
bool operator==(const QSMember &a, const QSMember &b);

#endif
