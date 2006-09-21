#ifndef __BASE_H__
#define __BASE_H__

#include "basecore.h"
#include <qobject.h>
#include <qstring.h>
#include <qvariant.h>

namespace gui {

class QSAnnotation;
class QSArray;
class QSDict;
class QSIProperty;
class QSIPropertyArray;
class QSPage;
class QSPdf;
class QSPdfOperator;
class QSPdfOperatorStack;
class TreeItemAbstract;

/**
 Class that hosts scripts and contains static script functions.
 <br>
 This class is also responsible for garbage collection of scripting
 objects and interaction of editor with scripts
 \brief Script hosting base class.
*/
class Base : public BaseCore {
 Q_OBJECT
public:
 Base();
 virtual ~Base();
 int runScriptList(const QStringList &initScripts);
 void runScriptsFromPath(const QStringList &initScriptPaths);
 void setError(const QString &errorMessage);
public slots: //This will be all exported to scripting
 /*-
  Creates and initializes new annotation of given type.
  Parameter rect specifies annotation rectangle in default user space units.
 */
 QSAnnotation* createAnnotation(QVariant rect,const QString &type);/*Variant=double[4]*/
 /*- Creates and returns new IProperty of type Array - empty array */
 QSIProperty* createArray();
 /*- Creates and returns new IProperty of type Bool */
 QSIProperty* createBool(bool value);
 /*- Creates new composite PDF Operator with specified starting and ending operator text*/
 QSPdfOperator* createCompositeOperator(const QString &beginText,const QString &endText);
 /*- Creates and returns new IProperty of type Dict - empty dictionary */
 QSIProperty* createDict();
 /*- Creates new empty (NULL) PDF Operator*/
 QSPdfOperator* createEmptyOperator();
 /*- Creates and returns new IProperty of type Int */
 QSIProperty* createInt(int value);
 /*-
  Creates new array of IProperty items.
  This array can be used for example as operator parameters
 */
 QSIPropertyArray* createIPropertyArray();
 /*- Creates and returns new IProperty of type String */
 QSIProperty* createName(const QString &value);
 /*- Creates new PDF Operator with specified operator text and parameters */
 QSPdfOperator* createOperator(const QString &text,QSIPropertyArray* parameters);
 QSPdfOperator* createOperator(const QString &text,QObject* parameters);
 /*- Create new empty PDF Operator stack */
 QSPdfOperatorStack* createPdfOperatorStack();
 /*- Creates and returns new IProperty of type Real */
 QSIProperty* createReal(double value);
 /*-
  Creates and returns new IProperty of type Ref.
  Does not check validity of the values
  (if the reference target exists, etc ...)
 */
 QSIProperty* createRef(int valueNum,int valueGen);
 /*- Creates and returns new IProperty of type String */
 QSIProperty* createString(const QString &value);
 /*-
  Tries to delinearize PDF, reading from input file and writing delinearized result to output file.
  Does not check for overwriting output.
  Returnis true if delinearization was successful, false in case of failure.
  In case of failure the error mesage is available via error() function.
 */
 bool delinearize(const QString &inFile,const QString &outFile);
 /*-
  Returns last error message (localized) from some operations
  (openFile, save, saveRevision, saveCopy, delinearize)
 */
 QString error();
 /*- Returns true if given file exists on disk, false otherwise */
 bool exists(const QString &chkFileName);
 /*-
  Debugging function usable by script developers.
  Returns list of all functions that are present in current script interpreter.
  Functions are sorted alphabetically.
  If includeSignatures is set, function signatures are returned, otherwise only names
 */
 QStringList functions(bool includeSignatures=false);
 /*-
  Loads a PDF file without replacing currently opened file in GUI.
  Scripts can manipulate the PDF file as necessary, but should close it with unloadPdf() method
  after it don't need to use it anymore.
  If advancedMode is set to true, document is opened in "advanced mode"
  (more advanced, but also more dangerous changes to it are possible)
 */
 QSPdf* loadPdf(const QString &name,bool advancedMode=false);
 /*-
  Debugging function usable by script developers.
  Returns list of all objects that are in current script interpreter
 */
 QStringList objects();
 /*-
  Outputs given string to command window, followed by newline.
  Useful to output various debugging or status messages
 */
 void print(const QString &str);
 /*-
  Loads and runs script from given filename.
  File is looked for in the script path, unless absolute filename is given or parameter skipScriptPath is specified.
  If the file is not found in script path, it is searched in current directory.
 */
 void run(QString scriptName,bool skipScriptPath=false);
 /*-
  Sets new debugging verbosity level. Accepts same parameter as -d option on commandline
  (either number or symbolic constant).
 */
 void setDebugLevel(const QString &param);
 /*-
  Translates given text to current locale. Optional parameter context can specify context
  of localized text. Returns translated text.
 */
 QString tr(const QString &text,const QString &context=QString::null);
 /*-
  You can call this to explicitly request the tree to reload after the script finishes execution.
 */
 void treeNeedReload();
 /*- Return string converted from utf8 encoding to unicode encoding */
 QString utf8(QString original);
 /*-
  Debugging function usable by script developers.
  Returns sorted list of all variables that are in current script interpreter.
 */
 QStringList variables();
 /*- Returns version of editor (in format 'major.minor.release') */
 QString version();

private:
 bool runFile(const QString &scriptName);
protected:
 /** Last error message from exception in load/save, etc ... */
 QString lastErrorMessage;
};

} // namespace gui

#endif
