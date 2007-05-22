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

#include "quickenv.h"
#include "quickinterpreter.h"

#include "quickcoordobjects.h"
#ifndef QSA_NO_GUI
#include "quickpixmapobject.h"
#endif
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
