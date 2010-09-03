//========================================================================
//
// XRef.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include "goo/gmem.h"
#include "xpdf/Object.h"
#include "xpdf/Stream.h"
#include "xpdf/Lexer.h"
#include "xpdf/Parser.h"
#include "xpdf/Dict.h"
#include "xpdf/Error.h"
#include "xpdf/ErrorCodes.h"
#include "xpdf/XRef.h"

//------------------------------------------------------------------------

#define xrefSearchSize 1024	// read this many bytes at end of file
				//   to look for 'startxref'

//------------------------------------------------------------------------
// Permission bits
//------------------------------------------------------------------------

#define permPrint    (1<<2)
#define permChange   (1<<3)
#define permCopy     (1<<4)
#define permNotes    (1<<5)
#define defPermFlags 0xfffc

//------------------------------------------------------------------------
// ObjectStream
//------------------------------------------------------------------------

class ObjectStream {
public:

  // Create an object stream, using object number <objStrNum>,
  // generation 0.
  ObjectStream(const XRef *xref, int objStrNumA);

  GBool isOk()const { return ok; }

  ~ObjectStream();

  // Return the object number of this object stream.
  int getObjStrNum() { return objStrNum; }

  // Get the <objIdx>th object from this stream, which should be
  // object number <objNum>, generation 0.
  Object *getObject(int objIdx, int objNum, Object *obj);

private:

  int objStrNum;		// object number of the object stream
  int nObjects;			// number of objects in the stream
  Object *objs;			// the objects (length = nObjects)
  int *objNums;			// the object numbers (length = nObjects)
  GBool ok;
};

ObjectStream::ObjectStream(const XRef *xref, int objStrNumA) {
  Stream *str;
  Parser *parser;
  int *offsets;
  Object objStr, obj1, obj2;
  int first, i;

  objStrNum = objStrNumA;
  nObjects = 0;
  objs = NULL;
  objNums = NULL;
  ok = gFalse;

  // we don't have to check for isOk here because fetch failure
  // is reported via returned objNull
  if (!xref->fetch(objStrNum, 0, &objStr)->isStream()) {
    goto err1;
  }

  if (!objStr.streamGetDict()->lookup("N", &obj1)->isInt()) {
    obj1.free();
    goto err1;
  }
  nObjects = obj1.getInt();
  obj1.free();
  if (nObjects <= 0) {
    goto err1;
  }

  if (!objStr.streamGetDict()->lookup("First", &obj1)->isInt()) {
    obj1.free();
    goto err1;
  }
  first = obj1.getInt();
  obj1.free();
  if (first < 0) {
    goto err1;
  }

  // this is an arbitrary limit to avoid integer overflow problems
  // in the 'new Object[nObjects]' call (Acrobat apparently limits
  // object streams to 100-200 objects)
  if (nObjects > 1000000) {
    error(-1, "Too many objects in an object stream");
    goto err1;
  }
  objs = new Object[nObjects];
  objNums = (int *)gmallocn(nObjects, sizeof(int));
  offsets = (int *)gmallocn(nObjects, sizeof(int));

  // parse the header: object numbers and offsets
  objStr.streamReset();
  obj1.initNull();
  str = new EmbedStream(objStr.getStream(), &obj1, gTrue, first);
  parser = new Parser(xref, new Lexer(xref, str), gFalse);
  for (i = 0; i < nObjects; ++i) {
    if(!parser->getObj(&obj1))
      goto malformedErr;
    if(!parser->getObj(&obj2)) {
      obj1.free();
      goto malformedErr;
    }
    if (!obj1.isInt() || !obj2.isInt()) {
      obj1.free();
      obj2.free();
      delete parser;
      gfree(offsets);
      goto err1;
    }
    objNums[i] = obj1.getInt();
    offsets[i] = obj2.getInt();
    obj1.free();
    obj2.free();
    if (objNums[i] < 0 || offsets[i] < 0 ||
	(i > 0 && offsets[i] < offsets[i-1])) {
      delete parser;
      gfree(offsets);
      goto err1;
    }
  }
  while (str->getChar() != EOF) ;
  delete parser;

  // skip to the first object - this shouldn't be necessary because
  // the First key is supposed to be equal to offsets[0], but just in
  // case...
  for (i = first; i < offsets[0]; ++i) {
    objStr.getStream()->getChar();
  }

  // parse the objects
  for (i = 0; i < nObjects; ++i) {
    obj1.initNull();
    if (i == nObjects - 1) {
      str = new EmbedStream(objStr.getStream(), &obj1, gFalse, 0);
    } else {
      str = new EmbedStream(objStr.getStream(), &obj1, gTrue,
			    offsets[i+1] - offsets[i]);
    }
    parser = new Parser(xref, new Lexer(xref, str), gFalse);
    if(!parser->getObj(&objs[i])) 
      goto malformedErr;
    
    while (str->getChar() != EOF) ;
    delete parser;
  }

  gfree(offsets);
  ok = gTrue;

 err1:
  objStr.free();
  return;
 malformedErr:
  gfree(offsets);
  if(parser)
    delete parser;
  // TODO getObj how to hande ObjectStream constructor
}

