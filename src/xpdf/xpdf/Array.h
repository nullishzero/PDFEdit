//========================================================================
//
// Array.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
// Changes: 
// Michal Hocko   
// 	- public clone method for deep copy of Array
// 	- getXRef added
// 	- const wehere possible
//
//========================================================================

#ifndef ARRAY_H
#define ARRAY_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "xpdf/Object.h"

class XRef;

//------------------------------------------------------------------------
// Array
//------------------------------------------------------------------------

class Array {
public:

  // Constructor.
  Array(const XRef *xrefA);

  // Destructor.
  ~Array();

  Array * clone()const;
  
  // Reference counting.
  int incRef() { return ++ref; }
  int decRef() { return --ref; }

  // Get number of elements.
  int getLength()const { return length; }

  // Add an element.
  void add(const Object *elem);

  // Accessors.
  Object *get(int i, Object *obj)const;
  Object *getNF(int i, Object *obj)const;

  const XRef *getXRef()const {return xref;}

private:

  const XRef *xref;		// the xref table for this PDF file
  Object *elems;		// array of elements
  int size;			// size of <elems> array
  int length;			// number of elements in array
  int ref;			// reference count
};

#endif
