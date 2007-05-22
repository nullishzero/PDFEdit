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

#ifndef QSREGEXP_OBJECT_H
#define QSREGEXP_OBJECT_H

#include "qsclass.h"
#include <QStringList>

class QSRegExpClass : public QSWritableClass 
{
public:
    QSRegExpClass(QSClass *b);
    QString name() const { return QString::fromLatin1("RegExp"); }

    virtual QSObject fetchValue(const QSObject *objPtr,
				 const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem,
			const QSObject &val) const;


    QString toString(const QSObject *obj) const;

    QSObject construct(const QSList &args) const;
    QSObject cast(const QSList &args) const;

    static QRegExp *regExp(const QSObject *obj);

    // ECMA API
    static QSObject exec(QSEnv *env);
    static QSObject test(QSEnv *env);
    static QSObject toStringScript(QSEnv *env);
    // QRegExp API
    static QSObject search(QSEnv *env);
    static QSObject searchRev(QSEnv *env);
    static QSObject exactMatch(QSEnv *env);
    static QSObject pos(QSEnv *env);
    static QSObject cap(QSEnv *env);

    QStringList lastCaptures;

    static QString source(const QSObject *re);
    static bool isGlobal(const QSObject *re);
    static bool isIgnoreCase(const QSObject *re);

private:
    static QRegExp* regExp(QSEnv *e);
};

#endif
