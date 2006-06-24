/** Functions used mainly for testing */

/** TEST: buffer integrity */
function buftest(x,at,st) {
 if (at) {
  a1=x.getBuffer();
  x.setBuffer(a1);
  a2=x.getBuffer();
  if (a1.length!=a2.length) {
   print ("Different length of arrays: "+a1.length+" vs "+a2.length);
  } else if (a1!=a2) {
   print("Different arrays: a1="+a1.length);
   print(a1)
   print("Different arrays: a2="+a2.length);
   print(a2);
  }
  print("Done array test");
 }
 if (st) {
  s1=x.getBufferString();
  x.setBuffer(s1);
  s2=x.getBufferString();
  if (s1.length!=s2.length) {
   print ("Different length of strings: "+s1.length+" vs "+s2.length);
  } else if (s1!=s2) {
   print("Different strings: s1="+s1.length);
   print(s1)
   print("Different strings: s2="+s2.length);
   print(s2);
  }
  print("Done string test");
 }
}

/** Turn on/off testing/debugging. */
function setTests(x) {
 settings.write("tests",x);
 tests=x;
}

/** Add stream teswting items to the menu */
function test_stream_items(menu) {
 menu.addSeparator();
 menu.addItemDef("item Decoded representation,print(firstSelected().getDecoded())");
 menu.addItemDef("item Stream integrity test\\, array,buftest(firstSelected()\\,1\\,0)");
 menu.addItemDef("item Stream integrity test\\, string,buftest(firstSelected()\\,0\\,1)");
 menu.addItemDef("item Stream integrity test\\, both,buftest(firstSelected()\\,1\\,1)");
}

//Include testing/debugging items in menus?
var tests=settings.readBool("tests");

