/** 
 * =====================================================================================
 *        Filename:  cobjectI.h
 *     Description:  Header file containing implementation of IProperty class.
 *         Created:  02/02/2006 
 *        Revision:  none
 *          Author:  jmisutka (06/01/19), 
 * 			
 * 			
 * =====================================================================================
 */

#ifndef COBJECTI_H
#define COBJECTI_H



#include "cpdf.h"

//=====================================================================================

namespace pdfobjects 
{


template<PropertyType Tp>
CObject<Tp>::CObject (CPdf* p,SpecialObjectType objTp) : specialObjectType(objTp),pdf(p)
{
	STATIC_CHECK (pOther != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther1 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther2 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther3 != Tp,COBJECT_BAD_TYPE);
	assert ((NULL != p) || (sPdf == objTp));
	printDbg (0,"CObject constructor.");
		
	//
	// If CPdf is beeing created, we don't have valid p
	// 
	if (sPdf == objTp)
	{
		assert (NULL == p);
		pdf = dynamic_cast<CPdf*>(this);
	}
	
	//Ref ref;
	// Create the object
	//IProperty::obj = pdf->getXref()->createObject((ObjType)Tp,&ref);
	//IProperty::setIndiRef (&ref);
	
	//
	// Save the mapping, because we have created new indirect object
	// 
	//pdf->setPropertyMapping (IProperty::getIndiRef(),this);
};


template<PropertyType Tp>
CObject<Tp>::CObject (CPdf* p, Object* o, IndiRef* ref, SpecialObjectType objTp) 
						:  IProperty(o),specialObjectType(objTp),pdf(p)
{
	STATIC_CHECK (pOther != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther1 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther2 != Tp,COBJECT_BAD_TYPE);
	STATIC_CHECK (pOther3 != Tp,COBJECT_BAD_TYPE);
	assert (sPdf != objTp);
	assert (NULL != p);
	assert (NULL != o);
	printDbg (0,"CObject constructor.");
	
	if (NULL != ref)
		// Save the mapping
		pdf->setPropertyMapping (IProperty::getIndiRef(),this);
};



//
// Better reimplement Object::print() because of
// the File parameter
//  
template<PropertyType Tp>
void
CObject<Tp>::getStringRepresentation (string& str) const
{
  assert (NULL != IProperty::obj);
  assert (IProperty::obj->getType() == (ObjType)Tp);
  
  CPdf::objToString (IProperty::obj,str);
};



//
// Notify all observers that this object has changed
//
template<PropertyType Tp>
void
CObject<Tp>::notifyObservers ()
{
  ObserverList::iterator it = IProperty::observers.begin ();

  for (; it != IProperty::observers.end(); it++)
      (*it)->notify (this);
};

/*
template<> class PropertyTrait<pNull>	{public: typedef NullType 		writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pBool>	{public: typedef const bool		writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pInt>	{public: typedef const int		writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pReal>	{public: typedef const double 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pString> {public: typedef const string 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pName>	{public: typedef const string 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pArray>	{public: typedef const string 	writeType; typedef const unsigned int 	PropertyId;};
template<> class PropertyTrait<pStream> {public: typedef const string 	writeType; typedef const NullType 	PropertyId;};
template<> class PropertyTrait<pDict>	{public: typedef const string 	writeType; typedef const string 	PropertyId;};
template<> class PropertyTrait<pRef> 	{public: typedef const string	writeType; typedef const NullType	PropertyId;};
*/
//
// Generic implementation. Equal to setStringRepresentation ()
//
/*template<PropertyType Tp>
void 
CObject<Tp>::writeValue (WriteType* val)
{
	setStringRepresentation (*val);
};
*/

template<>
void
CObject<pBool>::writeValue (WriteType* /*val*/)
{
		
};



}; /* namespace pdfobjects */


#endif // COBJECTI_H
