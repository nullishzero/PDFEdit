#ifndef __CONSOLEWRITERCONSOLE_H__
#define __CONSOLEWRITERCONSOLE_H__

#include "consolewriter.h"

class QString;

namespace gui {

/**
 class handling console output by sending it to STDOUT
 \brief Console output in commandline
*/
class ConsoleWriterConsole : public ConsoleWriter {
public:
 ConsoleWriterConsole();
 virtual ~ConsoleWriterConsole();
 virtual void printLine(const QString &line);
 void printErrorLine(const QString &line);
 virtual void printCommand(const QString &cmd);
};

} // namespace gui

#endif
