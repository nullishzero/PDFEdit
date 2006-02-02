/*
 * =====================================================================================
 *        Filename:  observer.h
 *     Description:  Observer interface
 *         Created:  01/29/2006 07:25:38 PM CET
 *          Author:  
 * =====================================================================================
 */

#ifndef OBSERVER_H
#define OBSERVER_H

#include "iproperty.h"


//=====================================================================================
namespace pdfobjects
{

/** 
 * Observer interface.
 * When a property is changed, observer will be 
 * notified by calling the notify method.
 */
class IObserver
{
public:
        /** 
		 * Notify method.
         * Property, which was changed, calls this method.
         * To be able to notify about changes, observer has to be
         * registered. To do so, use IProperty::registerObserver 
         * method.
         * @see IProperty
		 * 
         * @param prop Changed property.
         */
        void notify (IProperty* /*prop*/) {};//= 0;
};


} // namespace pdfobjects




#endif  // OBSERVER_H
