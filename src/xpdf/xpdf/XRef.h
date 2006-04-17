/*
 * $RCSfile$
 *
 * $log: $
 *
 */

//========================================================================
//
// XRef.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
// Changes:
// Michal Hocko - all public methods are virtual and private fields are
//                protected (we need to create transparent wrapper/decorator
//                to this class)
//              - initialization and destruction of internal structures is 
//                done separately in methods. Constructor and destructor code
//                was moved to those methods and they just call this methods.
//              - eofPos position added -> points to file offset where it is 
//                safe to put new data
//
//========================================================================

#ifndef XREF_H
#define XREF_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "gtypes.h"
#include "Object.h"

class Dict;
class Stream;
class Parser;
class ObjectStream;

//------------------------------------------------------------------------
// XRef
//------------------------------------------------------------------------

enum XRefEntryType {
  xrefEntryFree,
  xrefEntryUncompressed,
  xrefEntryCompressed
};

struct XRefEntry {
  Guint offset;
  int gen;
  XRefEntryType type;
};

class XRef {
public:

  // Constructor.  Read xref table from stream.
  XRef(BaseStream *strA);

  // Destructor.
  virtual ~XRef();

  // Is xref table valid?
  virtual GBool isOk() { return ok; }

  // Get the error code (if isOk() returns false).
  virtual int getErrorCode() { return errCode; }

  // Set the encryption parameters.
  virtual void setEncryption(int permFlagsA, GBool ownerPasswordOkA,
		     Guchar *fileKeyA, int keyLengthA, int encVersionA);

  // Is the file encrypted?
  virtual GBool isEncrypted() { return encrypted; }

  // Check various permissions.
  virtual GBool okToPrint(GBool ignoreOwnerPW = gFalse);
  virtual GBool okToChange(GBool ignoreOwnerPW = gFalse);
  virtual GBool okToCopy(GBool ignoreOwnerPW = gFalse);
  virtual GBool okToAddNotes(GBool ignoreOwnerPW = gFalse);

  // Get catalog object.
  virtual Object *getCatalog(Object *obj) { return fetch(rootNum, rootGen, obj); }

  // Fetch an indirect reference.
  virtual Object *fetch(int num, int gen, Object *obj);

  // Return the document's Info dictionary (if any).
  virtual Object *getDocInfo(Object *obj);
  virtual Object *getDocInfoNF(Object *obj);

  // Return the number of objects in the xref table.
  virtual int getNumObjects() { return size; }

  // Return the offset of the last xref table.
  virtual Guint getLastXRefPos() { return lastXRefPos; }

  // Return the catalog object reference.
  virtual int getRootNum() { return rootNum; }
  virtual int getRootGen() { return rootGen; }

  // Get end position for a stream in a damaged file.
  // Returns false if unknown or file is not damaged.
  virtual GBool getStreamEnd(Guint streamStart, Guint *streamEnd);

  // Direct access.
  virtual int getSize() { return size; }
  virtual XRefEntry *getEntry(int i) { return &entries[i]; }
  virtual Object *getTrailerDict() { return &trailerDict; }

protected:

  BaseStream *str;		// input stream
  Guint start;			// offset in file (to allow for garbage
				//   at beginning of file)
  XRefEntry *entries;		// xref entries
  int size;			// size of <entries> array
  int rootNum, rootGen;		// catalog dict
  GBool ok;			// true if xref table is valid
  int errCode;			// error code (if <ok> is false)
  Object trailerDict;		// trailer dictionary
  Guint lastXRefPos;		// offset of last xref table
  Guint eofPos;                 // %%EOF marker position or safe position to 
                                //   store new data 
  Guint *streamEnds;		// 'endstream' positions - only used in
				//   damaged files
  int streamEndsLen;		// number of valid entries in streamEnds
  ObjectStream *objStr;		// cached object stream
  GBool encrypted;		// true if file is encrypted
  int permFlags;		// permission bits
  GBool ownerPasswordOk;	// true if owner password is correct
  Guchar fileKey[16];		// file decryption key
  int keyLength;		// length of key, in bytes
  int encVersion;		// encryption algorithm

  // inits all internal structures which may change
  void initInternals();

  // destroy all internal structures which may be reinitialized
  void destroyInternals();
  
  Guint getStartXref();
  GBool readXRef(Guint *pos);
  GBool readXRefTable(Parser *parser, Guint *pos);
  GBool readXRefStreamSection(Stream *xrefStr, int *w, int first, int n);
  GBool readXRefStream(Stream *xrefStr, Guint *pos);
  GBool constructXRef();
  Guint strToUnsigned(char *s);
};

#endif
