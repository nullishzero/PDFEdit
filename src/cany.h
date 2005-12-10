#ifndef __CANY_H__
#define __CANY_H__
class CAny {

/** ID bude poznat s ktorym PDF suborom je asociovana. */
  int id;

/*
    Vrati pole nazvov properties, ktore maju dany view mode (view_mode &
  name->property_flag == TRUE), ktore urcuje vlastnosti
  daneho objektu.(zadaneho identifikatorom)
  FLAG viewmode:
  0x00 VM_NORMAL - Normalne hodnoty
0x01 VM_READONLY - Hodnoty, ktore menit moze byt destruktivne
0x02 VM_HIDDEN - Hodnoty, ktore nema velky vyznam menit
0x04 VM_UNSET - Neinicializovane hodnoty
*/
  array [name]  get_property_all (CAny&, FLAG view_mode );

/** Vrati kopiu hodnotu vlastnosti zadanej menom daneho objektu. */
  value get_property (CAny&, string name );

/**  Vrati typ danej property urcenej menom. */
  PROP_TYPE get_property_type (CAny&, string name );

/**  Vrati hodnotu flagov danej property urcenej menom. (VM_*) */

  FLAG get_property_flag (CAny&, string name);

/**  Nastavi hodnotu vlastnosti zadanej menom daneho objektu. Tato funkcie
  bude pretazena pre vsetky PROP_TYPE.*/
  void set_property (CAny&, string name, string value );
  void set_property (CAny&, string name, int value );
//  ...
  void set_property (CAny&, string name, void* value );

/*  Zmaze dany objekt zo struktury pdf. Mala by to byt virtualna funkcia.*/
  virtual void delete ();

}#endif
