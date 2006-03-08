/*
 * =====================================================================================
 *        Filename:  cpdf.cc
 *     Description: CPdf class implementation.
 *         Created:  01/28/2006 03:48:14 AM CET
 *          Author:  jmisutka (), 
 *         Changes: 2006/1/28 added mapping support
 *         			2006/1/30 tested objToString () -- ok
 *         			2006/2/8  after a long battle against g++ and ld + collect I made
 *         					  the implementation of CPdf a .cc file
 * =====================================================================================
 */

#include "static.h"

#include "iproperty.h"
#include "cobject.h"
#include "cpdf.h"


// =====================================================================================

using namespace pdfobjects;

namespace 
{
		/**
		 * Get all child IProperty that you can get to from a root IProperty. 
		 * It is not sorted and the position does not depend on the position in the tree.
		 *
		 * @param ip Root object.
		 */
		template<typename Storage>
		void
		getAllChildIProperty (const IProperty& ip, Storage& store) 
		{
				// Create a temporary storage
				Storage tmp;
				
				switch (ip.getType())
				{
					case pArray:
						ip.getCObjectPtr<const CArray>()->_getAllChildObjects (tmp);
						break;
									
					case pDict:
						ip.getCObjectPtr<const CDict>()->_getAllChildObjects (tmp);
						break;

					case pStream:
						assert (!"I'm not implemented yet...");
						break;

					default:	// Null, Bool, Int, Real, String, Name
						break;
				}	
				
				// Recursively get all objects
				typename Storage::iterator it = tmp.begin ();
				for (; it != tmp.end (); it++)
				{
					assert (NULL != *it);
					store.push_back (*it);
					getAllChildIProperty (*(*it), store);
				}

		}
		
		
} // namespace
		




/* TODO: refactor
 * doesn't check leaf because those are real objects. Just checks for all kids 
 * if they are Page and if so, puts reference to the array. For Pages elements 
 * we have to go deeper in recursion.
 *
 * TODO: think of IProperty interface to use to ease manipulation with Object 
 * and to prevent code duplication
 */
void CPdf::fillPages(::Dict * /*pageNode*/)
{
        /*
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
        */
}

void CPdf::initRevisionSpecific(::Dict * /*trailer*/)
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
CPdf::getExistingProperty (const IndiRef& ref) const
{
	printDbg (0,"getExistingProperty(" << ref.num << "," << ref.gen << ")");

	// find the key, if it exists
	IndirectMapping::const_iterator it = indMap.find (ref);
	return (it != indMap.end()) ? const_cast<IProperty*>((*it).second) : NULL ;
}


//
//
//
void
CPdf::setIndMapping (const IndiRef& ref, const IProperty* ip)
{
	assert (NULL != ip);
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL == getExistingProperty(ref));
	printDbg (0,"setIndMapping();");
	
	indMap [ref] = ip;
}


//
//
//
void
CPdf::delIndMapping (const IndiRef& ref)
{
	// Just check if there is no such mapping
	// if there is such mapping that is a problem
	assert (NULL != getExistingProperty(ref));
	printDbg (0,"delIndMapping();");
	
	indMap.erase (ref);
}


//
//
//
IndiRef
CPdf::addIndirectObject (IProperty& ip)
{
	assert (NULL == ip.getPdf());
	
	// Indicate that it will belong to this pdf
	ip.setPdf (this);

	// Find free ref
	IndiRef rf;	// = xref->findFreeRef ();
	
	// Find all objects in tree starting in ip
	std::list<IProperty*> ipList;
	getAllChildIProperty (ip,ipList);
	
	// Indicate that all objects in the tree starting in ip
	//   -- belong to this pdf
	//   -- are in indirect object with whose ref is rf
	std::list<IProperty*>::iterator it = ipList.begin ();
	for (; it != ipList.end (); it++)
	{
			assert (NULL == (*it)->getPdf());

			(*it)->setPdf (this);
			(*it)->setIndiRef (rf);
	}
	
	return rf;
}


CPdf * CPdf::getInstance(const char * filename, OpenMode mode)
{
       // openMode is read-only by default
       const char * openMode="r";
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

int CPdf::close(bool saveFlag)
{
        // saves if necessary
        if(saveFlag)
                save(NULL);
        
        // deletes this instance
        // all clean-up is made in destructor
        delete this;

		return 0;
}

