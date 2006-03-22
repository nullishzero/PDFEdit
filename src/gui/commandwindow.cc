/** @file
 CommandWindow - class representing command window (editation of command and output of previous commands)
*/
#include "commandwindow.h"
#include "settings.h"
#include "util.h"
#include <iostream>
#include <qlayout.h>

using namespace std;

/** constructor of CommandWindow, creates window and fills it with elements, parameters are ignored */
CommandWindow::CommandWindow(QWidget *parent/*=0*/,const char *name/*=0*/):QWidget(parent,name) {
 QBoxLayout *l=new QVBoxLayout(this);
 out=new QTextEdit(this);
 cmd=new QLineEdit(this);
//todo: subclass qlineedit, add history ... 
 QObject::connect(cmd, SIGNAL(returnPressed()), this, SLOT(execute()));
 l->addWidget(out);
 l->addWidget(cmd);
 out->setTextFormat(LogText);
 out->setWrapPolicy(QTextEdit::AtWordOrDocumentBoundary);
 out->show();
 cmd->show();
}

/** Execute and clear current command */
void CommandWindow::execute() {
 QString command=cmd->text();
// addCommand(command);			//add to console
 cmd->setText("");			//clear commandline
 emit commandExecuted(command);		//execute command via signal
}

/** Add command executed from menu or any source to be echoed to command window */
void CommandWindow::addCommand(const QString &command) {
 out->append("<b>&gt; </b>"+htmlEnt(command));
}

/** Add string to be echoed to command window */
void CommandWindow::addString(const QString &str) {
 out->append(htmlEnt(str));
}

/** Add error message to be echoed to command window */
void CommandWindow::addError(const QString &message) {
 out->append("<font color=red>! </font>"+htmlEnt(message));
}

/** default destructor */
CommandWindow::~CommandWindow() {
}
