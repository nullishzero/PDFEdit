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

#include "qsmath_object.h"
#include "qstypes.h"
#include "qsoperations.h"
#include "qsnumeric.h"
#include <math.h>
#include <stdlib.h>

using namespace QS;

QSMathClass::QSMathClass(QSClass *b)
    : QSClass(b, AttributeAbstract)
{
    int a = AttributePublic|AttributeStatic;
    addStaticVariableMember(QString::fromLatin1("E"), createNumber(::exp(1.0)), a);
    addStaticVariableMember(QString::fromLatin1("LN2"), createNumber(::log(2.0)), a);
    addStaticVariableMember(QString::fromLatin1("LN10"), createNumber(::log(10.0)), a);
    addStaticVariableMember(QString::fromLatin1("LOG2E"), createNumber(1.0/::log(2.0)), a);
    addStaticVariableMember(QString::fromLatin1("LOG10E"), createNumber(1.0/::log(10.0)), a);
    addStaticVariableMember(QString::fromLatin1("PI"), createNumber(2.0 * ::asin(1.0)), a);
    addStaticVariableMember(QString::fromLatin1("SQRT1_2"), createNumber(::sqrt(0.5)), a);
    addStaticVariableMember(QString::fromLatin1("SQRT2"), createNumber(::sqrt(2.0)), a);

    // static functions
    addMember(QString::fromLatin1("abs"), QSMember(&qs_abs, AttributeStatic));
    addMember(QString::fromLatin1("acos"), QSMember(&qs_acos, AttributeStatic));
    addMember(QString::fromLatin1("asin"), QSMember(&qs_asin, AttributeStatic));
    addMember(QString::fromLatin1("atan"), QSMember(&qs_atan, AttributeStatic));
    addMember(QString::fromLatin1("atan2"), QSMember(&qs_atan2, AttributeStatic));
    addMember(QString::fromLatin1("ceil"), QSMember(&qs_ceil, AttributeStatic));
    addMember(QString::fromLatin1("cos"), QSMember(&qs_cos, AttributeStatic));
    addMember(QString::fromLatin1("exp"), QSMember(&qs_exp, AttributeStatic));
    addMember(QString::fromLatin1("floor"), QSMember(&qs_floor, AttributeStatic));
    addMember(QString::fromLatin1("log"), QSMember(&qs_log, AttributeStatic));
    addMember(QString::fromLatin1("max"), QSMember(&qs_max, AttributeStatic));
    addMember(QString::fromLatin1("min"), QSMember(&qs_min, AttributeStatic));
    addMember(QString::fromLatin1("pow"), QSMember(&qs_pow, AttributeStatic));
    addMember(QString::fromLatin1("random"), QSMember(&qs_random, AttributeStatic));
    addMember(QString::fromLatin1("round"), QSMember(&qs_round, AttributeStatic));
    addMember(QString::fromLatin1("sin"), QSMember(&qs_sin, AttributeStatic));
    addMember(QString::fromLatin1("sqrt"), QSMember(&qs_sqrt, AttributeStatic));
    addMember(QString::fromLatin1("tan"), QSMember(&qs_tan, AttributeStatic));
}

QSMathClass::~QSMathClass() {

}

QSObject QSMathClass::qs_abs(QSEnv *env)
{
    double arg = env->arg(0).toNumber();
    return env->createNumber(arg < 0 ? (-arg) : arg);
}

QSObject QSMathClass::qs_acos(QSEnv *env)
{
    return env->createNumber(::acos(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_asin(QSEnv *env)
{
    return env->createNumber(::asin(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_atan(QSEnv *env)
{
    return env->createNumber(::atan(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_atan2(QSEnv *env)
{
    return env->createNumber(::atan2(env->arg(0).toNumber(), env->arg(1).toNumber()));
}

QSObject QSMathClass::qs_ceil(QSEnv *env)
{
    return env->createNumber(::ceil(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_cos(QSEnv *env)
{
    return env->createNumber(::cos(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_exp(QSEnv *env)
{
    return env->createNumber(::exp(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_floor(QSEnv *env)
{
    return env->createNumber(::floor(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_log(QSEnv *env)
{
    return env->createNumber(::log(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_max(QSEnv *env)
{
    double arg0 = env->arg(0).toNumber();
    double arg1 = env->arg(1).toNumber();
    return env->createNumber( qMax(arg0, arg1));
}

QSObject QSMathClass::qs_min(QSEnv *env)
{
    double arg0 = env->arg(0).toNumber();
    double arg1 = env->arg(1).toNumber();
    return env->createNumber( qMin(arg0, arg1));
}

QSObject QSMathClass::qs_pow(QSEnv *env)
{
    double arg0 = env->arg(0).toNumber();
    double arg1 = env->arg(1).toNumber();
    return env->createNumber(::pow(arg0, arg1));
}

QSObject QSMathClass::qs_random(QSEnv *env)
{
    return env->createNumber(::rand() / (double) RAND_MAX);
}

QSObject QSMathClass::qs_round(QSEnv *env)
{
    double arg = env->arg(0).toNumber();
    double result;
    if (qsaIsNan(arg))
	result = arg;
    else if (qsaIsInf(arg) || qsaIsInf(-arg))
	result = arg;
    else if (arg == -0.5)
	result = 0;
    else
	result = qRound(arg);

    return env->createNumber(result);
}

QSObject QSMathClass::qs_sin(QSEnv *env)
{
    return env->createNumber(::sin(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_sqrt(QSEnv *env)
{
    return env->createNumber(::sqrt(env->arg(0).toNumber()));
}

QSObject QSMathClass::qs_tan(QSEnv *env)
{
    return env->createNumber(::tan(env->arg(0).toNumber()));
}
