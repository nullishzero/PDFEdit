//========================================================================
//
// GfxState.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef GFXSTATE_H
#define GFXSTATE_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "xpdf/Object.h"
#include "xpdf/Function.h"

class Array;
class GfxFont;
class PDFRectangle;
class GfxShading;

//------------------------------------------------------------------------
// GfxBlendMode
//------------------------------------------------------------------------

enum GfxBlendMode {
  gfxBlendNormal,
  gfxBlendMultiply,
  gfxBlendScreen,
  gfxBlendOverlay,
  gfxBlendDarken,
  gfxBlendLighten,
  gfxBlendColorDodge,
  gfxBlendColorBurn,
  gfxBlendHardLight,
  gfxBlendSoftLight,
  gfxBlendDifference,
  gfxBlendExclusion,
  gfxBlendHue,
  gfxBlendSaturation,
  gfxBlendColor,
  gfxBlendLuminosity
};

//------------------------------------------------------------------------
// GfxColorComp
//------------------------------------------------------------------------

// 16.16 fixed point color component
typedef int GfxColorComp;

#define gfxColorComp1 0x10000

static inline GfxColorComp dblToCol(double x) {
  return (GfxColorComp)(x * gfxColorComp1);
}

static inline double colToDbl(GfxColorComp x) {
  return (double)x / (double)gfxColorComp1;
}

static inline GfxColorComp byteToCol(Guchar x) {
  // (x / 255) << 16  =  (0.0000000100000001... * x) << 16
  //                  =  ((x << 8) + (x) + (x >> 8) + ...) << 16
  //                  =  (x << 8) + (x) + (x >> 7)
  //                                      [for rounding]
  return (GfxColorComp)((x << 8) + x + (x >> 7));
}

static inline Guchar colToByte(GfxColorComp x) {
  // 255 * x + 0.5  =  256 * x - x + 0x8000
  return (Guchar)(((x << 8) - x + 0x8000) >> 16);
}

//------------------------------------------------------------------------
// GfxColor
//------------------------------------------------------------------------

#define gfxColorMaxComps funcMaxOutputs

struct GfxColor {
  GfxColorComp c[gfxColorMaxComps];
};

//------------------------------------------------------------------------
// GfxGray
//------------------------------------------------------------------------

typedef GfxColorComp GfxGray;

//------------------------------------------------------------------------
// GfxRGB
//------------------------------------------------------------------------

struct GfxRGB {
  GfxColorComp r, g, b;
};

//------------------------------------------------------------------------
// GfxCMYK
//------------------------------------------------------------------------

struct GfxCMYK {
  GfxColorComp c, m, y, k;
};

//------------------------------------------------------------------------
// GfxColorSpace
//------------------------------------------------------------------------

// NB: The nGfxColorSpaceModes constant and the gfxColorSpaceModeNames
// array defined in GfxState.cc must match this enum.
enum GfxColorSpaceMode {
  csDeviceGray,
  csCalGray,
  csDeviceRGB,
  csCalRGB,
  csDeviceCMYK,
  csLab,
  csICCBased,
  csIndexed,
  csSeparation,
  csDeviceN,
  csPattern
};

class GfxColorSpace {
public:

  GfxColorSpace();
  virtual ~GfxColorSpace();
  virtual GfxColorSpace *copy()const = 0;
  virtual GfxColorSpaceMode getMode()const = 0;

  // Construct a color space.  Returns NULL if unsuccessful.
  static GfxColorSpace *parse(const Object *csObj);

  // Convert to gray, RGB, or CMYK.
  virtual void getGray(const GfxColor *color, GfxGray *gray)const = 0;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const = 0;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const = 0;

  // Return the number of color components.
  virtual int getNComps()const = 0;

  // Get this color space's default color.
  virtual void getDefaultColor(GfxColor *color)const = 0;

  // Return the default ranges for each component, assuming an image
  // with a max pixel value of <maxImgPixel>.
  virtual void getDefaultRanges(double *decodeLow, double *decodeRange,
				int maxImgPixel)const;

  // Returns true if painting operations in this color space never
  // mark the page (e.g., the "None" colorant).
  virtual GBool isNonMarking()const { return gFalse; }

  // Return the number of color space modes
  static int getNumColorSpaceModes();

  // Return the name of the <idx>th color space mode.
  static const char *getColorSpaceModeName(int idx);

private:
};

//------------------------------------------------------------------------
// GfxDeviceGrayColorSpace
//------------------------------------------------------------------------

class GfxDeviceGrayColorSpace: public GfxColorSpace {
public:

  GfxDeviceGrayColorSpace();
  virtual ~GfxDeviceGrayColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csDeviceGray; }

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return 1; }
  virtual void getDefaultColor(GfxColor *color)const;

private:
};

//------------------------------------------------------------------------
// GfxCalGrayColorSpace
//------------------------------------------------------------------------

