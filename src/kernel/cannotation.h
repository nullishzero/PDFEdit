#ifndef __CANNOTATION_H__
#define __CANNOTATION_H__


#include "cobject.h"

using namespace pdfobjects;


class CAnnotation : public CDict {
  //
  // Konstruktor
  //
  ~CAnnotation ();

  //
  // Destruktor
  //
  ~CAnnotation ();

/**  Funkcie pre najpouzivanejsich parametrov anotacie podla jej typu.
  (PDFref p580)*/
//  void set_STYLE (CAnnotation&, STYLE_spec_parameters);

/**  Upravi polohu anotacie na strane*/
//  void set_pos (CAnnotation&, position);

}



#endif // __CANNOTATION_H__

