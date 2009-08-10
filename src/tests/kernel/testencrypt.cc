/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#include "kernel/static.h"
#include "tests/kernel/testmain.h"
#include "tests/kernel/testcpdf.h"
#include "kernel/factories.h"
#include "kernel/cobjecthelpers.h"
#include "kernel/cpdf.h"
#include "kernel/pdfwriter.h"
#include "kernel/delinearizator.h"

using namespace pdfobjects;
using namespace utils;
using namespace observer;
using namespace boost;


bool parsePasswdLine(string & inputLine, string & file, string & passwd)
{
	size_t idx = inputLine.find_first_of(':');
	if(idx == string::npos)
		return false;
	file = inputLine.substr(0, idx);
	passwd = inputLine.substr(idx+1);
	return true;
}

class TestEncryptCPdf: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(TestEncryptCPdf);
		CPPUNIT_TEST(Test);
	CPPUNIT_TEST_SUITE_END();


/* Helper macro for code reduction. This will call given method
 * on the given pdf and says whether this function should fail
 * or not.
 * CPPUNIT_FAIL is triggered if the method doesn't fail and it
 * should or otherwise. If NotImplementedException or ReadOnlyDocumentException
 * is thrown then it checjs linearization resp. encryption
 * or current mode.
 * Use NO_PARAM if the method doesn't get any parameters or one of
 * the PARAM_# with appropriate numbers of parameters.
 */
