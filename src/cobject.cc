/**
 * =====================================================================================
 * 
 *        Filename:  cobject.cc
 * 
 *     Description:  cobjects and iproperty implementation
 * 
 *         Version:  1.0
 *         Created:  01/19/2006 10:09:13 PM CET
 *        Revision:  none
 * 
 *          Author:  jmisutka (06/01/19), 
 * 
 * =====================================================================================
 */

#include "cobject.h"



/* ===================================================================================== */
//
// IProperty implemetation
//

//
//
//
template <typename T>
void IProperty::getAllPropertyNames (T& container) const
{
	  //
	  // add names to the container with function e.g. T::push_back
	  //
	 /**/ container.push_back ("name1");
	  container.push_back ("name2");
	  container.push_back ("name3");
	  container.push_back ("name4");
 
}

//
//
//
template <typename T>
T IProperty::getPropertyValue (const PropertyName& /*name*/) const
{
	  /**/return "value";
};

//
//
//
PropertyCount getPropertyCount ()
{
		return 0;
}











