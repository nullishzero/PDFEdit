//========================================================================
//
// OutputDev.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef OUTPUTDEV_H
#define OUTPUTDEV_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "xpdf/CharTypes.h"

class GString;
class GfxState;
struct GfxColor;
class GfxColorSpace;
class GfxImageColorMap;
class GfxFunctionShading;
class GfxAxialShading;
class GfxRadialShading;
class Stream;
class Links;
class Link;
class Catalog;
class Page;
class Function;

//------------------------------------------------------------------------
// OutputDev
//------------------------------------------------------------------------

class OutputDev {
public:

  // Constructor.
  OutputDev() {}

  // Destructor.
  virtual ~OutputDev() {}

  //----- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown()const = 0;

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar()const = 0;

  // Does this device use tilingPatternFill()?  If this returns false,
  // tiling pattern fills will be reduced to a series of other drawing
  // operations.
  virtual GBool useTilingPatternFill()const { return gFalse; }

  // Does this device use functionShadedFill(), axialShadedFill(), and
  // radialShadedFill()?  If this returns false, these shaded fills
  // will be reduced to a series of other drawing operations.
  virtual GBool useShadedFills()const { return gFalse; }

  // Does this device use drawForm()?  If this returns false,
  // form-type XObjects will be interpreted (i.e., unrolled).
  virtual GBool useDrawForm()const { return gFalse; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars()const = 0;

  // Does this device need non-text content?
  virtual GBool needNonText()const { return gTrue; }

  //----- initialization and control

  // Set default transform matrix.
  virtual void setDefaultCTM(const double *ctm);

  // Check to see if a page slice should be displayed.  If this
  // returns false, the page display is aborted.  Typically, an
  // OutputDev will use some alternate means to display the page
  // before returning false.
  virtual GBool checkPageSlice(UNUSED_PARAM const Page *page, 
		  UNUSED_PARAM double hDPI, 
		  UNUSED_PARAM double vDPI,
		  UNUSED_PARAM int rotate, 
		  UNUSED_PARAM GBool useMediaBox, 
		  UNUSED_PARAM GBool crop,
		  UNUSED_PARAM int sliceX, 
		  UNUSED_PARAM int sliceY, 
		  UNUSED_PARAM int sliceW, 
		  UNUSED_PARAM int sliceH,
		  UNUSED_PARAM GBool printing, 
		  UNUSED_PARAM const Catalog *catalog,
		  UNUSED_PARAM GBool (*abortCheckCbk)(void *data) = NULL,
		  UNUSED_PARAM void *abortCheckCbkData = NULL)
    { return gTrue; }

  // Start a page.
  virtual void startPage(UNUSED_PARAM int pageNum, UNUSED_PARAM GfxState *state) {}

  // End a page.
  virtual void endPage() {}

  // Dump page contents to display.
  virtual void dump() {}

  //----- coordinate conversion

  // Convert between device and user coordinates.
  virtual void cvtDevToUser(double dx, double dy, double *ux, double *uy);
  virtual void cvtUserToDev(double ux, double uy, int *dx, int *dy);

  double *getDefCTM() { return defCTM; }
  double *getDefICTM() { return defICTM; }

  //----- save/restore graphics state
  virtual void saveState(UNUSED_PARAM GfxState *state) {}
  virtual void restoreState(UNUSED_PARAM GfxState *state) {}

  //----- update graphics state
  virtual void updateAll(GfxState *state);
  virtual void updateCTM(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM double m11, 
		  UNUSED_PARAM double m12,
		  UNUSED_PARAM double m21, 
		  UNUSED_PARAM double m22, 
		  UNUSED_PARAM double m31, 
		  UNUSED_PARAM double m32) {}
  virtual void updateLineDash(UNUSED_PARAM GfxState *state) {}
  virtual void updateFlatness(UNUSED_PARAM GfxState *state) {}
  virtual void updateLineJoin(UNUSED_PARAM GfxState *state) {}
  virtual void updateLineCap(UNUSED_PARAM GfxState *state) {}
  virtual void updateMiterLimit(UNUSED_PARAM GfxState *state) {}
  virtual void updateLineWidth(UNUSED_PARAM GfxState *state) {}
  virtual void updateStrokeAdjust(UNUSED_PARAM GfxState *state) {}
  virtual void updateFillColorSpace(UNUSED_PARAM GfxState *state) {}
  virtual void updateStrokeColorSpace(UNUSED_PARAM GfxState *state) {}
  virtual void updateFillColor(UNUSED_PARAM GfxState *state) {}
  virtual void updateStrokeColor(UNUSED_PARAM GfxState *state) {}
  virtual void updateBlendMode(UNUSED_PARAM GfxState *state) {}
  virtual void updateFillOpacity(UNUSED_PARAM GfxState *state) {}
  virtual void updateStrokeOpacity(UNUSED_PARAM GfxState *state) {}
  virtual void updateFillOverprint(UNUSED_PARAM GfxState *state) {}
  virtual void updateStrokeOverprint(UNUSED_PARAM GfxState *state) {}
  virtual void updateTransfer(UNUSED_PARAM GfxState *state) {}

  //----- update text state
  virtual void updateFont(UNUSED_PARAM GfxState *state) {}
  virtual void updateTextMat(UNUSED_PARAM GfxState *state) {}
  virtual void updateCharSpace(UNUSED_PARAM GfxState *state) {}
  virtual void updateRender(UNUSED_PARAM GfxState *state) {}
  virtual void updateRise(UNUSED_PARAM GfxState *state) {}
  virtual void updateWordSpace(UNUSED_PARAM GfxState *state) {}
  virtual void updateHorizScaling(UNUSED_PARAM GfxState *state) {}
  virtual void updateTextPos(UNUSED_PARAM GfxState *state) {}
  virtual void updateTextShift(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM double shift) {}

  //----- path painting
  virtual void stroke(UNUSED_PARAM GfxState *state) {}
  virtual void fill(UNUSED_PARAM GfxState *state) {}
  virtual void eoFill(UNUSED_PARAM GfxState *state) {}
  virtual void tilingPatternFill(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM const Object *str,
		  UNUSED_PARAM int paintType, 
		  UNUSED_PARAM const Dict *resDict,
		  UNUSED_PARAM const double *mat, 
		  UNUSED_PARAM const double *bbox,
		  UNUSED_PARAM int x0, 
		  UNUSED_PARAM int y0, 
		  UNUSED_PARAM int x1, 
		  UNUSED_PARAM int y1,
		  UNUSED_PARAM double xStep, 
		  UNUSED_PARAM double yStep) {}
  virtual GBool functionShadedFill(UNUSED_PARAM GfxState *state,
		  UNUSED_PARAM GfxFunctionShading *shading)
    { return gFalse; }
  virtual GBool axialShadedFill(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM GfxAxialShading *shading)
    { return gFalse; }
  virtual GBool radialShadedFill(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM GfxRadialShading *shading)
    { return gFalse; }

  //----- path clipping
  virtual void clip(UNUSED_PARAM GfxState *state) {}
  virtual void eoClip(UNUSED_PARAM GfxState *state) {}
  virtual void clipToStrokePath(UNUSED_PARAM GfxState *state) {}

  //----- text drawing
  virtual void beginStringOp(UNUSED_PARAM GfxState *state) {}
  virtual void endStringOp(UNUSED_PARAM GfxState *state) {}
  virtual void beginString(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM const GString *s) {}
  virtual void endString(UNUSED_PARAM GfxState *state) {}
  virtual void drawChar(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM double x, 
		  UNUSED_PARAM double y,
		  UNUSED_PARAM double dx, 
		  UNUSED_PARAM double dy,
		  UNUSED_PARAM double originX, 
		  UNUSED_PARAM double originY,
		  UNUSED_PARAM CharCode code, 
		  UNUSED_PARAM int nBytes, 
		  UNUSED_PARAM const Unicode *u, 
		  UNUSED_PARAM int uLen) {}
  virtual void drawString(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM const GString *s) {}
  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);
  virtual void endType3Char(UNUSED_PARAM GfxState *state) {}
  virtual void endTextObject(UNUSED_PARAM GfxState *state) {}

