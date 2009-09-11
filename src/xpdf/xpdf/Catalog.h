//========================================================================
//
// Catalog.h
//
// Copyright 1996-2007 Glyph & Cog, LLC
//
//========================================================================

#ifndef CATALOG_H
#define CATALOG_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

class XRef;
class Object;
class Page;
class PageAttrs;
struct Ref;
class LinkDest;

//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

class Catalog {
public:

  // Constructor.
  Catalog(XRef *xrefA);

  // Destructor.
  ~Catalog();

  // Is catalog valid?
  GBool isOk()const { return ok; }

  // Get number of pages.
  int getNumPages()const { return numPages; }

  // Get a page.
  const Page *getPage(int i)const { return pages[i-1]; }

  // Get the reference for a page object.
  const Ref *getPageRef(int i)const { return &pageRefs[i-1]; }

  // Return base URI, or NULL if none.
  const GString *getBaseURI()const { return baseURI; }

  // Return the contents of the metadata stream, or NULL if there is
  // no metadata.
  GString *readMetadata();

  // Return the structure tree root object.
  const Object *getStructTreeRoot()const { return &structTreeRoot; }

  // Find a page, given its object ID.  Returns page number, or 0 if
  // not found.
  int findPage(int num, int gen)const;

  // Find a named destination.  Returns the link destination, or
  // NULL if <name> is not a destination.
  LinkDest *findDest(const GString *name)const;

  const Object *getDests()const { return &dests; }

  const Object *getNameTree()const { return &nameTree; }

  const Object *getOutline()const { return &outline; }

  const Object *getAcroForm()const { return &acroForm; }

private:

  XRef *xref;			// the xref table for this PDF file
  Page **pages;			// array of pages
  Ref *pageRefs;		// object ID for each page
  int numPages;			// number of pages
  int pagesSize;		// size of pages array
  Object dests;			// named destination dictionary
  Object nameTree;		// name tree
  GString *baseURI;		// base URI for URI-type links
  Object metadata;		// metadata stream
  Object structTreeRoot;	// structure tree root dictionary
  Object outline;		// outline dictionary
  Object acroForm;		// AcroForm dictionary
  GBool ok;			// true if catalog is valid

  int readPageTree(const Dict *pages, PageAttrs *attrs, int start,
		   char *alreadyRead);
  Object *findDestInTree(const Object *tree, const GString *name, Object *obj)const;
};

#endif
