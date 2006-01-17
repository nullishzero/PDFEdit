#ifndef __CPDF_H__
#define __CPDF_H__

#include "cobject.h"

using namespace pdfobjects;

class CPdf : public CDict {
  //
  // Konstruktor
  //
//  CPdf (filename,mode);

 /** Vytvori vnutornu reprezentaciu pdf (pre pristup k objektom sa bude
  uplatnovat mode - READ_ONLY, READ_WRITE).*/
//  int open (filename,mode);

  //
  // Destruktor
  //
//  virtual ~CPdf();

/**  !open.*/
//  void close ();

/**  Do suboru ulozi aktualnu verziu (reviziu) dokumentu. Neoptimalizuje
  vyskyt nepotrebnych objektov.*/
//  void save (filename);

/**  Vrati identifikator strany zadanej poradim od uplne prvej
  stranky.*/
//  CPage& get_page (int cislo_stranky);

/**  Vrati referenciu na stranku zadanej menom (I,IV,a...).*/
//  CPage& get_page (string meno_stranky);

/**  Vrati uplne prvu stranku.*/
//  CPage& get_page_first ();

/**  Vrati uplne poslednu stranku.*/
//  CPage& get_page_last ();

/**  Vrati nasledujuci stranku.*/
//  CPage& get_page_next (CPage&);

/**  Vrati predchadzajucu stranku. Zo zadaneho identifikatora, sa bude
  dat zistit, ku ktoremu pdf sa dana stranka viaze.*/
//  CPage& get_page_prev (CPage&)

/**  Vlozi stranku do zadaneho pdf na zadane miesto, vrati novy
  identifikator vlozenej stranky.*/
//  CPage& insert_page (CPage&, int page_position)

/**  Odstrani stranku z pdf. (v novej verzii nebude zobrazena)*/
//  void delete_page (CPage*);

/**  Vrati pocet stranok.*/
//  int page_count ();

/** Vrati vsetky outline najvyssej urovne. Ich potomkov vieme
  ziskat s pomocou get_outline_children.*/
//  array [COutline*] get_outlines ();

/**  Vytvori novu prazdnu stranku, asociovanu so zadanym pdf. Umiestni ju
  na ord_pos (od zaciatku dokumentu). Name urcuje "cislo" stranky.*/
//  CPage& create_page (int ord_pos, string name);

/**  Vytvori novy outline a vlozi ho do hierarchie. Toplevel outline je mozne
  riesit ako vhodnym zvolenim niektoreho parametra (napr. parent == NULL )
  Name urcuje meno outline, target miesto, kam sa dostaneme po klinkuti na nu.
  Target bude struktura obsahujici: stranku (CPage &),
   pozici na strance (souradnice) a zoom.
           (+ asi par preddefinovanych konstant zoomu na "fit to width" apod..)*/
//  COutline& create_outline (COutline& parent, int position, string name,target);

/**  Z pdf odstrani objekty, na ktore nie su ziadne referencie.
  Mode urcuje sposob priebehu operacie (zakomprimuje ret., ...).*/
//  void optimize (mode);

/**  Prehlasi danu reviziu za poslednu (teda nastavi mode na READ_WRITE),
  zahodi vsetky zmeny v poslednej revizii.*/
//  void set_lastver ();

/**  Vrati meno predchadzajucej revizie a nastavi ju ako aktualnu.
  Nastavi editovaci mod na READ_ONLY*/
//  string set_prevver ();

/**  Vrati meno nasledujucej revizie a nastavi ju ako aktualnu. Pre poslednu
  reviziu nastavi mode na READ_WRITE.*/
//  string set_nextver ();

/**  Vrati meno aktualnej revizie.*/
//  string get_namever ();

/**  Vrati mode danej revizie.*/
//  mode get_modever ();

}


#endif // __CPDF_H__
