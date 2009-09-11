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
//              - getNumObjects rewriten to return just number of real objects
//                not size of entries array
//              - new knowsRef method
//              - setErrCode method added
//              - maxObj field added which contains the maximum present 
//                indirect object number
//              - pdfVersion and getPDFVersion added
//
//========================================================================

#ifndef XREF_H
#define XREF_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "xpdf/Object.h"

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

/** State of reference type.
 *
 * Describes state of reference. Use *_REF defined values.
 */
typedef int RefState;

/** Reference is unknown. */
#define UNUSED_REF          0

/** Reference is reserved, but not initialized yet. */
#define RESERVED_REF        1

/** Reference is known. */
#define INITIALIZED_REF     2

class XRef {
public:

  // Constructor.  Read xref table from stream.
  XRef(BaseStream *strA);

  // Destructor.
  virtual ~XRef();

  // Is xref table valid?
  virtual GBool isOk()const { return ok; }

  // sets error code and ok accordingly
  // if err == errNone then ok is set to true
  virtual void setErrCode(int err)const;

  // Get the error code (if isOk() returns false).
  virtual int getErrorCode()const { return errCode; }

  // Set the encryption parameters.
  virtual void setEncryption(int permFlagsA, GBool ownerPasswordOkA,
  		     const Guchar *fileKeyA, int keyLengthA, int encVersionA,
		     CryptAlgorithm encAlgorithmA);

  // Is the file encrypted?
  virtual GBool isEncrypted()const { return encrypted; }

  // Check various permissions.
  virtual GBool okToPrint(GBool ignoreOwnerPW = gFalse)const;
  virtual GBool okToChange(GBool ignoreOwnerPW = gFalse)const;
  virtual GBool okToCopy(GBool ignoreOwnerPW = gFalse)const;
  virtual GBool okToAddNotes(GBool ignoreOwnerPW = gFalse)const;
  
  // Get catalog object.
  virtual Object *getCatalog(Object *obj)const { return fetch(getRootNum(), getRootGen(), obj); }
  
  // Fetch an indirect reference.
  virtual Object *fetch(int num, int gen, Object *obj)const;
  
  // Return the document's Info dictionary (if any).
  virtual Object *getDocInfo(Object *obj);
  virtual Object *getDocInfoNF(Object *obj);
  
  // Return the number of objects in the xref table.
  virtual int getNumObjects()const
  { 
     int count=0;
     for(int i=0; i<size; i++)
        // counts just not free entries
        if(entries[i].type != xrefEntryFree)
           count++;

     return count; 
  }

  /** Ckecks if given reference is known.
   * @param ref Reference to examine.
   *
   */
  virtual RefState knowsRef(const Ref &ref)const;

  // Return the offset of the last xref table.
  virtual Guint getLastXRefPos()const { return lastXRefPos; }

  // Return the catalog object reference.
  virtual int getRootNum()const;
  virtual int getRootGen()const;

  // Get end position for a stream in a damaged file.
  // Returns false if unknown or file is not damaged.
  virtual GBool getStreamEnd(Guint streamStart, Guint *streamEnd)const;

  // Direct access.
  virtual int getSize()const { return size; }
  virtual XRefEntry *getEntry(int i)const { return &entries[i]; }
  virtual const Object *getTrailerDict()const { return &trailerDict; }

  virtual const char *getPDFVersion()const {return pdfVersion.getCString(); }
private:
  Object trailerDict;		// trailer dictionary - keep it private because
  				// we want to force all descendants to use 
				// getTrailerDict accessor method which they can
				// override
protected:

  BaseStream *str;		// input stream
  Guint start;			// offset in file (to allow for garbage
				//   at beginning of file)
  XRefEntry *entries;		// xref entries
  int size;			// size of <entries> array
  mutable GBool ok;		// true if xref table is valid
  mutable int errCode;		// error code (if <ok> is false)
  Guint lastXRefPos;		// offset of last xref table
  Guint eofPos;                 // %%EOF marker position or safe position to 
                                //   store new data 
  Guint maxObj;                 // Maximum present indirect object number (for
                                //   all previous revisions)
  mutable GString pdfVersion;	// PDF version used for document
  Guint *streamEnds;		// 'endstream' positions - only used in
				//   damaged files
  int streamEndsLen;		// number of valid entries in streamEnds
  mutable ObjectStream *objStr;	// cached object stream
  GBool useEncrypt;		// true if we want to decrypt content
  // TODO where is this field initialized ???
  GBool encrypted;		// Flag whether document is encrypted.
  int permFlags;		// permission bits
  GBool ownerPasswordOk;	// true if owner password is correct
  Guchar fileKey[16];		// file decryption key
  int keyLength;		// length of key, in bytes
  int encVersion;		// encryption version
  CryptAlgorithm encAlgorithm;	// encryption algorithm

  // inits all internal structures which may change
  void initInternals(Guint pos);
  // destroy all internal structures which may be reinitialized
  void destroyInternals();

  Guint getStartXref();
  GBool readXRef(Guint *pos);
  GBool readXRefTable(Parser *parser, Guint *pos);
  GBool readXRefStreamSection(Stream *xrefStr, int *w, int first, int n);
  GBool readXRefStream(Stream *xrefStr, Guint *pos);
  GBool constructXRef();
  Guint strToUnsigned(const char *s)const;
};

#endif
