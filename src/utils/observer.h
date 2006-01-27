#ifndef _OBSERVER_H_
#define _OBSERVER_H_

#include "kernel/object.h"

/**
 * @file observer.h
 *
 * File with observer interfaces.
 */

/** Interface for observer.
 *
 * Implementator should hanadle change of the property value in 
 * notify method.
 */
class IObserver
{
public:
        /** Notify method.
         * @param prop Property where the chanche has been done.
         *
         * This method is called by property which value has been
         * changed (changed property is given as parameter).
         * <br>
         * To be able to notify about changes, observer has to be
         * registrated on property. To do so, use IProperty::registerObserver 
         * method.
         *
         * @see IProperty
         */
        void notify(IProperty * prop)=0;
}

#endif
