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

#define CHECK_FOR_PERMISSIONS(pdf, shouldFail, method, params...)	\
	do {							\
	try {							\
		OUTPUT << " " << #method;			\
		pdf->method(params);				\
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
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getIndirectProperty, ref);

		shared_ptr<CInt> intProp(CIntFactory::getInstance(1));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, addIndirectProperty, intProp);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, changeIndirectProperty, intProp);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, save);
		FILE * file = fopen("testfile", "w");
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, clone, file);
		fclose(file);

		shared_ptr<CDict> pageDict(CDictFactory::getInstance());
		shared_ptr<CPage> page;
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getFirstPage);
		if(!haveCredentials)
			page = shared_ptr<CPage>(new CPage(pageDict));
		else
			page = pdf->getFirstPage();
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPagePosition, page);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPageCount);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, hasNextPage, page);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, hasPrevPage, page);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getLastPage);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPage, 1);
		if(haveCredentials)
		{
			// for haveCredentials case we have to be carefull to
			// prevent from PageNotFound exception
			if(pdf->getPageCount() != 1)
			{
				CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getNextPage, page);
				page = pdf->getLastPage();
				CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPrevPage, page);

			}
		}else
		{
			CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getPrevPage, page);
			CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getNextPage, page);
		}
		page = shared_ptr<CPage>(new CPage(pageDict));
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, insertPage, page, 1);
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, removePage, 1);
		/* FIXME
		vector<COutline> container;
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, getOutlines, container);
		*/
		CHECK_FOR_PERMISSIONS(pdf, shouldThrow, canChange);
		try
		{
			CHECK_FOR_PERMISSIONS(pdf, shouldThrow, changeRevision, 1);
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
		CHECK_FOR_PERMISSIONS(pdf, false, getId);
		CHECK_FOR_PERMISSIONS(pdf, false, getCXref);
		CHECK_FOR_PERMISSIONS(pdf, false, getModeController);
		CHECK_FOR_PERMISSIONS(pdf, false, setModeController, NULL);
		CHECK_FOR_PERMISSIONS(pdf, false, isChanged);
		CHECK_FOR_PERMISSIONS(pdf, false, getTrailer);
		CHECK_FOR_PERMISSIONS(pdf, false, getMode);
		CHECK_FOR_PERMISSIONS(pdf, false, isLinearized);
		CHECK_FOR_PERMISSIONS(pdf, false, getActualRevision);
		CHECK_FOR_PERMISSIONS(pdf, false, getRevisionSize, 0);
		CHECK_FOR_PERMISSIONS(pdf, false, getRevisionsCount);
		CHECK_FOR_PERMISSIONS(pdf, false, getPdfWriter);
		CHECK_FOR_PERMISSIONS(pdf, false, needsCredentials);
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
