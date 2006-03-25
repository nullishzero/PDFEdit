#ifndef __TEST_H__
#define __TEST_H__

#include <cobject.h>
#include <cpdf.h>

using namespace std;
using namespace pdfobjects;

namespace test {
 CPdf* testPDF(void);
 void makeArTest1(CArray & arTest1);
 void makeDcTest1(CDict & dcTest1);
 void makeArTest2(CArray & arTest2,CArray & arTest1,CDict & dcTest1);
 void makeDcTest2(CDict & dcTest2,CArray & arTest1,CDict & dcTest1);
}
#endif
