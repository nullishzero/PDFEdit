//========================================================================
//
// PreScanOutputDev.h
//
// Copyright 2005 Glyph & Cog, LLC
//
//========================================================================

#ifndef PRESCANOUTPUTDEV_H
#define PRESCANOUTPUTDEV_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "xpdf/GfxState.h"
#include "xpdf/OutputDev.h"

//------------------------------------------------------------------------
// PreScanOutputDev
//------------------------------------------------------------------------

class PreScanOutputDev: public OutputDev {
public:

  // Constructor.
  PreScanOutputDev();

  // Destructor.
  virtual ~PreScanOutputDev();

  //----- get info about output device

  // Does this device use upside-down coordinates?
  // (Upside-down means (0,0) is the top left corner of the page.)
  virtual GBool upsideDown()const { return gTrue; }

  // Does this device use drawChar() or drawString()?
  virtual GBool useDrawChar()const { return gTrue; }

  // Does this device use beginType3Char/endType3Char?  Otherwise,
  // text in Type 3 fonts will be drawn with drawChar/drawString.
  virtual GBool interpretType3Chars()const { return gTrue; }

  //----- initialization and control

  // Start a page.
  virtual void startPage(int pageNum, GfxState *state);

  // End a page.
  virtual void endPage();

  //----- path painting
  virtual void stroke(GfxState *state);
  virtual void fill(GfxState *state);
  virtual void eoFill(GfxState *state);

  //----- path clipping
  virtual void clip(GfxState *state);
  virtual void eoClip(GfxState *state);

  //----- text drawing
  virtual void beginStringOp(GfxState *state);
  virtual void endStringOp(GfxState *state);
  virtual GBool beginType3Char(GfxState *state, double x, double y,
			       double dx, double dy,
			       CharCode code, Unicode *u, int uLen);
  virtual void endType3Char(GfxState *state);

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

  //----- transparency groups and soft masks
  virtual void beginTransparencyGroup(GfxState *state, const double *bbox,
				      const GfxColorSpace *blendingColorSpace,
				      GBool isolated, GBool knockout,
				      GBool forSoftMask);

  //----- special access

  // Returns true if the operations performed since the last call to
  // clearStats() are all monochrome (black or white).
  GBool isMonochrome()const { return mono; }

  // Returns true if the operations performed since the last call to
  // clearStats() are all gray.
  GBool isGray()const { return gray; }

  // Returns true if the operations performed since the last call to
  // clearStats() included any transparency.
  GBool usesTransparency()const { return transparency; }

  // Returns true if the operations performed since the last call to
  // clearStats() are all rasterizable by GDI calls in GDIOutputDev.
  GBool isAllGDI()const { return gdi; }

  // Clear the stats used by the above functions.
  void clearStats();

private:

  void check(const GfxColorSpace *colorSpace, const GfxColor *color,
	     double opacity, GfxBlendMode blendMode);

  GBool mono;
  GBool gray;
  GBool transparency;
  GBool gdi;
};

#endif
