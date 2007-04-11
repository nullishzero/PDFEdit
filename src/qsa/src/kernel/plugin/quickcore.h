#ifndef QUICKCORE_H
#define QUICKCORE_H

#include <qcom.h>

class QuickEventInterfaceImpl;
class QuickLanguageInterfaceImpl;
class QuickInterpreterInterfaceImpl;

class QuickCoreInterfaceImpl : public QComponentInterface
{
public:
    QuickCoreInterfaceImpl();
    virtual ~QuickCoreInterfaceImpl();

    QRESULT queryInterface( const QUuid&, QUnknownInterface ** );
    unsigned long addRef();
    unsigned long release();

    QString name() const { return "Qt Script"; }
    QString description() const { return "Qt Script Core Functionality"; }
    QString version() const { return "0.1"; }
    QString author() const { return "Trolltech ASA"; }

private:
    ulong ref;
    QuickEventInterfaceImpl *eventIface;
    QuickLanguageInterfaceImpl *langIface;
    QuickInterpreterInterfaceImpl *interpIface;

};

#endif
