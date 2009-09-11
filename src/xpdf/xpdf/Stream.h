//========================================================================
//
// Stream.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
// Changes:
// Michal Hocko - FileStream, MemStream and EmbedStream private members are 
//                protected now to enable StreamWriter descendants to access 
//                them.
//              - All direct Stream and BaseStream subclasses have virtual 
//                inheritance
//              - Stream, BaseStream and FilterStream define abstract clone
//                All descendants implement this method
//              - All filter stream using StremPredictor stores PredictorContext
//                to enable cloning
//              - dictionary modificator access methods
//
//========================================================================

#ifndef STREAM_H
#define STREAM_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include "goo/gtypes.h"
#include "xpdf/Object.h"

class BaseStream;

//------------------------------------------------------------------------

enum StreamKind {
  strFile,
  strASCIIHex,
  strASCII85,
  strLZW,
  strRunLength,
  strCCITTFax,
  strDCT,
  strFlate,
  strJBIG2,
  strJPX,
  strWeird			// internal-use stream types
};

enum StreamColorSpaceMode {
  streamCSNone,
  streamCSDeviceGray,
  streamCSDeviceRGB,
  streamCSDeviceCMYK
};

//------------------------------------------------------------------------

// This is in Stream.h instead of Decrypt.h to avoid really annoying
// include file dependency loops.
enum CryptAlgorithm {
  cryptRC4,
  cryptAES
};

//------------------------------------------------------------------------
// Stream (base class)
//------------------------------------------------------------------------

class Stream {
public:

  // Constructor.
  Stream();

  // Destructor.
  virtual ~Stream();

  // Reference counting.
  int incRef() { return ++ref; }
  int decRef() { return --ref; }

  // Get kind of stream.
  virtual StreamKind getKind()const = 0;

  // support for cloning
  virtual Stream * clone()=0;
  
  // Reset stream to beginning.
  virtual void reset() = 0;

  // Close down the stream.
  virtual void close();

  // Get next char from stream.
  virtual int getChar() = 0;

  // Peek at next char in stream.
  virtual int lookChar() = 0;

  // Get next char from stream without using the predictor.
  // This is only used by StreamPredictor.
  virtual int getRawChar();

  // Get next line from stream.
  virtual char *getLine(char *buf, int size);

  // Get current position in file.
  virtual int getPos()const = 0;

  // Go to a position in the stream.  If <dir> is negative, the
  // position is from the end of the file; otherwise the position is
  // from the start of the file.
  virtual void setPos(Guint pos, int dir = 0) = 0;

  // Get PostScript command for the filter(s).
  virtual GString *getPSFilter(int psLevel, const char *indent)const;

  // Does this stream type potentially contain non-printable chars?
  virtual GBool isBinary(GBool last = gTrue)const = 0;

  // Get the BaseStream of this stream.
  virtual BaseStream *getBaseStream() = 0;

  // Get the stream after the last decoder (this may be a BaseStream
  // or a DecryptStream).
  virtual Stream *getUndecodedStream() = 0;

  // Get the dictionary associated with this stream.
  virtual const Dict *getDict()const = 0;

  // Is this an encoding filter?
  virtual GBool isEncoder()const { return gFalse; }

  // Get image parameters which are defined by the stream contents.
  virtual void getImageParams(UNUSED_PARAM int *bitsPerComponent,
			      UNUSED_PARAM StreamColorSpaceMode *csMode) {}

  // Return the next stream in the "stack".
  virtual Stream *getNextStream() { return NULL; }

  // Add filters to this stream according to the parameters in <dict>.
  // Returns the new stream.
  Stream *addFilters(const Object *dict);

private:

  Stream *makeFilter(const char *name, Stream *str, const Object *params);

  int ref;			// reference count
};

//------------------------------------------------------------------------
// BaseStream
//
// This is the base class for all streams that read directly from a file.
//------------------------------------------------------------------------

class BaseStream: virtual public Stream {
public:

