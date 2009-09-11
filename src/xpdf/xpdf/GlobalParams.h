//========================================================================
//
// GlobalParams.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
// Changes: 
// Michal Hocko   - initGlobalParams, destroyGlobalParams methods added
//
//========================================================================

#ifndef GLOBALPARAMS_H
#define GLOBALPARAMS_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include "goo/gtypes.h"
#include "xpdf/CharTypes.h"

#if MULTITHREADED
#include "goo/GMutex.h"
#endif

class GString;
class GList;
class GHash;
class NameToCharCode;
class CharCodeToUnicode;
class CharCodeToUnicodeCache;
class UnicodeMap;
class UnicodeMapCache;
class CMap;
class CMapCache;
struct XpdfSecurityHandler;
class GlobalParams;
#ifdef WIN32
class WinFontList;
#endif

//------------------------------------------------------------------------

// The global parameters object.
extern GlobalParams *globalParams;

//------------------------------------------------------------------------

enum DisplayFontParamKind {
  displayFontT1,
  displayFontTT
};

struct DisplayFontParamT1 {
  GString *fileName;
};

struct DisplayFontParamTT {
  GString *fileName;
};

class DisplayFontParam {
public:

  GString *name;		// font name for 8-bit fonts and named
				//   CID fonts; collection name for
				//   generic CID fonts
  DisplayFontParamKind kind;
  union {
    DisplayFontParamT1 t1;
    DisplayFontParamTT tt;
  };

  DisplayFontParam(GString *nameA, DisplayFontParamKind kindA);
  virtual ~DisplayFontParam();
};

//------------------------------------------------------------------------

class PSFontParam {
public:

  GString *pdfFontName;		// PDF font name for 8-bit fonts and
				//   named 16-bit fonts; char collection
				//   name for generic 16-bit fonts
  int wMode;			// writing mode (0=horiz, 1=vert) for
				//   16-bit fonts
  GString *psFontName;		// PostScript font name
  GString *encoding;		// encoding, for 16-bit fonts only

  PSFontParam(GString *pdfFontNameA, int wModeA,
	      GString *psFontNameA, GString *encodingA);
  ~PSFontParam();
};

//------------------------------------------------------------------------

enum PSLevel {
  psLevel1,
  psLevel1Sep,
  psLevel2,
  psLevel2Sep,
  psLevel3,
  psLevel3Sep
};

//------------------------------------------------------------------------

enum EndOfLineKind {
  eolUnix,			// LF
  eolDOS,			// CR+LF
  eolMac			// CR
};

//------------------------------------------------------------------------

enum ScreenType {
  screenUnset,
  screenDispersed,
  screenClustered,
  screenStochasticClustered
};

//------------------------------------------------------------------------

class KeyBinding {
public:

  int code;			// 0x20 .. 0xfe = ASCII,
				//   >=0x10000 = special keys, mouse buttons,
				//   etc. (xpdfKeyCode* symbols)
  int mods;			// modifiers (xpdfKeyMod* symbols, or-ed
				//   together)
  int context;			// context (xpdfKeyContext* symbols, or-ed
				//   together)
  GList *cmds;			// list of commands [GString]

  KeyBinding(int codeA, int modsA, int contextA, char *cmd0);
  KeyBinding(int codeA, int modsA, int contextA, char *cmd0, char *cmd1);
  KeyBinding(int codeA, int modsA, int contextA, GList *cmdsA);
  ~KeyBinding();
};

#define xpdfKeyCodeTab            0x1000
#define xpdfKeyCodeReturn         0x1001
#define xpdfKeyCodeEnter          0x1002
#define xpdfKeyCodeBackspace      0x1003
#define xpdfKeyCodeInsert         0x1004
#define xpdfKeyCodeDelete         0x1005
#define xpdfKeyCodeHome           0x1006
#define xpdfKeyCodeEnd            0x1007
#define xpdfKeyCodePgUp           0x1008
#define xpdfKeyCodePgDn           0x1009
#define xpdfKeyCodeLeft           0x100a
#define xpdfKeyCodeRight          0x100b
#define xpdfKeyCodeUp             0x100c
#define xpdfKeyCodeDown           0x100d
#define xpdfKeyCodeF1             0x1100
#define xpdfKeyCodeF35            0x1122
#define xpdfKeyCodeMousePress1    0x2001
#define xpdfKeyCodeMousePress2    0x2002
#define xpdfKeyCodeMousePress3    0x2003
#define xpdfKeyCodeMousePress4    0x2004
#define xpdfKeyCodeMousePress5    0x2005
#define xpdfKeyCodeMousePress6    0x2006
#define xpdfKeyCodeMousePress7    0x2007
#define xpdfKeyCodeMouseRelease1  0x2101
#define xpdfKeyCodeMouseRelease2  0x2102
#define xpdfKeyCodeMouseRelease3  0x2103
#define xpdfKeyCodeMouseRelease4  0x2104
#define xpdfKeyCodeMouseRelease5  0x2105
#define xpdfKeyCodeMouseRelease6  0x2106
#define xpdfKeyCodeMouseRelease7  0x2107
#define xpdfKeyModNone            0
#define xpdfKeyModShift           (1 << 0)
#define xpdfKeyModCtrl            (1 << 1)
#define xpdfKeyModAlt             (1 << 2)
#define xpdfKeyContextAny         0
#define xpdfKeyContextFullScreen  (1 << 0)
#define xpdfKeyContextWindow      (2 << 0)
#define xpdfKeyContextContinuous  (1 << 2)
#define xpdfKeyContextSinglePage  (2 << 2)
#define xpdfKeyContextOverLink    (1 << 4)
#define xpdfKeyContextOffLink     (2 << 4)
#define xpdfKeyContextOutline     (1 << 6)
#define xpdfKeyContextMainWin     (2 << 6)
#define xpdfKeyContextScrLockOn   (1 << 8)
#define xpdfKeyContextScrLockOff  (2 << 8)

