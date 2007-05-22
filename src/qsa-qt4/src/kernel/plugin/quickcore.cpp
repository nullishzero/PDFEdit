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
#include "quickcore.h"
#include "quickeventinterfaceimpl.h"
#include "quicklanguageinterfaceimpl.h"
#include "quickinterpreterinterfaceimpl.h"

QuickCoreInterfaceImpl::QuickCoreInterfaceImpl()
    : QComponentInterface(), ref( 0 )
{
    eventIface = new QuickEventInterfaceImpl;
    eventIface->addRef();
    langIface = new QuickLanguageInterfaceImpl;
    langIface->addRef();
    interpIface = new QuickInterpreterInterfaceImpl;
    interpIface->addRef();
}

QuickCoreInterfaceImpl::~QuickCoreInterfaceImpl()
{
    eventIface->release();
    langIface->release();
    interpIface->release();
}

QRESULT QuickCoreInterfaceImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( uuid == IID_QUnknownInterface )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QComponentInterface )
	*iface = (QComponentInterface*)this;
    else if ( uuid == IID_EventInterface )
	*iface = eventIface;
    else if ( uuid == IID_LanguageInterface )
	*iface = langIface;
    else if ( uuid == IID_InterpreterInterface )
	*iface = interpIface;

    if ( *iface )
	(*iface)->addRef();

    return;
}

unsigned long QuickCoreInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long QuickCoreInterfaceImpl::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( QuickCoreInterfaceImpl )
}

