/** @file
 Test class with methods to generate various test data 
*/
#include "test.h"

namespace test {
/** Create and return some testing CPdf */
CPdf* testPDF(void) {
 CPdf *pdf=CPdf::getInstance(NULL,CPdf::ReadWrite);
 boost::shared_ptr<CDict> pd=pdf->getDictionary();
 CDict dict;
 CString val1("val1");
 CString val2("val2");
 CString item1("fero");
 dict.addProperty("jano",item1);
 dict.addProperty("item1",val1);
 dict.addProperty("item2",val2);
 CString str3("val5");
 dict.addProperty("item5",str3);
 CArray arTest1;
 CArray arTest2;
 CDict dcTest1;
 CDict dcTest2;
 pd->addProperty("somedict",dict);
 pd->addProperty("atest1",arTest1);
 pd->addProperty("atest2",arTest2);
 pd->addProperty("dtest1",dcTest1);
 pd->addProperty("dtest2",dcTest2);
 makeArTest1(arTest1);
 makeDcTest1(dcTest1);
 makeArTest2(arTest2,arTest1,dcTest1);
 makeDcTest2(dcTest2,arTest1,dcTest1);
 return pdf;
}

/** Fill arTest1 with some testing junk */
void makeArTest1(CArray & arTest1) {
 CString it1("item1");
 CString it2("item2");
 CString it3("item3");
 arTest1.addProperty(it1);
 arTest1.addProperty(it2);
 arTest1.addProperty(it3);
 CInt i1(1);
 CInt i2(2);
 CInt i3(3);
 CInt i4(4);
 arTest1.addProperty(i1);
 arTest1.addProperty(i2);
 arTest1.addProperty(i3);
 arTest1.addProperty(i4);
 IndiRef ref = { 3,4 };
 CRef rf(ref);
 arTest1.addProperty(rf);
 CBool bl(false);
 arTest1.addProperty(bl);
}

/** Fill dcTest1 with some testing junk */
void makeDcTest1(CDict & dcTest1) {
 CString it1("item1");
 CString it2("item2");
 CString it3("item3");
 dcTest1.addProperty("item1",it1);
 dcTest1.addProperty("item2",it2);
 dcTest1.addProperty("item3",it3);
 CInt i1(1);
 CInt i2(2);
 CInt i3(3);
 CInt i4(4);
 dcTest1.addProperty("item4",i1);
 dcTest1.addProperty("item5",i2);
 dcTest1.addProperty("item6",i3);
 dcTest1.addProperty("item7",i4);
 IndiRef ref = { 3,4 };
 CRef rf(ref);
 dcTest1.addProperty("item8",rf);
 CBool bl(false);
 dcTest1.addProperty("item9",bl);
}

/** Fill arTest2 with some testing junk */
void makeArTest2(CArray & arTest2,CArray & arTest1,CDict & dcTest1) {
 arTest2.addProperty(arTest1);
 arTest2.addProperty(arTest1);
 arTest2.addProperty(arTest1);
 arTest2.addProperty(dcTest1);
 arTest2.addProperty(dcTest1);
 arTest2.addProperty(dcTest1);
}

/** Fill dcTest2 with some testing junk */
void makeDcTest2(CDict & dcTest2,CArray & arTest1,CDict & dcTest1) {
 dcTest2.addProperty("1",arTest1);
 dcTest2.addProperty("2",arTest1);
 dcTest2.addProperty("3",arTest1);
 dcTest2.addProperty("4",dcTest1);
 dcTest2.addProperty("5",dcTest1);
 dcTest2.addProperty("6",dcTest1);
}

}
