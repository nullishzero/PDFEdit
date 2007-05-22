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

#ifndef QUICKCOLOROBJECT_H
#define QUICKCOLOROBJECT_H

#include "qsclass.h"
#include <QPalette>

class QColor;

class QSColorClass : public QSSharedClass
{
public:
    QSColorClass(QSClass *b);
    QString name() const { return QString::fromLatin1("Color"); }

    virtual QSObject fetchValue(const QSObject *objPtr, const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem, const QSObject &val) const;

    double toNumber(const QSObject *) const;

    QString toString(const QSObject *) const;
    QVariant toVariant(const QSObject *obj, QVariant::Type) const;

    QSObject construct(const QSList &args) const;
    QSObject construct(const QColor &c) const;

    static QColor *color(const QSObject *obj);

    static void setRgb(QSEnv *env);
    static void setRgba(QSEnv *env);
    static QSObject light(QSEnv *env);
    static QSObject dark(QSEnv *env);
};

class QSPaletteClass : public QSSharedClass
{
public:
    enum Properties { Disabled, Active, Inactive };
    QSPaletteClass(QSClass *b);
    QString name() const { return QString::fromLatin1("Palette"); }

    virtual QSObject fetchValue(const QSObject *objPtr, const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem, const QSObject &val) const;

    QSObject construct(const QSList &args) const;
    QSObject construct(const QPalette &p) const;

    QVariant toVariant(const QSObject *obj, QVariant::Type) const;

    static QPalette *palette(const QSObject *obj);
};

class QSColorGroup
{
public:
    inline QSColorGroup(const QPalette &pal = QPalette(), QPalette::ColorGroup group = QPalette::Active);
    inline QColor color(QPalette::ColorRole role) const;
    inline void setColor(QPalette::ColorRole role, const QColor &color);

    void copyToPalette(QPalette *pal, QPalette::ColorGroup group) const;

private:
    QPalette palette;
};

inline QSColorGroup::QSColorGroup(const QPalette &pal, QPalette::ColorGroup group)
: palette(pal)
{
    palette.setCurrentColorGroup(group);
}

inline QColor QSColorGroup::color(QPalette::ColorRole role) const
{
    return palette.color(role);
}

inline void QSColorGroup::setColor(QPalette::ColorRole role, const QColor &color)
{
    palette.setColor(role, color);
}

class QSColorGroupClass : public QSSharedClass
{
public:
    QSColorGroupClass(QSClass *b);
    QString name() const { return QString::fromLatin1("ColorGroup"); }

    virtual QSObject fetchValue(const QSObject *objPtr, const QSMember &mem) const;
    virtual void write(QSObject *objPtr, const QSMember &mem, const QSObject &val) const;

    QSObject construct(const QSList &args) const;
    QSObject construct(const QSColorGroup &colorGroup) const;

    QVariant toVariant(const QSObject *obj, QVariant::Type) const;

    static QSColorGroup *colorGroup(const QSObject *obj);
};

#endif
