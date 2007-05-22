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

#ifndef QUICKCOORDOBJECTS_H
#define QUICKCOORDOBJECTS_H

#include "qsclass.h"
#include "quickenv.h"

class QPoint;
class QSize;
class QRect;

class QSPointClass : public QSSharedClass, public QuickEnvClass 
{
public:
    QSPointClass(QSClass *b, QuickInterpreter *i);
    QString name() const { return QString::fromLatin1("Point"); }

    virtual QSObject fetchValue(const QSObject *o,
				 const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem,
			const QSObject &val) const;

    QString toString(const QSObject *) const;
    QVariant toVariant(const QSObject *obj, QVariant::Type) const;

    QSObject construct(const QSList &args) const;
    QSObject construct(const QPoint &p) const;

    QPoint *point(const QSObject *obj) const;

protected:
    virtual QSEqualsResult isEqual(const QSObject &a, const QSObject &b) const;
};

class QSSizeClass : public QSSharedClass, public QuickEnvClass 
{
public:
    QSSizeClass(QSClass *b, QuickInterpreter *i);
    QString name() const { return QString::fromLatin1("Size"); }

    virtual QSObject fetchValue(const QSObject *o,
				 const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem,
			const QSObject &val) const;

/*     virtual QSObject invoke(QSObject * objPtr, const QSMember &mem, */
/* 			     const QSList &args) const; */


    QString toString(const QSObject *) const;
    QVariant toVariant(const QSObject *obj, QVariant::Type) const;

    QSObject construct(const QSList &args) const;
    QSObject construct(const QSize &s) const;

    QSize *size(const QSObject *obj) const;
};

class QSRectClass : public QSSharedClass, public QuickEnvClass 
{
public:
    QSRectClass(QSClass *b, QuickInterpreter *i);
    QString name() const { return QString::fromLatin1("Rect"); }

    virtual QSObject fetchValue(const QSObject *o,
				 const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem,
			const QSObject &val) const;

    QString toString(const QSObject *) const;
    QVariant toVariant(const QSObject *obj, QVariant::Type) const;

    QSObject construct(const QSList &args) const;
    QSObject construct(const QRect &r) const;

    QRect *rect(const QSObject *obj) const;

    static QSObject isNull(QSEnv *env);
    static QSObject isEmpty(QSEnv *env);
    static QSObject contains(QSEnv *env);
    static QSObject intersection(QSEnv *env);
    static QSObject union_(QSEnv *env);
    static QSObject intersects(QSEnv *env);
    static QSObject normalize(QSEnv *env);
    static void moveLeft(QSEnv *env);
    static void moveRight(QSEnv *env);
    static void moveTop(QSEnv *env);
    static void moveBottom(QSEnv *env);
    static void moveBy(QSEnv *env);

protected:
    virtual QSEqualsResult isEqual(const QSObject &a, const QSObject &b) const;

private:
    static QRect *rect(QSEnv *e);
};

#endif
