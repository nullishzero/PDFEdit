//========================================================================
//
// Array.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
// Changes:
// Michal Hocko   
// 	- public clone method for deep copy of Array
//	- const where possible 
//
//========================================================================

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <stddef.h>
#include "goo/gmem.h"
#include "xpdf/Object.h"
#include "xpdf/Array.h"

//------------------------------------------------------------------------
// Array
//------------------------------------------------------------------------

Array::Array(const XRef *xrefA) {
  xref = xrefA;
  elems = NULL;
  size = length = 0;
  ref = 1;
}

Array::~Array() {
  int i;

  for (i = 0; i < length; ++i)
    elems[i].free();
  gfree(elems);
}

/** Deep copier.
 *
 * Creates new Array instance with the same (deep) content.
 * Uses clone method to each element.
 */
Array * Array::clone()const
{
   Array * result=new Array(xref);

   // initialize 
   result->size=size;
   result->length=length;
   result->elems=(Object *)gmallocn(size, sizeof(Object));
   for(int i=0; i < length; i++)
   {
      // creates clone because elems[i] may keep value as pointer
      // (e. g. objDict, objArray ...)
      Object * clone=elems[i].clone();
      
      // copies just elements with no = operator
      result->elems[i]=*clone;

      // no destruct for Object available so internal pointers
      // are kept and result->elems[i] is not affected
      gfree(clone);
   }

   return result;
}

void Array::add(const Object *elem) {
  if (length == size) {
    if (length == 0) {
      size = 8;
    } else {
      size *= 2;
    }
    elems = (Object *)greallocn(elems, size, sizeof(Object));
  }
  elems[length] = *elem;
  ++length;
}

Object *Array::get(int i, Object *obj)const {
  if (i < 0 || i >= length) {
#ifdef DEBUG_MEM
    abort();
#else
    return obj->initNull();
#endif
  }
  return elems[i].fetch(xref, obj);
}

Object *Array::getNF(int i, Object *obj)const {
  if (i < 0 || i >= length) {
#ifdef DEBUG_MEM
    abort();
#else
    return obj->initNull();
#endif
  }
  return elems[i].copy(obj);
}
