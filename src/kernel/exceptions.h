/*
 * =====================================================================================
 *        Filename:  exceptions.h
 *     Description:  Exceptions.
 *         Created:  01/29/2006 08:06:57 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>



/**
 * Exceptions occurs when (x)pdf object has incorrect type.
 */
struct ObjBadTypeE : std::exception
{
	char const* what() const throw() {return "(x)pdf object has bad type.";}
};

struct ObjBadValueE : std::exception
{
	char const* what() const throw() {return "(x)pdf object has bad value.";}
};






#endif // EXCEPTIONS_H