ObjectStream::~ObjectStream() {
  int i;

  if (objs) {
    for (i = 0; i < nObjects; ++i) {
      objs[i].free();
    }
    delete[] objs;
  }
  gfree(objNums);
}

Object *ObjectStream::getObject(int objIdx, int objNum, Object *obj) {
  if (objIdx < 0 || objIdx >= nObjects || objNum != objNums[objIdx]) {
    return obj->initNull();
  }
  return objs[objIdx].copy(obj);
}

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

static const char * PDFHEADER="%PDF-";
XRef::XRef(BaseStream *strA):entries(NULL), streamEnds(NULL), objStr(NULL) {
  // inits stream and initializes internals
  str = strA;

  setErrCode(errNone);
  // get PDF specification version from file
  char buffer[1024];
  char * header;
  str->reset();
  do {
    if(!str->getLine(buffer, sizeof(buffer)))
    {
      setErrCode(errBadHeader);
      error(-1, "PDF file doesn't contain proper header");
      return;
    }
  }while (!(header = strstr(buffer, PDFHEADER)));
  header+=strlen(PDFHEADER);
  pdfVersion.append(header);

  // gets position of last xref section
  Guint pos = getStartXref();
  if(isOk())
    initInternals(pos);
}

void XRef::setErrCode(int err)const
{
  errCode = err;
  ok = (errCode == errNone)?gTrue:gFalse;
}

/** Initializes all XRef internal structures.
 * @param pos Position of xref table.
 *
 * Assumes that str field is already initialized.
 */
void XRef::initInternals(Guint pos)
{
  Object obj;

  ok = gTrue;
  setErrCode(errNone);
  size = 0;
  entries = NULL;
  streamEnds = NULL;
  streamEndsLen = 0;
  objStr = NULL;
  maxObj = 0;

  useEncrypt = gFalse;
  permFlags = defPermFlags;
  ownerPasswordOk = gFalse;

  // read the trailer
  start = str->getStart();

  // if there was a problem with the 'startxref' position, try to
  // reconstruct the xref table
  if (pos == 0) {
    if (!(ok = constructXRef())) {
      setErrCode(errDamaged);
      return;
    }

  // read the xref table
  } else {
    while (readXRef(&pos)) ;

    // if there was a problem with the xref table,
    // try to reconstruct it
    if (!ok) {
      if (!(ok = constructXRef())) {
	setErrCode(errDamaged);
	return;
      }
    }
  }

  // get the root dictionary (catalog) object
  getTrailerDict()->dictLookupNF("Root", &obj);
  if (!obj.isRef()) {
    obj.free();
    if (!(ok = constructXRef())) {
      setErrCode(errDamaged);
      return;
    }
  }

  // now set the trailer dictionary's xref pointer so we can fetch
  // indirect objects from it
  Dict *d = (Dict *)getTrailerDict()->getDict();
  d->setXRef(this);
}

void XRef::destroyInternals()
{
  if(entries)
  {
    gfree(entries);
    entries=NULL;
  }
  // Don't use getTrailerDict here because we have to be sure that we
  // are deallocating the correct trailer (not the one from descendant
  // class which replaces the original one because of changes)
  trailerDict.free();
  if (streamEnds) {
    gfree(streamEnds);
    streamEnds=NULL;
  }
  if (objStr) {
    delete objStr;
    objStr=NULL;
  }
}

XRef::~XRef() {
  destroyInternals();
}

