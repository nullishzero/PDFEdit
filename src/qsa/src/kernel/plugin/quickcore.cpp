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

