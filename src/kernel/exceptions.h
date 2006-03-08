/*
 * =====================================================================================
 *        Filename:  exceptions.h
 *     Description:  Exceptions.
 *         Created:  01/29/2006 08:06:57 PM CET
 *          Author:  jmisutka ()
 * =====================================================================================
 */

#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

// stl
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

struct ObjInvalidObject : std::exception
{
	char const* what() const throw() {return "(x)pdf object is invalid.";}
};

struct ObjInvalidPositionInComplex : std::exception
{
	char const* what() const throw() {return "(x)pdf object position is invalid.";}
};

struct ObjInvalidOperation : std::exception
{
	char const* what() const throw() {return "Invalid operation.";}
};





#endif // _EXCEPTIONS_H
