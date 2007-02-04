/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __CTEXT_H__
#define __CTEXT_H__

#include "cobject.h"

using namespace pdfobjects;

class CText // : public CString /*...*/
{

  //
  // Konstruktor
  //
  CText ();

  //
  // Destruktor
  //
  ~CText ();

/**  Zmeni poziciu textoveho objektu.*/
//  void set_pos (CText&,position);

/**  Vrati pole bbox pre kazde pismeno od ord_pos (od zaciatku textoveho
  objektu) s danou dlzkou (ako vyriesime, pocet znakov v objekte)*/
//  array [rectangle] get_rects (CText&, ord_pos, len);

}


#endif  // __CTEXT_H__
