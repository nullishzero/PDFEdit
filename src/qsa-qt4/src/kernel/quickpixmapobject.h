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

#ifndef QUICKPIXMAPOBJECT_H
#define QUICKPIXMAPOBJECT_H

#include <qsclass.h>
#include "quickenv.h"
#include <QPixmap>

class QPixmap;

class QSPixmapClass : public QSSharedClass,
				      public QuickEnvClass {
public:
    QSPixmapClass(QSClass *b, QuickInterpreter *i);
    QString name() const { return QString::fromLatin1("Pixmap"); }

    virtual QSObject fetchValue(const QSObject *obj,
				 const QSMember &mem) const;

    QVariant toVariant(const QSObject *obj, QVariant::Type) const;
    QString debugString(const QSObject *obj) const;

    QSObject construct(const QSList &args) const;
    QSObject construct(const QPixmap &p) const;

    QPixmap *pixmap(const QSObject *obj) const;

    static QSObject isNull(QSEnv *env);
    static void fill(QSEnv *env);
    static void resize(QSEnv *env);
    static void load(QSEnv *env);
    static void save(QSEnv *env);
};

#endif
