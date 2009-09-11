//========================================================================
//
// Dict.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//
// Changes: 
// Michal Hocko   
// 	- public clone method for deep copy of Dict
//      - const where possible
//
//========================================================================

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include <string.h>
#include "goo/gmem.h"
#include "xpdf/Object.h"
#include "xpdf/XRef.h"
#include "xpdf/Dict.h"

//------------------------------------------------------------------------
// Dict
//------------------------------------------------------------------------

Dict::Dict(const XRef *xrefA) {
  xref = xrefA;
  entries = NULL;
  size = length = 0;
  ref = 1;
}

Dict::~Dict() {
  int i;

  for (i = 0; i < length; ++i) 
  {
    // skip non valid entries
    if(entries[i].key)
      gfree(entries[i].key);

    if(entries[i].val)
    {
      entries[i].val->free();
      gfree(entries[i].val);
    }
  }
  gfree(entries);
}


/** Deep copier.
 *
 * Creates new Dict instance with the same (deep) content.
 * Uses clone method for each element.
 */
Dict * Dict::clone()const
{
   Dict * result=new Dict(xref);

   // initializes
   result->size=size;
   result->length=length;
   result->entries=(DictEntry *)gmallocn(size, sizeof(DictEntry));
   for(int i=0; i < length; i++)
   {
      result->entries[i].key=copyString(entries[i].key);   
      result->entries[i].val=entries[i].val->clone();
   }

   return result;
}

void Dict::add(char *key, const Object *val) 
{
  int pos = length;
  
  if (length == size) {
    if (length == 0) {
      size = 8;
    } else {
      size *= 2;
    }
    entries = (DictEntry *)greallocn(entries, size, sizeof(DictEntry));
  }
   
  // when we add, length must be increased and val has to be allocated
  if(pos==length)
  {
     entries[pos].val=(Object *)gmalloc(sizeof(Object));
     ++length;
  }
  
  entries[pos].key = key;
  *(entries[pos].val) = *val;

}

Object * Dict::del(const char * key)
{
   if(!key)
      return NULL;

   for(int i=0; i < length; i++)
   {
       if(!strcmp(key, entries[i].key))
       {
           // key found
           // deallocates key, moves everything from behind and returns value
           Object * val=entries[i].val;
           gfree(entries[i].key);
           for(;i<length-1; i++)
           {
              entries[i].key=entries[i+1].key;
              entries[i].val=entries[i+1].val;
           }
           length--;
           return val;
       }
   }

   // nothing deleted
   return NULL;
}


inline DictEntry *Dict::find(const char *key)const {
  int i;

  for (i = 0; i < length; ++i) 
  {
    if (entries[i].key && !strcmp(key, entries[i].key))
      return &entries[i];
  }
  return NULL;
}

GBool Dict::is(const char *type)const {
  DictEntry *e;

  return (e = find("Type")) && e->val->isName(type);
}

Object *Dict::lookup(const char *key, Object *obj)const {
  DictEntry *e;

  return (e = find(key)) ? e->val->fetch(xref, obj) : obj->initNull();
}

Object *Dict::lookupNF(const char *key, Object *obj)const {
  DictEntry *e;

  return (e = find(key)) ? e->val->copy(obj) : obj->initNull();
}

char *Dict::getKey(int i)const {
  return entries[i].key;
}

Object *Dict::getVal(int i, Object *obj)const {

  // boundary checks
  if(i<0 || i>=length)
     return obj->initNull();

  DictEntry * entry=&entries[i];

  // entry is valid, so fetches value
  return entry->val->fetch(xref, obj);
}

Object *Dict::getValNF(int i, Object *obj)const {

  // boundary checks
  if(i<0 || i>=length)
     return obj->initNull();

  DictEntry * entry=&entries[i];

  return entry->val->copy(obj);
}

Object * Dict::update(char * key, const Object * val)
{
  DictEntry * entry=find(key);
  if(!entry)
  {
     // not present, so adds 
     add(key, val);

     return NULL;
  }
  
  // available, so return old value, allocates new and intializes it
  Object *old = entry->val;
  entry->val=(Object *)gmalloc(sizeof(Object));
  *(entry->val) = *val;
  
  return old;
}
