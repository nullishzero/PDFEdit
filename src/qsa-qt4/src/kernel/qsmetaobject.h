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
#ifndef QSMETAOBJECT_H
#define QSMETAOBJECT_H

#include "qsslot.h"

#include <QObject>
#include <QHash>

class QSMetaObject: public QObject
{
    // Q_OBJECT
    virtual const QMetaObject *metaObject();
    virtual void *qt_metacast(const char *classname);
    virtual int qt_metacall(QMetaObject::Call c, int id, void **arguments);
    static const QMetaObject staticMetaObject;

public:
    QSMetaObject(QObject *parent = 0);
    virtual ~QSMetaObject();

    virtual int registerSlot(QObject *obj, const char *signature, PtrCallbackForSlot callback);
    virtual bool dynamicConnect(const QObject *sender, const char *signal, const char *method);
    virtual bool dynamicConnect(const QObject *sender, const char *signal, int slot_id);

//     virtual bool dynamicConnect(const QObject *sender, const char *signal, const char *method);
    virtual bool dynamicDisconnect(const QObject *sender, const char *signal, int slot_id);
private:
    QVector<QSSlot> m_slots;
    QHash<QByteArray, int> m_slotnames;
};

#endif // QSMETAOBJECT_H

