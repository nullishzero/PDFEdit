//========================================================================
//
// CMap.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef CMAP_H
#define CMAP_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "xpdf/CharTypes.h"

#if MULTITHREADED
#include "goo/GMutex.h"
#endif

class GString;
struct CMapVectorEntry;
class CMapCache;

//------------------------------------------------------------------------

class CMap {
public:

  // Create the CMap specified by <collection> and <cMapName>.  Sets
  // the initial reference count to 1.  Returns NULL on failure.
  static CMap *parse(CMapCache *cache, const GString *collectionA,
		     const GString *cMapNameA);

  ~CMap();

  void incRefCnt();
  void decRefCnt();

  // Return collection name (<registry>-<ordering>).
  const GString *getCollection()const { return collection; }

  // Return true if this CMap matches the specified <collectionA>, and
  // <cMapNameA>.
  GBool match(const GString *collectionA, const GString *cMapNameA)const;

  // Return the CID corresponding to the character code starting at
  // <s>, which contains <len> bytes.  Sets *<nUsed> to the number of
  // bytes used by the char code.
  CID getCID(const char *s, int len, int *nUsed)const;

  // Return the writing mode (0=horizontal, 1=vertical).
  int getWMode()const { return wMode; }

private:

  CMap(GString *collectionA, GString *cMapNameA);
  CMap(GString *collectionA, GString *cMapNameA, int wModeA);
  void useCMap(CMapCache *cache, const char *useName);
  void copyVector(CMapVectorEntry *dest, const CMapVectorEntry *src);
  void addCodeSpace(CMapVectorEntry *vec, Guint start, Guint end,
		    Guint nBytes);
  void addCIDs(Guint start, Guint end, Guint nBytes, CID firstCID);
  void freeCMapVector(CMapVectorEntry *vec);

  GString *collection;
  GString *cMapName;
  int wMode;			// writing mode (0=horizontal, 1=vertical)
  CMapVectorEntry *vector;	// vector for first byte (NULL for
				//   identity CMap)
  int refCnt;
#if MULTITHREADED
  GMutex mutex;
#endif
};

//------------------------------------------------------------------------

#define cMapCacheSize 4

class CMapCache {
public:

  CMapCache();
  ~CMapCache();

  // Get the <cMapName> CMap for the specified character collection.
  // Increments its reference count; there will be one reference for
  // the cache plus one for the caller of this function.  Returns NULL
  // on failure.
  CMap *getCMap(const GString *collection, const GString *cMapName);

private:

  CMap *cache[cMapCacheSize];
};

#endif
