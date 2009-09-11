//========================================================================
//
// Object.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
// Changes:
// Michal Hocko   - public clone method for deep copy of object
//
//========================================================================

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include "xpdf/Object.h"
#include "xpdf/Array.h"
#include "xpdf/Dict.h"
#include "xpdf/Error.h"
#include "xpdf/Stream.h"
#include "xpdf/XRef.h"

//------------------------------------------------------------------------
// Object
//------------------------------------------------------------------------

const char *objTypeNames[numObjTypes] = {
  "boolean",
  "integer",
  "real",
  "string",
  "name",
  "null",
  "array",
  "dictionary",
  "stream",
  "ref",
  "cmd",
  "error",
  "eof",
  "none"
};

#ifdef DEBUG_MEM
int Object::numAlloc[numObjTypes] =
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

/** Deep copier.
 *
 * Creates new Object instance with the same (deep) content.
 * Changes made to the returned object takes no effect to this
 * instanve.
 * <br>
 * Type specific behaviour:
 * <ul>
 * <li>stream - figure out
 * </ul>
 * NOTE: Object instance is allocated by gmalloc method and should
 * be deallocated by gfree function. Value inside Object should be
 * deallocated before Object instance by Object::free method which
 * knows type specific behaviour.
 *
 */
Object * Object::clone()const
{
   Object * result=(Object *)gmalloc(sizeof(Object));

   // initializes type
   result->type = this->type;
   
   // value depends on type
   switch(result->type)
   {
      case objBool:
         result->booln=booln;
         break;
      case objInt:
         result->intg=intg;
         break;
      case objReal:
         result->real=real;
         break;
      case objString:
         result->string=new GString(string);
         if(!result->string)
           goto cloneerror;
         break;
      case objName:
         result->name=copyString(name);
         if(!result->name)
           goto cloneerror;
         break;
      case objArray:
         result->array=array->clone();
         if(!result->array)
           goto cloneerror;
         break;
      case objDict:
         result->dict=dict->clone();
         if(!result->dict)
           goto cloneerror;
         break;
      case objStream:
         result->stream=stream->clone();
         if(!result->stream)
           goto cloneerror;
         break;
      case objRef:
         result->ref=ref;
         break;
      case objCmd:
         result->cmd=copyString(cmd);
         if(!result->cmd)
           goto cloneerror;
         break;

      // special object types with no value to initialize
      case objNull:
      case objError:
      case objEOF:
      case objNone:
         break;
   }

   return result;

cloneerror:
   // unable to clone value holder, so returns with NULL and
   // deallocates result
   gfree(result);
   return NULL;
}

Object *Object::initArray(const XRef *xref) {
  initObj(objArray);
  array = new Array(xref);
  return this;
}

Object *Object::initArray(Array *arrayA) {
  initObj(objArray);
  array = arrayA;
  array->incRef();
  return this;
}

Object *Object::initDict(const XRef *xref) {
  initObj(objDict);
  dict = new Dict(xref);
  return this;
}

Object *Object::initDict(Dict *dictA) {
  initObj(objDict);
  dict = dictA;
  dict->incRef();
  return this;
}

Object *Object::initStream(Stream *streamA) {
  initObj(objStream);
  stream = streamA;
  return this;
}

Object *Object::copy(Object *obj)const {
  *obj = *this;
  switch (type) {
  case objString:
    obj->string = string->copy();
    break;
  case objName:
    obj->name = copyString(name);
    break;
  case objArray:
    array->incRef();
    break;
  case objDict:
    dict->incRef();
    break;
  case objStream:
    stream->incRef();
    break;
  case objCmd:
    obj->cmd = copyString(cmd);
    break;
  default:
    break;
  }
#ifdef DEBUG_MEM
  ++numAlloc[type];
#endif
  return obj;
}

Object *Object::fetch(const XRef *xref, Object *obj)const {
  return (type == objRef && xref) ?
         xref->fetch(ref.num, ref.gen, obj) : copy(obj);
}

void Object::free() {
  switch (type) {
  case objString:
    delete string;
    break;
  case objName:
    gfree(name);
    break;
  case objArray:
    if (!array->decRef()) {
      delete array;
    }
    break;
  case objDict:
    if (!dict->decRef()) {
      delete dict;
    }
    break;
  case objStream:
    if (!stream->decRef()) {
      delete stream;
    }
    break;
  case objCmd:
    gfree(cmd);
    break;
  default:
    break;
  }
#ifdef DEBUG_MEM
  --numAlloc[type];
#endif
  type = objNone;
}

const char *Object::getTypeName()const {
  return objTypeNames[type];
}

void Object::print(FILE *f)const {
  Object obj;
  int i;

  switch (type) {
  case objBool:
    fprintf(f, "%s", booln ? "true" : "false");
    break;
  case objInt:
    fprintf(f, "%d", intg);
    break;
  case objReal:
    fprintf(f, "%g", real);
    break;
  case objString:
    fprintf(f, "(");
    fwrite(string->getCString(), 1, string->getLength(), f);
    fprintf(f, ")");
    break;
  case objName:
    fprintf(f, "/%s", name);
    break;
  case objNull:
    fprintf(f, "null");
    break;
  case objArray:
    fprintf(f, "[");
    for (i = 0; i < arrayGetLength(); ++i) {
      if (i > 0)
	fprintf(f, " ");
      arrayGetNF(i, &obj);
      obj.print(f);
      obj.free();
    }
    fprintf(f, "]");
    break;
  case objDict:
    fprintf(f, "<<");
    for (i = 0; i < dictGetLength(); ++i) {
      fprintf(f, " /%s ", dictGetKey(i));
      dictGetValNF(i, &obj);
      obj.print(f);
      obj.free();
    }
    fprintf(f, " >>");
    break;
  case objStream:
    fprintf(f, "<stream>");
    break;
  case objRef:
    fprintf(f, "%d %d R", ref.num, ref.gen);
    break;
  case objCmd:
    fprintf(f, "%s", cmd);
    break;
  case objError:
    fprintf(f, "<error>");
    break;
  case objEOF:
    fprintf(f, "<EOF>");
    break;
  case objNone:
    fprintf(f, "<none>");
    break;
  }
}

void Object::memCheck(FILE *f) {
#ifdef DEBUG_MEM
  int i;
  int t;

  t = 0;
  for (i = 0; i < numObjTypes; ++i)
    t += numAlloc[i];
  if (t > 0) {
    fprintf(f, "Allocated objects:\n");
    for (i = 0; i < numObjTypes; ++i) {
      if (numAlloc[i] > 0)
	fprintf(f, "  %-20s: %6d\n", objTypeNames[i], numAlloc[i]);
    }
  }
#endif
}
