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
/** @file
 Base - class that host scripts and contain static script functions
 This class is also responsible for garbage collection of scripting objects
 @author Martin Petricek
*/

#include "passworddialog.h"
#include "pdfutil.h"
#include "base.h"
#include "qtcompat.h"
#include "qsannotation.h"
#include "qsarray.h"
#include "qscobject.h"
#include "qsdict.h"
#include "qsimporter.h"
#include "qsiproperty.h"
#include "qsipropertyarray.h"
#include "qspage.h"
#include "qspdf.h"
#include "qspdfoperator.h"
#include "qspdfoperatorstack.h"
#include "settings.h"
#include "treeitemabstract.h"
#include "util.h"
#include "version.h"
#include <string.h>
#include <stdlib.h>
#include <kernel/cpdf.h>
#include <kernel/cannotation.h>
#include <kernel/delinearizator.h>
#include <kernel/flattener.h>
#include <kernel/factories.h>
#include <kernel/pdfwriter.h>
#include <qdir.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qsinterpreter.h>
#include <utils/debug.h>
#include <kernel/textoutput.h>
#include <kernel/cpage.h>

namespace gui {

using namespace textoutput;
using namespace std;
using namespace util;
using namespace pdfobjects;

/** (for pdftoxml)*/
typedef vector<size_t> PageNums;

/**
 Create new Base class
*/
Base::Base() : BaseCore() {
 //Autodelete is on
 treeWrap.setAutoDelete(true);
 treeReloadFlag=false;
}

/** destructor */
Base::~Base() {
 //Empty
}

/**
 Return current date and time
 @param format Date/time format
*/
QString Base::time(const QString &format/*=QString::null*/) {
 QDateTime d=QDateTime::currentDateTime();
 if (format.isNull()) return d.toString(Qt::LocalDate);
 if (format=="") return d.toString(Qt::LocalDate);
 if (format=="ISO") return d.toString(Qt::ISODate);
 return d.toString(format);
}

/**
 Return (pseudo)random value between 0 and 1 inclusive
*/
double Base::rand() {
 return ((double)::rand())/RAND_MAX;
}

/** Base datetime (for tick counter) */
static QDateTime basetime=QDateTime::currentDateTime();

/**
 Return current tick counter
 (msecs since some arbitrary point in time, usually aplication start.
 Reference point will not change while running the application,
 but can (and probably will) change across different application runs
 May overflow in time)
*/
int Base::tick() {
 QDateTime now=QDateTime::currentDateTime();
 int dt=basetime.date().daysTo(now.date());
 int tt=basetime.time().msecsTo(now.time());
 return dt*(1000*3600*24)+tt;
}

/**
 Multiply transformation matrix or vector[2] by another transformation matrix ( a * b )
 @param ma first operand
 @param mb second operand
 */
QVariant Base::transformationMatrixMul(const QVariant &ma,const QVariant &mb) {
 double a[6];
 double b[6];
 int al=varToDoubleArray(ma,a,6);
 varToDoubleArray(mb,b,6);
 if (al==6) {
  double c[6];
  // 3x3 matrix multiply 3x3 matrix
  c[0]=a[0]*b[0]+a[1]*b[2];
  c[1]=a[0]*b[1]+a[1]*b[3];
  c[2]=a[2]*b[0]+a[3]*b[2];
  c[3]=a[2]*b[1]+a[3]*b[3];
  c[4]=a[4]*b[0]+a[5]*b[2]+b[4];
  c[5]=a[4]*b[1]+a[5]*b[3]+b[5];
  return varFromDoubleArray(c,6);
 }
 if (al==2) {
  double c[2];
  // 1x3 vector multiply 3x3 matrix
  c[0]=a[0]*b[0]+a[1]*b[2]+b[4];
  c[1]=a[0]*b[1]+a[1]*b[3]+b[5];
  return varFromDoubleArray(c,2);
 }
 //Invalid
 return QVariant();//NULL variant
}

/**
 Multiply vector[2] by transformation matrix
 ( [a0,a1] * mb, transform vector with matrix mb )
 @param a0 first vector coordinate
 @param a1 second vector coordinate
 @param mb transformation matrix
 */
QVariant Base::transformationMatrixMul(double a0,double a1,const QVariant &mb) {
 double b[6];
 varToDoubleArray(mb,b,6);
 double c[2];
 // 1x3 vector multiply 3x3 matrix
 c[0]=a0*b[0]+a1*b[2]+b[4];
 c[1]=a0*b[1]+a1*b[3]+b[5];
 return varFromDoubleArray(c,2);
}

/**
 Solve equation oldCTM * requiredCTM = newCTM (find inverse transformation).
 Return requiredCTM.
 NULL is returned if no such matrix exists
 All transformation matrixes are represented as array of 6 doubles.
 */
QVariant Base::transformationMatrixDiv(const QVariant &oldCTM,const QVariant &newCTM) {
 double nowM[6];
 double oldM[6];
 varToDoubleArray(oldCTM,oldM,6);
 double newM[6];
 varToDoubleArray(newCTM,newM,6);
 double menovatel = oldM[0]*oldM[3]-oldM[1]*oldM[2];
 if (menovatel==0) {
  //Cannot find proper inverse matrix
  //warn(tr("Matrix is in bad state !"));
  return QVariant();//NULL variant
 }
 nowM[0]=(-oldM[2]*newM[1]+newM[0]*oldM[3]) / menovatel;
 nowM[1]=(oldM[0]*newM[1]-oldM[1]*newM[0]) / menovatel;
 nowM[2]=(-oldM[2]*newM[3]+newM[2]*oldM[3]) / menovatel;
 nowM[3]=(oldM[0]*newM[3]-oldM[1]*newM[2]) / menovatel;
 nowM[4]=-(-oldM[2]*oldM[5]+oldM[2]*newM[5]+oldM[4]*oldM[3]-newM[4]*oldM[3]) / menovatel;
 nowM[5]=(oldM[1]*oldM[4]-oldM[0]*oldM[5]+oldM[0]*newM[5]-oldM[1]*newM[4]) / menovatel;
 return varFromDoubleArray(nowM,6);
}

/**
 Set error message that will be returned by error() function
 \see error
 @param errorMessage String containing the error message
*/
void Base::setError(const QString &errorMessage) {
 lastErrorMessage=errorMessage;
}

/**
 Try to run each file from given list as script (if it exists)
 @param initScripts List of script files that will be attempted to run
 @return number of script files actually run.
*/
int Base::runScriptList(const QStringList &initScripts) {
 int scriptsRun=0;
 clearError();
 for (unsigned int i=0;i<initScripts.count();i++) {
  QString initScriptFilename=initScripts[i];
  //guiPrintDbg(debug::DBG_INFO,"Considering init script: " << initScriptFilename);
  //Check if the script exists. If not, it is silently skipped
  if (exists(initScriptFilename)) {
   guiPrintDbg(debug::DBG_INFO,"Running init script: " << Q_OUT(initScriptFilename));
   //Any document-related classes are NOT available to the initscript, as no document is currently loaded
   if (!runFile(initScriptFilename)) {
    errorMessage();
    guiPrintDbg(debug::DBG_INFO,"Error running file: " << Q_OUT(initScriptFilename));
    conPrintError(tr("Error running")+" "+initScriptFilename);
   }
   scriptsRun++;
  }
 }
 return scriptsRun;
}

/**
 Try to run each script file (*.qs) present in one of the directories from given list
 If same file is present in multiple directories, only that from later directory is run
 Scripts are run in alphabetical order.
 @param initScriptPaths List of directories with script files
*/
void Base::runScriptsFromPath(const QStringList &initScriptPaths) {
 QMap<QString,QString> initScriptAbsPaths;
 for (unsigned int ip=0;ip<initScriptPaths.count();ip++) {
  QString initPath=initScriptPaths[ip];
  if (!exists(initPath)) {
   guiPrintDbg(debug::DBG_WARN,"Init path does not exist: " << Q_OUT(initPath));
   continue;
  }
  QDir dir(initPath);
  if (dir.isReadable()) {
   QStringList initScripts=dir.entryList("*.qs",QDir::Files | QDir::Readable,QDir::IgnoreCase | QDir::Name);
   for (unsigned int i=0;i<initScripts.count();i++) {
    QString initScriptFilename=initPath+"/"+initScripts[i];
    initScriptAbsPaths.insert(initScripts[i],initScriptFilename);
//    guiPrintDbg(debug::DBG_INFO,"Adding init script: " << initScriptFilename);
   }
  }
  //Path is ok, check for scripts there
 }
 QMap<QString,QString>::Iterator it;
 for (it=initScriptAbsPaths.begin();it!=initScriptAbsPaths.end();++it) {
  QString initScriptFilename=it.data();
  guiPrintDbg(debug::DBG_INFO,"Running init script: " << Q_OUT(initScriptFilename));
  //Any document-related classes are NOT available to the initscript, as no document is currently loaded
  if (!runFile(initScriptFilename)) {
   errorMessage();
   guiPrintDbg(debug::DBG_INFO,"Error running file: " << Q_OUT(initScriptFilename));
   conPrintError(tr("Error running")+" "+initScriptFilename);
  }
 }
}

/**
 Runs script from given file in current interpreter
 @param scriptName name of file with QT Script to run
 @return true if success, false if error (file not exist, etc ...)
 */
bool Base::runFile(const QString &scriptName) {
 QString code=loadFromFile(scriptName);
 if (code.isNull()) return false;
 qs->evaluate(code,this,scriptName);
 if (qs->hadError()) return false;
 return true;
}

// === Scripting functions ===

/**
 Create new operator of type SimpleGenericOperator
 @param parameters Array with operator parameters
 @param text Operator text
 @return new PDF operator
*/
QSPdfOperator* Base::createOperator(const QString &text,QSIPropertyArray* parameters) {
 std::string opTxt=util::convertFromUnicode(text,util::PDF);
 PdfOperator::Operands param;
 parameters->copyTo(param);
 return new QSPdfOperator(pdfobjects::createOperator(opTxt, param),this);
}

/**
 QSA-Bugfix version
 \copydoc createOperator(const QString &,QSIPropertyArray*)
*/
QSPdfOperator* Base::createOperator(const QString &text,QObject* parameters) {
 QSIPropertyArray* par=dynamic_cast<QSIPropertyArray*>(parameters);
 if (!par) {
  errorBadParameter("","createOperator",2,parameters,"IPropertyArray");
  return NULL;
 }
 return createOperator(text,par);
}

/**
 Create new operator of type UnknownCompositePdfOperator
 @param beginText Start operator name text representation.
 @param endText End operator name text representation.
 @return new PDF operator
*/
QSPdfOperator* Base::createCompositeOperator(const QString &beginText,const QString &endText) {
 boost::shared_ptr<UnknownCompositePdfOperator> op(new UnknownCompositePdfOperator(beginText,endText));
 return new QSPdfOperator(op,this);
}

/**
 Create new empty (NULL) PdfOperator
 @return new empty PDF operator
*/
QSPdfOperator* Base::createEmptyOperator() {
 return new QSPdfOperator(this);
}

/**
 Create and initialize new annotation of given type
 @param rect Annotation redctangle
 @param type Type of annotation
 @return created Annotation
*/
QSAnnotation* Base::createAnnotation(QVariant rect,const QString &type) {
 double tm[4]={0};
 Q_List<QVariant> list=rect.toList();
 Q_List<QVariant>::Iterator it = list.begin();
 int i=0;
 while(it!=list.end()) {
  if (i>=4) break;//We filled all values
  tm[i]=(*it).toDouble();
  ++it;
  ++i;
 }
 libs::Rectangle rc(tm[0],tm[1],tm[2],tm[3]);
 boost::shared_ptr<CPage> nullPage;
 boost::shared_ptr<CAnnotation> annot=CAnnotation::createAnnotation(rc,util::convertFromUnicode(type,util::PDF));
 return new QSAnnotation(annot,nullPage,this);
}

/**
 Create new IProperty of type Array - an empty array
 @return created IProperty
*/
QSIProperty* Base::createArray() {
 return new QSArray(boost::shared_ptr<CArray>(CArrayFactory::getInstance()),this);
}

/**
 Create new IProperty of type Bool
 @param value Value assigned to this property
 @return created IProperty
*/
QSIProperty* Base::createBool(bool value) {
 return new QSIProperty(boost::shared_ptr<IProperty>(CBoolFactory::getInstance(value)),this);
}

/**
 Create new IProperty of type Dict - an empty dictionary
 @return created IProperty
*/
QSIProperty* Base::createDict() {
 return new QSDict(boost::shared_ptr<CDict>(CDictFactory::getInstance()),this);
}

/**
 Create new IProperty of type Int
 @param value Value assigned to this property
 @return created IProperty
*/
QSIProperty* Base::createInt(int value) {
 return new QSIProperty(boost::shared_ptr<IProperty>(CIntFactory::getInstance(value)),this);
}

/**
 Create new IProperty of type Real
 @param value Value assigned to this property
 @return created IProperty
*/
QSIProperty* Base::createReal(double value) {
 return new QSIProperty(boost::shared_ptr<IProperty>(CRealFactory::getInstance(value)),this);
}

/**
 Create new IProperty of type Ref
 Does not check for validity of reference
 \see QSPdf::referenceValid
 @param valueNum Number assigned to this reference
 @param valueGen Generation assigned to this reference
 @return created IProperty
*/
QSIProperty* Base::createRef(int valueNum,int valueGen) {
 IndiRef ref;
 ref.num=valueNum;
 ref.gen=valueGen;
 return new QSIProperty(boost::shared_ptr<IProperty>(CRefFactory::getInstance(ref)),this);
}

/**
 Create new IProperty of type String
 @param value Value assigned to this property
 @return created IProperty
*/
QSIProperty* Base::createString(const QString &value) {
 return new QSIProperty(boost::shared_ptr<IProperty>(CStringFactory::getInstance(util::convertFromUnicode(value,util::PDF))),this);
}

/**
 Create new IProperty of type Name
 @param value Value assigned to this property
 @return created IProperty
*/
QSIProperty* Base::createName(const QString &value) {
 return new QSIProperty(boost::shared_ptr<IProperty>(CNameFactory::getInstance(util::convertFromUnicode(value,util::PDF))),this);
}


/**
 Create new array of IProperty items.
 This array can be used for example as operator parameters
 @return new Iproperty array
*/
QSIPropertyArray* Base::createIPropertyArray() {
 return new QSIPropertyArray(this);
}

/**
 Create new empty PDF Operator stack
 @return new operator stack
*/
QSPdfOperatorStack* Base::createPdfOperatorStack() {
 return new QSPdfOperatorStack(this);
}

/**
 Common code to flattener and delinearizator
 Ask for password for opening the PDF file if one is needed
 @param inFile name of input file
 @param wr PdfDocumentWriter
 @return true if either valid password is entered or no password is needed
*/
bool pdfWriterAskPassword(const QString &inFile,boost::shared_ptr<utils::PdfDocumentWriter> wr) {
 for(;wr->getNeedCredentials();) {
  //Ask for password until we either get the right one or user gets bored with retrying
  // FIXME is the first NULL parameter OK?
  QString pwd=gui::PasswordDialog::ask(NULL, QObject::tr("Enter password for %1:").arg(inFile));

  //Dialog aborted -> exit
  if (pwd.isNull()) { 
   guiPrintDbg(debug::DBG_ERR, "No password available");
   return false;
  }

  //We succedded with passwod -> exit
  string passwd;
  passwd = pwd.utf8();
  try {
    wr->setCredentials(passwd.c_str(), passwd.c_str());
  } catch(PermissionException &e) {
   guiPrintDbg(debug::DBG_ERR, "Bad password");
  }
 }
 return true;
}

/**
 Try to flatten PDF, reading from input file and writing flattened result to output file.
 Does not check for overwriting output.
 Return true if flattening was successful, false in case of failure.<br>
 In case of failure the error mesage is available via error()
 \see error
 @param inFile input file
 @param outFile output file
*/
bool Base::flatten(const QString &inFile,const QString &outFile) {
 boost::shared_ptr<utils::Flattener> flat;
 utils::OldStylePdfWriter* wr=NULL;
 try {
  guiPrintDbg(debug::DBG_DBG,"Flattener started");
  wr=new utils::OldStylePdfWriter();
  //TODO: nekde tady zaregistrovat progress observer
  flat=utils::Flattener::getInstance(inFile,wr);
  if (!flat) return false;//No delinearizator instance?
  guiPrintDbg(debug::DBG_DBG,"Flattener created");

  if (!pdfWriterAskPassword(inFile,flat)) return false;

  int ret=flat->flatten(outFile);
  guiPrintDbg(debug::DBG_DBG,"Flattener finished");
  if (ret) {
   const char *whatWasWrong=strerror(ret);
   lastErrorMessage=whatWasWrong;
  }
  guiPrintDbg(debug::DBG_DBG,"Flattener exit");
  return (ret==0);
 } catch (NotImplementedException &e) {
  lastErrorMessage=tr("Flattening of encrypted documents is not supported");
  return false;
 } catch (...) {
  //This is the case of failure ..
  lastErrorMessage=tr("Unknown error occured");
  return false;
 }
}

/**
 Try to delinearize PDF, reading from input file and writing delinearized result to output file.
 Does not check for overwriting output.
 Return true if delinearization was successful, false in case of failure.<br>
 In case of failure the error mesage is available via error()
 \see error
 @param inFile input file
 @param outFile output file
*/
bool Base::delinearize(const QString &inFile,const QString &outFile) {
 boost::shared_ptr<utils::Delinearizator> delin;
 utils::OldStylePdfWriter* wr=NULL;
 try {
  guiPrintDbg(debug::DBG_DBG,"Delinearizator started");
  wr=new utils::OldStylePdfWriter();
  //TODO: nekde tady zaregistrovat progress observer
  delin=utils::Delinearizator::getInstance(inFile,wr);
  if (!delin) return false;//No delinearizator instance?
  guiPrintDbg(debug::DBG_DBG,"Delinearizator created");

  if (!pdfWriterAskPassword(inFile,delin)) return false;

  int ret=delin->delinearize(outFile);
  guiPrintDbg(debug::DBG_DBG,"Delinearizator finished");
  if (ret) {
   const char *whatWasWrong=strerror(ret);
   lastErrorMessage=whatWasWrong;
  }
  guiPrintDbg(debug::DBG_DBG,"Delinearizator exit");
  return (ret==0);
 } catch (NotImplementedException &e) {
  lastErrorMessage=tr("Delinearization of encrypted documents is not supported");
  return false;
 } catch (...) {
  //This is the case of failure ..
  lastErrorMessage=tr("Unknown error occured");
  return false;
 }
}

/**
 * Convert pdf to xml.
 * @param inFile input file
 * @param pagenums List of page numbers.
 * @param outFile output file
 */
QString Base::pdftoxml (const QString& inFile, QVariant pagenums, const QString& outFile) {

	guiPrintDbg (debug::DBG_DBG, "pdftoxml started...");
	
	// Create normal container
	PageNums nums;
	Q_List<QVariant> pagenumslist = pagenums.toList();
	for (Q_List<QVariant>::Iterator it = pagenumslist.begin(); it != pagenumslist.end(); ++it)
		nums.push_back ((*it).toUInt());
	
	// Create cpdf
	boost::shared_ptr<CPdf> pdf;

	try {

		guiPrintDbg (debug::DBG_DBG,"Opening document.");
		pdf = getBasePdfInstance (util::convertFromUnicode(inFile,util::NAME).c_str(),"readonly");
		assert(pdf);
		guiPrintDbg (debug::DBG_DBG,"Document opened.");
	
	}catch (PdfOpenException& e)
	{
		std::string err;
		e.getMessage(err);
		guiPrintDbg(debug::DBG_DBG,"Failed opening document " << err);
		return QString ();
	}

	//
	// Build the output
	//
	XmlOutputBuilder out;
	try
	{
		// Get xml from all pages
		for (PageNums::iterator it = nums.begin(); it != nums.end(); ++it)
			pdf->getPage(*it)->convert<SimpleWordEngine,
									   SimpleLineEngine,
									   SimpleColumnEngine> (out);
	}catch (CObjectException& e)
	{
		setError (tr(e.what()));
		pdf.reset();
		return QString ();
	}

	// Cleanup
	pdf.reset();

	// Save it
	ofstream of;
	of.open (outFile);
	of << XmlOutputBuilder::xml (out) << flush;
	of.close();

	// Do something with the result
	return QString (util::convertToUnicode(XmlOutputBuilder::xml(out),UTF8));
}

/**
 Return last error message from some operations (like load, save, etc ...)
 If last command was successfull, it is undefined what this function returns
 @return Last error message
 */
QString Base::error() {
 return lastErrorMessage;
}

/**
 Check whether given file exists
 @param chkFileName Name of file to check
 @return true if file exists, false otherwise
*/
bool Base::exists(const QString &chkFileName) {
  return QFile::exists(chkFileName);
}

/**
 Return list of all functions that are in current script interpreter.
 Functions are sorted alphabetically
 @param includeSignatures if true, function signatures will be returned, otherwise only names
 */
QStringList Base::functions(bool includeSignatures/*=false*/) {
 QSInterpreter::FunctionFlags flags=QSInterpreter::FunctionNames;
 if (includeSignatures) flags=QSInterpreter::FunctionSignatures;
 //QSInterpreter::IncludeMemberFunctions -?
 QStringList func=qs->functions(this,flags);
 func.sort();
 return func;
}

/**
 Load some PDF file without replacing currently opened file in GUI
 script should also take care to close the file after he does not need to use it anymore
 @param name Name of file to load
 @param advancedMode Set to true to use Advanced mode whilwe opening the file
 @param askPassword if true, attempt to ask user for password would be made if document is encrypted
 @return Loaded document, or NULL if error occured while loading it.
*/
QSPdf* Base::loadPdf(const QString &name,bool advancedMode/*=false*/, bool askPassword/*=true*/) {
 if (name.isNull()) return NULL;
 CPdf::OpenMode mode=advancedMode?(CPdf::Advanced):(CPdf::ReadWrite);
 try {
   boost::shared_ptr<CPdf> opened=getBasePdfInstance(util::convertFromUnicode(name,util::NAME).c_str(),advancedMode?"advanced":"readwrite",askPassword);
  //Return pdf wrapper with 'destructive close' behavior
  return new QSPdf(opened,this,true);
 } catch (...) {
  return NULL;
 }
}

/**
 Open PDF instance.
 Specific password solicitation (GUI, console) should be implemented in subclasses - BaseGUI ands BaseConsole
 If the PDF cannot be opened in read-write mode, another attempt to open the file at least as read-only is done.
 @param filename filename to open
 @param openMode Mode in which to open the file (advanced, readonly and readwrite - default)
 @param askPassword if true, attempt to ask user for password would be made if document is encrypted
*/
boost::shared_ptr<pdfobjects::CPdf> Base::getBasePdfInstance(const QString &filename, const QString &openMode/*=QString::null*/, bool askPassword/*=true*/) {
 CPdf::OpenMode mode=CPdf::ReadWrite;
 if (openMode=="advanced") mode=CPdf::Advanced;
 if (openMode=="readonly") mode=CPdf::ReadOnly;
 if (openMode=="readwrite") mode=CPdf::ReadWrite;
 //Basic mode without asking a password (we do not know how)
 return openPdfWithFallback(filename,mode);
}

/** \copydoc loadFromFile */
QString Base::loadFile(const QString &name) {
 return loadFromFile(name);
}

/** \copydoc saveToFile */
bool Base::saveFile(const QString &name, const QString &content) {
 return saveToFile(name,content);
}

/** \copydoc saveRawToFile */
bool Base::saveRawFile(const QString &name, const QByteArray &content) {
 return saveRawToFile(name, content);
}

/** Return list of all objects that are in current script interpreter */
QStringList Base::objects() {
 QObjectList objs=qs->presentObjects();
 QObjectListIterator it(objs);
 QObject *obj;
 QStringList ret;
 while ((obj=it.current())!=0) {
  ++it;
  ret+=obj->name();
 }
 return ret;
}

/**
 Print given string to console, followed by newline
 @param str String to add
 */
void Base::print(const QString &str) {
 conPrintLine(str);
}

/**
 Runs script from given file
 Not to be called directly, only from script (via slot), as this does not prepare correct script variables before execution
 File is looked for in the script path, unless absolute filename is given.
 If the file is not found in script path, it is looked for in current directory
 @param scriptName name of file with QT Script to run
 @param skipScriptPath Do not look into script path, just try current directory
*/
void Base::run(QString scriptName,bool skipScriptPath/*=false*/) {
 //Look in path for full filename of script
 QString scriptFileName;
 if (!skipScriptPath) {
  scriptFileName=globalSettings->getFullPathName("script",scriptName);
 }
 if (scriptFileName.isNull()) {
  //Try looking in current directory for the script
  if (exists(scriptName)) {
   //Found ...
   scriptFileName=scriptName;
  }
 }
 //No script found by that name
 if (scriptFileName.isNull()) {
  qs->throwError(tr("Script not found")+" : "+scriptName);
  return;
 }
 //Run the script
 if (!runFile(scriptFileName)) {
  qs->throwError(tr("Error running")+" "+scriptName);
 }
}

/**
 Set new debug verbosity level
 @param param New debug verbosity level
 \see util::setDebugLevel
*/
void Base::setDebugLevel(const QString &param) {
 util::setDebugLevel(param);
}

/**
 call QObject::tr to translate specific string
 @param text text to translate to current locale
 @param context Optional context identifier for localized text
 @return translated text
*/
QString Base::tr(const QString &text,const QString &context/*=QString::null*/) {
 if (context.isNull()) return QObject::tr(text);
 return QObject::tr(text,context);
}

/**
 Call after some action causes changes in the treeview that cannot be handled by observers.
 This will cause tree to be reloaded after the script finishes.
*/
void Base::treeNeedReload() {
 treeReloadFlag=true;
}

/** Convert string as it is read by QFile/QSFile/File in QSA to
 Unicode, asuming the string was utf8
 @param original Original string
 @return converted string
 */
QString Base::utf8(QString original) {
 return QString::fromUtf8(original.latin1());
}

/** Return sorted list of all variables that are in current script interpreter */
QStringList Base::variables() {
 QStringList objs=qs->variables(this);
 objs.sort();
 return objs;
}

/** Return version of editor as string
 @return Version of editor (major.minor.release or major.minor.relase-suffix) */
QString Base::version() {
 return PDFEDIT_VERSION;
}


} // namespace gui
