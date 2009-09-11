//========================================================================
//
// Outline.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef OUTLINE_H
#define OUTLINE_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "xpdf/Object.h"
#include "xpdf/CharTypes.h"

class GString;
class GList;
class XRef;
class LinkAction;

//------------------------------------------------------------------------

class Outline {
public:

  Outline(const Object *outlineObj, XRef *xref);
  ~Outline();

  const GList *getItems()const { return items; }

private:

  GList *items;			// NULL if document has no outline
				//   [OutlineItem]
};

//------------------------------------------------------------------------

class OutlineItem {
public:

  OutlineItem(const Dict *dict, XRef *xrefA);
  ~OutlineItem();

  static GList *readItemList(const Object *firstItemRef, const Object *lastItemRef,
			     XRef *xrefA);

  void open();
  void close();

  const Unicode *getTitle()const { return title; }
  int getTitleLength()const { return titleLen; }
  const LinkAction *getAction()const { return action; }
  GBool isOpen()const { return startsOpen; }
  GBool hasKids()const { return firstRef.isRef(); }
  const GList *getKids()const { return kids; }

private:

  XRef *xref;
  Unicode *title;
  int titleLen;
  LinkAction *action;
  Object firstRef;
  Object lastRef;
  Object nextRef;
  GBool startsOpen;
  GList *kids;			// NULL unless this item is open [OutlineItem]
};

#endif
