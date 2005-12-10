#ifndef __CTEXT_H__
#define __CTEXT_H__
#include "cany.h"

class CText : public CAny
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
  void set_pos (CText&,position);

/**  Vrati pole bbox pre kazde pismeno od ord_pos (od zaciatku textoveho
  objektu) s danou dlzkou (ako vyriesime, pocet znakov v objekte)*/
  array [rectangle] get_rects (CText&, ord_pos, len);

}


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