  BaseStream(const Object *dictA);
  virtual ~BaseStream();
  virtual Stream *makeSubStream(Guint start, GBool limited,
				Guint length, const Object *dict) = 0;
  virtual void setPos(Guint pos, int dir = 0) = 0;
  virtual GBool isBinary(GBool last = gTrue)const { return last; }
  virtual BaseStream *getBaseStream() { return this; }
  virtual Stream *getUndecodedStream() { return this; }
  virtual const Dict *getDict()const { return dict.getDict(); }
  virtual GString *getFileName() { return NULL; }

  virtual Stream * clone()=0;

  // Get/set position of first byte of stream within the file.
  virtual Guint getStart()const = 0;
  virtual void moveStart(int delta) = 0;

  // Dict accessors.
  void dictAdd(char *key, Object *val);
  Object *dictUpdate(char *key, Object *val);
  Object *dictDel(const char *key);

protected:
  Object dict;
};

inline void BaseStream::dictAdd(char *key, Object *val)
  { dict.dictAdd(key, val); }

inline Object *BaseStream::dictUpdate(char *key, Object *val)
  { return dict.dictUpdate(key, val); }

inline Object *BaseStream::dictDel(const char *key)
  { return dict.dictDel(key); }

//------------------------------------------------------------------------
// FilterStream
//
// This is the base class for all streams that filter another stream.
//------------------------------------------------------------------------

class FilterStream: public Stream {
public:

  FilterStream(Stream *strA);
  virtual ~FilterStream();
  virtual void close();
  virtual Stream * clone()=0;
  virtual int getPos()const { return str->getPos(); }
  virtual void setPos(Guint pos, int dir = 0);
  virtual BaseStream *getBaseStream() { return str->getBaseStream(); }
  virtual Stream *getUndecodedStream() { return str->getUndecodedStream(); }
  virtual const Dict *getDict()const { return str->getDict(); }
  virtual Stream *getNextStream() { return str; }

protected:

  Stream *str;
};

//------------------------------------------------------------------------
// ImageStream
//------------------------------------------------------------------------

class ImageStream {
public:

  // Create an image stream object for an image with the specified
  // parameters.  Note that these are the actual image parameters,
  // which may be different from the predictor parameters.
  ImageStream(Stream *strA, int widthA, int nCompsA, int nBitsA);

  ~ImageStream();

  // Reset the stream.
  void reset();

  // Gets the next pixel from the stream.  <pix> should be able to hold
  // at least nComps elements.  Returns false at end of file.
  GBool getPixel(Guchar *pix);

  // Returns a pointer to the next line of pixels.  Returns NULL at
  // end of file.
  Guchar *getLine();

  // Skip an entire line from the image.
  void skipLine();

private:

  Stream *str;			// base stream
  int width;			// pixels per line
  int nComps;			// components per pixel
  int nBits;			// bits per component
  int nVals;			// components per line
  Guchar *imgLine;		// line buffer
  int imgIdx;			// current index in imgLine
};

//------------------------------------------------------------------------
// StreamPredictor
//------------------------------------------------------------------------

// initial context for StreamPredictor creation
struct PredictorContext
{
  int predictor;
  int width;
  int comps;
  int bits;
};

class StreamPredictor {
public:

  // Create a predictor object.  Note that the parameters are for the
  // predictor, and may not match the actual image parameters.
  StreamPredictor(Stream *strA, int predictorA,
		  int widthA, int nCompsA, int nBitsA);

  // same as pervious except uses structure for initialization
  StreamPredictor(Stream * str, PredictorContext & context);

  ~StreamPredictor();

  GBool isOk()const { return ok; }

  int lookChar();
  int getChar();

private:

  GBool getNextLine();

  Stream *str;			// base stream
  int predictor;		// predictor
  int width;			// pixels per line
  int nComps;			// components per pixel
  int nBits;			// bits per component
  int nVals;			// components per line
  int pixBytes;			// bytes per pixel
  int rowBytes;			// bytes per line
  Guchar *predLine;		// line buffer
  int predIdx;			// current index in predLine
  GBool ok;