class GfxCalGrayColorSpace: public GfxColorSpace {
public:

  GfxCalGrayColorSpace();
  virtual ~GfxCalGrayColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csCalGray; }

  // Construct a CalGray color space.  Returns NULL if unsuccessful.
  static GfxColorSpace *parse(const Array *arr);

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return 1; }
  virtual void getDefaultColor(GfxColor *color)const;

  // CalGray-specific access.
  double getWhiteX()const { return whiteX; }
  double getWhiteY()const { return whiteY; }
  double getWhiteZ()const { return whiteZ; }
  double getBlackX()const { return blackX; }
  double getBlackY()const { return blackY; }
  double getBlackZ()const { return blackZ; }
  double getGamma()const { return gamma; }

private:

  double whiteX, whiteY, whiteZ;    // white point
  double blackX, blackY, blackZ;    // black point
  double gamma;			    // gamma value
};

//------------------------------------------------------------------------
// GfxDeviceRGBColorSpace
//------------------------------------------------------------------------

class GfxDeviceRGBColorSpace: public GfxColorSpace {
public:

  GfxDeviceRGBColorSpace();
  virtual ~GfxDeviceRGBColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csDeviceRGB; }

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return 3; }
  virtual void getDefaultColor(GfxColor *color)const;

private:
};

//------------------------------------------------------------------------
// GfxCalRGBColorSpace
//------------------------------------------------------------------------

class GfxCalRGBColorSpace: public GfxColorSpace {
public:

  GfxCalRGBColorSpace();
  virtual ~GfxCalRGBColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csCalRGB; }

  // Construct a CalRGB color space.  Returns NULL if unsuccessful.
  static GfxColorSpace *parse(const Array *arr);

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return 3; }
  virtual void getDefaultColor(GfxColor *color)const;

  // CalRGB-specific access.
  double getWhiteX()const { return whiteX; }
  double getWhiteY()const { return whiteY; }
  double getWhiteZ()const { return whiteZ; }
  double getBlackX()const { return blackX; }
  double getBlackY()const { return blackY; }
  double getBlackZ()const { return blackZ; }
  double getGammaR()const { return gammaR; }
  double getGammaG()const { return gammaG; }
  double getGammaB()const { return gammaB; }
  const double *getMatrix()const { return mat; }

private:

  double whiteX, whiteY, whiteZ;    // white point
  double blackX, blackY, blackZ;    // black point
  double gammaR, gammaG, gammaB;    // gamma values
  double mat[9];		    // ABC -> XYZ transform matrix
};

//------------------------------------------------------------------------
// GfxDeviceCMYKColorSpace
//------------------------------------------------------------------------

class GfxDeviceCMYKColorSpace: public GfxColorSpace {
public:

  GfxDeviceCMYKColorSpace();
  virtual ~GfxDeviceCMYKColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csDeviceCMYK; }

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return 4; }
  virtual void getDefaultColor(GfxColor *color)const;

private:
};

//------------------------------------------------------------------------
// GfxLabColorSpace
//------------------------------------------------------------------------

class GfxLabColorSpace: public GfxColorSpace {
public:

  GfxLabColorSpace();
  virtual ~GfxLabColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csLab; }

  // Construct a Lab color space.  Returns NULL if unsuccessful.
  static GfxColorSpace *parse(const Array *arr);

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return 3; }
  virtual void getDefaultColor(GfxColor *color)const;

  virtual void getDefaultRanges(double *decodeLow, double *decodeRange,
				int maxImgPixel)const;

  // Lab-specific access.
  double getWhiteX()const { return whiteX; }
  double getWhiteY()const { return whiteY; }
  double getWhiteZ()const { return whiteZ; }
  double getBlackX()const { return blackX; }
  double getBlackY()const { return blackY; }
  double getBlackZ()const { return blackZ; }
  double getAMin()const { return aMin; }
  double getAMax()const { return aMax; }
  double getBMin()const { return bMin; }
  double getBMax()const { return bMax; }

private:

  double whiteX, whiteY, whiteZ;    // white point
  double blackX, blackY, blackZ;    // black point
  double aMin, aMax, bMin, bMax;    // range for the a and b components
  double kr, kg, kb;		    // gamut mapping mulitpliers
};

//------------------------------------------------------------------------
// GfxICCBasedColorSpace
//------------------------------------------------------------------------

class GfxICCBasedColorSpace: public GfxColorSpace {
public:

  GfxICCBasedColorSpace(int nCompsA, GfxColorSpace *altA,
			const Ref *iccProfileStreamA);
  virtual ~GfxICCBasedColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csICCBased; }

  // Construct an ICCBased color space.  Returns NULL if unsuccessful.
  static GfxColorSpace *parse(const Array *arr);

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return nComps; }
  virtual void getDefaultColor(GfxColor *color)const;

  virtual void getDefaultRanges(double *decodeLow, double *decodeRange,
				int maxImgPixel)const;

  // ICCBased-specific access.
  const GfxColorSpace *getAlt()const { return alt; }

