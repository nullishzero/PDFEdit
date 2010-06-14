//========================================================================
//
// GfxFont.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef GFXFONT_H
#define GFXFONT_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "goo/GString.h"
#include "xpdf/Object.h"
#include "xpdf/CharTypes.h"

class Dict;
class CMap;
class CharCodeToUnicode;
class FoFiTrueType;
struct GfxFontCIDWidths;

//------------------------------------------------------------------------
// GfxFontType
//------------------------------------------------------------------------

enum GfxFontType {
  //----- Gfx8BitFont
  fontUnknownType,
  fontType1,
  fontType1C,
  fontType1COT,
  fontType3,
  fontTrueType,
  fontTrueTypeOT,
  //----- GfxCIDFont
  fontCIDType0,
  fontCIDType0C,
  fontCIDType0COT,
  fontCIDType2,
  fontCIDType2OT
};

//------------------------------------------------------------------------
// GfxFontCIDWidths
//------------------------------------------------------------------------

struct GfxFontCIDWidthExcep {
  CID first;			// this record applies to
  CID last;			//   CIDs <first>..<last>
  double width;			// char width
};

struct GfxFontCIDWidthExcepV {
  CID first;			// this record applies to
  CID last;			//   CIDs <first>..<last>
  double height;		// char height
  double vx, vy;		// origin position
};

struct GfxFontCIDWidths {
  double defWidth;		// default char width
  double defHeight;		// default char height
  double defVY;			// default origin position
  GfxFontCIDWidthExcep *exceps;	// exceptions
  int nExceps;			// number of valid entries in exceps
  GfxFontCIDWidthExcepV *	// exceptions for vertical font
    excepsV;
  int nExcepsV;			// number of valid entries in excepsV
};

//------------------------------------------------------------------------
// GfxFont
//------------------------------------------------------------------------

#define fontFixedWidth (1 << 0)
#define fontSerif      (1 << 1)
#define fontSymbolic   (1 << 2)
#define fontItalic     (1 << 6)
#define fontBold       (1 << 18)

class GfxFont {
public:

  // Build a GfxFont object.
  static GfxFont *makeFont(XRef *xref, const char *tagA, Ref idA, const Dict *fontDict);

  GfxFont(const char *tagA, Ref idA, GString *nameA);

  virtual ~GfxFont();

  GBool isOk()const { return ok; }

  // Get font tag.
  const GString *getTag()const { return tag; }

  // Get font dictionary ID.
  const Ref *getID()const { return &id; }

  // Does this font match the tag?
  GBool matches(const char *tagA)const { return !tag->cmp(tagA); }

  // Get base font name.
  const GString *getName()const { return name; }

  // Get the original font name (ignornig any munging that might have
  // been done to map to a canonical Base-14 font name).
  const GString *getOrigName()const { return origName; }

  // Get font type.
  GfxFontType getType()const { return type; }
  virtual GBool isCIDFont()const { return gFalse; }

  // Get embedded font ID, i.e., a ref for the font file stream.
  // Returns false if there is no embedded font.
  GBool getEmbeddedFontID(Ref *embID)const
    { *embID = embFontID; return embFontID.num >= 0; }

  // Get the PostScript font name for the embedded font.  Returns
  // NULL if there is no embedded font.
  const GString *getEmbeddedFontName()const { return embFontName; }

  // Get the name of the external font file.  Returns NULL if there
  // is no external font file.
  const GString *getExtFontFile()const { return extFontFile; }

  // Get font descriptor flags.
  int getFlags()const { return flags; }
  GBool isFixedWidth()const { return flags & fontFixedWidth; }
  GBool isSerif()const { return flags & fontSerif; }
  GBool isSymbolic()const { return flags & fontSymbolic; }
  GBool isItalic()const { return flags & fontItalic; }
  GBool isBold()const { return flags & fontBold; }

  // Return the font matrix.
  const double *getFontMatrix()const { return fontMat; }

  // Return the font bounding box.
  const double *getFontBBox()const { return fontBBox; }

  // Return the ascent and descent values.
  double getAscent()const { return ascent; }
  double getDescent()const { return descent; }

  // Return the writing mode (0=horizontal, 1=vertical).
  virtual int getWMode()const { return 0; }

  // Read an external or embedded font file into a buffer.
  char *readExtFontFile(int *len)const;
  char *readEmbFontFile(XRef *xref, int *len);

  // Get the next char from a string <s> of <len> bytes, returning the
  // char <code>, its Unicode mapping <u>, its displacement vector
  // (<dx>, <dy>), and its origin offset vector (<ox>, <oy>).  <uSize>
  // is the number of entries available in <u>, and <uLen> is set to
  // the number actually used.  Returns the number of bytes used by
  // the char code.
  virtual int getNextChar(const char *s, int len, CharCode *code,
			  Unicode *u, int uSize, int *uLen,
			  double *dx, double *dy, double *ox, double *oy)const = 0;

  // Transfroms given unicode to the code which can be stored to the
  // text operator - this is kind of inversion function to getNextChar
  virtual CharCode getCodeFromUnicode(const Unicode *u, int uSize)const;
protected:

