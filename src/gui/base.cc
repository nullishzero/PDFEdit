/** @file
 Base - class that host scripts and contain static script functions
 This class is also responsible for garbage collection of scripting objects
 @author Martin Petricek
*/

#include "base.h"
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
#include <cpdf.h>
#include <delinearizator.h> 
#include <factories.h> 
#include <pdfwriter.h> 
#include <qdir.h>
#include <qfile.h>
#include <qsinterpreter.h>
#include <utils/debug.h>

namespace gui {

using namespace util;
using namespace pdfobjects;

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
   guiPrintDbg(debug::DBG_INFO,"Running init script: " << initScriptFilename);
   //Any document-related classes are NOT available to the initscript, as no document is currently loaded
   if (!runFile(initScriptFilename)) {  
    errorMessage();
    guiPrintDbg(debug::DBG_INFO,"Error running file: " << initScriptFilename);
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
   guiPrintDbg(debug::DBG_WARN,"Init path does not exist: " << initPath);
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
  guiPrintDbg(debug::DBG_INFO,"Running init script: " << initScriptFilename);
  //Any document-related classes are NOT available to the initscript, as no document is currently loaded
  if (!runFile(initScriptFilename)) {  
   errorMessage();
   guiPrintDbg(debug::DBG_INFO,"Error running file: " << initScriptFilename);
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
 std::string opTxt=text;
 PdfOperator::Operands param;
 parameters->copyTo(param);
 boost::shared_ptr<SimpleGenericOperator> op(new SimpleGenericOperator(opTxt,param));
 return new QSPdfOperator(op,this); 
}

/**
 QSA-Bugfix version
 \copydoc createOperator(const QString &,QSIPropertyArray*)
*/
QSPdfOperator* Base::createOperator(const QString &text,QObject* parameters) {
 QSIPropertyArray* par=dynamic_cast<QSIPropertyArray*>(parameters);
 if (!par) return NULL;
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
 QValueList<QVariant> list=rect.toList();
 QValueList<QVariant>::Iterator it = list.begin();
 int i=0;
 while(it!=list.end()) {
  if (i>=4) break;//We filled all values
  tm[i]=(*it).toDouble();
  ++it;
  ++i;
 }
 Rectangle rc(tm[0],tm[1],tm[2],tm[3]);
 boost::shared_ptr<CPage> nullPage;
 boost::shared_ptr<CAnnotation> annot=CAnnotation::createAnnotation(rc,type);
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
 return new QSIProperty(boost::shared_ptr<IProperty>(CStringFactory::getInstance(value)),this);
}

/**
 Create new IProperty of type Name
 @param value Value assigned to this property
 @return created IProperty
*/
QSIProperty* Base::createName(const QString &value) {
 return new QSIProperty(boost::shared_ptr<IProperty>(CNameFactory::getInstance(value)),this);
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
 Try to delinearize PDF, reading from input file and writing delinearized result to output file.
 Does not check for overwriting output.
 Return true if delinearization was successful, false in case of failure.<br>
 In case of failure the error mesage is available via error()
 \see error
 @param inFile input file
 @param outFile output file
*/
bool Base::delinearize(const QString &inFile,const QString &outFile) {
 utils::Delinearizator* delin=NULL;
 utils::OldStylePdfWriter* wr=NULL;
 try {
  guiPrintDbg(debug::DBG_DBG,"Delinearizator started");
  wr=new utils::OldStylePdfWriter();
  //TODO: nekde tady zaregistrovat progress observer
  delin=utils::Delinearizator::getInstance(inFile,wr);
  guiPrintDbg(debug::DBG_DBG,"Delinearizator created");
  int ret=delin->delinearize(outFile);
  guiPrintDbg(debug::DBG_DBG,"Delinearizator finished");
  if (ret) {
   const char *whatWasWrong=strerror(ret);
   lastErrorMessage=whatWasWrong;
  } 
//  guiPrintDbg(debug::DBG_DBG,"deleting pdf writer");
//  if (wr) delete wr;
  guiPrintDbg(debug::DBG_DBG,"deleting delinearizator");
  if (delin) delete delin;  
  guiPrintDbg(debug::DBG_DBG,"Delinearizator exit");
  return (ret==0);
 } catch (...) {
  //This is the case of failure ..
  if (wr) delete wr;
  if (delin) delete delin;  
  return false;
 }
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
 @return Loaded document, or NULL if error occured while loading it.
*/
QSPdf* Base::loadPdf(const QString &name,bool advancedMode/*=false*/) {
 if (name.isNull()) return NULL;
 CPdf::OpenMode mode=advancedMode?(CPdf::Advanced):(CPdf::ReadWrite);
 try {
  CPdf *opened=CPdf::getInstance(name,mode);
  //Return pdf wrapper with 'destructive close' behavior
  return new QSPdf(opened,this,true);
 } catch (...) {
  return NULL;
 }
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

/** Return sorted list of all variables that are in current script interpreter */
QStringList Base::variables() {
 QStringList objs=qs->variables(this);
 objs.sort();
 return objs;
}

/** Return version of editor
 @return Version of editor (major.minor.release) */
QString Base::version() {
 return VERSION;
}

} // namespace gui
