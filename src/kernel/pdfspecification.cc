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

	// Null
	const std::string Specification::CNULL_NULL ("null");
	// Bool		
	const std::string Specification::CBOOL_TRUE  ("true");
	const std::string Specification::CBOOL_FALSE ("false");
	// Name
	const std::string Specification::CNAME_PREFIX ("/");
	// String
	const std::string Specification::CSTRING_PREFIX ("(");
	const std::string Specification::CSTRING_SUFFIX (")");
	// Ref
	const std::string Specification::CREF_MIDDLE (" ");
	const std::string Specification::CREF_SUFFIX (" R");

	// Array
	const std::string Specification::CARRAY_PREFIX ("[");
	const std::string Specification::CARRAY_MIDDLE (" ");
	const std::string Specification::CARRAY_SUFFIX (" ]");
	// Dict
	const std::string Specification::CDICT_PREFIX ("<<");
	const std::string Specification::CDICT_MIDDLE ("\n/");
	const std::string Specification::CDICT_BETWEEN_NAMES (" ");
	const std::string Specification::CDICT_SUFFIX ("\n>>");
	// Stream
	const std::string Specification::CSTREAM_HEADER ("\nstream\n");
	const std::string Specification::CSTREAM_FOOTER ("\nendstream");
	// Indirect
	const std::string Specification::INDIRECT_HEADER ("obj ");
	const std::string Specification::INDIRECT_FOOTER ("\nendobj");

	// Object Error
	const std::string Specification::OBJERROR ("\n");


	// 
	// CPage
	//
	const std::string Specification::CPage::PARENT   ("Parent");
	const std::string Specification::CPage::CONTENTS ("Contents");
	const std::string Specification::CPage::RESOURCES("Resources");
	const std::string Specification::CPage::MEDIABOX ("MediaBox");
	const std::string Specification::CPage::CROPBOX  ("CropBox");
	const std::string Specification::CPage::ROTATE   ("Rotate");
	const std::string Specification::CPage::ANNOTS   ("Annots");

// =====================================================================================
} // namespace pdfobjects
// =====================================================================================