  // initialize internal structures from context variables
  void initContext(int predictorA, int widthA, int nCompsA, int nBitsA);             
};

//------------------------------------------------------------------------
// FileStream
//------------------------------------------------------------------------

#define fileStreamBufSize 256

class FileStream: virtual public BaseStream {
public:

  FileStream(FILE *fA, Guint startA, GBool limitedA,
	     Guint lengthA, const Object *dictA);
  // Note that file handle is not closed in constructor!!!
  // This is because of makeSubStream which creates a new
  // file stream with the shared file handled. 
  virtual ~FileStream();
  virtual Stream *makeSubStream(Guint startA, GBool limitedA,
				Guint lengthA, const Object *dictA);
  virtual StreamKind getKind()const { return strFile; }
  virtual void reset();
  virtual void close();
  virtual Stream * clone();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual int getPos()const { return bufPos + (bufPtr - buf); }
  virtual void setPos(Guint pos, int dir = 0);
  virtual Guint getStart()const { return start; }
  virtual void moveStart(int delta);

protected:

  GBool fillBuf();

  FILE *f;
  Guint start;
  GBool limited;
  Guint length;
  char buf[fileStreamBufSize];
  char *bufPtr;
  char *bufEnd;
  Guint bufPos;
  int savePos;
  GBool saved;
};

//------------------------------------------------------------------------
// MemStream
//------------------------------------------------------------------------

class MemStream: virtual public BaseStream {
public:

  MemStream(char *bufA, Guint startA, Guint lengthA, const Object *dictA, GBool needFreeA=false);
  virtual ~MemStream();
  virtual Stream *makeSubStream(Guint start, GBool limited,
				Guint lengthA, const Object *dictA);
  virtual StreamKind getKind()const { return strWeird; }
  virtual void reset();
  virtual void close();
  virtual Stream * clone();
  virtual int getChar()
    { return (bufPtr < bufEnd) ? (*bufPtr++ & 0xff) : EOF; }
  virtual int lookChar()
    { return (bufPtr < bufEnd) ? (*bufPtr & 0xff) : EOF; }
  virtual int getPos()const { return (int)(bufPtr - buf); }
  virtual void setPos(Guint pos, int dir = 0);
  virtual Guint getStart()const { return start; }
  virtual void moveStart(int delta);

protected:
  char *buf;
  Guint start;
  Guint length;
  char *bufEnd;
  char *bufPtr;
  GBool needFree;
};

//------------------------------------------------------------------------
// EmbedStream
//
// This is a special stream type used for embedded streams (inline
// images).  It reads directly from the base stream -- after the
// EmbedStream is deleted, reads from the base stream will proceed where
// the BaseStream left off.  Note that this is very different behavior
// that creating a new FileStream (using makeSubStream).
//------------------------------------------------------------------------

class EmbedStream: virtual public BaseStream {
public:

  EmbedStream(Stream *strA, const Object *dictA, GBool limitedA, Guint lengthA);
  virtual ~EmbedStream();
  virtual Stream *makeSubStream(Guint start, GBool limitedA,
				Guint lengthA, const Object *dictA);
  virtual StreamKind getKind()const { return str->getKind(); }
  virtual void reset() {}
  virtual int getChar();
  virtual Stream * clone();
  virtual int lookChar();
  virtual int getPos()const { return str->getPos(); }
  virtual void setPos(Guint pos, int dir = 0);
  virtual Guint getStart()const;
  virtual void moveStart(int delta);


protected:

  Stream *str;
  GBool limited;
  Guint length;
};

//------------------------------------------------------------------------
// ASCIIHexStream
//------------------------------------------------------------------------

class ASCIIHexStream: public FilterStream {
public:

