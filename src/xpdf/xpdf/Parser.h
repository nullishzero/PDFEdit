//========================================================================
//
// Parser.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef PARSER_H
#define PARSER_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "xpdf/Lexer.h"

//------------------------------------------------------------------------
// Parser
//------------------------------------------------------------------------

class Parser {
public:

  // Constructor.
  Parser(const XRef *xrefA, Lexer *lexerA, GBool allowStreamsA);

  // Destructor.
  ~Parser();

  // Get the next object from the input stream.
  // Returns pointer to the given obj or NULL if not able to parse
  // (given obj is initialized to objError in such a case)
WARN_UNUSED_RESULT 
  Object *getObj(Object *obj, const Guchar *fileKey = NULL,
		 CryptAlgorithm encAlgorithm = cryptRC4, int keyLength = 0,
		 int objNum = 0, int objGen = 0);

  // Get stream.
  Stream *getStream()const { return lexer->getStream(); }

  // Get current position in file.
  int getPos()const { return lexer->getPos(); }

  // End of actual stream
  bool eofOfActualStream () const { return (1 == endOfActStream); }
  // Get bext token -- be carefull, it need not point to real next object
  void getNextObj (Object& obj) 
  	{ buf1.copy (&obj); }

private:

  const XRef *xref;		// the xref table for this PDF file
  Lexer *lexer;			// input stream
  GBool allowStreams;		// parse stream objects?
  Object buf1, buf2;		// next two tokens
  int inlineImg;		// set when inline image data is encountered
  size_t endOfActStream; // 1 means end of act stream

  Stream *makeStream(const Object *dict, const Guchar *fileKey,
		     CryptAlgorithm encAlgorithm, int keyLength,
		     int objNum, int objGen);
  void shift();
};

#endif