private:

  int nComps;			// number of color components (1, 3, or 4)
  GfxColorSpace *alt;		// alternate color space
  double rangeMin[4];		// min values for each component
  double rangeMax[4];		// max values for each component
  Ref iccProfileStream;		// the ICC profile
};

//------------------------------------------------------------------------
// GfxIndexedColorSpace
//------------------------------------------------------------------------

class GfxIndexedColorSpace: public GfxColorSpace {
public:

  GfxIndexedColorSpace(GfxColorSpace *baseA, int indexHighA);
  virtual ~GfxIndexedColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csIndexed; }

  // Construct a Lab color space.  Returns NULL if unsuccessful.
  static GfxColorSpace *parse(const Array *arr);

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return 1; }
  virtual void getDefaultColor(GfxColor *color)const;

  virtual void getDefaultRanges(double *decodeLow, double *decodeRange,
				int maxImgPixel)const;

  // Indexed-specific access.
  const GfxColorSpace *getBase()const { return base; }
  int getIndexHigh()const { return indexHigh; }
  const Guchar *getLookup()const { return lookup; }
  GfxColor *mapColorToBase(const GfxColor *color, GfxColor *baseColor)const;

private:

  GfxColorSpace *base;		// base color space
  int indexHigh;		// max pixel value
  Guchar *lookup;		// lookup table
};

//------------------------------------------------------------------------
// GfxSeparationColorSpace
//------------------------------------------------------------------------

class GfxSeparationColorSpace: public GfxColorSpace {
public:

  GfxSeparationColorSpace(GString *nameA, GfxColorSpace *altA,
			  Function *funcA);
  virtual ~GfxSeparationColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csSeparation; }

  // Construct a Separation color space.  Returns NULL if unsuccessful.
  static GfxColorSpace *parse(const Array *arr);

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return 1; }
  virtual void getDefaultColor(GfxColor *color)const;

  virtual GBool isNonMarking()const { return nonMarking; }

  // Separation-specific access.
  const GString *getName()const { return name; }
  const GfxColorSpace *getAlt()const { return alt; }
  Function *getFunc()const { return (Function *)func; }

private:

  GString *name;		// colorant name
  GfxColorSpace *alt;		// alternate color space
  Function *func;		// tint transform (into alternate color space)
  GBool nonMarking;
};

//------------------------------------------------------------------------
// GfxDeviceNColorSpace
//------------------------------------------------------------------------

class GfxDeviceNColorSpace: public GfxColorSpace {
public:

  GfxDeviceNColorSpace(int nCompsA, GfxColorSpace *alt, Function *func);
  virtual ~GfxDeviceNColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csDeviceN; }

  // Construct a DeviceN color space.  Returns NULL if unsuccessful.
  static GfxColorSpace *parse(const Array *arr);

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return nComps; }
  virtual void getDefaultColor(GfxColor *color)const;

  virtual GBool isNonMarking()const { return nonMarking; }

  // DeviceN-specific access.
  const GString *getColorantName(int i)const { return names[i]; }
  const GfxColorSpace *getAlt()const { return alt; }
  Function *getTintTransformFunc()const { return (Function *)func; }

private:

  int nComps;			// number of components
  GString			// colorant names
    *names[gfxColorMaxComps];
  GfxColorSpace *alt;		// alternate color space
  Function *func;		// tint transform (into alternate color space)
  GBool nonMarking;
};

//------------------------------------------------------------------------
// GfxPatternColorSpace
//------------------------------------------------------------------------

class GfxPatternColorSpace: public GfxColorSpace {
public:

  GfxPatternColorSpace(GfxColorSpace *underA);
  virtual ~GfxPatternColorSpace();
  virtual GfxColorSpace *copy()const;
  virtual GfxColorSpaceMode getMode()const { return csPattern; }

  // Construct a Pattern color space.  Returns NULL if unsuccessful.
  static GfxColorSpace *parse(const Array *arr);

  virtual void getGray(const GfxColor *color, GfxGray *gray)const;
  virtual void getRGB(const GfxColor *color, GfxRGB *rgb)const;
  virtual void getCMYK(const GfxColor *color, GfxCMYK *cmyk)const;

  virtual int getNComps()const { return 0; }
  virtual void getDefaultColor(GfxColor *color)const;

  // Pattern-specific access.
  GfxColorSpace *getUnder() { return under; }

private:

  GfxColorSpace *under;		// underlying color space (for uncolored
				//   patterns)
};

//------------------------------------------------------------------------
// GfxPattern
//------------------------------------------------------------------------

class GfxPattern {
public:

  GfxPattern(int typeA);
  virtual ~GfxPattern();