  ASCIIHexStream(Stream *strA);
  virtual ~ASCIIHexStream();
  virtual StreamKind getKind()const { return strASCIIHex; }
  virtual Stream * clone();
  virtual void reset();
  virtual int getChar()
    { int c = lookChar(); buf = EOF; return c; }
  virtual int lookChar();
  virtual GString *getPSFilter(int psLevel, const char *indent)const;
  virtual GBool isBinary(GBool last = gTrue)const;

private:

  int buf;
  GBool eof;
};

//------------------------------------------------------------------------
// ASCII85Stream
//------------------------------------------------------------------------

class ASCII85Stream: public FilterStream {
public:

  ASCII85Stream(Stream *strA);
  virtual ~ASCII85Stream();
  virtual StreamKind getKind()const { return strASCII85; }
  virtual void reset();
  virtual Stream * clone();
  virtual int getChar()
    { int ch = lookChar(); ++index; return ch; }
  virtual int lookChar();
  virtual GString *getPSFilter(int psLevel, const char *indent)const;
  virtual GBool isBinary(GBool last = gTrue)const;

private:

  int c[5];
  int b[4];
  int index, n;
  GBool eof;
};

//------------------------------------------------------------------------
// LZWStream
//------------------------------------------------------------------------

class LZWStream: public FilterStream {
public:

  LZWStream(Stream *strA, int predictor, int columns, int colors,
	    int bits, int earlyA);
  virtual ~LZWStream();
  virtual StreamKind getKind()const { return strLZW; }
  virtual void reset();
  virtual int getChar();
  virtual Stream * clone();
  virtual int lookChar();
  virtual int getRawChar();
  virtual GString *getPSFilter(int psLevel, const char *indent)const;
  virtual GBool isBinary(GBool last = gTrue)const;

private:
  PredictorContext predContext; // context for predictor creation

  StreamPredictor *pred;	// predictor
  int early;			// early parameter
  GBool eof;			// true if at eof
  int inputBuf;			// input buffer
  int inputBits;		// number of bits in input buffer
  struct {			// decoding table
    int length;
    int head;
    Guchar tail;
  } table[4097];
  int nextCode;			// next code to be used
  int nextBits;			// number of bits in next code word
  int prevCode;			// previous code used in stream
  int newChar;			// next char to be added to table
  Guchar seqBuf[4097];		// buffer for current sequence
  int seqLength;		// length of current sequence
  int seqIndex;			// index into current sequence
  GBool first;			// first code after a table clear

  GBool processNextCode();
  void clearTable();
  int getCode();
};

//------------------------------------------------------------------------
// RunLengthStream
//------------------------------------------------------------------------

class RunLengthStream: public FilterStream {
public:

  RunLengthStream(Stream *strA);
  virtual ~RunLengthStream();
  virtual Stream * clone();
  virtual StreamKind getKind()const { return strRunLength; }
  virtual void reset();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual GString *getPSFilter(int psLevel, const char *indent)const;
  virtual GBool isBinary(GBool last = gTrue)const;

private:

  char buf[128];		// buffer
  char *bufPtr;			// next char to read
  char *bufEnd;			// end of buffer
  GBool eof;

  GBool fillBuf();
};

//------------------------------------------------------------------------
// CCITTFaxStream
//------------------------------------------------------------------------

struct CCITTCodeTable;

class CCITTFaxStream: public FilterStream {
public:

  CCITTFaxStream(Stream *strA, int encodingA, GBool endOfLineA,
		 GBool byteAlignA, int columnsA, int rowsA,
		 GBool endOfBlockA, GBool blackA);
  virtual ~CCITTFaxStream();
  virtual StreamKind getKind()const { return strCCITTFax; }
  virtual void reset();
  virtual Stream * clone();
  virtual int getChar()
    { int c = lookChar(); buf = EOF; return c; }
  virtual int lookChar();
  virtual GString *getPSFilter(int psLevel, const char *indent)const;
  virtual GBool isBinary(GBool last = gTrue)const;

private:

