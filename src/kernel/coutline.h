/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __COUTLINE_H__
#define __COUTLINE_H__


// all basic includes
#include "static.h"


//=====================================================================================
namespace pdfobjects {
//=====================================================================================

/**
 * Checks whether the object is an ouline according to pdf specification.
 *
 * @param ip This iproperty is checked whether it is an outline.
 */
bool isOutline (boost::shared_ptr<IProperty> ip);

		
/**
 * Get text from an outline.
 *
 * @param ip Outline dictionary.
 */
std::string getOutlineText (boost::shared_ptr<IProperty> ip);
		

// not needed yet
//class COutline {};


//=====================================================================================
} /* namespace pdfobjects */
//=====================================================================================


#endif // __COUTLINE_H__
