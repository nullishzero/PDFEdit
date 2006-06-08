// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
/* 
 * =====================================================================================
 *        Filename:  cobject.h
 *     Description:  Header file containing definition of IProperty and CObject classes.
 *         Created:  01/18/2006 
 *          Author:  jmisutka (06/01/19), 
 *
 * =====================================================================================
 */
#ifndef _COBJECT_H
#define _COBJECT_H

#include "static.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================


//=====================================================================================
//	Memory checker classes -- DEBUGGING PURPOSES
//=====================================================================================

//
// Forward declaration
//
class IProperty;	

/**
 * No memory checks done.
 */
class NoMemChecker 
{public: 
	NoMemChecker () {};
	void objectCreated (IProperty*) {};
	void objectDeleted (IProperty*) {};
};

/**
 * This class stores pointer to every created class in a container. When a class is destroyed, it is removed
 * from the container.
 *
 * After the end of a program, we can count how many objects have not been released. If zero left, we know 
 * that we do not have a memory leak.
 */
class BasicMemChecker
{
public:
	typedef std::list<const IProperty*> _IPsList;

private:
	_IPsList& getList () {static _IPsList ips; return ips;};
	size_t& getMax () {static size_t mx; return mx;};

public:
	//
	BasicMemChecker () {};

	//
	//
	//
	void objectCreated (IProperty* ip)
	{
		#if MEM_CHECKER_OUTPUT
		_printHeader (std::cerr);
		std::cerr << "IProperty created.";
		_printFooter (std::cerr);
		#endif
		
		getList().push_back (ip);
		++getMax ();
	};

	//
	//
	//
	void objectDeleted (IProperty* ip)
	{
		getList().pop_back ();
		return;
		#if MEM_CHECKER_OUTPUT
		_printHeader (std::cerr);
		std::cerr << "IProperty deleted.";
		#endif

		_IPsList::iterator it = find (getList().begin(), getList().end(), ip);
		if (it != getList().end())
		{
				getList().erase (it);
		}
		else
		{
				utilsPrintDbg (debug::DBG_CRIT, "!!!!!!!!!! deleting what was not created !!!!!!!!!!");
		}
		#if MEM_CHECKER_OUTPUT
		_printFooter (std::cerr);
		#endif
	};
		
	//
	// Get living IProperty count
	//
	size_t getCount () {return getList().size (); };
	size_t getMaxCount () {return getMax(); };

private:
	void _printHeader (std::ostream& oss)
	{
		oss << std::setw (10) << std::setfill ('<') << "\t";
		oss << std::setbase (16);
	}

	void _printFooter (std::ostream& oss)
	{
		oss << std::setbase (10);
		oss << "\t" << std::setw (10) << std::setfill ('>') << "" << std::endl;
	}
	
};

//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================




//=====================================================================================
// CObjectSimple
//=====================================================================================
#include "cobjectsimple.h"

//=====================================================================================
// CObjectComplex
//=====================================================================================
#include "cobjectcomplex.h"

//=====================================================================================
// CObjectStream
//=====================================================================================
#include "cobjectstream.h"



#endif // _COBJECT_H