  int encoding;			// 'K' parameter
  GBool endOfLine;		// 'EndOfLine' parameter
  GBool byteAlign;		// 'EncodedByteAlign' parameter
  int columns;			// 'Columns' parameter
  int rows;			// 'Rows' parameter
  GBool endOfBlock;		// 'EndOfBlock' parameter
  GBool black;			// 'BlackIs1' parameter
  GBool eof;			// true if at eof
  GBool nextLine2D;		// true if next line uses 2D encoding
  int row;			// current row
  int inputBuf;			// input buffer
  int inputBits;		// number of bits in input buffer
  int *codingLine;		// coding line changing elements
  int *refLine;			// reference line changing elements
  int a0i;			// index into codingLine
  GBool err;			// error on current line
  int outputBits;		// remaining ouput bits
  int buf;			// character buffer

  void addPixels(int a1, int black);
  void addPixelsNeg(int a1, int black);
  short getTwoDimCode();
  short getWhiteCode();
  short getBlackCode();
  short lookBits(int n);
  void eatBits(int n) { if ((inputBits -= n) < 0) inputBits = 0; }
};

//------------------------------------------------------------------------
// DCTStream
//------------------------------------------------------------------------

// DCT component info
struct DCTCompInfo {
  int id;			// component ID
  int hSample, vSample;		// horiz/vert sampling resolutions
  int quantTable;		// quantization table number
  int prevDC;			// DC coefficient accumulator
};

struct DCTScanInfo {
  GBool comp[4];		// comp[i] is set if component i is
				//   included in this scan
  int numComps;			// number of components in the scan
  int dcHuffTable[4];		// DC Huffman table numbers
  int acHuffTable[4];		// AC Huffman table numbers
  int firstCoeff, lastCoeff;	// first and last DCT coefficient
  int ah, al;			// successive approximation parameters
};

// DCT Huffman decoding table
struct DCTHuffTable {
  Guchar firstSym[17];		// first symbol for this bit length
  Gushort firstCode[17];	// first code for this bit length
  Gushort numCodes[17];		// number of codes of this bit length
  Guchar sym[256];		// symbols
};

class DCTStream: public FilterStream {
public:

  DCTStream(Stream *strA, int colorXformA);
  virtual ~DCTStream();
  virtual StreamKind getKind()const { return strDCT; }
  virtual Stream * clone();
  virtual void reset();
  virtual void close();
  virtual int getChar();
  virtual int lookChar();
  virtual GString *getPSFilter(int psLevel, const char *indent)const;
  virtual GBool isBinary(GBool last = gTrue)const;
  Stream *getRawStream() { return str; }

private:

  GBool progressive;		// set if in progressive mode
  GBool interleaved;		// set if in interleaved mode
  int width, height;		// image size
  int mcuWidth, mcuHeight;	// size of min coding unit, in data units
  int bufWidth, bufHeight;	// frameBuf size
  DCTCompInfo compInfo[4];	// info for each component
  DCTScanInfo scanInfo;		// info for the current scan
  int numComps;			// number of components in image
  int colorXform;		// color transform: -1 = unspecified
				//                   0 = none
				//                   1 = YUV/YUVK -> RGB/CMYK
  GBool gotJFIFMarker;		// set if APP0 JFIF marker was present
  GBool gotAdobeMarker;		// set if APP14 Adobe marker was present
  int restartInterval;		// restart interval, in MCUs
  Gushort quantTables[4][64];	// quantization tables
  int numQuantTables;		// number of quantization tables
  DCTHuffTable dcHuffTables[4];	// DC Huffman tables
  DCTHuffTable acHuffTables[4];	// AC Huffman tables
  int numDCHuffTables;		// number of DC Huffman tables
  int numACHuffTables;		// number of AC Huffman tables
  Guchar *rowBuf[4][32];	// buffer for one MCU (non-progressive mode)
  int *frameBuf[4];		// buffer for frame (progressive mode)
  int comp, x, y, dy;		// current position within image/MCU
  int restartCtr;		// MCUs left until restart
  int restartMarker;		// next restart marker
  int eobRun;			// number of EOBs left in the current run
  int inputBuf;			// input buffer for variable length codes
  int inputBits;		// number of valid bits in input buffer