  static GfxPattern *parse(const Object *obj);

  virtual GfxPattern *copy()const = 0;

  int getType()const { return type; }

private:

  int type;
};

//------------------------------------------------------------------------
// GfxTilingPattern
//------------------------------------------------------------------------

class GfxTilingPattern: public GfxPattern {
public:

  static GfxTilingPattern *parse(const Object *patObj);
  virtual ~GfxTilingPattern();

  virtual GfxPattern *copy()const;

  int getPaintType()const { return paintType; }
  int getTilingType()const { return tilingType; }
  const double *getBBox()const { return bbox; }
  double getXStep()const { return xStep; }
  double getYStep()const { return yStep; }
  const Dict *getResDict()const
    { return resDict.isDict() ? resDict.getDict() : (Dict *)NULL; }
  const double *getMatrix()const { return matrix; }
  Object *getContentStream() { return &contentStream; }

private:

  GfxTilingPattern(int paintTypeA, int tilingTypeA,
		   const double *bboxA, double xStepA, double yStepA,
		   const Object *resDictA, const double *matrixA,
		   const Object *contentStreamA);

  int paintType;
  int tilingType;
  double bbox[4];
  double xStep, yStep;
  Object resDict;
  double matrix[6];
  Object contentStream;
};

//------------------------------------------------------------------------
// GfxShadingPattern
//------------------------------------------------------------------------

class GfxShadingPattern: public GfxPattern {
public:

  static GfxShadingPattern *parse(const Object *patObj);
  virtual ~GfxShadingPattern();

  virtual GfxPattern *copy()const;

  GfxShading *getShading() { return shading; }
  const double *getMatrix()const { return matrix; }

private:

  GfxShadingPattern(GfxShading *shadingA, const double *matrixA);

  GfxShading *shading;
  double matrix[6];
};

//------------------------------------------------------------------------
// GfxShading
//------------------------------------------------------------------------

class GfxShading {
public:

  GfxShading(int typeA);
  GfxShading(const GfxShading *shading);
  virtual ~GfxShading();

  static GfxShading *parse(const Object *obj);

  virtual GfxShading *copy()const = 0;

  int getType()const { return type; }
  const GfxColorSpace *getColorSpace()const { return colorSpace; }
  const GfxColor *getBackground()const { return &background; }
  GBool getHasBackground()const { return hasBackground; }
  void getBBox(double *xMinA, double *yMinA, double *xMaxA, double *yMaxA)const
    { *xMinA = xMin; *yMinA = yMin; *xMaxA = xMax; *yMaxA = yMax; }
  GBool getHasBBox()const { return hasBBox; }

protected:

  GBool init(const Dict *dict);

  int type;
  GfxColorSpace *colorSpace;
  GfxColor background;
  GBool hasBackground;
  double xMin, yMin, xMax, yMax;
  GBool hasBBox;
};

//------------------------------------------------------------------------
// GfxFunctionShading
//------------------------------------------------------------------------

class GfxFunctionShading: public GfxShading {
public:

  GfxFunctionShading(double x0A, double y0A,
		     double x1A, double y1A,
		     double *matrixA,
		     Function **funcsA, int nFuncsA);
  GfxFunctionShading(const GfxFunctionShading *shading);
  virtual ~GfxFunctionShading();

  static GfxFunctionShading *parse(const Dict *dict);

  virtual GfxShading *copy()const;

  void getDomain(double *x0A, double *y0A, double *x1A, double *y1A)const
    { *x0A = x0; *y0A = y0; *x1A = x1; *y1A = y1; }
  const double *getMatrix()const { return matrix; }
  int getNFuncs()const { return nFuncs; }
  Function *getFunc(int i)const { return funcs[i]; }
  void getColor(double x, double y, GfxColor *color)const;

private:

  double x0, y0, x1, y1;
  double matrix[6];
  Function *funcs[gfxColorMaxComps];
  int nFuncs;
};

//------------------------------------------------------------------------
// GfxAxialShading
//------------------------------------------------------------------------

class GfxAxialShading: public GfxShading {
public:

  GfxAxialShading(double x0A, double y0A,
		  double x1A, double y1A,
		  double t0A, double t1A,
		  Function **funcsA, int nFuncsA,
		  GBool extend0A, GBool extend1A);
  GfxAxialShading(const GfxAxialShading *shading);
  virtual ~GfxAxialShading();

  static GfxAxialShading *parse(const Dict *dict);

  virtual GfxShading *copy()const;

  void getCoords(double *x0A, double *y0A, double *x1A, double *y1A)
    { *x0A = x0; *y0A = y0; *x1A = x1; *y1A = y1; }
  double getDomain0()const { return t0; }
  double getDomain1()const { return t1; }
  GBool getExtend0()const { return extend0; }
  GBool getExtend1()const { return extend1; }
  int getNFuncs()const { return nFuncs; }
  Function *getFunc(int i)const { return funcs[i]; }
  void getColor(double t, GfxColor *color)const;

private:

