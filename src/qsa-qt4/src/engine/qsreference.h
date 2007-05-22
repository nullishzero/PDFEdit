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

#ifndef QSREFERENCE_H
#define QSREFERENCE_H

#include "qsobject.h"
#include "qsclass.h"

class QSReference 
{
public:
    QSReference() : cls(0) { }
    QSReference(const QSObject &b, const QSMember &m, const QSClass *c)
	: bs(b), mem(m), cls(c) { }
    QSReference(const QSObject &b) : bs(b), cls(0) { }

    bool isWritable() const { return isReference() && mem.isWritable(); }
    bool isDefined() const { return isReference() && mem.isDefined(); }
    bool isReference() const { return cls!=0; }
    QSObject base() const { return bs; }
    QSMember member() const { return mem; }
//     void setBase(const QSObject &b) { bs = b; }
    void setIdentifier(const QString &i) { ident = i; }
    QString identifier() const { return ident; }
    QSObject dereference() const;
    void assign(const QSObject &o);
    bool deleteProperty();

private:
    QSObject bs;
    QSMember mem;
    const QSClass *cls;
    QString ident;
};

#endif
