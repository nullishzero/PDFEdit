//========================================================================
//
// Page.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef PAGE_H
#define PAGE_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "xpdf/Object.h"

class Dict;
class XRef;
class OutputDev;
class Links;
class Catalog;

//------------------------------------------------------------------------

class PDFRectangle {
public:
  double x1, y1, x2, y2;

  PDFRectangle() { x1 = y1 = x2 = y2 = 0; }
  PDFRectangle(double x1A, double y1A, double x2A, double y2A)
    { x1 = x1A; y1 = y1A; x2 = x2A; y2 = y2A; }
  GBool isValid() { return x1 != 0 || y1 != 0 || x2 != 0 || y2 != 0; }
  void clipTo(PDFRectangle *rect);
};

//------------------------------------------------------------------------
// PageAttrs
//------------------------------------------------------------------------

class PageAttrs {
public:

  // Construct a new PageAttrs object by merging a dictionary
  // (of type Pages or Page) into another PageAttrs object.  If
  // <attrs> is NULL, uses defaults.
  PageAttrs(const PageAttrs *attrs, const Dict *dict);

  // Destructor.
  ~PageAttrs();

  // Accessors.
  const PDFRectangle *getMediaBox()const { return &mediaBox; }
  const PDFRectangle *getCropBox()const { return &cropBox; }
  GBool isCropped()const { return haveCropBox; }
  const PDFRectangle *getBleedBox()const { return &bleedBox; }
  const PDFRectangle *getTrimBox()const { return &trimBox; }
  const PDFRectangle *getArtBox()const { return &artBox; }
  int getRotate()const { return rotate; }
  const GString *getLastModified()const
    { return lastModified.isString()
	? lastModified.getString() : (GString *)NULL; }
  const Dict *getBoxColorInfo()const
    { return boxColorInfo.isDict() ? boxColorInfo.getDict() : (Dict *)NULL; }
  const Dict *getGroup()const
    { return group.isDict() ? group.getDict() : (Dict *)NULL; }
  Stream *getMetadata()
    { return metadata.isStream() ? metadata.getStream() : (Stream *)NULL; }
  const Dict *getPieceInfo()const
    { return pieceInfo.isDict() ? pieceInfo.getDict() : (Dict *)NULL; }
  const Dict *getSeparationInfo()const
    { return separationInfo.isDict()
	? separationInfo.getDict() : (Dict *)NULL; }
  const Dict *getResourceDict()const
    { return resources.isDict() ? resources.getDict() : (Dict *)NULL; }

private:

  GBool readBox(const Dict *dict, const char *key, PDFRectangle *box);

  PDFRectangle mediaBox;
  PDFRectangle cropBox;
  GBool haveCropBox;
  PDFRectangle bleedBox;
  PDFRectangle trimBox;
  PDFRectangle artBox;
  int rotate;
  Object lastModified;
  Object boxColorInfo;
  Object group;
  Object metadata;
  Object pieceInfo;
  Object separationInfo;
  Object resources;
};

//------------------------------------------------------------------------
// Page
//------------------------------------------------------------------------

class Page {
public:

  // Constructor.
  Page(XRef *xrefA, int numA, const Dict *pageDict, PageAttrs *attrsA);

  // Destructor.
  ~Page();

  // Is page valid?
  GBool isOk()const { return ok; }

  // Get page parameters.
  int getNum()const { return num; }
  const PDFRectangle *getMediaBox()const { return attrs->getMediaBox(); }
  const PDFRectangle *getCropBox()const { return attrs->getCropBox(); }
  GBool isCropped()const { return attrs->isCropped(); }
  double getMediaWidth()const 
    { return attrs->getMediaBox()->x2 - attrs->getMediaBox()->x1; }
  double getMediaHeight()const
    { return attrs->getMediaBox()->y2 - attrs->getMediaBox()->y1; }
  double getCropWidth()const 
    { return attrs->getCropBox()->x2 - attrs->getCropBox()->x1; }
  double getCropHeight()const
    { return attrs->getCropBox()->y2 - attrs->getCropBox()->y1; }
  const PDFRectangle *getBleedBox()const { return attrs->getBleedBox(); }
  const PDFRectangle *getTrimBox()const { return attrs->getTrimBox(); }
  const PDFRectangle *getArtBox()const { return attrs->getArtBox(); }
  int getRotate()const { return attrs->getRotate(); }
  const GString *getLastModified()const { return attrs->getLastModified(); }
  const Dict *getBoxColorInfo()const { return attrs->getBoxColorInfo(); }
  const Dict *getGroup()const { return attrs->getGroup(); }
  Stream *getMetadata() { return attrs->getMetadata(); }
  const Dict *getPieceInfo()const { return attrs->getPieceInfo(); }
  const Dict *getSeparationInfo()const { return attrs->getSeparationInfo(); }

  // Get resource dictionary.
  const Dict *getResourceDict()const { return attrs->getResourceDict(); }

  // Get annotations array.
  Object *getAnnots(Object *obj)const { return annots.fetch(xref, obj); }

  // Return a list of links.
  Links *getLinks(const Catalog *catalog)const;

  // Get contents.
  Object *getContents(Object *obj)const { return contents.fetch(xref, obj); }

  // Display a page.
  void display(OutputDev *out, double hDPI, double vDPI,
	       int rotate, GBool useMediaBox, GBool crop,
	       GBool printing, const Catalog *catalog,
	       GBool (*abortCheckCbk)(void *data) = NULL,
	       void *abortCheckCbkData = NULL)const;

  // Display part of a page.
  void displaySlice(OutputDev *out, double hDPI, double vDPI,
		    int rotate, GBool useMediaBox, GBool crop,
		    int sliceX, int sliceY, int sliceW, int sliceH,
		    GBool printing, const Catalog *catalog,
		    GBool (*abortCheckCbk)(void *data) = NULL,
		    void *abortCheckCbkData = NULL)const;

  void makeBox(double hDPI, double vDPI, int rotate,
	       GBool useMediaBox, GBool upsideDown,
	       double sliceX, double sliceY, double sliceW, double sliceH,
	       PDFRectangle *box, GBool *crop)const;

  void processLinks(OutputDev *out, const Catalog *catalog)const;

  // Get the page's default CTM.
  void getDefaultCTM(double *ctm, double hDPI, double vDPI,
		     int rotate, GBool useMediaBox, GBool upsideDown);

private:

  XRef *xref;			// the xref table for this PDF file
  int num;			// page number
  PageAttrs *attrs;		// page attributes
  Object annots;		// annotations array
  Object contents;		// page contents
  GBool ok;			// true if page is valid
};

#endif
