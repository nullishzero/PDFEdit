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

#ifndef QSMATH_OBJECT_H
#define QSMATH_OBJECT_H

#include "qsclass.h"

class QSMathClass : public QSClass {
public:
    QSMathClass( QSClass *b );
    ~QSMathClass();

    QString name() const { return QString::fromLatin1("Math"); }

    static QSObject abs( QSEnv * );
    static QSObject acos( QSEnv * );
    static QSObject asin( QSEnv * );
    static QSObject atan( QSEnv * );
    static QSObject atan2( QSEnv * );
    static QSObject ceil( QSEnv * );
    static QSObject cos( QSEnv * );
    static QSObject exp( QSEnv * );
    static QSObject floor( QSEnv * );
    static QSObject log( QSEnv * );
    static QSObject max( QSEnv * );
    static QSObject min( QSEnv * );
    static QSObject pow( QSEnv * );
    static QSObject random( QSEnv * );
    static QSObject round( QSEnv * );
    static QSObject sin( QSEnv * );
    static QSObject sqrt( QSEnv * );
    static QSObject tan( QSEnv * );
};

#endif