  void restart();
  GBool readMCURow();
  void readScan();
  GBool readDataUnit(DCTHuffTable *dcHuffTable,
		     DCTHuffTable *acHuffTable,
		     int *prevDC, int data[64]);
  GBool readProgressiveDataUnit(DCTHuffTable *dcHuffTable,
				DCTHuffTable *acHuffTable,
				int *prevDC, int data[64]);
  void decodeImage();
  void transformDataUnit(Gushort *quantTable,
			 int dataIn[64], Guchar dataOut[64]);
  int readHuffSym(DCTHuffTable *table);
  int readAmp(int size);
  int readBit();
  GBool readHeader();
  GBool readBaselineSOF();
  GBool readProgressiveSOF();
  GBool readScanInfo();
  GBool readQuantTables();
  GBool readHuffmanTables();
  GBool readRestartInterval();
  GBool readJFIFMarker();
  GBool readAdobeMarker();
  GBool readTrailer();
  int readMarker();
  int read16();
};

//------------------------------------------------------------------------
// FlateStream
//------------------------------------------------------------------------

#define flateWindow          32768    // buffer size
#define flateMask            (flateWindow-1)
#define flateMaxHuffman         15    // max Huffman code length
#define flateMaxCodeLenCodes    19    // max # code length codes
#define flateMaxLitCodes       288    // max # literal codes
#define flateMaxDistCodes       30    // max # distance codes

// Huffman code table entry
struct FlateCode {
  Gushort len;			// code length, in bits
  Gushort val;			// value represented by this code
};

struct FlateHuffmanTab {
  FlateCode *codes;
  int maxLen;
};

// Decoding info for length and distance code words
struct FlateDecode {
  int bits;			// # extra bits
  int first;			// first length/distance
};

class FlateStream: public FilterStream {
public:

  FlateStream(Stream *strA, int predictor, int columns,
	      int colors, int bits);
  virtual ~FlateStream();
  virtual StreamKind getKind()const { return strFlate; }
  virtual void reset();
  virtual Stream * clone();
  virtual int getChar();
  virtual int lookChar();
  virtual int getRawChar();
  virtual GString *getPSFilter(int psLevel, const char *indent)const;
  virtual GBool isBinary(GBool last = gTrue)const;

private:
  PredictorContext predContext; // creation context for predictor

  StreamPredictor *pred;	// predictor
  Guchar buf[flateWindow];	// output data buffer
  int index;			// current index into output buffer
  int remain;			// number valid bytes in output buffer
  int codeBuf;			// input buffer
  int codeSize;			// number of bits in input buffer
  int				// literal and distance code lengths
    codeLengths[flateMaxLitCodes + flateMaxDistCodes];
  FlateHuffmanTab litCodeTab;	// literal code table
  FlateHuffmanTab distCodeTab;	// distance code table
  GBool compressedBlock;	// set if reading a compressed block
  int blockLen;			// remaining length of uncompressed block
  GBool endOfBlock;		// set when end of block is reached
  GBool eof;			// set when end of stream is reached

  static int			// code length code reordering
    codeLenCodeMap[flateMaxCodeLenCodes];
  static FlateDecode		// length decoding info
    lengthDecode[flateMaxLitCodes-257];
  static FlateDecode		// distance decoding info
    distDecode[flateMaxDistCodes];
  static FlateHuffmanTab	// fixed literal code table
    fixedLitCodeTab;
  static FlateHuffmanTab	// fixed distance code table
    fixedDistCodeTab;

  void readSome();
  GBool startBlock();
  void loadFixedCodes();
  GBool readDynamicCodes();
  void compHuffmanCodes(int *lengths, int n, FlateHuffmanTab *tab);
  int getHuffmanCodeWord(FlateHuffmanTab *tab);
  int getCodeWord(int bits);
};

//------------------------------------------------------------------------
// EOFStream
//------------------------------------------------------------------------

