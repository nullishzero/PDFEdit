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

#ifndef QSOPERATIONS_H
#define QSOPERATIONS_H

#include "qsobject.h"

namespace QS {

    /**
     * @return True if d is not a number (platform support required).
     */
    bool isNaN( double d );

    /**
     * @return True if d is infinite (platform support required).
     */
    bool isInf( double d );

    /**
     * This operator performs an abstract relational comparision of the two
     * arguments that can be of arbitrary type. If possible, conversions to the
     * string or number type will take place before the comparison.
     *
     * @return 1 if v1 is "less-than" v2, 0 if the relation is "greater-than-or-
     * equal". -1 if the result is undefined.
     */
    double max( double d1, double d2 );
    double min( double d1, double d2 );

    /**
     * Additive operator. Either performs an addition or substraction of v1
     * and v2.
     * @param oper '+' or '-' for an addition or substraction, respectively.
     * @return The result of the operation.
     */
    QSObject add( const QSEnv *env, const QSObject &v1, const QSObject &v2, char oper );

    /**
     * Multiplicative operator. Either multiplies/divides v1 and v2 or
     * calculates the remainder from an division.
     * @param oper '*', '/' or '%' for a multiplication, division or
     * modulo operation.
     * @return The result of the operation.
     */
    QSObject mult( const QSEnv *env, const QSObject &v1, const QSObject &v2, char oper );
};

#endif