  double x0, y0, x1, y1;
  double t0, t1;
  Function *funcs[gfxColorMaxComps];
  int nFuncs;
  GBool extend0, extend1;
};

//------------------------------------------------------------------------
// GfxRadialShading
//------------------------------------------------------------------------

class GfxRadialShading: public GfxShading {
public:

  GfxRadialShading(double x0A, double y0A, double r0A,
		   double x1A, double y1A, double r1A,
		   double t0A, double t1A,
		   Function **funcsA, int nFuncsA,
		   GBool extend0A, GBool extend1A);
  GfxRadialShading(const GfxRadialShading *shading);
  virtual ~GfxRadialShading();

  static GfxRadialShading *parse(const Dict *dict);

  virtual GfxShading *copy()const;

  void getCoords(double *x0A, double *y0A, double *r0A,
		 double *x1A, double *y1A, double *r1A)
    { *x0A = x0; *y0A = y0; *r0A = r0; *x1A = x1; *y1A = y1; *r1A = r1; }
  double getDomain0()const { return t0; }
  double getDomain1()const { return t1; }
  GBool getExtend0()const { return extend0; }
  GBool getExtend1()const { return extend1; }
  int getNFuncs()const { return nFuncs; }
  Function *getFunc(int i)const { return funcs[i]; }
  void getColor(double t, GfxColor *color)const;

private:

  double x0, y0, r0, x1, y1, r1;
  double t0, t1;
  Function *funcs[gfxColorMaxComps];
  int nFuncs;
  GBool extend0, extend1;
};

//------------------------------------------------------------------------
// GfxGouraudTriangleShading
//------------------------------------------------------------------------

struct GfxGouraudVertex {
  double x, y;
  GfxColor color;
};

class GfxGouraudTriangleShading: public GfxShading {
public:

  GfxGouraudTriangleShading(int typeA,
			    GfxGouraudVertex *verticesA, int nVerticesA,
			    int (*trianglesA)[3], int nTrianglesA,
			    Function **funcsA, int nFuncsA);
  GfxGouraudTriangleShading(const GfxGouraudTriangleShading *shading);
  virtual ~GfxGouraudTriangleShading();

  static GfxGouraudTriangleShading *parse(int typeA, const Dict *dict, Stream *str);

  virtual GfxShading *copy()const;

  int getNTriangles()const { return nTriangles; }
  void getTriangle(int i, double *x0, double *y0, GfxColor *color0,
		   double *x1, double *y1, GfxColor *color1,
		   double *x2, double *y2, GfxColor *color2)const;

private:

  GfxGouraudVertex *vertices;
  int nVertices;
  int (*triangles)[3];
  int nTriangles;
  Function *funcs[gfxColorMaxComps];
  int nFuncs;
};

//------------------------------------------------------------------------
// GfxPatchMeshShading
//------------------------------------------------------------------------

struct GfxPatch {
  double x[4][4];
  double y[4][4];
  GfxColor color[2][2];
};

class GfxPatchMeshShading: public GfxShading {
public:

  GfxPatchMeshShading(int typeA, GfxPatch *patchesA, int nPatchesA,
		      Function **funcsA, int nFuncsA);
  GfxPatchMeshShading(const GfxPatchMeshShading *shading);
  virtual ~GfxPatchMeshShading();

  static GfxPatchMeshShading *parse(int typeA, const Dict *dict, Stream *str);

  virtual GfxShading *copy()const;

  int getNPatches()const { return nPatches; }
  GfxPatch *getPatch(int i)const { return &patches[i]; }

private:

  GfxPatch *patches;
  int nPatches;
  Function *funcs[gfxColorMaxComps];
  int nFuncs;
};

//------------------------------------------------------------------------
// GfxImageColorMap
//------------------------------------------------------------------------

class GfxImageColorMap {
public:

  // Constructor.
  GfxImageColorMap(int bitsA, const Object *decode, GfxColorSpace *colorSpaceA);

  // Destructor.
  ~GfxImageColorMap();

  // Return a copy of this color map.
  GfxImageColorMap *copy()const { return new GfxImageColorMap(this); }

  // Is color map valid?
  GBool isOk()const { return ok; }

  // Get the color space.
  const GfxColorSpace *getColorSpace()const { return colorSpace; }

  // Get stream decoding info.
  int getNumPixelComps()const { return nComps; }
  int getBits()const { return bits; }

  // Get decode table.
  double getDecodeLow(int i)const { return decodeLow[i]; }
  double getDecodeHigh(int i)const { return decodeLow[i] + decodeRange[i]; }

