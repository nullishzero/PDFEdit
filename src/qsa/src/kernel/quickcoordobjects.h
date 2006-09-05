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

#ifndef QUICKCOORDOBJECTS_H
#define QUICKCOORDOBJECTS_H

#include "qsclass.h"
#include "quickenv.h"

class QPoint;
class QSize;
class QRect;

class QUICKCORE_EXPORT QSPointClass : public QSSharedClass, public QuickEnvClass {
public:
    QSPointClass( QSClass *b, QuickInterpreter *i );
    QString name() const { return QString::fromLatin1("Point"); }

    virtual QSObject fetchValue( const QSObject *o,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;

    QString toString( const QSObject * ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;
    QString debugString( const QSObject *obj ) const;

    QSObject construct( const QSList &args ) const;
    QSObject construct( const QPoint &p ) const;

    QPoint *point( const QSObject *obj ) const;

protected:
    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;
};

class QUICKCORE_EXPORT QSSizeClass : public QSSharedClass, public QuickEnvClass {
public:
    QSSizeClass( QSClass *b, QuickInterpreter *i );
    QString name() const { return QString::fromLatin1("Size"); }

    virtual QSObject fetchValue( const QSObject *o,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;

/*     virtual QSObject invoke( QSObject * objPtr, const QSMember &mem, */
/* 			     const QSList &args ) const; */


    QString toString( const QSObject * ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;
    QString debugString( const QSObject *obj ) const;

    QSObject construct( const QSList &args ) const;
    QSObject construct( const QSize &s ) const;

    QSize *size( const QSObject *obj ) const;
};

class  QUICKCORE_EXPORT QSRectClass : public QSSharedClass, public QuickEnvClass {
public:
    QSRectClass( QSClass *b, QuickInterpreter *i );
    QString name() const { return QString::fromLatin1("Rect"); }

    virtual QSObject fetchValue( const QSObject *o,
				 const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem,
			const QSObject &val ) const;

    QString toString( const QSObject * ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;
    QString debugString( const QSObject *obj ) const;

    QSObject construct( const QSList &args ) const;
    QSObject construct( const QRect &r ) const;

    QRect *rect( const QSObject *obj ) const;

    static QSObject isNull( QSEnv *env );
    static QSObject isEmpty( QSEnv *env );
    static QSObject contains( QSEnv *env );
    static QSObject intersection( QSEnv *env );
    static QSObject union_( QSEnv *env );
    static QSObject intersects( QSEnv *env );
    static QSObject normalize( QSEnv *env );
    static void moveLeft( QSEnv *env );
    static void moveRight( QSEnv *env );
    static void moveTop( QSEnv *env );
    static void moveBottom( QSEnv *env );
    static void moveBy( QSEnv *env );

protected:
    virtual QSEqualsResult isEqual( const QSObject &a, const QSObject &b ) const;

private:
    static QRect *rect( QSEnv *e );
};

#endif
