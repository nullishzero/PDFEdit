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

#ifndef QSREFERENCE_H
#define QSREFERENCE_H

#include "qsobject.h"
#include "qsclass.h"

class QSReference {
public:
    QSReference() : cls(0) { }
    QSReference( const QSObject &b, const QSMember &m, const QSClass *c )
	: bs( b ), mem( m ), cls(c) { }
    QSReference( const QSObject &b ) : bs( b ), cls(0) { }

    bool isWritable() const { return isReference() && mem.isWritable(); }
    bool isDefined() const { return isReference() && mem.isDefined(); }
    bool isReference() const { return cls!=0; }
    QSObject base() const { return bs; }
    QSMember member() const { return mem; }
//     void setBase( const QSObject &b ) { bs = b; }
    void setIdentifier( const QString &i ) { ident = i; }
    QString identifier() const { return ident; }
    QSObject dereference() const;
    void assign( const QSObject &o );
    bool deleteProperty();

private:
    QSObject bs;
    QSMember mem;
    const QSClass *cls;
    QString ident;
};

#endif
