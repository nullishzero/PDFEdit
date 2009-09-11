//========================================================================
//
// Annot.h
//
// Copyright 2000-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef ANNOT_H
#define ANNOT_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

class XRef;
class Catalog;
class Gfx;
class GfxFontDict;

//------------------------------------------------------------------------
// AnnotBorderStyle
//------------------------------------------------------------------------

enum AnnotBorderType {
  annotBorderSolid,
  annotBorderDashed,
  annotBorderBeveled,
  annotBorderInset,
  annotBorderUnderlined
};

class AnnotBorderStyle {
public:

  AnnotBorderStyle(AnnotBorderType typeA, double widthA,
		   double *dashA, int dashLengthA,
		   double rA, double gA, double bA);
  ~AnnotBorderStyle();

  AnnotBorderType getType() { return type; }
  double getWidth() { return width; }
  void getDash(double **dashA, int *dashLengthA)
    { *dashA = dash; *dashLengthA = dashLength; }
  void getColor(double *rA, double *gA, double *bA)
    { *rA = r; *gA = g; *bA = b; }

private:

  AnnotBorderType type;
  double width;
  double *dash;
  int dashLength;
  double r, g, b;
};

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

class Annot {
public:

  Annot(XRef *xrefA, const Dict *acroForm, const Dict *dict, const Ref *refA);
  ~Annot();
  GBool isOk() { return ok; }

  void draw(Gfx *gfx, GBool printing);

  // Get appearance object.
  Object *getAppearance(Object *obj) { return appearance.fetch(xref, obj); }

  AnnotBorderStyle *getBorderStyle() { return borderStyle; }

  GBool match(const Ref *refA)const
    { return ref.num == refA->num && ref.gen == refA->gen; }

  void generateFieldAppearance(const Dict *field, const Dict *annot, const Dict *acroForm);

private:
 
  void setColor(const Array *a, GBool fill, int adjust);
  void drawText(const GString *text, const GString *da, const GfxFontDict *fontDict,
		GBool multiline, int comb, int quadding,
		GBool txField, GBool forceZapfDingbats);
  void drawListBox(GString *const *text, const GBool *selection,
		   int nOptions, int topIdx,
		   GString *da, GfxFontDict *fontDict, GBool quadding);
  void getNextLine(const GString *text, int start,
		   const GfxFont *font, double fontSize, double wMax,
		   int *end, double *width, int *next);
  void drawCircle(double cx, double cy, double r, GBool fill);
  void drawCircleTopLeft(double cx, double cy, double r);
  void drawCircleBottomRight(double cx, double cy, double r);
  Object *fieldLookup(const Dict *field, const char *key, Object *obj)const;

  XRef *xref;			// the xref table for this PDF file
  Ref ref;			// object ref identifying this annotation
  GString *type;		// annotation type
  Object appearance;		// a reference to the Form XObject stream
				//   for the normal appearance
  GString *appearBuf;
  double xMin, yMin,		// annotation rectangle
         xMax, yMax;
  Guint flags;
  AnnotBorderStyle *borderStyle;
  GBool ok;
};

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

class Annots {
public:

  // Build a list of Annot objects.
  Annots(XRef *xref, const Catalog *catalog, const Object *annotsObj);

  ~Annots();

  // Iterate through list of annotations.
  int getNumAnnots()const { return nAnnots; }
  Annot *getAnnot(int i)const { return annots[i]; }

  // (Re)generate the appearance streams for all annotations belonging
  // to a form field.
  void generateAppearances(const Dict *acroForm);

private:

  void scanFieldAppearances(const Dict *node, const Ref *ref, const Dict *parent,
			    const Dict *acroForm);
  Annot *findAnnot(const Ref *ref)const;

  Annot **annots;
  int nAnnots;
};

#endif
