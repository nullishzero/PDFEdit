#ifndef __CPAGE_H__
#define __CPAGE_H__
#include "cany.h"

class CPage : public CAny {
  //
  // Konstruktor
  //
  CPage ();

  //
  // Destruktor
  //
  virtual ~CPage ();

/**  Vlozi na stranku existujuci objekt a vytvori novy
  identifikator vlozeneho objektu, ktory vrati.*/
  CAny& insert_object (CAny,position);

/**  Vrati cislo stranky v nasom cislovani.*/
  uint get_number ();

/**  Vrati bounding box zadaneho objektu*/
  bbox get_object_bbox (CAny&);

/**  Vrati pole vsetkych objektov na stranke.*/
  array [CAny*] get_objects (family mask);

/**  Vrati plain text zadanej stranky (ziadne zazraky)*/
  string text_export ();

/**  Vrati pole bbox pre kazde pismeno od ord_pos (v plaintexte)
  s dlzkou len.*/
  array [rectangle] get_textpos (ord_pos,len);

/**  Odstrani zadany objekt zo stranky.*/
  void delete_object (CAny*);

/**  Vytvori novy graficky objekt so zadanou velkostou a poziciou.
  Graficky objekt je prazdny a pre vkladanie sa pouzivaju metody
  objektu graphic*/
  CGraphic& create_graphic (bbox, position);

/**  Importuje graficky objekt (obrazok) do stranky na
  poziciu.*/
  CGraphic& import_graphic (filename,position);

/**  Vytvori a vlozi na stranku novu anotaciu zadaneho typu. Typovo specificke
  nastavenie je mozne urobit pomocou (get/set)_property.*/
  CAnnotation create_annotation (type,rectangle);

/**  Vytvori novy textovy objekt, ktory obsahuje dany retazec a je umiestneny
  na pozicii position (pozn. vzhladom k comu sa urcuje pozicia)*/
  CText create_text (string,position);

}#endif
