#ifndef __COUTLINE_H__
#define __COUTLINE_H__
#include "cany.h"

class COutline : public CAny {

  //
  // Konstruktor
  //
  COutline ();

  //
  // Destruktor
  //
  ~COutline ();

/**  Vrati pole potomkov zadanej outline.*/
  array [COutline*] get_children ();

}
#endif