// Read the 'startxref' position.
Guint XRef::getStartXref() {
  char buf[xrefSearchSize+1];
  int c, n, i;

  setErrCode(errNone);

  // read last xrefSearchSize bytes
  str->setPos(xrefSearchSize, -1);
  for (n = 0; n < xrefSearchSize; ++n) {
    if ((c = str->getChar()) == EOF) {
      break;
    }
    buf[n] = c;
  }
  buf[n] = '\0';

  // find startxref
  for (i = n - 9; i >= 0; --i) {
    if (!strncmp(&buf[i], "startxref", 9)) {
      break;
    }
  }
  if (i < 0) {
    setErrCode(errDamaged);
    return 0;
  }

  // get value for startxref
  for (i += strlen("startxref"); isspace((unsigned char)buf[i]); ++i) ;
  lastXRefPos = strToUnsigned(&buf[i]);

  // We are immediatelly after startxref value now. We will try to 
  // find %%EOF from here. If not found, we will use end of buffer as
  // end of file position
  for(; i<n; i++)
     if(!strncmp(&buf[i], "%%EOF", 5))
        break;

  // calculates eofPos from position in buffer
  // sets position to %%EOF start from buffer begining
  // which starts at xrefSearchSize from the end of stream
  str->setPos(n-i, -1);
  eofPos=str->getPos();

  return lastXRefPos;
}

// Read one xref table section.  Also reads the associated trailer
// dictionary, and returns the prev pointer (if any).
GBool XRef::readXRef(Guint *pos) {
  Parser *parser = NULL;
  Object obj;
  GBool more;

  // start up a parser, parse one token
  obj.initNull();
  parser = new Parser(NULL,
	     new Lexer(NULL,
	       str->makeSubStream(start + *pos, gFalse, 0, &obj)),
	     gTrue);
  if (!parser->getObj(&obj))
    goto malformedErr;

  // parse an old-style xref table
  if (obj.isCmd("xref")) {
    obj.free();
    more = readXRefTable(parser, pos);

  // parse an xref stream
  } else if (obj.isInt()) {
    obj.free();
    Object * fetch;
    if (!(fetch = parser->getObj(&obj)))
      goto malformedErr;
    if (!fetch->isInt()) 
      goto err1;
    obj.free();
    if (!(fetch = parser->getObj(&obj)))
      goto malformedErr;
    if (!fetch->isCmd("obj")) 
      goto err1;
    obj.free();
    if (!(fetch = parser->getObj(&obj)))
      goto malformedErr;
    if (!fetch->isStream()) 
      goto err1;
    more = readXRefStream(obj.getStream(), pos);
    obj.free();

  } else {
    goto err1;
  }

  delete parser;
  return more;

err1:
  obj.free();
malformedErr:
  if(parser)
    delete parser;
  ok = gFalse;
  return gFalse;
}

