#include "testcpdf.h"

void cpdf_tests(pdfobjects::CPdf * pdf)
{
}

pdfobjects::CPdf * getTestCPdf(const char* filename)
{
	return pdfobjects::CPdf::getInstance(filename, CPdf::Advanced);
}
