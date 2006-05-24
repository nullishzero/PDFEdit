/** @file
 QObject wrapper around PdfOperator (one operator found inside content stream)<br>
 Export some functions to scripting.<br>
 See doc/user/scripting.xml or kernel documentation for more informations about these functions
 @author Martin Petricek
*/

#include "qspdfoperator.h"
#include "qsimporter.h"
#include "qsiproperty.h"
#include <ccontentstream.h>

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

/**
 Return child operator with given number from this operator.
 Get the list of operators from this PDF operator and store for later use if necessary
 @param childNumber number of child to get
 @return Child operator with given number, or NULL if number is outside range
*/
QSPdfOperator* QSPdfOperator::child(int childNumber) {
 if (numChilds<0) loadChilds();
 if (childNumber<0) return NULL;		//child number outside range
 if (childNumber>=numChilds) return NULL;	//child number outside range
 return new QSPdfOperator(childs[childNumber],base);
}

/**
 Return number of child operators under this operator
 Get the list of operators from this PDF operator and store for later use if necessary
 @return number of child operators under this operator
*/
int QSPdfOperator::childCount() {
 if (numChilds<0) loadChilds();
 return numChilds;
}

/**
 Return parameter with given number from this operator. Parameter is IProperty
 Get the list of parameters from this PDF operator and store for later use if necessary
 @param paramNumber number of parameter to get
 @return Parameter with given number, or NULL if number is outside range
*/
QSCObject* QSPdfOperator::param(int paramNumber) {
 if (numParams<0) loadParams();
 if (paramNumber<0) return NULL;		//Param number outside range
 if (paramNumber>=numParams) return NULL;	//Param number outside range
 return QSImporter::createQSObject(params[paramNumber],base);
}

/**
 Return number of parameters for this operator
 Get the list of parameters from this PDF operator and store for later use if necessary
 @return number of parameters fro this operator
*/
int QSPdfOperator::paramCount() {
 if (numParams<0) loadParams();
 return numParams;
}

/** Call PdfOperator::getChildren(), store result */
void QSPdfOperator::loadParams() {
 PdfOperator::PdfOperators opList;
 obj->getChildren(opList);
 //We need vector. We get list. We must copy.
 childs.clear();
 std::copy(opList.begin(),opList.end(),std::back_inserter(childs));
 //Store number of childs
 numChilds=childs.size(); 
}

/** Call PdfOperator::getChildren(), store result */
void QSPdfOperator::loadChilds() {
 PdfOperator::Operands paramList;
 obj->getParameters(paramList);
 //We need vector. We get deque. We must copy.
 params.clear();
 std::copy(paramList.begin(),paramList.end(),std::back_inserter(params));
 //Store number of childs
 numParams=params.size(); 
}


/** Remove itself from the stream. After this operation, the operator cannot be used - it will delete itself */
void QSPdfOperator::remove() {
 obj->getContentStream()->deleteOperator(obj);
 treeNeedReload();
 //TODO: Need to figure out two things:
 //TODO: 1. how will QSA survive deletion of this object
 //TODO: 2. what if reference to this operator is elsewhere
 delete this;
}

} // namespace gui