GBool XRef::readXRefTable(Parser *parser, Guint *pos) {
  XRefEntry entry;
  GBool more;
  Object obj, obj2;
  Guint pos2;
  Object * fetch;
  int first, n, newSize;
  Guint i;

  while (1) {
    if (!parser->getObj(&obj))
      goto malformedErr;
    if (obj.isCmd("trailer")) {
      obj.free();
      break;
    }
    if (!obj.isInt()) {
      goto err1;
    }
    first = obj.getInt();
    obj.free();
    if (!(fetch = parser->getObj(&obj)))
      goto malformedErr;
    if (!fetch->isInt()) {
      goto err1;
    }
    n = obj.getInt();
    obj.free();
    if (first < 0 || n < 0 || first + n < 0) {
      goto err1;
    }
    if (first + n > size) {
      for (newSize = size ? 2 * size : 1024;
	   first + n > newSize && newSize > 0;
	   newSize <<= 1) ;
      if (newSize < 0) {
	goto err1;
      }
      entries = (XRefEntry *)greallocn(entries, newSize, sizeof(XRefEntry));
      for (i = size; i < newSize; ++i) {
	entries[i].offset = 0xffffffff;
	entries[i].gen = 0;
	entries[i].type = xrefEntryFree;
      }
      size = newSize;
    }
    for (i = first; i < first + n; ++i) {
      if (!(fetch = parser->getObj(&obj)))
        goto malformedErr;
      if (!fetch->isInt()) {
	goto err1;
      }
      entry.offset = (Guint)obj.getInt();
      obj.free();
      if (!(fetch = parser->getObj(&obj)))
        goto malformedErr;
      if (!fetch->isInt()) {
	goto err1;
      }
      entry.gen = obj.getInt();
      obj.free();
      if (!parser->getObj(&obj))
        goto malformedErr;
      if (obj.isCmd("n")) {
	entry.type = xrefEntryUncompressed;
      } else if (obj.isCmd("f")) {
	entry.type = xrefEntryFree;
      } else {
	goto err1;
      }
      obj.free();
      if (entries[i].offset == 0xffffffff) {
	entries[i] = entry;
	// PDF files of patents from the IBM Intellectual Property
	// Network have a bug: the xref table claims to start at 1
	// instead of 0.
	if (i == 1 && first == 1 &&
	    entries[1].offset == 0 && entries[1].gen == 65535 &&
	    entries[1].type == xrefEntryFree) {
	  i = first = 0;
	  entries[0] = entries[1];
	  entries[1].offset = 0xffffffff;
	}

	// store maximum present indirect object number
	if (entries[i].type != xrefEntryFree && i > maxObj)
	  maxObj = i;
      }
    }
  }

  // read the trailer dictionary
  if (!(fetch = parser->getObj(&obj)))
    goto malformedErr;
  if (!fetch->isDict()) {
    goto err1;
  }

  // get the 'Prev' pointer
  obj.getDict()->lookupNF("Prev", &obj2);
  if (obj2.isInt()) {
    *pos = (Guint)obj2.getInt();
    more = gTrue;
  } else if (obj2.isRef()) {
    // certain buggy PDF generators generate "/Prev NNN 0 R" instead
    // of "/Prev NNN"
    *pos = (Guint)obj2.getRefNum();
    more = gTrue;
  } else {
    more = gFalse;
  }
  obj2.free();

  // save the first trailer dictionary
  // XPDF don't care about older trailers and it always uses the
  // most recent one
  if (trailerDict.isNone()) {
    obj.copy(&trailerDict);
  }

  // check for an 'XRefStm' key
  if (obj.getDict()->lookup("XRefStm", &obj2)->isInt()) {
    pos2 = (Guint)obj2.getInt();
    readXRef(&pos2);
    if (!ok) {
      obj2.free();
      goto err1;
    }
  }
  obj2.free();

  obj.free();
  return more;

err1:
  obj.free();
malformedErr:
  ok = gFalse;
  return gFalse;
}

GBool XRef::readXRefStream(Stream *xrefStr, Guint *pos) {
  const Dict *dict;
  int w[3];
  GBool more;
  Object obj, obj2, idx;
  int newSize, first, n, i;

  dict = xrefStr->getDict();

  if (!dict->lookupNF("Size", &obj)->isInt()) {
    goto err1;
  }
  newSize = obj.getInt();
  obj.free();
  if (newSize < 0) {
    goto err1;
  }
  if (newSize > size) {
    entries = (XRefEntry *)greallocn(entries, newSize, sizeof(XRefEntry));
    for (i = size; i < newSize; ++i) {
      entries[i].offset = 0xffffffff;
      entries[i].gen = 0;
      entries[i].type = xrefEntryFree;
    }
    size = newSize;
  }

  if (!dict->lookupNF("W", &obj)->isArray() ||
      obj.arrayGetLength() < 3) {
    goto err1;
  }
  for (i = 0; i < 3; ++i) {
    if (!obj.arrayGet(i, &obj2)->isInt()) {
      obj2.free();
      goto err1;
    }
    w[i] = obj2.getInt();
    obj2.free();
    if (w[i] < 0 || w[i] > 4) {
      goto err1;
    }
  }
  obj.free();

  xrefStr->reset();
  dict->lookupNF("Index", &idx);
  if (idx.isArray()) {
    for (i = 0; i+1 < idx.arrayGetLength(); i += 2) {
      if (!idx.arrayGet(i, &obj)->isInt()) {
	idx.free();
	goto err1;
      }
      first = obj.getInt();
      obj.free();
      if (!idx.arrayGet(i+1, &obj)->isInt()) {
	idx.free();
	goto err1;
      }
      n = obj.getInt();
      obj.free();
      if (first < 0 || n < 0 ||
	  !readXRefStreamSection(xrefStr, w, first, n)) {
	idx.free();
	goto err0;
      }
    }
  } else {
    if (!readXRefStreamSection(xrefStr, w, 0, newSize)) {
      idx.free();
      goto err0;
    }
  }
  idx.free();

  dict->lookupNF("Prev", &obj);
  if (obj.isInt()) {
    *pos = (Guint)obj.getInt();
    more = gTrue;
  } else {
    more = gFalse;
  }
  obj.free();
  // save the first trailer dictionary
  // XPDF don't care about older trailers and it always uses the
  // most recent one
  if (trailerDict.isNone()) {
    trailerDict.initDict((Dict *)dict);
  }

  return more;

 err1:
  obj.free();
 err0:
  ok = gFalse;
  return gFalse;
}

