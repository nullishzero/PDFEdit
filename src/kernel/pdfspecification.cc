/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80
#include "kernel/static.h"

using namespace std;

// =====================================================================================
namespace pdfobjects {
// =====================================================================================

	// Object Null std::string Specification::representation. */
	const std::string Specification::CNULL_NULL = "null";
		
	// Object Bool false repersentation. */
	const std::string Specification::CBOOL_TRUE  = "true";
	// Object Bool true representation. */
	const std::string Specification::CBOOL_FALSE = "false";

	// Object Name std::string Specification::representation. */
	const std::string Specification::CNAME_PREFIX = "/";

	// Object String representation prefix string. */
	const std::string Specification::CSTRING_PREFIX = "(";
	// Object String representation suffix string. */
	const std::string Specification::CSTRING_SUFFIX = ")";

	// Object Ref representation middle string. */
	const std::string Specification::CREF_MIDDLE = " ";
	// Object Ref representation std::string Specification::suffix. */
	const std::string Specification::CREF_SUFFIX = " R";

	// CObjectComplex
	// Object Array representation prefix string. */
	const std::string Specification::CARRAY_PREFIX = "[";
	// Object Array representation middle string. */
	const std::string Specification::CARRAY_MIDDLE = " ";
	// Object Array representation suffix string. */
	const std::string Specification::CARRAY_SUFFIX = " ]";

	// Object Dictionary representation specifics. */
	const std::string Specification::CDICT_PREFIX = "<<";
	// Object Dictionary representation specifics. */
	const std::string Specification::CDICT_MIDDLE = "\n/";
	// Object Dictionary representation specifics. */
	const std::string Specification::CDICT_BETWEEN_NAMES = " ";
	// Object Dictionary representation specifics. */
	const std::string Specification::CDICT_SUFFIX = "\n>>";

	// Object Stream std::string Specification::representation specifics. */
	const std::string Specification::CSTREAM_HEADER = "\nstream\n";
	// Object Stream std::string Specification::representation specifics. */
	const std::string Specification::CSTREAM_FOOTER = "\nendstream";

	// Xpdf error object std::string Specification::representation specifics. */
	const std::string Specification::OBJERROR = "\n";

	// Indirect Object heaser. */
	const std::string Specification::INDIRECT_HEADER = "obj ";
	// Indirect Object footer. */
	const std::string Specification::INDIRECT_FOOTER = "\nendobj";


// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