  void readFontDescriptor(XRef *xref, const Dict *fontDict);
  CharCodeToUnicode *readToUnicodeCMap(const Dict *fontDict, int nBits,
				       CharCodeToUnicode *ctu)const;
  void findExtFontFile();
  virtual const CharCodeToUnicode * getCtu()const =0;

  GString *tag;			// PDF font tag
  Ref id;			// reference (used as unique ID)
  GString *name;		// font name
  GString *origName;		// original font name
  GfxFontType type;		// type of font
  int flags;			// font descriptor flags
  GString *embFontName;		// name of embedded font
  Ref embFontID;		// ref to embedded font file stream
  GString *extFontFile;		// external font file name
  double fontMat[6];		// font matrix (Type 3 only)
  double fontBBox[4];		// font bounding box (Type 3 only)
  double missingWidth;		// "default" width
  double ascent;		// max height above baseline
  double descent;		// max depth below baseline
  GBool ok;
};

//------------------------------------------------------------------------
// Gfx8BitFont
//------------------------------------------------------------------------

class Gfx8BitFont: public GfxFont {
public:

  Gfx8BitFont(XRef *xref, const char *tagA, Ref idA, GString *nameA,
	      GfxFontType typeA, const Dict *fontDict);

  virtual ~Gfx8BitFont();

  virtual int getNextChar(const char *s, int len, CharCode *code,
			  Unicode *u, int uSize, int *uLen,
			  double *dx, double *dy, double *ox, double *oy)const;

  // Return the encoding.
  char ** getEncoding()const { return (char**)enc; }

  // Return the Unicode map.
  CharCodeToUnicode *getToUnicode();

  // Return the character name associated with <code>.
  const char *getCharName(int code)const { return enc[code]; }

  // Returns true if the PDF font specified an encoding.
  GBool getHasEncoding() { return hasEncoding; }

  // Returns true if the PDF font specified MacRomanEncoding.
  GBool getUsesMacRomanEnc() { return usesMacRomanEnc; }

  // Get width of a character.
  double getWidth(Guchar c) { return widths[c]; }

  // Return a char code-to-GID mapping for the provided font file.
  // (This is only useful for TrueType fonts.)
  Gushort *getCodeToGIDMap(FoFiTrueType *ff);

  // Return the Type 3 CharProc dictionary, or NULL if none.
  const Dict *getCharProcs()const;

  // Return the Type 3 CharProc for the character associated with <code>.
  Object *getCharProc(int code, Object *proc)const;

  // Return the Type 3 Resources dictionary, or NULL if none.
  const Dict *getResources()const;

protected:
  const CharCodeToUnicode *getCtu()const { return ctu; }

private:

  char *enc[256];		// char code --> char name
  char encFree[256];		// boolean for each char name: if set,
				//   the string is malloc'ed
  CharCodeToUnicode *ctu;	// char code --> Unicode
  GBool hasEncoding;
  GBool usesMacRomanEnc;
  double widths[256];		// character widths
  Object charProcs;		// Type 3 CharProcs dictionary
  Object resources;		// Type 3 Resources dictionary
};

//------------------------------------------------------------------------
// GfxCIDFont
//------------------------------------------------------------------------

class GfxCIDFont: public GfxFont {
public:

  GfxCIDFont(XRef *xref, const char *tagA, Ref idA, GString *nameA,
	     const Dict *fontDict);

  virtual ~GfxCIDFont();

  virtual GBool isCIDFont()const { return gTrue; }

  virtual int getNextChar(const char *s, int len, CharCode *code,
			  Unicode *u, int uSize, int *uLen,
			  double *dx, double *dy, double *ox, double *oy)const;

  // Return the writing mode (0=horizontal, 1=vertical).
  virtual int getWMode()const;

  // Return the Unicode map.
  CharCodeToUnicode *getToUnicode();

  // Get the collection name (<registry>-<ordering>).
  const GString *getCollection()const;

  // Return the CID-to-GID mapping table.  These should only be called
  // if type is fontCIDType2.
  const Gushort *getCIDToGID()const { return cidToGID; }
  int getCIDToGIDLen()const { return cidToGIDLen; }
protected:
  const CharCodeToUnicode *getCtu()const { return ctu; }
private:

  CMap *cMap;			// char code --> CID
  CharCodeToUnicode *ctu;	// CID --> Unicode
  GfxFontCIDWidths widths;	// character widths
  Gushort *cidToGID;		// CID --> GID mapping (for embedded
				//   TrueType fonts)
  int cidToGIDLen;
};

//------------------------------------------------------------------------
// GfxFontDict
//------------------------------------------------------------------------

class GfxFontDict {
public:

  // Build the font dictionary, given the PDF font dictionary.
  GfxFontDict(XRef *xref, const Ref *fontDictRef, const Dict *fontDict);

  // Destructor.
  ~GfxFontDict();

  // Get the specified font.
  GfxFont *lookup(const char *tag)const;

  // Iterative access.
  int getNumFonts()const { return numFonts; }
  GfxFont *getFont(int i)const { return fonts[i]; }

private:

  GfxFont **fonts;		// list of fonts
  int numFonts;			// number of fonts
};

#endif
