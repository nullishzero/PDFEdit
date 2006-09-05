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

#ifndef QUICKCOLOROBJECT_H
#define QUICKCOLOROBJECT_H

#include "qsclass.h"

class QColor;
class QPalette;
class QColorGroup;

class QUICKCORE_EXPORT QSColorClass : public QSSharedClass {
public:
    QSColorClass( QSClass *b );
    QString name() const { return QString::fromLatin1("Color"); }

    virtual QSObject fetchValue( const QSObject *objPtr, const QSMember &mem ) const;
    virtual void write( QSObject *objPtr, const QSMember &mem, const QSObject &val ) const;

    double toNumber( const QSObject * ) const;

    QString toString( const QSObject * ) const;
    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;
    QString debugString( const QSObject *obj ) const;

    QSObject construct( const QSList &args ) const;
    QSObject construct( const QColor &c ) const;

    static QColor *color( const QSObject *obj );

    static void setRgb( QSEnv *env );
    static QSObject light( QSEnv *env );
    static QSObject dark( QSEnv *env );
};

class QUICKCORE_EXPORT QSPaletteClass : public QSSharedClass
{
public:
    enum Properties { Disabled, Active, Inactive };
    QSPaletteClass(QSClass *b);
    QString name() const { return QString::fromLatin1("Palette"); }

    virtual QSObject fetchValue(const QSObject *objPtr, const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem, const QSObject &val) const;

    QSObject construct(const QSList &args) const;
    QSObject construct(const QPalette &p) const;

    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;

    static QPalette *palette(const QSObject *obj);
};

class QUICKCORE_EXPORT QSColorGroupClass : public QSSharedClass
{
public:
    QSColorGroupClass(QSClass *b);
    QString name() const { return QString::fromLatin1("ColorGroup"); }

    virtual QSObject fetchValue(const QSObject *objPtr, const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem, const QSObject &val) const;

    QSObject construct(const QSList &args) const;
    QSObject construct(const QColorGroup &cg) const;

    QVariant toVariant( const QSObject *obj, QVariant::Type ) const;

    static QColorGroup *colorGroup(const QSObject *obj);
};

#endif
