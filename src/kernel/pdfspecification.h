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

#ifndef _PDFSPECIFICATION_H_
#define _PDFSPECIFICATION_H_

#include <string>

//====================================================================================
namespace pdfobjects {
//====================================================================================

	/**
	 * Stuff tightly connected with pdf specification.
	 */
	struct Specification {

		//
		// String constants used when converting objects to strings
		//
		// CObjectSimple
		/** Object Null string representation. */
		static const std::string CNULL_NULL;  // "null";
			
		/** Object Bool false repersentation. */
		static const std::string CBOOL_TRUE ;  // "true";
		/** Object Bool true representation. */
		static const std::string CBOOL_FALSE;  // "false";

		/** Object Name string representation. */
		static const std::string CNAME_PREFIX;  // "/";

		/** Object String representation prefix string. */
		static const std::string CSTRING_PREFIX;  // "(";
		/** Object String representation suffix string. */
		static const std::string CSTRING_SUFFIX;  // ")";

		/** Object Ref representation middle string. */
		static const std::string CREF_MIDDLE;  // " ";
		/** Object Ref representation string suffix. */
		static const std::string CREF_SUFFIX;  // " R";

		// CObjectComplex
		/** Object Array representation prefix string. */
		static const std::string CARRAY_PREFIX;  // "[";
		/** Object Array representation middle string. */
		static const std::string CARRAY_MIDDLE;  // " ";
		/** Object Array representation suffix string. */
		static const std::string CARRAY_SUFFIX;  // " ]";

		/** Object Dictionary representation specifics. */
		static const std::string CDICT_PREFIX;  // "<<";
		/** Object Dictionary representation specifics. */
		static const std::string CDICT_MIDDLE;  // "\n/";
		/** Object Dictionary representation specifics. */
		static const std::string CDICT_BETWEEN_NAMES;  // " ";
		/** Object Dictionary representation specifics. */
		static const std::string CDICT_SUFFIX;  // "\n>>";

		/** Object Stream string representation specifics. */
		static const std::string CSTREAM_HEADER;  // "\nstream\n";
		/** Object Stream string representation specifics. */
		static const std::string CSTREAM_FOOTER;  // "\nendstream";

		/** Xpdf error object string representation specifics. */
		static const std::string OBJERROR;  // "\n";

		/** Indirect Object heaser. */
		static const std::string INDIRECT_HEADER;  // "obj ";
		/** Indirect Object footer. */
		static const std::string INDIRECT_FOOTER;  // "\nendobj";

	}; // struct Specification


//====================================================================================
} // namespace pdfobjects
//====================================================================================


#endif // _PDFSPECIFICATION_H_

