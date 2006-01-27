#ifndef __COUTLINE_H__
#define __COUTLINE_H__

#include "cobject.h"

using namespace pdfobjects;

class COutline : public CDict {

  //
  // Konstruktor
  //
  COutline ();

  //
  // Destruktor
  //
  ~COutline ();

/**  Vrati pole potomkov zadanej outline.*/
//  array [COutline*] get_children ();

}


#endif // __COUTLINE_H__
