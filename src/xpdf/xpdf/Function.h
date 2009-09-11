//========================================================================
//
// Function.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef FUNCTION_H
#define FUNCTION_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "goo/gtypes.h"
#include "xpdf/Object.h"

class Dict;
class Stream;
struct PSObject;
class PSStack;

//------------------------------------------------------------------------
// Function
//------------------------------------------------------------------------

#define funcMaxInputs        32
#define funcMaxOutputs       32
#define sampledFuncMaxInputs 16

class Function {
public:

  Function();

  virtual ~Function();

  // Construct a function.  Returns NULL if unsuccessful.
  static Function *parse(const Object *funcObj);

  // Initialize the entries common to all function types.
  GBool init(const Dict *dict);

  virtual Function *copy()const = 0;

  // Return the function type:
  //   -1 : identity
  //    0 : sampled
  //    2 : exponential
  //    3 : stitching
  //    4 : PostScript
  virtual int getType()const = 0;

  // Return size of input and output tuples.
  int getInputSize()const { return m; }
  int getOutputSize()const { return n; }

  double getDomainMin(int i)const { return domain[i][0]; }
  double getDomainMax(int i)const { return domain[i][1]; }
  double getRangeMin(int i)const { return range[i][0]; }
  double getRangeMax(int i)const { return range[i][1]; }
  GBool getHasRange()const { return hasRange; }

  // Transform an input tuple into an output tuple.
  virtual void transform(const double *in, double *out)const = 0;

  virtual GBool isOk()const = 0;

protected:

  int m, n;			// size of input and output tuples
  double			// min and max values for function domain
    domain[funcMaxInputs][2];
  double			// min and max values for function range
    range[funcMaxOutputs][2];
  GBool hasRange;		// set if range is defined
};

//------------------------------------------------------------------------
// IdentityFunction
//------------------------------------------------------------------------

class IdentityFunction: public Function {
public:

  IdentityFunction();
  virtual ~IdentityFunction();
  virtual Function *copy()const { return new IdentityFunction(); }
  virtual int getType()const { return -1; }
  virtual void transform(const double *in, double *out)const;
  virtual GBool isOk()const { return gTrue; }

private:
};

//------------------------------------------------------------------------
// SampledFunction
//------------------------------------------------------------------------

class SampledFunction: public Function {
public:

  SampledFunction(const Object *funcObj, const Dict *dict);
  virtual ~SampledFunction();
  virtual Function *copy()const { return new SampledFunction(this); }
  virtual int getType()const { return 0; }
  virtual void transform(const double *in, double *out)const;
  virtual GBool isOk()const { return ok; }

  int getSampleSize(int i) { return sampleSize[i]; }
  double getEncodeMin(int i) { return encode[i][0]; }
  double getEncodeMax(int i) { return encode[i][1]; }
  double getDecodeMin(int i) { return decode[i][0]; }
  double getDecodeMax(int i) { return decode[i][1]; }
  double *getSamples() { return samples; }

private:

  SampledFunction(const SampledFunction *func);

  int				// number of samples for each domain element
    sampleSize[funcMaxInputs];
  double			// min and max values for domain encoder
    encode[funcMaxInputs][2];
  double			// min and max values for range decoder
    decode[funcMaxOutputs][2];
  double			// input multipliers
    inputMul[funcMaxInputs];
  int idxMul[funcMaxInputs];	// sample array index multipliers
  double *samples;		// the samples
  int nSamples;			// size of the samples array
  double *sBuf;			// buffer for the transform function
  GBool ok;
};

//------------------------------------------------------------------------
// ExponentialFunction
//------------------------------------------------------------------------

class ExponentialFunction: public Function {
public:

  ExponentialFunction(const Object *funcObj, const Dict *dict);
  virtual ~ExponentialFunction();
  virtual Function *copy()const  { return new ExponentialFunction(this); }
  virtual int getType()const { return 2; }
  virtual void transform(const double *in, double *out)const;
  virtual GBool isOk()const { return ok; }

  double *getC0() { return c0; }
  double *getC1() { return c1; }
  double getE() { return e; }

private:

  ExponentialFunction(const ExponentialFunction *func);

  double c0[funcMaxOutputs];
  double c1[funcMaxOutputs];
  double e;
  GBool ok;
};

//------------------------------------------------------------------------
// StitchingFunction
//------------------------------------------------------------------------

class StitchingFunction: public Function {
public:

  StitchingFunction(const Object *funcObj, const Dict *dict);
  virtual ~StitchingFunction();
  virtual Function *copy()const { return new StitchingFunction(this); }
  virtual int getType()const { return 3; }
  virtual void transform(const double *in, double *out)const;
  virtual GBool isOk()const { return ok; }

  int getNumFuncs() { return k; }
  Function *getFunc(int i) { return funcs[i]; }
  double *getBounds() { return bounds; }
  double *getEncode() { return encode; }
  double *getScale() { return scale; }

private:

  StitchingFunction(const StitchingFunction *func);

  int k;
  Function **funcs;
  double *bounds;
  double *encode;
  double *scale;
  GBool ok;
};

//------------------------------------------------------------------------
// PostScriptFunction
//------------------------------------------------------------------------

class PostScriptFunction: public Function {
public:

  PostScriptFunction(const Object *funcObj, const Dict *dict);
  virtual ~PostScriptFunction();
  virtual Function *copy()const { return new PostScriptFunction(this); }
  virtual int getType()const { return 4; }
  virtual void transform(const double *in, double *out)const;
  virtual GBool isOk()const { return ok; }

  GString *getCodeString() { return codeString; }

private:

  PostScriptFunction(const PostScriptFunction *func);
  GBool parseCode(Stream *str, int *codePtr);
  GString *getToken(Stream *str);
  void resizeCode(int newSize);
  void exec(PSStack *stack, int codePtr)const;

  GString *codeString;
  PSObject *code;
  int codeSize;
  GBool ok;
};

#endif
