#ifndef __CONSOLEWRITERGUI_H__
#define __CONSOLEWRITERGUI_H__

#include "consolewriter.h"

class QString;

namespace gui {

class CommandWindow;

/**
 class handling console output by sending it to command window
*/
class ConsoleWriterGui : public ConsoleWriter {
public:
 ConsoleWriterGui(CommandWindow *_cmdLine);
 virtual ~ConsoleWriterGui();
 virtual void printLine(const QString &line);
 void printErrorLine(const QString &line);
 virtual void printCommand(const QString &cmd);
private:
 /** Command window used for output */
 CommandWindow *cmdLine;
};

} // namespace gui

#endif
