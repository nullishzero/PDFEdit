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

#include "quickenv.h"
#include "quickinterpreter.h"

#include "quickcoordobjects.h"
#include "quickpixmapobject.h"
#include "quickbytearrayobject.h"

QuickEnvClass::QuickEnvClass( QSEnv *e )
{
    ip = QuickInterpreter::fromEnv( e );
}

QuickInterpreter *QuickEnvClass::interpreter( QSEnv *e )
{
    Q_ASSERT( e );
    const QSClass *c = e->thisValue().objectType();
//     const QuickEnvClass *qc = dynamic_cast<const QuickEnvClass*>(c);
    // ### doesn't work properly without dynamic_cast<>
    const QuickEnvClass *qc = (const QuickEnvClass*) c;
    Q_ASSERT( qc );
    return qc->interpreter();
}

QSPointClass* QuickEnvClass::pointClass() const
{
    return interpreter()->pointClass();
}

QSSizeClass* QuickEnvClass::sizeClass() const
{
    return interpreter()->sizeClass();
}

QSRectClass *QuickEnvClass::rectClass() const
{
    return interpreter()->rectClass();
}

QSByteArrayClass *QuickEnvClass::byteArrayClass() const
{
    return interpreter()->byteArrayClass();
}

QSPixmapClass *QuickEnvClass::pixmapClass() const
{
    return interpreter()->pixmapClass();
}
