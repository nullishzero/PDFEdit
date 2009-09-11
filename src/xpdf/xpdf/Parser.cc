//========================================================================
//
// Parser.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include "xpdf/Object.h"
#include "xpdf/Array.h"
#include "xpdf/Dict.h"
#include "xpdf/Decrypt.h"
#include "xpdf/Parser.h"
#include "xpdf/XRef.h"
#include "xpdf/Error.h"

Parser::Parser(const XRef *xrefA, Lexer *lexerA, GBool allowStreamsA) {
  xref = xrefA;
  lexer = lexerA;
  inlineImg = 0;
  endOfActStream = 0;
  allowStreams = allowStreamsA;
  lexer->getObj(&buf1);
  lexer->getObj(&buf2);
}

Parser::~Parser() {
  buf1.free();
  buf2.free();
  delete lexer;
}

Object *Parser::getObj(Object *obj, const Guchar *fileKey,
		       CryptAlgorithm encAlgorithm, int keyLength,
		       int objNum, int objGen) {
  char *key;
  Stream *str;
  Object obj2;
  int num;
  DecryptStream *decrypt;
  const GString *s; 
  GString *s2;
  int c;

  // refill buffer after inline image data
  if (inlineImg == 2) {
    buf1.free();
    buf2.free();
    lexer->getObj(&buf1);
    lexer->getObj(&buf2);
    inlineImg = 0;
  }

  // array
  if (buf1.isCmd("[")) {
    shift();
    obj->initArray(xref);
    while (!buf1.isCmd("]") && !buf1.isEOF()) {
      Object * child = getObj(&obj2, fileKey, encAlgorithm, keyLength,
			   objNum, objGen);
      if(!child) {
	goto err;
      }
      obj->arrayAdd(child);
    }
    if (buf1.isEOF()) {
      error(getPos(), "End of file inside array");
      goto err;
    }
    shift();

  // dictionary or stream
  } else if (buf1.isCmd("<<")) {
    shift();
    obj->initDict(xref);
    while (!buf1.isCmd(">>") && !buf1.isEOF()) {
      if (!buf1.isName()) {
	error(getPos(), "Dictionary key must be a name object");
	buf1.free();
	goto err;
      }

      key = copyString(buf1.getName());
      shift();
      if (buf1.isEOF() || buf1.isError()) {
        gfree(key);
        break;
      }
      Object * child = getObj(&obj2, fileKey, encAlgorithm, keyLength,
      			 objNum, objGen);
      if(!child) {
        goto err;
      }
      obj->dictAdd(key, child);
    }
    if (buf1.isEOF()) {
      error(getPos(), "End of file inside dictionary");
      buf1.free();
      goto err;
    }
    // stream objects are not allowed inside content streams or
    // object streams
    if (allowStreams && buf2.isCmd("stream")) {
      if ((str = makeStream(obj, fileKey, encAlgorithm, keyLength,
			    objNum, objGen))) {
	obj->initStream(str);
      } else {
	obj->free();
	goto err;
      }
    } else {
      shift();
    }

  // indirect reference or integer
  } else if (buf1.isInt()) {
    num = buf1.getInt();
    shift();
    if (buf1.isInt() && buf2.isCmd("R")) {
      obj->initRef(num, buf1.getInt());
      shift();
      shift();
    } else {
      obj->initInt(num);
    }

  // string
  } else if (buf1.isString() && fileKey) {
    s = buf1.getString();
    s2 = new GString();
    obj2.initNull();
    decrypt = new DecryptStream(new MemStream(s->getCString(), 0,
					      s->getLength(), &obj2),
				fileKey, encAlgorithm, keyLength,
				objNum, objGen);
    decrypt->reset();
    while ((c = decrypt->getChar()) != EOF) {
      s2->append((char)c);
    }
    delete decrypt;
    obj->initString(s2);
    shift();

  // simple object
  } else {
    buf1.copy(obj);
    shift();
  }

  return obj;
err:
  if (obj)
    obj->free();
  obj->initError();
  return NULL;
}

Stream *Parser::makeStream(const Object *dict, const Guchar *fileKey,
			   CryptAlgorithm encAlgorithm, int keyLength,
			   int objNum, int objGen) {
  Object obj;
  BaseStream *baseStr;
  Stream *str;
  Guint pos, endPos, length;

  // get stream start position
  lexer->skipToNextLine();
  pos = lexer->getPos();

  // get length
  dict->dictLookup("Length", &obj);
  if (obj.isInt()) {
    length = (Guint)obj.getInt();
    obj.free();
  } else {
    error(getPos(), "Bad 'Length' attribute in stream");
    obj.free();
    return NULL;
  }

  // check for length in damaged file
  if (xref && xref->getStreamEnd(pos, &endPos)) {
    if (length != (endPos - pos)) {
      error(endPos, "Stream doesn't have correct length");
      return NULL;
    }
  }

  // in badly damaged PDF files, we can run off the end of the input
  // stream immediately after the "stream" token
  if (!lexer->getStream()) {
    return NULL;
  }
  baseStr = lexer->getStream()->getBaseStream();

  // skip over stream data
  lexer->setPos(pos + length);

  // refill token buffers and check for 'endstream'
  shift();  // kill '>>'
  shift();  // kill 'stream'
  if (buf1.isCmd("endstream")) {
    shift();
  } else {
    error(getPos(), "Missing 'endstream'");
    return NULL;
  }

  // make base stream
  str = baseStr->makeSubStream(pos, gTrue, length, dict);

  // handle decryption
  if (fileKey) {
    str = new DecryptStream(str, fileKey, encAlgorithm, keyLength,
			    objNum, objGen);
  }

  // get filters
  str = str->addFilters(dict);

  return str;
}

void Parser::shift() {
  if (inlineImg > 0) {
    if (inlineImg < 2) {
      ++inlineImg;
    } else {
      // in a damaged content stream, if 'ID' shows up in the middle
      // of a dictionary, we need to reset
      inlineImg = 0;
    }
  } else if (buf2.isCmd("ID")) {
    lexer->skipChar();		// skip char after 'ID' command
    inlineImg = 1;
  }
  buf1.free();
  buf1 = buf2;
  if (inlineImg > 0)		// don't buffer inline image data
    buf2.initNull();
  else
  {
	size_t pos = lexer->strIndex ();
	lexer->getObj(&buf2);
	if (pos != lexer->strIndex())
		endOfActStream = 2;
	else if (0 < endOfActStream)
		--endOfActStream;
  }

}