GBool XRef::readXRefStreamSection(Stream *xrefStr, int *w, int first, int n) {
  Guint offset;
  int type, gen, c, newSize, i, j;

  if (first + n < 0) {
    return gFalse;
  }
  if (first + n > size) {
    for (newSize = size ? 2 * size : 1024;
	 first + n > newSize && newSize > 0;
	 newSize <<= 1) ;
    if (newSize < 0) {
      return gFalse;
    }
    entries = (XRefEntry *)greallocn(entries, newSize, sizeof(XRefEntry));
    for (i = size; i < newSize; ++i) {
      entries[i].offset = 0xffffffff;
      entries[i].gen = 0;
      entries[i].type = xrefEntryFree;
    }
    size = newSize;
  }
  for (i = first; i < first + n; ++i) {
    if (w[0] == 0) {
      type = 1;
    } else {
      for (type = 0, j = 0; j < w[0]; ++j) {
	if ((c = xrefStr->getChar()) == EOF) {
	  return gFalse;
	}
	type = (type << 8) + c;
      }
    }
    for (offset = 0, j = 0; j < w[1]; ++j) {
      if ((c = xrefStr->getChar()) == EOF) {
	return gFalse;
      }
      offset = (offset << 8) + c;
    }
    for (gen = 0, j = 0; j < w[2]; ++j) {
      if ((c = xrefStr->getChar()) == EOF) {
	return gFalse;
      }
      gen = (gen << 8) + c;
    }
    if (entries[i].offset == 0xffffffff) {
      switch (type) {
      case 0:
	entries[i].offset = offset;
	entries[i].gen = gen;
	entries[i].type = xrefEntryFree;
	break;
      case 1:
	entries[i].offset = offset;
	entries[i].gen = gen;
	entries[i].type = xrefEntryUncompressed;
	break;
      case 2:
	entries[i].offset = offset;
	entries[i].gen = gen;
	entries[i].type = xrefEntryCompressed;
	break;
      default:
	return gFalse;
      }
    }

    // store maximum present indirect object number
    if (entries[i].type != xrefEntryFree && i > maxObj)
      maxObj = i;
  }

  return gTrue;
}

/** Returns the size of EOL marker preceding the given possition.
 * @param pos Position at the begginning of the line.
 * @param str Stream
 *
 * Stream will keep its current possition.
 *
 * @return length of the EOL (if any) preceeing pos.
 */
static int eolLength(Guint pos, BaseStream *str)
{
	Guint currPos = str->getPos();
	str->setPos(pos-2);
	int ch1 = str->getChar();
	int ch2 = str->getChar();
	int eoln = 0;
	if (ch1 == '\r' && ch2 == '\n')
		eoln = 2;
	else if (ch2 == '\n')
			eoln = 1;
	str->setPos(currPos);
	return eoln;
}

