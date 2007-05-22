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
#ifndef QSSLOT_H
#define QSSLOT_H

#include <QByteArray>
#include <QVector>
#include <QMetaType>

class QObject;

typedef void (*PtrCallbackForSlot)(QObject *_this, int callId, int paramCount, const QMetaType::Type *types, const QByteArray *typeNames, void *argumentList[]);

struct QSSlot
{
    QVector<QMetaType::Type> argumentTypes;
    QVector<QByteArray> argumentTypeNames;
    PtrCallbackForSlot callbackFunc;
    QObject *receiver;
    int callId;

    inline QSSlot(const QVector<QMetaType::Type> &types = QVector<QMetaType::Type>(),
                  const QVector<QByteArray> &typeNames = QVector<QByteArray>(),
                  int callId = -1, PtrCallbackForSlot callbackFunc = 0,
                  QObject *receiver = 0);

    inline void call(void *arguments[]) const;
};

inline QSSlot::QSSlot(const QVector<QMetaType::Type> &types,
                      const QVector<QByteArray> &typeNames, int id,
                      PtrCallbackForSlot func, QObject *obj)
    : argumentTypes(types),
      argumentTypeNames(typeNames),
      callbackFunc(func),
      receiver(obj),
      callId(id)
{ }

inline void QSSlot::call(void *arguments[]) const
{
    callbackFunc(receiver, callId, argumentTypes.size(),
                 argumentTypes.constData(),
                 argumentTypeNames.constData(), arguments);
}


#endif // QSSLOT_H


