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

#ifndef QSMATH_OBJECT_H
#define QSMATH_OBJECT_H

#include "qsclass.h"

class QSMathClass : public QSClass
{
public:
    QSMathClass(QSClass *b);
    ~QSMathClass();

    QString name() const { return QString::fromLatin1("Math"); }

    static QSObject qs_abs(QSEnv *);
    static QSObject qs_acos(QSEnv *);
    static QSObject qs_asin(QSEnv *);
    static QSObject qs_atan(QSEnv *);
    static QSObject qs_atan2(QSEnv *);
    static QSObject qs_ceil(QSEnv *);
    static QSObject qs_cos(QSEnv *);
    static QSObject qs_exp(QSEnv *);
    static QSObject qs_floor(QSEnv *);
    static QSObject qs_log(QSEnv *);
    static QSObject qs_max(QSEnv *);
    static QSObject qs_min(QSEnv *);
    static QSObject qs_pow(QSEnv *);
    static QSObject qs_random(QSEnv *);
    static QSObject qs_round(QSEnv *);
    static QSObject qs_sin(QSEnv *);
    static QSObject qs_sqrt(QSEnv *);
    static QSObject qs_tan(QSEnv *);
};

#endif