#define CHECK_FOR_PERMISSIONS(pdf, shouldFail, method, params)	\
	do {							\
	try {							\
		OUTPUT << " " << #method;			\
		pdf->method params ;				\
		if(shouldFail)					\
			CPPUNIT_FAIL(#method" should fail");	\
	}catch(PermissionException e)				\
	{							\
		if(!shouldFail)					\
			CPPUNIT_FAIL(#method" shouldn't fail");	\
	}catch(NotImplementedException e)			\
	{							\
		OUTPUT << "(is not implemented)";	\
		CPPUNIT_ASSERT(pdf->isLinearized()||isEncrypted(pdf));\
	}							\
	catch(ReadOnlyDocumentException e)			\
	{							\
		OUTPUT << "(failed because of read-only document setting)";\
		CPPUNIT_ASSERT(pdf->getMode()==CPdf::ReadOnly);	\
	}							\
	}while(0)

/* Hack for variable number of parameters workaround which is not
 * defined in ANSI C++ (gcc extension would use params... in the
 * above macro and this would be useless).
 */
#define NO_PARAM ()
#define PARAM_1(p) (p)
#define PARAM_2(p1, p2) (p1, p2)


	shared_ptr<CPdf> checkInstancing(const string& fileName)
	{
		shared_ptr<CPdf> pdf;

		try
		{
			pdf = getTestCPdf(fileName.c_str());
			if(!utils::isEncrypted(pdf))
			{
				OUTPUT << "Document \"" << fileName << "\" is not encrypted. Skipping...";
				return shared_ptr<CPdf>();
			}

		}catch(PermissionException e)
		{
			CPPUNIT_FAIL("Document open failed because of permissions.");
		}catch(PdfOpenException e)
		{
			OUTPUT << "Could not open \"" << fileName << "\" file reason=\"" << e.what() << "\"" << endl;
		}
		return pdf;
	}

	// TODO need credentials?
	// 	getDictionary
	void checkNeedCredentialMethods(shared_ptr<CPdf> pdf, bool haveCredentials)
	{
		OUTPUT << "Checking methods which do require credentials. Available: "
			<<haveCredentials << endl << "\t";
		IndiRef ref(1,0);
		bool shouldThrow = !haveCredentials;
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getIndirectProperty, PARAM_1(ref));

		shared_ptr<CInt> intProp(CIntFactory::getInstance(1));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, addIndirectProperty, PARAM_1(intProp));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, changeIndirectProperty, PARAM_1(intProp));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, save, NO_PARAM);
		FILE * file = fopen("testfile", "wb");
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, clone, PARAM_1(file));
		fclose(file);

		shared_ptr<CDict> pageDict(CDictFactory::getInstance());
		shared_ptr<CPage> page;
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getFirstPage, NO_PARAM);
		if(!haveCredentials)
			page = shared_ptr<CPage>(new CPage(pageDict));
		else
			page = pdf->getFirstPage();
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPagePosition, PARAM_1(page));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPageCount, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, hasNextPage, PARAM_1(page));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, hasPrevPage, PARAM_1(page));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getLastPage, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPage, PARAM_1(1));
		if(haveCredentials)
		{
			// for haveCredentials case we have to be carefull to
			// prevent from PageNotFound exception
			if(pdf->getPageCount() != 1)
			{
				CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getNextPage, PARAM_1(page));
				page = pdf->getLastPage();
				CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPrevPage, PARAM_1(page));

			}
		}else
		{
			CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPrevPage, PARAM_1(page));
			CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getNextPage, PARAM_1(page));
		}
		page = shared_ptr<CPage>(new CPage(pageDict));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, insertPage, PARAM_2(page, 1));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, removePage, PARAM_1(1));
		/* FIXME
		vector<COutline> container;
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getOutlines, container);
		*/
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, canChange, NO_PARAM);
		try
		{
			CHECK_FOR_PERMISSIONS(pdf, shouldThrow, changeRevision, PARAM_1(1));
		}catch(OutOfRange e)
		{
			// OK - we haven't check for number
		}catch(NotImplementedException e)
		{
			// OK - it can be linearized
		}
		OUTPUT << endl;
	}

	void checkDontNeedCredentialMethods(shared_ptr<CPdf> pdf)
	{
		OUTPUT << "Checking methods which don't require credentials\n\t";
		CHECK_FOR_PERMISSIONS(pdf, false, getId, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, getCXref, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, getModeController, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, setModeController, PARAM_1(NULL));
		CHECK_FOR_PERMISSIONS(pdf, false, isChanged, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, getTrailer, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, getMode, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, isLinearized, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, getActualRevision, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, getRevisionSize, PARAM_1(0));
		CHECK_FOR_PERMISSIONS(pdf, false, getRevisionsCount, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, getPdfWriter, NO_PARAM);
		CHECK_FOR_PERMISSIONS(pdf, false, needsCredentials, NO_PARAM);
		OUTPUT << endl;
	}

	void noCredentialsTC(shared_ptr<CPdf> pdf)
	{
		OUTPUT << "TC01: no credentials available\n";
		checkNeedCredentialMethods(pdf, false);
		checkDontNeedCredentialMethods(pdf);
	}

	void credentialsTC(shared_ptr<CPdf> pdf, const string & passwd)
	{
		OUTPUT << "TC02: credentials available\n";
		OUTPUT << "\tIncorrect passwd\n";
		string badPasswd = passwd + "bad";
		try
		{
			pdf->setCredentials(badPasswd.c_str(), badPasswd.c_str());
			CPPUNIT_FAIL("setCredentials should have failed");
		}catch(PermissionException e)
		{}
		checkNeedCredentialMethods(pdf, false);

		OUTPUT << "\tCorrect passwd\n";
		try
		{
			pdf->setCredentials(passwd.c_str(), passwd.c_str());
		}catch(PermissionException e)
		{
			CPPUNIT_FAIL("setCredentials should success with correct passwd");
		}
		checkNeedCredentialMethods(pdf, true);
	}
public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

	void Test()
	{
		const char *passwd_file = TestParams::instance().passwd_rc.c_str();
		ifstream str;
		str.open(passwd_file);
		if(!str)
		{
			OUTPUT << "Unable to open passwd file (\""<<passwd_file<<"\") for encrypted documents";
			return;
		}
		printf("Checking encrypted documents from:\"%s\"\n", passwd_file);
		string line;
		while (getline(str, line))
		{
			string fileName, passwd;
			// skip lines with not valid file:passwd format
			if(!parsePasswdLine(line, fileName, passwd))
				continue;

			printf("\nTests for file:%s\n", fileName.c_str());
			shared_ptr<CPdf> pdf = checkInstancing(fileName);
			if(!pdf)
				continue;
			// only encrypted documents are cheched
			noCredentialsTC(pdf);
			credentialsTC(pdf, passwd);
		}
		str.close();
	}
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestEncryptCPdf);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestEncryptCPdf, "TEST_ENCRYPT_CPDF");