//------------------------------------------------------------------------

class GlobalParams {
public:

  // Initialize the global parameters by attempting to read a config
  // file.
  GlobalParams(const char *cfgFileName);

  // initializes globalParams global variable if it is NULL
  static GlobalParams *initGlobalParams(const char *cfgFileName);
  // destroys globalParams globalParams variable unless NULL
  static void destroyGlobalParams();

  ~GlobalParams();

  void setBaseDir(const char *dir);
  void setupBaseFonts(const char *dir);

  void parseLine(const char *buf, const GString *fileName, int line);

  //----- accessors

  CharCode getMacRomanCharCode(const char *charName)const;

  GString *getBaseDir()const;
  Unicode mapNameToUnicode(const char *charName)const;
  UnicodeMap *getResidentUnicodeMap(const GString *encodingName)const;
  FILE *getUnicodeMapFile(const GString *encodingName)const;
  FILE *findCMapFile(const GString *collection, const GString *cMapName)const;
  FILE *findToUnicodeFile(const GString *name)const;
  DisplayFontParam *getDisplayFont(const GString *fontName)const;
  DisplayFontParam *getDisplayCIDFont(const GString *fontName, const GString *collection)const;
  GString *getPSFile()const;
  int getPSPaperWidth()const;
  int getPSPaperHeight()const;
  void getPSImageableArea(int *llx, int *lly, int *urx, int *ury)const;
  GBool getPSDuplex()const;
  GBool getPSCrop()const;
  GBool getPSExpandSmaller()const;
  GBool getPSShrinkLarger()const;
  GBool getPSCenter()const;
  PSLevel getPSLevel()const;
  const PSFontParam *getPSFont(const GString *fontName)const;
  const PSFontParam *getPSFont16(const GString *fontName, const GString *collection, int wMode)const;
  GBool getPSEmbedType1()const;
  GBool getPSEmbedTrueType()const;
  GBool getPSEmbedCIDPostScript()const;
  GBool getPSEmbedCIDTrueType()const;
  GBool getPSPreload()const;
  GBool getPSOPI()const;
  GBool getPSASCIIHex()const;
  GString *getTextEncodingName()const;
  EndOfLineKind getTextEOL()const;
  GBool getTextPageBreaks()const;
  GBool getTextKeepTinyChars()const;
  GString *findFontFile(const GString *fontName, char **exts)const;
  GString *getInitialZoom()const;
  GBool getContinuousView()const;
  GBool getEnableT1lib()const;
  GBool getEnableFreeType()const;
  GBool getAntialias()const;
  GBool getVectorAntialias()const;
  GBool getStrokeAdjust()const;
  ScreenType getScreenType()const;
  int getScreenSize()const;
  int getScreenDotRadius()const;
  double getScreenGamma()const;
  double getScreenBlackThreshold()const;
  double getScreenWhiteThreshold()const;
  const GString *getURLCommand()const { return urlCommand; }
  const GString *getMovieCommand() { return movieCommand; }
  GBool getMapNumericCharNames()const;
  GBool getMapUnknownCharNames()const;
  GList *getKeyBinding(int code, int mods, int context)const;
  GBool getPrintCommands()const;
  GBool getErrQuiet()const;

  CharCodeToUnicode *getCIDToUnicode(const GString *collection)const;
  CharCodeToUnicode *getUnicodeToUnicode(const GString *fontName)const;
  UnicodeMap *getUnicodeMap(const GString *encodingName)const;
  CMap *getCMap(const GString *collection, const GString *cMapName)const;
  UnicodeMap *getTextEncoding()const;

  //----- functions to set parameters

