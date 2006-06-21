#ifndef __BASECONSOLE_H__
#define __BASECONSOLE_H__

#include "base.h"

namespace gui {

class ConsoleWriterConsole;

/**
 Class that host scripts and contain static script functions<br>
 This class is also responsible for garbage collection of scripting
 objects and interaction of editor and scripts
 \brief Script hosting base class
*/
class BaseConsole : public Base {
 Q_OBJECT
public:
 BaseConsole(const QStringList &_params);
 virtual ~BaseConsole();
 void runInitScript();
public slots: //This will be all exported to scripting
 /*- 
 Terminate the application. You can specify return code of application in parameter
 */
 void exitApp(int returnCode=0);
 /*-
  Return list of commandline parameters
  (excluding any possible switches processed by pdfeditor itself)
 */
 QStringList parameters();

private:
 /** Console writer class writing to STDOUT */
 ConsoleWriterConsole* consoleWriter;
 /** Commandline parameters */
 QStringList params;
};

} // namespace gui

#endif
