//========================================================================
//
// OutputDev.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef OUTPUTDEV_H
#define OUTPUTDEV_H

#include <aconf.h>

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
  virtual GBool upsideDown() = 0;

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar() = 0;

  // Does this device use tilingPatternFill()?  If this returns false,
  // tiling pattern fills will be reduced to a series of other drawing
  // operations.
  virtual GBool useTilingPatternFill() { return gFalse; }

  // Does this device use functionShadedFill(), axialShadedFill(), and
  // radialShadedFill()?  If this returns false, these shaded fills
  // will be reduced to a series of other drawing operations.
  virtual GBool useShadedFills() { return gFalse; }

  // Does this device use drawForm()?  If this returns false,
  // form-type XObjects will be interpreted (i.e., unrolled).
  virtual GBool useDrawForm() { return gFalse; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars() = 0;

  // Does this device need non-text content?
  virtual GBool needNonText() { return gTrue; }

  //----- initialization and control

  // Set default transform matrix.
  virtual void setDefaultCTM(double *ctm);

  // Check to see if a page slice should be displayed.  If this
  // returns false, the page display is aborted.  Typically, an
  // OutputDev will use some alternate means to display the page
  // before returning false.
  virtual GBool checkPageSlice(__attribute__((unused))Page *page, 
		  __attribute__((unused))double hDPI, 
		  __attribute__((unused))double vDPI,
		  __attribute__((unused))int rotate, 
		  __attribute__((unused))GBool useMediaBox, 
		  __attribute__((unused))GBool crop,
		  __attribute__((unused))int sliceX, 
		  __attribute__((unused))int sliceY, 
		  __attribute__((unused))int sliceW, 
		  __attribute__((unused))int sliceH,
		  __attribute__((unused))GBool printing, 
		  __attribute__((unused))Catalog *catalog,
		  __attribute__((unused))GBool (*abortCheckCbk)(void *data) = NULL,
		  __attribute__((unused))void *abortCheckCbkData = NULL)
    { return gTrue; }

  // Start a page.
  virtual void startPage(__attribute__((unused))int pageNum, __attribute__((unused))GfxState *state) {}

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
  virtual void saveState(__attribute__((unused))GfxState *state) {}
  virtual void restoreState(__attribute__((unused))GfxState *state) {}

  //----- update graphics state
  virtual void updateAll(GfxState *state);
  virtual void updateCTM(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))double m11, 
		  __attribute__((unused))double m12,
		  __attribute__((unused))double m21, 
		  __attribute__((unused))double m22, 
		  __attribute__((unused))double m31, 
		  __attribute__((unused))double m32) {}
  virtual void updateLineDash(__attribute__((unused))GfxState *state) {}
  virtual void updateFlatness(__attribute__((unused))GfxState *state) {}
  virtual void updateLineJoin(__attribute__((unused))GfxState *state) {}
  virtual void updateLineCap(__attribute__((unused))GfxState *state) {}
  virtual void updateMiterLimit(__attribute__((unused))GfxState *state) {}
  virtual void updateLineWidth(__attribute__((unused))GfxState *state) {}
  virtual void updateStrokeAdjust(__attribute__((unused))GfxState *state) {}
  virtual void updateFillColorSpace(__attribute__((unused))GfxState *state) {}
  virtual void updateStrokeColorSpace(__attribute__((unused))GfxState *state) {}
  virtual void updateFillColor(__attribute__((unused))GfxState *state) {}
  virtual void updateStrokeColor(__attribute__((unused))GfxState *state) {}
  virtual void updateBlendMode(__attribute__((unused))GfxState *state) {}
  virtual void updateFillOpacity(__attribute__((unused))GfxState *state) {}
  virtual void updateStrokeOpacity(__attribute__((unused))GfxState *state) {}
  virtual void updateFillOverprint(__attribute__((unused))GfxState *state) {}
  virtual void updateStrokeOverprint(__attribute__((unused))GfxState *state) {}
  virtual void updateTransfer(__attribute__((unused))GfxState *state) {}

  //----- update text state
  virtual void updateFont(__attribute__((unused))GfxState *state) {}
  virtual void updateTextMat(__attribute__((unused))GfxState *state) {}
  virtual void updateCharSpace(__attribute__((unused))GfxState *state) {}
  virtual void updateRender(__attribute__((unused))GfxState *state) {}
  virtual void updateRise(__attribute__((unused))GfxState *state) {}
  virtual void updateWordSpace(__attribute__((unused))GfxState *state) {}
  virtual void updateHorizScaling(__attribute__((unused))GfxState *state) {}
  virtual void updateTextPos(__attribute__((unused))GfxState *state) {}
  virtual void updateTextShift(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))double shift) {}

  //----- path painting
  virtual void stroke(__attribute__((unused))GfxState *state) {}
  virtual void fill(__attribute__((unused))GfxState *state) {}
  virtual void eoFill(__attribute__((unused))GfxState *state) {}
  virtual void tilingPatternFill(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))Object *str,
		  __attribute__((unused))int paintType, 
		  __attribute__((unused))Dict *resDict,
		  __attribute__((unused))double *mat, 
		  __attribute__((unused))double *bbox,
		  __attribute__((unused))int x0, 
		  __attribute__((unused))int y0, 
		  __attribute__((unused))int x1, 
		  __attribute__((unused))int y1,
		  __attribute__((unused))double xStep, 
		  __attribute__((unused))double yStep) {}
  virtual GBool functionShadedFill(__attribute__((unused))GfxState *state,
		  __attribute__((unused))GfxFunctionShading *shading)
    { return gFalse; }
  virtual GBool axialShadedFill(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))GfxAxialShading *shading)
    { return gFalse; }
  virtual GBool radialShadedFill(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))GfxRadialShading *shading)
    { return gFalse; }

  //----- path clipping
  virtual void clip(__attribute__((unused))GfxState *state) {}
  virtual void eoClip(__attribute__((unused))GfxState *state) {}
  virtual void clipToStrokePath(__attribute__((unused))GfxState *state) {}

  //----- text drawing
  virtual void beginStringOp(__attribute__((unused))GfxState *state) {}
  virtual void endStringOp(__attribute__((unused))GfxState *state) {}
  virtual void beginString(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))GString *s) {}
  virtual void endString(__attribute__((unused))GfxState *state) {}
  virtual void drawChar(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))double x, 
		  __attribute__((unused))double y,
		  __attribute__((unused))double dx, 
		  __attribute__((unused))double dy,
		  __attribute__((unused))double originX, 
		  __attribute__((unused))double originY,
		  __attribute__((unused))CharCode code, 
		  __attribute__((unused))int nBytes, 
		  __attribute__((unused))Unicode *u, 
		  __attribute__((unused))int uLen) {}
  virtual void drawString(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))GString *s) {}
  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);
  virtual void endType3Char(__attribute__((unused))GfxState *state) {}
  virtual void endTextObject(__attribute__((unused))GfxState *state) {}

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
  virtual void type3D0(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))double wx, 
		  __attribute__((unused))double wy) {}
  virtual void type3D1(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))double wx, 
		  __attribute__((unused))double wy,
		  __attribute__((unused))double llx, 
		  __attribute__((unused))double lly, 
		  __attribute__((unused))double urx, 
		  __attribute__((unused))double ury) {}

  //----- form XObjects
  virtual void drawForm(__attribute__((unused))Ref id) {}

  //----- PostScript XObjects
  virtual void psXObject(__attribute__((unused))Stream *psStream, 
		  __attribute__((unused))Stream *level1Stream) {}

  //----- transparency groups and soft masks
  virtual void beginTransparencyGroup(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))double *bbox, 
		  __attribute__((unused))GfxColorSpace *blendingColorSpace,
		  __attribute__((unused))GBool isolated, 
		  __attribute__((unused))GBool knockout,
		  __attribute__((unused))GBool forSoftMask) {}
  virtual void endTransparencyGroup(__attribute__((unused))GfxState *state) {}
  virtual void paintTransparencyGroup(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))double *bbox) {}
  virtual void setSoftMask(__attribute__((unused))GfxState *state, 
		  __attribute__((unused))double *bbox, 
		  __attribute__((unused))GBool alpha,
		  __attribute__((unused))Function *transferFunc, 
		  __attribute__((unused))GfxColor *backdropColor) {}
  virtual void clearSoftMask(__attribute__((unused))GfxState *state) {}

  //----- links
  virtual void processLink(__attribute__((unused))Link *link, 
		  __attribute__((unused))Catalog *catalog) {}

#if 1 //~tmp: turn off anti-aliasing temporarily
  virtual GBool getVectorAntialias() { return gFalse; }
  virtual void setVectorAntialias(__attribute__((unused))GBool vaa) {}
#endif

private:

  double defCTM[6];		// default coordinate transform matrix
  double defICTM[6];		// inverse of default CTM
};

#endif
