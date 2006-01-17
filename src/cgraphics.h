#ifndef __CGRAPHICS_H__
#define __CGRAPHICS_H__

#include "cobject.h"

using namespace pdfobjects;

class CGraphic : public CDict {
  //
  // Konstruktor
  //
  CGraphic ();

  //
  // Destruktor
  //
  ~CGraphic ();

/**  Zmeni poziciu grafickeho objektu na stranke.*/
//  void set_pos (CGraphic&, position);

/**  Zmeni bounding box grafickeho objektu podla zvoleneho
  modu (zachovavat velkost objektov vo vnutri, alebo
  menit ich velkost podla bboxu).*/
//  void set_bbox (CGraphic&, rect, mode);

/**  Do grafickeho objektu podla zadaneho stylu nakresli ciaru
  z startpos do endpos. Style urcuje styl vykreslovania (PDFref. p186)*/
//  void add_line (CGraphic&,startpos,endpos,style,);

//  void add_ellipse (CGraphic&, ...);
//  void add_polygon (CGraphic&, ...);

}


#endif // __CGRAPHICS_H__
