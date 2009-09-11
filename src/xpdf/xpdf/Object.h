//========================================================================
//
// Object.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
// Changes:
// Michal Hocko   - public clone method for deep copy of object
//                - all methods which can have const char * instead of 
//                  char * are changed to const char *
//                - initArray from Array added
//
//========================================================================

#ifndef OBJECT_H
#define OBJECT_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include <string.h>
#include "goo/gtypes.h"
#include "goo/gmem.h"
#include "goo/GString.h"

class XRef;
class Array;
class Dict;
class Stream;

//------------------------------------------------------------------------
// Ref
//------------------------------------------------------------------------

struct Ref {
  int num;			// object number
  int gen;			// generation number
};

//------------------------------------------------------------------------
// object types
//------------------------------------------------------------------------

enum ObjType {
  // simple objects
  objBool,			// boolean
  objInt,			// integer
  objReal,			// real
  objString,			// string
  objName,			// name
  objNull,			// null

  // complex objects
  objArray,			// array
  objDict,			// dictionary
  objStream,			// stream
  objRef,			// indirect reference

  // special objects
  objCmd,			// command name
  objError,			// error return from Lexer
  objEOF,			// end of file return from Lexer
  objNone			// uninitialized object
};

#define numObjTypes 14		// total number of object types

//------------------------------------------------------------------------
// Object
//------------------------------------------------------------------------

#ifdef DEBUG_MEM
#define initObj(t) ++numAlloc[type = t]
#else
#define initObj(t) type = t
#endif

class Object {
public:

  // Default constructor.
  Object():
    type(objNone) {}

  // Deep copier
  Object * clone()const;
  
  // Initialize an object.
  Object *initBool(GBool boolnA)
    { initObj(objBool); booln = boolnA; return this; }
  Object *initInt(int intgA)
    { initObj(objInt); intg = intgA; return this; }
  Object *initReal(double realA)
    { initObj(objReal); real = realA; return this; }
  Object *initString(GString *stringA)
    { initObj(objString); string = stringA; return this; }
  Object *initName(const char *nameA)
    { initObj(objName); name = copyString(nameA); return this; }
  Object *initNull()
    { initObj(objNull); return this; }
  Object *initArray(const XRef *xref);
  Object *initArray(Array *array);
  Object *initDict(const XRef *xref);
  Object *initDict(Dict *dictA);
  Object *initStream(Stream *streamA);
  Object *initRef(int numA, int genA)
    { initObj(objRef); ref.num = numA; ref.gen = genA; return this; }
  Object *initCmd(const char *cmdA)
    { initObj(objCmd); cmd = copyString(cmdA); return this; }
  Object *initError()
    { initObj(objError); return this; }
  Object *initEOF()
    { initObj(objEOF); return this; }

  // Copy an object.
  Object *copy(Object *obj)const;

  // If object is a Ref, fetch and return the referenced object.
  // Otherwise, return a copy of the object.
  Object *fetch(const XRef *xref, Object *obj)const;

  // Free object contents.
  void free();

  // Type checking.
  ObjType getType()const { return type; }
  GBool isBool()const { return type == objBool; }
  GBool isInt()const { return type == objInt; }
  GBool isReal()const { return type == objReal; }
  GBool isNum()const { return type == objInt || type == objReal; }
  GBool isString()const { return type == objString; }
  GBool isName()const { return type == objName; }
  GBool isNull()const { return type == objNull; }
  GBool isArray()const { return type == objArray; }
  GBool isDict()const { return type == objDict; }
  GBool isStream()const { return type == objStream; }
  GBool isRef()const { return type == objRef; }
  GBool isCmd()const { return type == objCmd; }
  GBool isError()const { return type == objError; }
  GBool isEOF()const { return type == objEOF; }
  GBool isNone()const { return type == objNone; }

  // Special type checking.
  GBool isName(const char *nameA)const
    { return type == objName && !strcmp(name, nameA); }
  GBool isDict(const char *dictType)const;
  GBool isStream(const char *dictType)const;
  GBool isCmd(const char *cmdA)const
    { return type == objCmd && !strcmp(cmd, cmdA); }

  // Accessors.  NB: these assume object is of correct type.
  GBool getBool()const { return booln; }
  int getInt()const { return intg; }
  double getReal()const { return real; }
  double getNum()const { return type == objInt ? (double)intg : real; }
  const GString *getString()const { return string; }
  const char *getName()const { return name; }
  const Array *getArray()const { return array; }
  const Dict *getDict()const  { return dict; }
  Stream *getStream()const { return stream; }
  const Ref& getRef()const { return ref; }
  int getRefNum()const { return ref.num; }
  int getRefGen()const { return ref.gen; }
  char *getCmd()const { return cmd; }

