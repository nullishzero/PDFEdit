#ifndef __CONSOLEWINDOW_H__
#define __CONSOLEWINDOW_H__

class QString;
class QStringList;

namespace gui {

class BaseConsole;

/**
 ConsoleWindow - class handling commandline mode
 \brief Commandline mode Main "window"
 */
class ConsoleWindow {
public:
 ConsoleWindow(const QStringList &params);
 void init();
 void run(const QString &scriptName);
 void runFile(const QString &scriptName);
 void eval(const QString &code);
 ~ConsoleWindow();
private:
 /** Base used to host scripts */
 BaseConsole *base;
};

} // namespace gui

#endif
