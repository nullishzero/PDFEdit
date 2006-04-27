/*
 * $RCSfile$
 *
 * $log: $
 *
 */

//========================================================================
//
// Dict.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
// Changes: 
// Michal Hocko   - public clone method for deep copy of Dict
//                - public update method for updating value without any 
//                  fetching or copying
//                - DictEntry changed, value is pointer
//                - all methods which doesn't store key are const char *
//                  instead of char *
//
//========================================================================

#ifndef DICT_H
#define DICT_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"

//------------------------------------------------------------------------
// Dict
//------------------------------------------------------------------------

struct DictEntry {
  char *key;
  Object *val;
};

class Dict {
public:

  // Constructor.
  Dict(XRef *xrefA);

  // Destructor.
  ~Dict();

  // deep copier
  Dict * clone()const;
  
  // Reference counting.
  int incRef() { return ++ref; }
  int decRef() { return --ref; }

  // Get number of entries.
  int getLength() { return length; }
  
  /* Updates value with given key.
   * If find with given key returns proper DictEntry,
   * sets new value and returns old one. Otherwise
   * calls add method.
   */
  Object * update(char * key, Object * val);

  // Add an entry.  NB: does not copy key.
  // uses shallow copy on val (Object doesn't have explicit copy constructore)
  void add(char *key, Object *val);

  // Check if dictionary is of specified type.
  GBool is(const char *type);

  // Look up an entry and return the value.  Returns a null object
  // if <key> is not in the dictionary.
  Object *lookup(const char *key, Object *obj);
  Object *lookupNF(const char *key, Object *obj);

  // Iterative accessors.
  char *getKey(int i);
  Object *getVal(int i, Object *obj);
  Object *getValNF(int i, Object *obj);

  // Set the xref pointer.  This is only used in one special case: the
  // trailer dictionary, which is read before the xref table is
  // parsed.
  void setXRef(XRef *xrefA) { xref = xrefA; }

private:

  XRef *xref;			// the xref table for this PDF file
  DictEntry *entries;		// array of entries
  int size;			// size of <entries> array
  int length;			// number of entries in dictionary
  int ref;			// reference count

  DictEntry *find(const char *key);
};

#endif
