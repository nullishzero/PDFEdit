#ifndef __COMMANDWINDOW_H__
#define __COMMANDWINDOW_H__

#include <qwidget.h>
#include <qtextedit.h> 
#include <qlineedit.h>

class CommandWindow : public QWidget {
 Q_OBJECT
public:
 CommandWindow(QWidget *parent=0,const char *name=0);
 void addCommand(const QString &command);
 void addError(const QString &message);
 ~CommandWindow();
public slots:
 void execute();
signals:
 void commandExecuted(QString);
private:
 /** Console widget */
 QTextEdit *out;
 /** Commandline widget */
 QLineEdit *cmd;
};

#endif
