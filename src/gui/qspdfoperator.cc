/** @file
 QObject wrapper around PdfOperator (one operator found inside content stream)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspdfoperator.h"

namespace gui {

/** Construct wrapper with given PdfOperator */
QSPdfOperator::QSPdfOperator(boost::shared_ptr<PdfOperator> _cs,Base *_base) : QSCObject ("PdfOperator",_base) {
 obj=_cs;
}

/** destructor */
QSPdfOperator::~QSPdfOperator() {
}

/** Call PdfOperator::getStringRepresentation(ret); return ret */
QString QSPdfOperator::getText() {
 std::string text;
 obj->getStringRepresentation(text);
 return text;
}

/** Call PdfOperator::getOperatorName(ret); return ret */
QString QSPdfOperator::getName() {
 std::string text;
 obj->getOperatorName(text);
 return text;
}

/** get PdfOperator held inside this class. Not exposed to scripting */
boost::shared_ptr<PdfOperator> QSPdfOperator::get() {
 return obj;
}

} // namespace gui