  void addDisplayFont(DisplayFontParam *param);
  void setPSFile(const char *file);
  GBool setPSPaperSize(const char *size);
  void setPSPaperWidth(int width);
  void setPSPaperHeight(int height);
  void setPSImageableArea(int llx, int lly, int urx, int ury);
  void setPSDuplex(GBool duplex);
  void setPSCrop(GBool crop);
  void setPSExpandSmaller(GBool expand);
  void setPSShrinkLarger(GBool shrink);
  void setPSCenter(GBool center);
  void setPSLevel(PSLevel level);
  void setPSEmbedType1(GBool embed);
  void setPSEmbedTrueType(GBool embed);
  void setPSEmbedCIDPostScript(GBool embed);
  void setPSEmbedCIDTrueType(GBool embed);
  void setPSPreload(GBool preload);
  void setPSOPI(GBool opi);
  void setPSASCIIHex(GBool hex);
  void setTextEncoding(const char *encodingName);
  GBool setTextEOL(const char *s);
  void setTextPageBreaks(GBool pageBreaks);
  void setTextKeepTinyChars(GBool keep);
  void setInitialZoom(const char *s);
  void setContinuousView(GBool cont);
  GBool setEnableT1lib(const char *s);
  GBool setEnableFreeType(const char *s);
  GBool setAntialias(const char *s);
  GBool setVectorAntialias(const char *s);
  void setScreenType(ScreenType t);
  void setScreenSize(int size);
  void setScreenDotRadius(int r);
  void setScreenGamma(double gamma);
  void setScreenBlackThreshold(double thresh);
  void setScreenWhiteThreshold(double thresh);
  void setMapNumericCharNames(GBool map);
  void setMapUnknownCharNames(GBool map);
  void setPrintCommands(GBool printCommandsA);
  void setErrQuiet(GBool errQuietA);

  //----- security handlers

  void addSecurityHandler(XpdfSecurityHandler *handler);
  XpdfSecurityHandler *getSecurityHandler(const char *name);

private:

  void createDefaultKeyBindings();
  void parseFile(GString *fileName, FILE *f);
  void parseNameToUnicode(const GList *tokens, const GString *fileName, int line);
  void parseCIDToUnicode(const GList *tokens, const GString *fileName, int line);
  void parseUnicodeToUnicode(const GList *tokens, const GString *fileName, int line);
  void parseUnicodeMap(const GList *tokens, const GString *fileName, int line);
  void parseCMapDir(const GList *tokens, const GString *fileName, int line);
  void parseToUnicodeDir(const GList *tokens, const GString *fileName, int line);
  void parseDisplayFont(const GList *tokens, GHash *fontHash,
			DisplayFontParamKind kind,
			const GString *fileName, int line);
  void parsePSFile(const GList *tokens, const GString *fileName, int line);
  void parsePSPaperSize(const GList *tokens, const GString *fileName, int line);
  void parsePSImageableArea(const GList *tokens, const GString *fileName, int line);
  void parsePSLevel(const GList *tokens, const GString *fileName, int line);
  void parsePSFont(const GList *tokens, const GString *fileName, int line);
  void parsePSFont16(const char *cmdName, GList *fontList,
		     const GList *tokens, const GString *fileName, int line);
  void parseTextEncoding(const GList *tokens, const GString *fileName, int line);
  void parseTextEOL(const GList *tokens, const GString *fileName, int line);
  void parseFontDir(const GList *tokens, const GString *fileName, int line);
  void parseInitialZoom(const GList *tokens, const GString *fileName, int line);
  void parseScreenType(const GList *tokens, const GString *fileName, int line);
  void parseBind(const GList *tokens, const GString *fileName, int line);
  void parseUnbind(const GList *tokens, const GString *fileName, int line);
  GBool parseKey(const GString *modKeyStr, const GString *contextStr,
		 int *code, int *mods, int *context,
		 const char *cmdName,
		 const GList *tokens, const GString *fileName, int line);
  void parseCommand(const char *cmdName, GString **val,
		    const GList *tokens, const GString *fileName, int line);
  void parseYesNo(const char *cmdName, GBool *flag,
		  const GList *tokens, const GString *fileName, int line);
  GBool parseYesNo2(const char *token, GBool *flag);
  void parseInteger(const char *cmdName, int *val,
		    const GList *tokens, const GString *fileName, int line);
  void parseFloat(const char *cmdName, double *val,
		  const GList *tokens, const GString *fileName, int line);
  UnicodeMap *getUnicodeMap2(const GString *encodingName)const;
#ifdef ENABLE_PLUGINS
  GBool loadPlugin(char *type, char *name);
#endif

  //----- static tables

  NameToCharCode *		// mapping from char name to
    macRomanReverseMap;		//   MacRomanEncoding index

  //----- user-modifiable settings