  //----- image drawing
  virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
			     int width, int height, GBool invert,
			     GBool inlineImg);
  virtual void drawImage(GfxState *state, Object *ref, Stream *str,
			 int width, int height, GfxImageColorMap *colorMap,
			 int *maskColors, GBool inlineImg);
  virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
			       int width, int height,
			       GfxImageColorMap *colorMap,
			       Stream *maskStr, int maskWidth, int maskHeight,
			       GBool maskInvert);
  virtual void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
				   int width, int height,
				   GfxImageColorMap *colorMap,
				   Stream *maskStr,
				   int maskWidth, int maskHeight,
				   GfxImageColorMap *maskColorMap);

#if OPI_SUPPORT
  //----- OPI functions
  virtual void opiBegin(GfxState *state, Dict *opiDict);
  virtual void opiEnd(GfxState *state, Dict *opiDict);
#endif

  //----- Type 3 font operators
  virtual void type3D0(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM double wx, 
		  UNUSED_PARAM double wy) {}
  virtual void type3D1(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM double wx, 
		  UNUSED_PARAM double wy,
		  UNUSED_PARAM double llx, 
		  UNUSED_PARAM double lly, 
		  UNUSED_PARAM double urx, 
		  UNUSED_PARAM double ury) {}

  //----- form XObjects
  virtual void drawForm(UNUSED_PARAM Ref id) {}

  //----- PostScript XObjects
  virtual void psXObject(UNUSED_PARAM Stream *psStream, 
		  UNUSED_PARAM Stream *level1Stream) {}

  //----- transparency groups and soft masks
  virtual void beginTransparencyGroup(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM const double *bbox, 
		  UNUSED_PARAM const GfxColorSpace *blendingColorSpace,
		  UNUSED_PARAM GBool isolated, 
		  UNUSED_PARAM GBool knockout,
		  UNUSED_PARAM GBool forSoftMask) {}
  virtual void endTransparencyGroup(UNUSED_PARAM GfxState *state) {}
  virtual void paintTransparencyGroup(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM const double *bbox) {}
  virtual void setSoftMask(UNUSED_PARAM GfxState *state, 
		  UNUSED_PARAM const double *bbox, 
		  UNUSED_PARAM GBool alpha,
		  UNUSED_PARAM Function *transferFunc, 
		  UNUSED_PARAM const GfxColor *backdropColor) {}
  virtual void clearSoftMask(UNUSED_PARAM GfxState *state) {}

  //----- links
  virtual void processLink(UNUSED_PARAM const Link *link, 
		  UNUSED_PARAM const Catalog *catalog) {}

#if 1 //~tmp: turn off anti-aliasing temporarily
  virtual GBool getVectorAntialias() { return gFalse; }
  virtual void setVectorAntialias(UNUSED_PARAM GBool vaa) {}
#endif

private:

  double defCTM[6];		// default coordinate transform matrix
  double defICTM[6];		// inverse of default CTM
};

#endif