// Attempt to construct an xref table for a damaged file.
GBool XRef::constructXRef() {
  Parser *parser = NULL;
  Object newTrailerDict, obj;
  char buf[256];
  Guint pos;
  int num, gen;
  int newSize;
  int streamEndsSize;
  char *p;
  int i;
  GBool gotRoot;

  gfree(entries);
  size = 0;
  entries = NULL;

  error(-1, "PDF file is damaged - attempting to reconstruct xref table...");
  gotRoot = gFalse;
  streamEndsLen = streamEndsSize = 0;

  str->reset();
  while (1) {
    pos = str->getPos();
    if (!str->getLine(buf, 256)) {
      break;
    }
    p = buf;

    // skip whitespace
    while (*p && Lexer::isSpace(*p & 0xff)) ++p;

    // got trailer dictionary
    if (!strncmp(p, "trailer", 7)) {
      obj.initNull();
      parser = new Parser(NULL,
		 new Lexer(NULL,
		   str->makeSubStream(pos + 7, gFalse, 0, &obj)),
		 gFalse);
      if (!parser->getObj(&newTrailerDict))
        goto malformedErr;
      if (newTrailerDict.isDict()) {
	newTrailerDict.dictLookupNF("Root", &obj);
	if (obj.isRef()) {
	  if (!trailerDict.isNone()) {
	    trailerDict.free();
	  }
	  newTrailerDict.copy(&trailerDict);
	  gotRoot = gTrue;
	}
	obj.free();
      }
      newTrailerDict.free();
      delete parser;

    // look for object
    } else if (isdigit((unsigned char)(*p))) {
      num = (int)strtoul(p, NULL, 10);
      if (num > 0) {
	do {
	  ++p;
	} while (*p && isdigit((unsigned char)*p));
	if (isspace((unsigned char)*p)) {
	  do {
	    ++p;
	  } while (*p && isspace((unsigned char)*p));
	  if (isdigit((unsigned char)*p)) {
	    gen = (int)strtoul(p, NULL, 10);
	    do {
	      ++p;
	    } while (*p && isdigit((unsigned char)*p));
	    if (isspace((unsigned char)*p)) {
	      do {
		++p;
	      } while (*p && isspace((unsigned char)*p));
	      if (!strncmp(p, "obj", 3)) {
		if (num >= size) {
		  newSize = (num + 1 + 255) & ~255;
		  if (newSize < 0) {
		    error(-1, "Bad object number");
		    return gFalse;
		  }
		  entries = (XRefEntry *)
		      greallocn(entries, newSize, sizeof(XRefEntry));
		  for (i = size; i < newSize; ++i) {
		    entries[i].offset = 0xffffffff;
		    entries[i].gen = 0;
		    entries[i].type = xrefEntryFree;
		  }
		  size = newSize;
		}
		if (entries[num].type == xrefEntryFree ||
		    gen >= entries[num].gen) {
		  entries[num].offset = pos - start;
		  entries[num].gen = gen;
		  entries[num].type = xrefEntryUncompressed;
		}
	      }
	    }
	  }
	}
      }

    } else if (!strncmp(p, "endstream", 9)) {
      if (streamEndsLen == streamEndsSize) {
	streamEndsSize += 64;
	streamEnds = (Guint *)greallocn(streamEnds,
					streamEndsSize, sizeof(int));
      }
      // We are reading lines and the pdf specification says that:
      // "The sequence of bytes that make up a stream lie between the stream
      // and endstream keywords; the stream dictionary specifies the exact 
      // number of bytes. It is recommended that there be an end-of-line 
      // marker after the data and before endstream; this marker is not 
      // included in the stream length."
      // So we have to either add stream data that preceeds the keyword
      // or decrease the lenght by the lenght of EOL marker if we are at 
      // the beggining of line
      streamEnds[streamEndsLen++] = pos + 
	      (p-buf)?p-buf
	      :-eolLength(pos, str);
    }
  }

  if (gotRoot)
    return gTrue;

  error(-1, "Couldn't find trailer dictionary");
  return gFalse;
malformedErr:
  error(-1, "malformed content. Not able to parse.");
  if (parser)
    delete parser;
  return gFalse;
}

void XRef::setEncryption(int permFlagsA, GBool ownerPasswordOkA,
			 const Guchar *fileKeyA, int keyLengthA, int encVersionA,
			 CryptAlgorithm encAlgorithmA) {
  int i;

  useEncrypt = gTrue;
  permFlags = permFlagsA;
  ownerPasswordOk = ownerPasswordOkA;
  if (keyLengthA <= 16) {
    keyLength = keyLengthA;
  } else {
    keyLength = 16;
  }
  for (i = 0; i < keyLength; ++i) {
    fileKey[i] = fileKeyA[i];
  }
  encVersion = encVersionA;
  encAlgorithm = encAlgorithmA;
}

GBool XRef::okToPrint(GBool ignoreOwnerPW)const {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permPrint);
}

GBool XRef::okToChange(GBool ignoreOwnerPW)const {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permChange);
}

GBool XRef::okToCopy(GBool ignoreOwnerPW)const {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permCopy);
}

GBool XRef::okToAddNotes(GBool ignoreOwnerPW)const {
  return (!ignoreOwnerPW && ownerPasswordOk) || (permFlags & permNotes);
}

