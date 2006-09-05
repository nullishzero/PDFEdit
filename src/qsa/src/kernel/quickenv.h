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

#ifndef QUICKENV_H
#define QUICKENV_H

#include "dlldefs.h"

class QuickInterpreter;
class QSEnv;
class QSWrapperClass;
class QSPointerClass;
class QSVariantClass;
class QSMapClass;
class QSPointClass;
class QSSizeClass;
class QSRectClass;
class QSColorClass;
class QSFontClass;
class QSByteArrayClass;
class QSPixmapClass;
class QSApplicationClass;

// to be used with multiple inheritance of convenient
// QuickInterpreter access
class QUICKCORE_EXPORT QuickEnvClass {
public:
    QuickEnvClass( QSEnv *e );
    QuickEnvClass( QuickInterpreter *i ) : ip( i ) { }

    QuickInterpreter *interpreter() const { return ip; }
    static QuickInterpreter *interpreter( QSEnv *e );

    QSPointClass *pointClass() const;
    QSSizeClass *sizeClass() const;
    QSRectClass *rectClass() const;
    QSByteArrayClass *byteArrayClass() const;
    QSPixmapClass *pixmapClass() const;

private:
    QuickInterpreter *ip;
};

#endif