  // Convert an image pixel to a color.
  void getGray(const Guchar *x, GfxGray *gray)const;
  void getRGB(const Guchar *x, GfxRGB *rgb)const;
  void getCMYK(const Guchar *x, GfxCMYK *cmyk)const;
  void getColor(const Guchar *x, GfxColor *color)const;

private:

  GfxImageColorMap(const GfxImageColorMap *colorMap);

  GfxColorSpace *colorSpace;	// the image color space
  int bits;			// bits per component
  int nComps;			// number of components in a pixel
  const GfxColorSpace *colorSpace2;	// secondary color space
  int nComps2;			// number of components in colorSpace2
  GfxColorComp *		// lookup table
    lookup[gfxColorMaxComps];
  double			// minimum values for each component
    decodeLow[gfxColorMaxComps];
  double			// max - min value for each component
    decodeRange[gfxColorMaxComps];
  GBool ok;
};

//------------------------------------------------------------------------
// GfxSubpath and GfxPath
//------------------------------------------------------------------------

class GfxSubpath {
public:

  // Constructor.
  GfxSubpath(double x1, double y1);

  // Destructor.
  ~GfxSubpath();

  // Copy.
  GfxSubpath *copy()const { return new GfxSubpath(this); }

  // Get points.
  int getNumPoints()const { return n; }
  double getX(int i)const { return x[i]; }
  double getY(int i)const { return y[i]; }
  GBool getCurve(int i)const { return curve[i]; }

  // Get last point.
  double getLastX()const { return x[n-1]; }
  double getLastY()const { return y[n-1]; }

  // Add a line segment.
  void lineTo(double x1, double y1);

  // Add a Bezier curve.
  void curveTo(double x1, double y1, double x2, double y2,
	       double x3, double y3);

  // Close the subpath.
  void close();
  GBool isClosed()const { return closed; }

  // Add (<dx>, <dy>) to each point in the subpath.
  void offset(double dx, double dy);

private:

  double *x, *y;		// points
  GBool *curve;			// curve[i] => point i is a control point
				//   for a Bezier curve
  int n;			// number of points
  int size;			// size of x/y arrays
  GBool closed;			// set if path is closed

  GfxSubpath(const GfxSubpath *subpath);
};

class GfxPath {
public:

  // Constructor.
  GfxPath();

  // Destructor.
  ~GfxPath();

  // Copy.
  GfxPath *copy()const
    { return new GfxPath(justMoved, firstX, firstY, subpaths, n, size); }

  // Is there a current point?
  GBool isCurPt()const { return n > 0 || justMoved; }

  // Is the path non-empty, i.e., is there at least one segment?
  GBool isPath()const { return n > 0; }

  // Get subpaths.
  int getNumSubpaths()const { return n; }
  GfxSubpath *getSubpath(int i)const { return subpaths[i]; }

  // Get last point on last subpath.
  double getLastX()const { return (0 < n) ? subpaths[n-1]->getLastX() : 0; }
  double getLastY()const { return (0 < n) ? subpaths[n-1]->getLastY() : 0; }

  // Move the current point.
  void moveTo(double x, double y);

  // Add a segment to the last subpath.
  void lineTo(double x, double y);

  // Add a Bezier curve to the last subpath
  void curveTo(double x1, double y1, double x2, double y2,
	       double x3, double y3);

  // Close the last subpath.
  void close();

  // Append <path> to <this>.
  void append(GfxPath *path);

  // Add (<dx>, <dy>) to each point in the path.
  void offset(double dx, double dy);

private:

  GBool justMoved;		// set if a new subpath was just started
  double firstX, firstY;	// first point in new subpath
  GfxSubpath **subpaths;	// subpaths
  int n;			// number of subpaths
  int size;			// size of subpaths array

  GfxPath(GBool justMoved1, double firstX1, double firstY1,
	  GfxSubpath **const subpaths1, int n1, int size1);
};

//------------------------------------------------------------------------
// GfxState
//------------------------------------------------------------------------

class GfxState {
public:

  // Construct a default GfxState, for a device with resolution <hDPI>
  // x <vDPI>, page box <pageBox>, page rotation <rotateA>, and
  // coordinate system specified by <upsideDown>.
  GfxState(double hDPIA, double vDPIA, const PDFRectangle *pageBox,
	   int rotateA, GBool upsideDown);

  // Destructor.
  ~GfxState();

  // Copy.
  GfxState *copy(bool onlyOnePath = true)const { return new GfxState(this, onlyOnePath); }