  // Array accessors.
  int arrayGetLength()const;
  void arrayAdd(const Object *elem)const;
  Object *arrayGet(int i, Object *obj)const;
  Object *arrayGetNF(int i, Object *obj)const;

  // Dict accessors.
  int dictGetLength()const;
  void dictAdd(char *key, const Object *val)const;
  Object *dictUpdate(char *key, const Object *val)const;
  GBool dictIs(const char *dictType)const;
  Object *dictLookup(const char *key, Object *obj)const;
  Object *dictLookupNF(const char *key, Object *obj)const;
  char *dictGetKey(int i)const;
  Object *dictGetVal(int i, Object *obj)const;
  Object *dictGetValNF(int i, Object *obj)const;
  Object *dictDel(const char *key)const;

  // Stream accessors.
  GBool streamIs(const char *dictType)const;
  void streamReset()const;
  void streamClose()const;
  int streamGetChar()const;
  int streamLookChar()const;
  char *streamGetLine(char *buf, int size)const;
  Guint streamGetPos()const;
  void streamSetPos(Guint pos, int dir = 0)const;
  const Dict *streamGetDict()const;

  // Output.
  const char *getTypeName()const;
  void print(FILE *f = stdout)const;

  // Memory testing.
  static void memCheck(FILE *f);

private:

  ObjType type;			// object type
  mutable union {		// value for each type:
    GBool booln;		//   boolean
    int intg;			//   integer
    double real;		//   real
    GString *string;		//   string
    char *name;			//   name
    Array *array;		//   array
    Dict *dict;			//   dictionary
    Stream *stream;		//   stream
    Ref ref;			//   indirect reference
    char *cmd;			//   command
  };

#ifdef DEBUG_MEM
  static int			// number of each type of object
    numAlloc[numObjTypes];	//   currently allocated
#endif
};

//------------------------------------------------------------------------
// Array accessors.
//------------------------------------------------------------------------

#include "xpdf/Array.h"

inline int Object::arrayGetLength()const
  { return array->getLength(); }

inline void Object::arrayAdd(const Object *elem)const
  { array->add(elem); }

inline Object *Object::arrayGet(int i, Object *obj)const
  { return array->get(i, obj); }

inline Object *Object::arrayGetNF(int i, Object *obj)const
  { return array->getNF(i, obj); }

//------------------------------------------------------------------------
// Dict accessors.
//------------------------------------------------------------------------

#include "xpdf/Dict.h"

inline int Object::dictGetLength()const
  { return dict->getLength(); }

inline void Object::dictAdd(char *key, const Object *val)const
  { dict->add(key, val); }

inline Object * Object::dictUpdate(char *key, const Object *val)const
  { return dict->update(key, val); }

inline GBool Object::dictIs(const char *dictType)const
  { return dict->is(dictType); }

inline GBool Object::isDict(const char *dictType)const
  { return type == objDict && dictIs(dictType); }

inline Object *Object::dictLookup(const char *key, Object *obj)const
  { return dict->lookup(key, obj); }

inline Object *Object::dictLookupNF(const char *key, Object *obj)const
  { return dict->lookupNF(key, obj); }

inline char *Object::dictGetKey(int i)const
  { return dict->getKey(i); }

inline Object *Object::dictGetVal(int i, Object *obj)const
  { return dict->getVal(i, obj); }

inline Object *Object::dictGetValNF(int i, Object *obj)const
  { return dict->getValNF(i, obj); }

inline Object *Object::dictDel(const char *key)const
  { return dict->del(key); }

//------------------------------------------------------------------------
// Stream accessors.
//------------------------------------------------------------------------

#include "xpdf/Stream.h"

inline GBool Object::streamIs(const char *dictType)const
  { return stream->getDict()->is(dictType); }

inline GBool Object::isStream(const char *dictType)const
  { return type == objStream && streamIs(dictType); }

inline void Object::streamReset()const
  { stream->reset(); }

inline void Object::streamClose()const
  { stream->close(); }

inline int Object::streamGetChar()const
  { return stream->getChar(); }

inline int Object::streamLookChar()const
  { return stream->lookChar(); }

inline char *Object::streamGetLine(char *buf, int size)const
  { return stream->getLine(buf, size); }

inline Guint Object::streamGetPos()const
  { return stream->getPos(); }

inline void Object::streamSetPos(Guint pos, int dir)const
  { stream->setPos(pos, dir); }

inline const Dict *Object::streamGetDict()const
  { return stream->getDict(); }

#endif
