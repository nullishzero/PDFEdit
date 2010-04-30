/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _XPDF_H_
#define _XPDF_H_


#include <xpdf/Object.h>
#include <xpdf/Parser.h>
#include <xpdf/Lexer.h>
#include <xpdf/Stream.h>
#include <xpdf/XRef.h>
#include <xpdf/Gfx.h>
#include <xpdf/GfxState.h>
#include <xpdf/GfxFont.h>
#include <xpdf/ErrorCodes.h>
#include <xpdf/Page.h>
#include <xpdf/TextOutputDev.h>
#include <xpdf/SplashOutputDev.h>
#include <xpdf/BuiltinFontTables.h>
// Note that GlobalParams::initGlobalParams has to be called before
// we can use globalParams.
// If we want to use fonts correctly, globalParams->setupBaseFonts(NULL)
// has to be called
#include <xpdf/GlobalParams.h>
#include <xpdf/Catalog.h>

#include <assert.h>


//=====================================================================================
namespace xpdf {
//=====================================================================================

/** Reference comparator class.
 *
 * Implements functional operator to compare two referencies defined as
 * Ref structures.
 * <br>
 * This can be used as comparator predicate in ObjectStorage where value is
 * Object type.
 */
class RefComparator
{
public:
        /** Comparator operator.
         * @param v1 First value.
         * @param v2 Second value.
         *
         * @returns true iff v1.num and v1.gen are less than v2.num and v2.gen.
         *
         */ 
        bool operator()(const Ref & v1, const Ref & v2)const
        {
                if(v1.num == v2.num)
                        return v1.gen < v2.gen;
                else
                        return v1.num < v2.num;
        }
};

/**
 * Xpdf object deleter.
 */
struct object_deleter
{
	void operator() (::Object* o)
		{ assert (o); o->free(); ::gfree(o); }
};

/**
 * Free an object. We assume that all child objects (if any)
 * have been already freed.
 *
 * Copy & paste from Object.h
 * 
 * @param obj Object to be freed.
 */
void freeXpdfObject (::Object* obj);


//=====================================================================================
} // namespace xpdf
//=====================================================================================

#endif // _XPDF_H_