  GString *baseDir;		// base directory - for plugins, etc.
  NameToCharCode *		// mapping from char name to Unicode
    nameToUnicode;
  GHash *cidToUnicodes;		// files for mappings from char collections
				//   to Unicode, indexed by collection name
				//   [GString]
  GHash *unicodeToUnicodes;	// files for Unicode-to-Unicode mappings,
				//   indexed by font name pattern [GString]
  GHash *residentUnicodeMaps;	// mappings from Unicode to char codes,
				//   indexed by encoding name [UnicodeMap]
  GHash *unicodeMaps;		// files for mappings from Unicode to char
				//   codes, indexed by encoding name [GString]
  GHash *cMapDirs;		// list of CMap dirs, indexed by collection
				//   name [GList[GString]]
  GList *toUnicodeDirs;		// list of ToUnicode CMap dirs [GString]
  GHash *displayFonts;		// display font info, indexed by font name
				//   [DisplayFontParam]
#ifdef WIN32
  WinFontList *winFontList;	// system TrueType fonts
#endif
  GHash *displayCIDFonts;	// display CID font info, indexed by
				//   collection [DisplayFontParam]
  GHash *displayNamedCIDFonts;	// display CID font info, indexed by
				//   font name [DisplayFontParam]
  GString *psFile;		// PostScript file or command (for xpdf)
  int psPaperWidth;		// paper size, in PostScript points, for
  int psPaperHeight;		//   PostScript output
  int psImageableLLX,		// imageable area, in PostScript points,
      psImageableLLY,		//   for PostScript output
      psImageableURX,
      psImageableURY;
  GBool psCrop;			// crop PS output to CropBox
  GBool psExpandSmaller;	// expand smaller pages to fill paper
  GBool psShrinkLarger;		// shrink larger pages to fit paper
  GBool psCenter;		// center pages on the paper
  GBool psDuplex;		// enable duplexing in PostScript?
  PSLevel psLevel;		// PostScript level to generate
  GHash *psFonts;		// PostScript font info, indexed by PDF
				//   font name [PSFontParam]
  GList *psNamedFonts16;	// named 16-bit fonts [PSFontParam]
  GList *psFonts16;		// generic 16-bit fonts [PSFontParam]
  GBool psEmbedType1;		// embed Type 1 fonts?
  GBool psEmbedTrueType;	// embed TrueType fonts?
  GBool psEmbedCIDPostScript;	// embed CID PostScript fonts?
  GBool psEmbedCIDTrueType;	// embed CID TrueType fonts?
  GBool psPreload;		// preload PostScript images and forms into
				//   memory
  GBool psOPI;			// generate PostScript OPI comments?
  GBool psASCIIHex;		// use ASCIIHex instead of ASCII85?
  GString *textEncoding;	// encoding (unicodeMap) to use for text
				//   output
  EndOfLineKind textEOL;	// type of EOL marker to use for text
				//   output
  GBool textPageBreaks;		// insert end-of-page markers?
  GBool textKeepTinyChars;	// keep all characters in text output
  GList *fontDirs;		// list of font dirs [GString]
  GString *initialZoom;		// initial zoom level
  GBool continuousView;		// continuous view mode
  GBool enableT1lib;		// t1lib enable flag
  GBool enableFreeType;		// FreeType enable flag
  GBool antialias;		// font anti-aliasing enable flag
  GBool vectorAntialias;	// vector anti-aliasing enable flag
  GBool strokeAdjust;		// stroke adjustment enable flag
  ScreenType screenType;	// halftone screen type
  int screenSize;		// screen matrix size
  int screenDotRadius;		// screen dot radius
  double screenGamma;		// screen gamma correction
  double screenBlackThreshold;	// screen black clamping threshold
  double screenWhiteThreshold;	// screen white clamping threshold
  GString *urlCommand;		// command executed for URL links
  GString *movieCommand;	// command executed for movie annotations
  GBool mapNumericCharNames;	// map numeric char names (from font subsets)?
  GBool mapUnknownCharNames;	// map unknown char names?
  GList *keyBindings;		// key & mouse button bindings [KeyBinding]
  GBool printCommands;		// print the drawing commands
  GBool errQuiet;		// suppress error messages?

  CharCodeToUnicodeCache *cidToUnicodeCache;
  CharCodeToUnicodeCache *unicodeToUnicodeCache;
  UnicodeMapCache *unicodeMapCache;
  CMapCache *cMapCache;

#ifdef ENABLE_PLUGINS
  GList *plugins;		// list of plugins [Plugin]
  GList *securityHandlers;	// list of loaded security handlers
				//   [XpdfSecurityHandler]
#endif

#if MULTITHREADED
  GMutex mutex;
  GMutex unicodeMapCacheMutex;
  GMutex cMapCacheMutex;
#endif
};

#endif
