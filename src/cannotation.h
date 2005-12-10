#ifndef __CANNOTATION_H__
#define __CANNOTATION_H__
#include "cany.h"

class CAnnotation : public CAny {
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
  void set_STYLE (CAnnotation&, STYLE_spec_parameters);

/**  Upravi polohu anotacie na strane*/
  void set_pos (CAnnotation&, position);

}
#endif