  // Accessors.
  double getHDPI()const { return hDPI; }
  double getVDPI()const { return vDPI; }
  const double *getCTM()const { return ctm; }
  double getX1()const { return px1; }
  double getY1()const { return py1; }
  double getX2()const { return px2; }
  double getY2()const { return py2; }
  double getPageWidth()const { return pageWidth; }
  double getPageHeight()const { return pageHeight; }
  int getRotate()const { return rotate; }
  const GfxColor *getFillColor()const { return &fillColor; }
  const GfxColor *getStrokeColor()const { return &strokeColor; }
  void getFillGray(GfxGray *gray)const
    { fillColorSpace->getGray(&fillColor, gray); }
  void getStrokeGray(GfxGray *gray)const
    { strokeColorSpace->getGray(&strokeColor, gray); }
  void getFillRGB(GfxRGB *rgb)const
    { fillColorSpace->getRGB(&fillColor, rgb); }
  void getStrokeRGB(GfxRGB *rgb)const
    { strokeColorSpace->getRGB(&strokeColor, rgb); }
  void getFillCMYK(GfxCMYK *cmyk)const
    { fillColorSpace->getCMYK(&fillColor, cmyk); }
  void getStrokeCMYK(GfxCMYK *cmyk)const
    { strokeColorSpace->getCMYK(&strokeColor, cmyk); }
  const GfxColorSpace *getFillColorSpace()const { return fillColorSpace; }
  const GfxColorSpace *getStrokeColorSpace()const { return strokeColorSpace; }
  const GfxPattern *getFillPattern()const { return fillPattern; }
  const GfxPattern *getStrokePattern()const { return strokePattern; }
  GfxBlendMode getBlendMode()const { return blendMode; }
  double getFillOpacity()const { return fillOpacity; }
  double getStrokeOpacity()const { return strokeOpacity; }
  GBool getFillOverprint()const { return fillOverprint; }
  GBool getStrokeOverprint()const { return strokeOverprint; }
  Function **getTransfer()const { return (Function **)transfer; }
  double getLineWidth()const { return lineWidth; }
  void getLineDash(double **dash, int *length, double *start)const
    { *dash = lineDash; *length = lineDashLength; *start = lineDashStart; }
  int getFlatness()const { return flatness; }
  int getLineJoin()const { return lineJoin; }
  int getLineCap()const { return lineCap; }
  double getMiterLimit()const { return miterLimit; }
  GBool getStrokeAdjust()const { return strokeAdjust; }
  const GfxFont *getFont()const { return font; }
  double getFontSize()const { return fontSize; }
  const double *getTextMat()const { return textMat; }
  double getCharSpace()const { return charSpace; }
  double getWordSpace()const { return wordSpace; }
  double getHorizScaling()const { return horizScaling; }
  double getLeading()const { return leading; }
  double getRise()const { return rise; }
  int getRender()const { return render; }
  GfxPath *getPath() { return path; }
  void setPath(GfxPath *pathA);
  double getCurX()const { return curX; }
  double getCurY()const { return curY; }
  void getClipBBox(double *xMin, double *yMin, double *xMax, double *yMax)const
    { *xMin = clipXMin; *yMin = clipYMin; *xMax = clipXMax; *yMax = clipYMax; }
  void getUserClipBBox(double *xMin, double *yMin, double *xMax, double *yMax)const;
  double getLineX()const { return lineX; }
  double getLineY()const { return lineY; }

  // Is there a current point/path?
  GBool isCurPt()const { return path->isCurPt(); }
  GBool isPath()const { return path->isPath(); }

  // Transforms.
  void transform(double x1, double y1, double *x2, double *y2)const
    { *x2 = ctm[0] * x1 + ctm[2] * y1 + ctm[4];
      *y2 = ctm[1] * x1 + ctm[3] * y1 + ctm[5]; }
  void transformDelta(double x1, double y1, double *x2, double *y2)const
    { *x2 = ctm[0] * x1 + ctm[2] * y1;
      *y2 = ctm[1] * x1 + ctm[3] * y1; }
  void textTransform(double x1, double y1, double *x2, double *y2)const
    { *x2 = textMat[0] * x1 + textMat[2] * y1 + textMat[4];
      *y2 = textMat[1] * x1 + textMat[3] * y1 + textMat[5]; }
  void textTransformDelta(double x1, double y1, double *x2, double *y2)const
    { *x2 = textMat[0] * x1 + textMat[2] * y1;
      *y2 = textMat[1] * x1 + textMat[3] * y1; }
  double transformWidth(double w)const;
  double getTransformedLineWidth()const
    { return transformWidth(lineWidth); }
  double getTransformedFontSize()const;
  void getFontTransMat(double *m11, double *m12, double *m21, double *m22)const;

