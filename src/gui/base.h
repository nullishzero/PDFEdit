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
#ifndef __BASE_H__
#define __BASE_H__

#include "basecore.h"
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>

namespace pdfobjects {
 class CPdf;
}
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
  Tries to delinearize the PDF, reading from input file and writing delinearized result to output file.
  Does not check for overwriting output.
  Returns true if delinearization was successful, false in case of failure.
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
  Tries to flatten the PDF, reading from input file and writing flattened result to output file.
  Does not check for overwriting output.
  Returns true if flattening was successful, false in case of failure.
  In case of failure the error mesage is available via error() function.
 */
 bool flatten(const QString &inFile,const QString &outFile);
 /*-
  Debugging function usable by script developers.
  Returns list of all functions that are present in current script interpreter.
  Functions are sorted alphabetically.
  If includeSignatures is set, function signatures are returned, otherwise only names
 */
 QStringList functions(bool includeSignatures=false);
 /*-
  Load content of file into string. File is expected to be in utf-8 encoding.
 */
 QString loadFile(const QString &name);
 /*-
  Loads a PDF file without replacing currently opened file in GUI.
  Scripts can manipulate the PDF file as necessary, but should close it with unloadPdf() method
  after it don't need to use it anymore.
  If advancedMode is set to true, document is opened in "advanced mode"
  (more advanced, but also more dangerous changes to it are possible)
  If askPassword is set to false, password to document (if the document is encrypted)
  will not be solicited from user and setPassword method of PDF object have to be
  called in script with correct password to allow manipulating the encrypted document.
 */
 QSPdf* loadPdf(const QString &name,bool advancedMode=false, bool askPassword=true);
 /*-
  Debugging function usable by script developers.
  Returns list of all objects that are in current script interpreter
 */
 QStringList objects();
 /*-
  Converts pdf to xml.
  inFile is name of PDF file to convert,
  pagenums is array with page numbers to convert,
  outFile is name of XML file to be created
 */
 QString pdftoxml (const QString& inFile, QVariant pagenums, const QString& outFile);
 /*-
  Outputs given string to command window, followed by newline.
  Useful to output various debugging or status messages
 */
 void print(const QString &str);
 /*-
  return floating point pseudorandom value between 0 and 1 inclusive
 */
 double rand();
 /*-
  Loads and runs script from given filename.
  File is looked for in the script path, unless absolute filename is given or parameter skipScriptPath is specified.
  If the file is not found in script path, it is searched in current directory.
 */
 void run(QString scriptName,bool skipScriptPath=false);
 /*-
  Save string into file. File will be saved in utf-8 encoding.
 */
 bool saveFile(const QString &name, const QString &content);
 /*-
  Save bytes into file. File will be saved verbatim.
 */
 bool saveRawFile(const QString &name, const QByteArray &content);
 /*-
  Sets new debugging verbosity level. Accepts same parameter as -d option on commandline
  (either number or symbolic constant).
 */
 void setDebugLevel(const QString &param);
 /*-
  Return current date/time using given format string
  If format string is "ISO", return time in ISO format. If format string is not specified or empty, return in default localized format.

  Format string may contain following:
  <informaltable frame="none">
   <tgroup cols="2"><tbody>
    <row><entry>d    </entry><entry>day without leading zero</entry></row>
    <row><entry>dd   </entry><entry>day with leading zero</entry></row>
    <row><entry>ddd  </entry><entry>short localized day name</entry></row>
    <row><entry>dddd </entry><entry>long localized day name</entry></row>
    <row><entry>M    </entry><entry>month without leading zero</entry></row>
    <row><entry>MM   </entry><entry>month with leading zero</entry></row>
    <row><entry>MMM  </entry><entry>short localized month name</entry></row>
    <row><entry>MMMM </entry><entry>long localized month name</entry></row>
    <row><entry>yy   </entry><entry>two digit year</entry></row>
    <row><entry>yyyy </entry><entry>four digit year</entry></row>

    <row><entry>h   </entry><entry>hour without leading zero</entry></row>
    <row><entry>hh  </entry><entry>hour with leading zero</entry></row>
    <row><entry>m   </entry><entry>minute without leading zero</entry></row>
    <row><entry>mm  </entry><entry>minute with leading zero</entry></row>
    <row><entry>s   </entry><entry>second without leading zero</entry></row>
    <row><entry>ss  </entry><entry>second with leading zero</entry></row>
    <row><entry>z   </entry><entry>milliseconds without leading zeroes</entry></row>
    <row><entry>zzz </entry><entry>milliseconds with leading zeroes</entry></row>
    <row><entry>AP  </entry><entry>use AM/PM display. AP will be replaced by either "AM" or "PM".</entry></row>
    <row><entry>ap  </entry><entry>use am/pm display. ap will be replaced by either "am" or "pm".</entry></row>
   </tbody></tgroup>
  </informaltable>
  Anything else is left as is.
 */
 QString time(const QString &format=QString::null);
 /*-
  Return "tick counter" in milliseconds, usable to measure time differences
 */
 int tick();
 /*-
  Solve equation oldCTM * requiredCTM = newCTM (find matrix for inverse transformation).
  Return requiredCTM.
  NULL is returned if no such matrix exists
  All transformation matrixes are represented as array of 6 doubles.
 */
 QVariant transformationMatrixDiv(const QVariant &oldCTM,const QVariant &newCTM);
 /*-
  Multiply vector by transformation matrix (resulting in transformad vector)
  or transformation matrix by another transformation matrix
  (joining transformations int single matrix)
  First parameter (ma) is vector or matrix to multiply.
  Second parameter (mb) is transformation matrix
  Transformation matrix is represented by array of 6 float numbers
  Vector is array of 2 float numbers
 */
 QVariant transformationMatrixMul(const QVariant &ma,const QVariant &mb);
 /*-
  Variant of transformationMatrixMul.
  Multiply vector [a0,a1] by transformation matrix mb and return resulting transformed vector
 */
 QVariant transformationMatrixMul(double a0,double a1,const QVariant &mb);
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
 /*-
  Returns version of editor. String is in format 'major.minor.release' for ordinary releases
  or 'major.minor.relase-suffix' for "special" versions, like cvs checkouts (suffix CVS appended)
  or versions from branches, which would have branch tag added)
 */
 QString version();

protected:
 virtual boost::shared_ptr<pdfobjects::CPdf> getBasePdfInstance(const QString &filename, const QString &openMode=QString::null, bool askPassword=true);

private:
 bool runFile(const QString &scriptName);
protected:
 /** Last error message from exception in load/save, etc ... */
 QString lastErrorMessage;
};

} // namespace gui

#endif
