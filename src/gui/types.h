/** @file
 File containing various type definitions, which are used across more than one file
*/

#ifndef __TYPES_H__
#define __TYPES_H__

#include <boost/shared_ptr.hpp>
#include <vector>
namespace pdfobjects {
 class PdfOperator;
}

namespace gui {

/** Vector with operators */
typedef std::vector<boost::shared_ptr<pdfobjects::PdfOperator> > OperatorVector;

}

#endif