class EOFStream: public FilterStream {
public:

  EOFStream(Stream *strA);
  virtual ~EOFStream();
  virtual StreamKind getKind()const { return strWeird; }
  virtual void reset() {}
  virtual Stream * clone();
  virtual int getChar() { return EOF; }
  virtual int lookChar() { return EOF; }
  virtual GString *getPSFilter(UNUSED_PARAM	int psLevel, UNUSED_PARAM	const char *indent)const  { return NULL; }
  virtual GBool isBinary(UNUSED_PARAM	GBool last = gTrue)const { return gFalse; }
};

//------------------------------------------------------------------------
// FixedLengthEncoder
//------------------------------------------------------------------------

class FixedLengthEncoder: public FilterStream {
public:

  FixedLengthEncoder(Stream *strA, int lengthA);
  ~FixedLengthEncoder();
  virtual StreamKind getKind()const { return strWeird; }
  virtual void reset();
  virtual int getChar();
  virtual Stream * clone();
  virtual int lookChar();
  virtual GString *getPSFilter(UNUSED_PARAM	int psLevel, UNUSED_PARAM	const char *indent)const { return NULL; }
  virtual GBool isBinary(GBool last = gTrue)const;
  virtual GBool isEncoder()const { return gTrue; }

private:

  int length;
  int count;
};

//------------------------------------------------------------------------
// ASCIIHexEncoder
//------------------------------------------------------------------------

class ASCIIHexEncoder: public FilterStream {
public:

  ASCIIHexEncoder(Stream *strA);
  virtual ~ASCIIHexEncoder();
  virtual StreamKind getKind()const { return strWeird; }
  virtual void reset();
  virtual Stream * clone();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual GString *getPSFilter(UNUSED_PARAM	int psLevel, UNUSED_PARAM	const char *indent)const { return NULL; }
  virtual GBool isBinary(UNUSED_PARAM	GBool last = gTrue)const { return gFalse; }
  virtual GBool isEncoder()const { return gTrue; }

private:

  char buf[4];
  char *bufPtr;
  char *bufEnd;
  int lineLen;
  GBool eof;

  GBool fillBuf();
};

//------------------------------------------------------------------------
// ASCII85Encoder
//------------------------------------------------------------------------

class ASCII85Encoder: public FilterStream {
public:

  ASCII85Encoder(Stream *strA);
  virtual ~ASCII85Encoder();
  virtual StreamKind getKind()const { return strWeird; }
  virtual void reset();
  virtual Stream * clone();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual GString *getPSFilter(UNUSED_PARAM	int psLevel, UNUSED_PARAM	const char *indent)const { return NULL; }
  virtual GBool isBinary(UNUSED_PARAM	GBool last = gTrue)const { return gFalse; }
  virtual GBool isEncoder()const { return gTrue; }

private:

  char buf[8];
  char *bufPtr;
  char *bufEnd;
  int lineLen;
  GBool eof;

  GBool fillBuf();
};

//------------------------------------------------------------------------
// RunLengthEncoder
//------------------------------------------------------------------------

class RunLengthEncoder: public FilterStream {
public:

  RunLengthEncoder(Stream *strA);
  virtual ~RunLengthEncoder();
  virtual StreamKind getKind()const { return strWeird; }
  virtual void reset();
  virtual Stream * clone();
  virtual int getChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr++ & 0xff); }
  virtual int lookChar()
    { return (bufPtr >= bufEnd && !fillBuf()) ? EOF : (*bufPtr & 0xff); }
  virtual GString *getPSFilter(UNUSED_PARAM	int psLevel, UNUSED_PARAM	const char *indent)const { return NULL; }
  virtual GBool isBinary(UNUSED_PARAM	GBool last = gTrue)const { return gTrue; }
  virtual GBool isEncoder()const { return gTrue; }

private:

  char buf[131];
  char *bufPtr;
  char *bufEnd;
  char *nextEnd;
  GBool eof;

  GBool fillBuf();
};

#endif
