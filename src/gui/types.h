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
/** @file
 File containing various type definitions, which are used across more than one file
 \brief Type definitions
*/

#ifndef __TYPES_H__
#define __TYPES_H__

#include <boost/shared_ptr.hpp>
#include <vector>
namespace pdfobjects {
 class PdfOperator;
 class CAnnotation;
 class CPage;
}

namespace gui {

/** Vector with operators */
typedef std::vector<boost::shared_ptr<pdfobjects::PdfOperator> > OperatorVector;

/** Vector with annotationss */
typedef std::vector<boost::shared_ptr<pdfobjects::CAnnotation> > AnnotationVector;

}

#endif
