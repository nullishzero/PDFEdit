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

#include "qserror_object.h"
#include "qsoperations.h"
#include "qstypes.h"
#include "qsinternal.h"
#include "qsfunction.h"
#include "qsdebugger.h"

static const char * const errNames[] = {
    "No Error",
    "Error",
    "EvalError",
    "RangeError",
    "ReferenceError",
    "SyntaxError",
    "TypeError",
    "URIError",
    "ThrowError"
};

class QSErrorShared : public QSWritable
{
public:
    QSErrorShared(ErrorType typ, const QString &mes, int lin)
        : type(typ), message(mes), line(lin)
    {
#ifdef QSDEBUGGER
        sid = 0;
#endif
    }

    ErrorType type;
    QString message;
    int line;
#ifdef QSDEBUGGER
    int sid;
#endif
};

QSObject QSErrorClass::construct( const QSList &args ) const
{
    QString m = !args.isEmpty() && args[0].isDefined() ?
		args[0].toString() : QString::null;

    return construct( GeneralError, m );
}

/*!
  \reimp
*/
QSObject QSErrorClass::cast( const QSList &args ) const
{
    // equivalent to 'new'
    return construct( args );
}


QString QSErrorClass::toString( const QSObject *obj ) const
{
    return QString::fromLatin1("Error: ") + errorMessage(obj);
}

/*!
  Construct an error object of type \a a, with text message \a msg
  with a line number information pointing to line \a ln.
*/

QSObject QSErrorClass::construct( ErrorType e,
				  const QString &msg, int ln ) const
{
#ifdef QSDEBUGGER
    QSErrorShared *error = new QSErrorShared(e, msg, ln);
    Debugger *dbg = env()->engine()->debugger();
    if (dbg) {
        error->sid = env()->engine()->debugger()->sourceId();
    }
#endif
    return QSObject(this, error);
}

ErrorType QSErrorClass::errorType(const QSObject *objPtr)
{
    Q_ASSERT(objPtr->objectType() == objPtr->objectType()->env()->errorClass());
    return ((QSErrorShared*) objPtr->shVal())->type;
}

QString QSErrorClass::errorName(const QSObject *objPtr)
{
    Q_ASSERT(objPtr->objectType() == objPtr->objectType()->env()->errorClass());
    Q_ASSERT(((QSErrorShared*) objPtr->shVal())->type >= 0);
    Q_ASSERT((((QSErrorShared*) objPtr->shVal()))->type <= 8);
    return QString::fromLatin1(errNames[((QSErrorShared*) objPtr->shVal())->type]);
}

QString QSErrorClass::errorMessage(const QSObject *objPtr)
{
    Q_ASSERT(objPtr->objectType() == objPtr->objectType()->env()->errorClass());
    return ((QSErrorShared*) objPtr->shVal())->message;
}

int QSErrorClass::errorLine(const QSObject *objPtr)
{
    Q_ASSERT(objPtr->objectType() == objPtr->objectType()->env()->errorClass());
    return ((QSErrorShared*) objPtr->shVal())->line;
}

void QSErrorClass::setErrorLine(QSObject *objPtr, int line)
{
    Q_ASSERT(objPtr->objectType() == objPtr->objectType()->env()->errorClass());
    ((QSErrorShared*) objPtr->shVal())->line = line;
}

#ifdef QSDEBUGGER
int QSErrorClass::errorSourceId(const QSObject *objPtr)
{
    Q_ASSERT(objPtr->objectType() == objPtr->objectType()->env()->errorClass());
    return ((QSErrorShared*) objPtr->shVal())->sid;
}
#endif
