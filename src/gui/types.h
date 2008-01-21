/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
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
