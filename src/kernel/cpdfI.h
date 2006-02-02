/*
 * =====================================================================================
 *        Filename:  cpdfI.h
 *     Description: CPdf class implementation.
 *         Created:  01/28/2006 03:48:14 AM CET
 *          Author:  jmisutka (), 
 *         Changes: 2006/1/28 added mapping support
 *         			2006/1/30 tested objToString () -- ok
 * =====================================================================================
 */

// debug
#include "debug.h"


// =====================================================================================
namespace pdfobjects
{

//
//
//
CPdf::CPdf () : CDict(NULL,sPdf)
{
	printDbg (0,"CPdf constructor.");
}

		
//
// 
//
IProperty*
CPdf::getExistingProperty (const IndiRef* ref) const
{
	assert (NULL != ref);
	assert (0 < ref->num);
	printDbg (0,"getExistingProperty(" << ref->num << "," <<  ref->gen << ");");

	// find the key, if it exists
	Mapping::const_iterator it = mapping.find (*ref);
	return (it != mapping.end()) ? const_cast<IProperty*>((*it).second) : NULL ;
}

//
//
//
void
CPdf::setPropertyMapping (const IndiRef* ref, const IProperty* ip)
{
	assert (NULL != ref);
	assert (0 < ref->num);
	assert (NULL != ip);
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL == getExistingProperty(ref));
	printDbg (0,"setPropertyMapping();");
	
	mapping [*ref] = ip;
}

//
//
//
void 
CPdf::objToString (Object* /*obj*/,string& /*str*/)
{
/*  assert (NULL != obj);
  printDbg (0,"objToString(" << (unsigned int)obj << ")");
  printDbg (0,"\tobjType = " << obj->getTypeName() );

  ostringstream oss;
  Object o;
  int i;

  switch (obj->getType()) 
  {
  
  case objBool:
    oss << ((obj->getBool()) ? "true" : "false");
    break;
  
  case objInt:
    oss << obj->getInt();
    break;
  
  case objReal:
    oss << obj->getReal ();
    break;
  
  case objString:
    oss << "("  << obj->getString()->getCString() << ")";
    break;
  
  case objName:
    oss << "/" << obj->getName();
    break;
  
  case objNull:
    oss << "null";
    break;
  
  case objArray:
    oss << "[";
	for (i = 0; i < obj->arrayGetLength(); ++i) 
	{
      if (i > 0)
		oss << " ";
      obj->arrayGetNF (i,&o);
      string tmp;
	  CPdf::objToString (&o,tmp);
	  oss << tmp;
      o.free();
    }
    oss << "]";
    break;
  
  case objDict:
    oss << "<<";
    for (i = 0; i <obj-> dictGetLength(); ++i) 
	{
      oss << " /" << obj->dictGetKey(i) << " ";
      obj->dictGetValNF(i, &o);
	  string tmp;
	  CPdf::objToString (&o,tmp);
	  oss << tmp;
      o.free();
    }
    oss << " >>";
    break;
  
  case objStream:
    oss << "<stream>";
    break;
  
  case objRef:
    oss << obj->getRefNum() << " " << obj->getRefGen() << " R";
    break;
  
  case objCmd:
  case objError:
  case objEOF:
  case objNone:
  default:
	assert (false);	
    break;
  }

  // convert oss to string
  str = oss.str ();
*/
}





} // namespace pdfobjects