Object *XRef::fetch(int num, int gen, Object *obj)const {
  XRefEntry *e;
  Parser *parser;
  Object obj1, obj2, obj3;
  GBool failed = gFalse;

  // check for bogus ref - this can happen in corrupted PDF files
  if (num < 0 || num >= size) {
    goto err_no_obj;
  }

  setErrCode(errNone);
  e = &entries[num];
  switch (e->type) {

  case xrefEntryUncompressed:
    if (e->gen != gen) {
      goto err_no_obj;
    }
    obj1.initNull();
    parser = new Parser(this,
	       new Lexer(this,
		 str->makeSubStream(start + e->offset, gFalse, 0, &obj1)),
	       gTrue);
    if (!parser->getObj(&obj1))
      goto err_damaged;
    if (!parser->getObj(&obj2)) {
      obj1.free();
      goto err_damaged;
    }
    if (!parser->getObj(&obj3)) {
      obj2.free();
      goto err_damaged;
    } 
    if (!obj1.isInt() || obj1.getInt() != num ||
	!obj2.isInt() || obj2.getInt() != gen ||
	!obj3.isCmd("obj")) {
      obj1.free();
      obj2.free();
      obj3.free();
      delete parser;
      goto err_damaged;
    }
    if (!parser->getObj(obj, useEncrypt ? fileKey : (const Guchar *)NULL,
		   encAlgorithm, keyLength, num, gen)) 
      failed = gTrue;

    obj1.free();
    obj2.free();
    obj3.free();
    delete parser;
    if (failed)
      goto err_damaged;
    break;

  case xrefEntryCompressed:
    if (gen != 0) {
      goto err_no_obj;
    }
    if (!objStr || objStr->getObjStrNum() != (int)e->offset) {
      if (objStr) {
	delete objStr;
      }
      objStr = new ObjectStream(this, e->offset);
      if (!objStr->isOk()) {
	delete objStr;
	objStr = NULL;
	goto err_damaged;
      }
    }
    objStr->getObject(e->gen, num, obj);
    break;

  default:
    // TODO is this correct?
    goto err_no_obj;
  }

  return obj;

 err_damaged:
  setErrCode(errDamaged);
  ok = false;
 err_no_obj:
  return obj->initNull();
}

Object *XRef::getDocInfo(Object *obj) {
  return getTrailerDict()->dictLookup("Info", obj);
}

// Added for the pdftex project.
Object *XRef::getDocInfoNF(Object *obj) {
  return getTrailerDict()->dictLookupNF("Info", obj);
}

GBool XRef::getStreamEnd(Guint streamStart, Guint *streamEnd)const {
  int a, b, m;

  if (streamEndsLen == 0 ||
      streamStart > streamEnds[streamEndsLen - 1]) {
    return gFalse;
  }

  a = -1;
  b = streamEndsLen - 1;
  // invariant: streamEnds[a] < streamStart <= streamEnds[b]
  while (b - a > 1) {
    m = (a + b) / 2;
    if (streamStart <= streamEnds[m]) {
      b = m;
    } else {
      a = m;
    }
  }
  *streamEnd = streamEnds[b];
  return gTrue;
}

Guint XRef::strToUnsigned(const char *s)const {
  Guint x;
  const char *p;
  int i;

  x = 0;
  for (p = s, i = 0; *p && isdigit((unsigned char)*p) && i < 10; ++p, ++i) {
    x = 10 * x + (*p - '0');
  }
  return x;
}

int getRootFromTrailer(const Object *trailer, Ref &ref)
{
	Object o;
	trailer->dictLookupNF("Root", &o);
	if (o.getType() == objRef)
	{
		ref = o.getRef();
		return 0;
	}
	return -1;
}

int XRef::getRootNum()const
{
	Ref r;
	if(!getRootFromTrailer(getTrailerDict(), r))
		return r.num;
	return -1;
}

int XRef::getRootGen()const
{
	Ref r;
	if(!getRootFromTrailer(getTrailerDict(), r))
		return r.gen;
	return -1;
}

RefState XRef::knowsRef(const Ref &ref)const
{
   // boundary checking
   if(ref.num<0 || ref.num>size)
      return UNUSED_REF;

   switch(entries[ref.num].type)
   {
      // must not be free entry
      case xrefEntryFree:
         return UNUSED_REF;

      // if uncompressed entry, also gen number must fit
      case xrefEntryUncompressed:
         return (ref.gen==entries[ref.num].gen)?INITIALIZED_REF:UNUSED_REF;

      // if compressed entry, gen number must be 0
      // NOTE: XRef internaly uses this number for indexing of object in 
      // object stream
      case xrefEntryCompressed:
         return (ref.gen==0)?INITIALIZED_REF:UNUSED_REF;
         
   }

   // unknown entry type
   return UNUSED_REF;
}
