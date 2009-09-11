//========================================================================
//
// GString.h
//
// Simple variable-length string type.
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef GSTRING_H
#define GSTRING_H

#include <xpdf-aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdarg.h>
#include "goo/gtypes.h"

class GString {
public:

  // Create an empty string.
  GString();

  // Create a string from a C string.
  GString(const char *sA);

  // Create a string from <lengthA> chars at <sA>.  This string
  // can contain null characters.
  GString(const char *sA, int lengthA);

  // Create a string from <lengthA> chars at <idx> in <str>.
  GString(const GString *str, int idx, int lengthA);

  // Copy a string.
  GString(const GString *str);
  GString *copy()const { return new GString(this); }

  // Concatenate two strings.
  GString(const GString *str1, const GString *str2);

  // Convert an integer to a string.
  static GString *fromInt(int x);

  // Create a formatted string.  Similar to printf, but without the
  // string overflow issues.  Formatting elements consist of:
  //     {<arg>:[<width>][.<precision>]<type>}
  // where:
  // - <arg> is the argument number (arg 0 is the first argument
  //   following the format string) -- NB: args must be first used in
  //   order; they can be reused in any order
  // - <width> is the field width -- negative to reverse the alignment;
  //   starting with a leading zero to zero-fill (for integers)
  // - <precision> is the number of digits to the right of the decimal
  //   point (for floating point numbers)
  // - <type> is one of:
  //     d, x, o, b -- int in decimal, hex, octal, binary
  //     ud, ux, uo, ub -- unsigned int
  //     ld, lx, lo, lb, uld, ulx, ulo, ulb -- long, unsigned long
  //     f, g -- double
  //     c -- char
  //     s -- string (char *)
  //     t -- GString *
  //     w -- blank space; arg determines width
  // To get literal curly braces, use {{ or }}.
  static GString *format(char *fmt, ...);
  static GString *formatv(char *fmt, va_list argList);

  // Destructor.
  ~GString();

  // Get length.
  int getLength()const { return length; }

  // Get C string.
  char *getCString()const { return s; }

  // Get <i>th character.
  char getChar(int i)const { return s[i]; }

  // Change <i>th character.
  void setChar(int i, char c) { s[i] = c; }

  // Clear string to zero length.
  GString *clear();

  // Append a character or string.
  GString *append(char c);
  GString *append(const GString *str);
  GString *append(const char *str);
  GString *append(const char *str, int lengthA);

  // Append a formatted string.
  GString *appendf(char *fmt, ...);
  GString *appendfv(char *fmt, va_list argList);

  // Insert a character or string.
  GString *insert(int i, char c);
  GString *insert(int i, GString *str);
  GString *insert(int i, const char *str);
  GString *insert(int i, const char *str, int lengthA);

  // Delete a character or range of characters.
  GString *del(int i, int n = 1);

  // Convert string to all-upper/all-lower case.
  GString *upperCase();
  GString *lowerCase();

  // Compare two strings:  -1:<  0:=  +1:>
  int cmp(const GString *str)const;
  int cmpN(const GString *str, int n)const;
  int cmp(const char *sA)const;
  int cmpN(const char *sA, int n)const;

private:

  int length;
  char *s;

  void resize(int length1);
  static void formatInt(long x, char *buf, int bufSize,
			GBool zeroFill, int width, int base,
			char **p, int *len);
  static void formatUInt(Gulong x, char *buf, int bufSize,
			 GBool zeroFill, int width, int base,
			 char **p, int *len);
  static void formatDouble(double x, char *buf, int bufSize, int prec,
			   GBool trim, char **p, int *len);
};

#if defined(WIN32) || defined(UNDER_CE)
#include <string.h>
inline wchar_t* ASCItoWide(const char* s)
{
	size_t len = s ? strlen(s) : 0;

	if (0 == len)
	{
		wchar_t* rv = new wchar_t[1];
		*rv = 0;
		return rv;
	}

	wchar_t* rv = new wchar_t[len + 1];
	for(size_t i=0;i<len;++i)
		rv[i] = s[i];
	rv[len] = 0;
	return rv;
}
#endif

#endif
