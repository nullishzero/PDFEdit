/*
 * aconf2.h
 *
 * This gets included by xpdf-xpdf-aconf.h, and contains miscellaneous global
 * settings not directly controlled by autoconf.  This is a separate
 * file because otherwise the configure script will munge any
 * #define/#undef constructs.
 *
 * Copyright 2002-2003 Glyph & Cog, LLC
 */

#ifndef ACONF2_H
#define ACONF2_H

#include "os/compiler.h"

/*
 * Make sure WIN32 is defined if appropriate.
 */
#if defined(_WIN32) && !defined(WIN32)
#  define WIN32
#endif

#endif
