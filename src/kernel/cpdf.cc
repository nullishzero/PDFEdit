/*
 * =====================================================================================
 *        Filename:  cpdf.cc
 *     Description: CPdf class implementation.
 *         Created:  01/28/2006 03:48:14 AM CET
 *          Author:  jmisutka (), 
 *         Changes: 2006/1/28 added mapping support
 *         			2006/1/30 tested objToString () -- ok
 *         			2006/2/8  after a long battle againd g++ and ld + collect I made
 *         					  the implementation of CPdf a .cc file
 * =====================================================================================
 */

// debug
#include "cpdf.h"
#include "debug.h"

//
#include "iproperty.h"
#include "cobject.h"
#include "cobjectI.h"


// =====================================================================================

namespace pdfobjects
{

/* TODO: refactor
 * doesn't check leaf because those are real objects. Just checks for all kids if they
 * are Page and if so, puts reference to the array. For Pages elements we have to go
 * deeper in recursion.
 *
 * TODO: think of IProperty interface to use to ease manipulation with Object and to
 * prevent code duplication
 */
CPdf::fillPages(::Dict * pageNode)
{
        Object type;
        bool leaf=true;
        
        pageNode->lookup("Type",&type);

        if(!type.isName())
        {
                type.free();
                // Corrupted PDF
                // TODO handle
        }

        // checks if given object is pages or page
        // if pages, then we are in tree node and so recursion
        // is required. Simple page is inserted to the pages array
        if((leaf=strcasecmp(type.getName(), "/Pages")) &&
           (strcasecmp(type.getName(), "/Page")))
        {
                // Wrong type of object
                // TODO handle
        }

        if(leaf)
                pages.push_back()
        else
        {
                // recursion for each child from kids array
                Object kids;
                pageNode->lookup("Kids", &kids);
                if(!kids.isArray())
                {
                        // Corrupted pdf
                        // TODO handle
                }
                for(int i=0; i<kids.arrayGetLength(); i++)
                {
                        Object element;
                        kids.arrayGet(i,&element);
                        if(!element.isDict())
                        {
                                // COrrupted pdf
                                // TODO handle
                        }
                        fillPages(element.getDict());
                        element.free();
                }

                kids.free();
        }

        // frees type object
        type.free();
}

void CPdf::initRevisionSpecific(::Dict * trailer)
{
        // gets catalog dictionary pointer (it is fetched, so
        // it has to be released)
        Object obj;
        xref->getCatalog(&obj);
        catalog=obj.getDict();

        // gets all pages information from page tree which is stored in 
        // document catalog - only referencies are collected
        catalog->lookup("Pages", &obj);
        if(!obj.isDict())
        {
                // pdf corrupted
                // TODO handle
        }
        ::Dict * pageTreeDict=obj.getDict();
        fillPages(pageTreeDict);
        
        // gets all outlines information
        // TODO
}

CPdf::CPdf(BaseStream * stream, FILE * file, OpenMode openMode)//:CDict(NULL, sPdf)
{
        // gets xref writer - if error occures, exception is thrown 
        xref=new XRefWriter(stream);
        pdfFile=file;
        mode=openMode;

        // initializes revision specific data for the newest revision
        initRevisionSpecific(xref->getTrailerDict()->getDict());
}

CPdf::~CPdf()
{
        // TODO implementation
}


//
// 
//
IProperty*
CPdf::getExistingProperty (const Object* o) const
{
	assert (NULL != o);
	printDbg (0,"getExistingProperty(" << (unsigned int) o << ");");

	// find the key, if it exists
	ObjectMapping::const_iterator it = objMap.find (o);
	return (it != objMap.end()) ? const_cast<IProperty*>((*it).second) : NULL ;
}

//
// 
//
IProperty*
CPdf::getExistingProperty (const IndiRef& pair) const
{
	printDbg (0,"getExistingProperty(pair);");

	// find the key, if it exists
	IndirectMapping::const_iterator it = indMap.find (pair);
	return (it != indMap.end()) ? const_cast<IProperty*>((*it).second) : NULL ;
}


//
//
//
void
CPdf::setObjectMapping (const Object* o, const IProperty* ip)
{
	assert (NULL != o);
	assert (NULL != ip);
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL == getExistingProperty(o));
	printDbg (0,"setObjectMapping();");
	
	objMap [o] = ip;
}

//
//
//
void
CPdf::setIndMapping (const IndiRef& pair, const IProperty* ip)
{
	assert (NULL != ip);
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL == getExistingProperty(pair));
	printDbg (0,"setIndMapping();");
	
	indMap [pair] = ip;
}


//
//
//
void
CPdf::delObjectMapping (const Object* o)
{
	assert (NULL != o);
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL != getExistingProperty(o));
	printDbg (0,"delPropertyMapping();");
	
	objMap.erase (o);
}


//
//
//
void
CPdf::delIndMapping (const IndiRef& pair)
{
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL != getExistingProperty(pair));
	printDbg (0,"delIndMapping();");
	
	indMap.erase (pair);
}


CPdf * CPdf::getInstance(const char * filename, OpenMode mode)
{
       // openMode is read-only by default
       char * openMode="r";
       // if mode is ReadWrite, set to read-write mode starting at the 
       // begining.
       if(mode == ReadWrite)
               openMode="r+";
       
       // opens file and creates (xpdf) FileStream
       FILE * file=fopen(filename, openMode);       
       if(!file)
       {
               // TODO some output
               // exception
       }
       Object obj;
       // NOTE: I didn't find meaning obj parameter meaning for BaseStream
       // maybe it is only bad design of BaseStream (only FilterStream uses
       // this object
       obj.initNull();
       BaseStream * stream=new FileStream(file, 0, gFalse, 0, &obj);
       if(!stream)
       {
               // TODO some output
               // exception
       }
       
       // stream is ready, creates CPdf instance
       CPdf * instance=new CPdf(stream, file, mode);
       if(!instance)
       {
               // TODO some output
               // exception
       }

       return instance;
}

int CPdf::close(bool save)
{
        // saves if necessary
        if(save)
                save(NULL);
        
        // deletes this instance
        // all clean-up is made in destructor
        delete this;
}


} // namespace pdfobjects
