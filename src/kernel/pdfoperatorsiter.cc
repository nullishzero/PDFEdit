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

// static
#include "kernel/static.h"
//
#include "kernel/pdfoperatorsiter.h"

//==========================================================
namespace pdfobjects {
//==========================================================

//==========================================================
// Iterators
//==========================================================

/** Text iterator accepted operators. */
template<>
const std::string TextOperatorIterator::accepted_opers[TextOperatorIterator::namecount] = {"Tj", "TJ", "'", "\""};
/** Inline image iterators. */
template<>
const std::string InlineImageOperatorIterator::accepted_opers[InlineImageOperatorIterator::namecount] = {"BI"};
/** Changeable operator are all operators except these. */
template<>
const std::string ChangeableOperatorIterator::rejected_opers[ChangeableOperatorIterator::namecount] = 
{
	"q", "Q", "cm", "w", "J", "j", "M", "d", "ri", "i", "gs", "s", "S", "f", "F", "f*", "B", "B*", "b", "b*"
	"n", "W", "W*", "BX", "EX", "rg", "CS", "cs", "SC", "SCN", "sc", "scn", "G", "g", "RG", "rg", "K", "k"
};
/** Non stroking iterator accepted operators. */
template<>
const std::string NonStrokingOperatorIterator::accepted_opers[NonStrokingOperatorIterator::namecount] = 
{
	"Tj", "TJ", "'", "\""
};
/** Stroking iterator accepted operators. */
template<>
const std::string StrokingOperatorIterator::accepted_opers[StrokingOperatorIterator::namecount] = 
{
	"", "", "", ""
};

/** Operators accepted by font iterator. */
template<>
const std::string FontOperatorIterator::accepted_opers[FontOperatorIterator::namecount] = 
{
	"Tf", "", "", ""
};

/** Operators accepted by font iterator. */
template<>
const std::string GraphicalOperatorIterator::accepted_opers[GraphicalOperatorIterator::namecount] = 
{
	"f", "F", "f*", "B", "S", "s", "b", "B", "B*", "n", "m", "l", "c", "v", "y", "h", "re", "w", "J", "j",
	"M", "d", "ri", "gs", "BI"
};


//==========================================================
} // namespace pdfobjects
//==========================================================
