#ifndef __COMMANDWINDOW_H__
#define __COMMANDWINDOW_H__

#include <qwidget.h>
class QTextEdit;
class QLineEdit;
class QComboBox;

namespace gui {

class CommandWindow : public QWidget {
 Q_OBJECT
public:
 CommandWindow(QWidget *parent=0,const char *name=0);
 void addCommand(const QString &command);
 void addError(const QString &message);
 void addString(const QString &str);
 ~CommandWindow();
public slots:
 void execute();
 void setHistorySize( int historySize );
 void setHistoryFile( const QString & historyFile );
signals:
 /** Signal emitted when command is executed from this command window */
 void commandExecuted(QString);
private:
 void loadHistory();
 void saveHistory();
 /** Console widget */
 QTextEdit *out;
 /** Commandline widget */
 QLineEdit *cmd;
 /** History widget */
 QComboBox *history;
};

} // namespace gui

#endif