  // Change state parameters.
  void setCTM(double a, double b, double c,
	      double d, double e, double f);
  void concatCTM(double a, double b, double c,
		 double d, double e, double f);
  void shiftCTM(double tx, double ty);
  void setFillColorSpace(GfxColorSpace *colorSpace);
  void setStrokeColorSpace(GfxColorSpace *colorSpace);
  void setFillColor(const GfxColor *color) { fillColor = *color; }
  void setStrokeColor(const GfxColor *color) { strokeColor = *color; }
  void setFillPattern(GfxPattern *pattern);
  void setStrokePattern(GfxPattern *pattern);
  void setBlendMode(GfxBlendMode mode) { blendMode = mode; }
  void setFillOpacity(double opac) { fillOpacity = opac; }
  void setStrokeOpacity(double opac) { strokeOpacity = opac; }
  void setFillOverprint(GBool op) { fillOverprint = op; }
  void setStrokeOverprint(GBool op) { strokeOverprint = op; }
  void setTransfer(Function **funcs);
  void setLineWidth(double width) { lineWidth = width; }
  void setLineDash(double *dash, int length, double start);
  void setFlatness(int flatness1) { flatness = flatness1; }
  void setLineJoin(int lineJoin1) { lineJoin = lineJoin1; }
  void setLineCap(int lineCap1) { lineCap = lineCap1; }
  void setMiterLimit(double limit) { miterLimit = limit; }
  void setStrokeAdjust(GBool sa) { strokeAdjust = sa; }
  void setFont(GfxFont *fontA, double fontSizeA)
    { font = fontA; fontSize = fontSizeA; }
  void setTextMat(double a, double b, double c,
		  double d, double e, double f)
    { textMat[0] = a; textMat[1] = b; textMat[2] = c;
      textMat[3] = d; textMat[4] = e; textMat[5] = f; }
  void setCharSpace(double space)
    { charSpace = space; }
  void setWordSpace(double space)
    { wordSpace = space; }
  void setHorizScaling(double scale)
    { horizScaling = 0.01 * scale; }
  void setLeading(double leadingA)
    { leading = leadingA; }
  void setRise(double riseA)
    { rise = riseA; }
  void setRender(int renderA)
    { render = renderA; }

  // Add to path.
  void moveTo(double x, double y)
    { path->moveTo(curX = x, curY = y); }
  void lineTo(double x, double y)
    { path->lineTo(curX = x, curY = y); }
  void curveTo(double x1, double y1, double x2, double y2,
	       double x3, double y3)
    { path->curveTo(x1, y1, x2, y2, curX = x3, curY = y3); }
  void closePath()
    { path->close(); curX = path->getLastX(); curY = path->getLastY(); }
  void clearPath();

  // Update clip region.
  void clip();
  void clipToStrokePath();

  // Text position.
  void textSetPos(double tx, double ty) { lineX = tx; lineY = ty; }
  void textMoveTo(double tx, double ty)
    { lineX = tx; lineY = ty; textTransform(tx, ty, &curX, &curY); }
  void textShift(double tx, double ty);
  void shift(double dx, double dy);

  // Push/pop GfxState on/off stack.
  GfxState *save();
  GfxState *restore();
  GBool hasSaves() { return saved != NULL; }

  // Misc
  GBool parseBlendMode(const Object *obj, GfxBlendMode *mode);

private:

  double hDPI, vDPI;		// resolution
  double ctm[6];		// coord transform matrix
  double px1, py1, px2, py2;	// page corners (user coords)
  double pageWidth, pageHeight;	// page size (pixels)
  int rotate;			// page rotation angle

  GfxColorSpace *fillColorSpace;   // fill color space
  GfxColorSpace *strokeColorSpace; // stroke color space
  GfxColor fillColor;		// fill color
  GfxColor strokeColor;		// stroke color
  GfxPattern *fillPattern;	// fill pattern
  GfxPattern *strokePattern;	// stroke pattern
  GfxBlendMode blendMode;	// transparency blend mode
  double fillOpacity;		// fill opacity
  double strokeOpacity;		// stroke opacity
  GBool fillOverprint;		// fill overprint
  GBool strokeOverprint;	// stroke overprint
  Function *transfer[4];	// transfer function (entries may be: all
				//   NULL = identity; last three NULL =
				//   single function; all four non-NULL =
				//   R,G,B,gray functions)

  double lineWidth;		// line width
  double *lineDash;		// line dash
  int lineDashLength;
  double lineDashStart;
  int flatness;			// curve flatness
  int lineJoin;			// line join style
  int lineCap;			// line cap style
  double miterLimit;		// line miter limit
  GBool strokeAdjust;		// stroke adjustment

  GfxFont *font;		// font
  double fontSize;		// font size
  double textMat[6];		// text matrix
  double charSpace;		// character spacing
  double wordSpace;		// word spacing
  double horizScaling;		// horizontal scaling
  double leading;		// text leading
  double rise;			// text rise
  int render;			// text rendering mode

  GfxPath *path;		// array of path elements
  double curX, curY;		// current point (user coords)
  double lineX, lineY;		// start of current text line (text coords)

  double clipXMin, clipYMin,	// bounding box for clip region
         clipXMax, clipYMax;

  GfxState *saved;		// next GfxState on stack

  GfxState(const GfxState *state, bool onlyOnePath = true);
};

#endif
