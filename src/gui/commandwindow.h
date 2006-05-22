#ifndef __COMMANDWINDOW_H__
#define __COMMANDWINDOW_H__

#include <qwidget.h>
class QTextEdit;
class QLineEdit;
class QComboBox;
class QSInterpreter;
class QSEditor;

namespace gui {

class CommandWindow : public QWidget {
 Q_OBJECT
public:
 enum cmd { CmdNone = 0, CmdHistory = 1, CmdLine = 2, CmdEditor = 4 };
 CommandWindow(QWidget *parent=0,const char *name=0);
 void addCommand(const QString &command);
 void addError(const QString &message);
 void addString(const QString &str);
 virtual bool	eventFilter( QObject * o, QEvent * e );
 virtual QSize	minimumSizeHint() const;
 ~CommandWindow();
public slots:
 void execute( enum cmd  from = CmdLine );
 void setHistorySize( int historySize );
 void setHistoryFile( const QString & historyFile );
 void setInterpreter( QSInterpreter * ainterpreter, QObject * context );
 void setCmdWindowMode( int mode = CmdHistory | CmdLine );
 int getCmdWindowMode();
 void loadHistory();
 void saveHistory();
private slots:
 void selectedHistoryItem( int );
signals:
 /** Signal emitted when command is executed from this command window */
 void commandExecuted(QString);
private:
 /** Console widget */
 QTextEdit *out;
 /** Commandline widget */
 QLineEdit *cmd;
 QSEditor  *in;
 /** History widget */
 QComboBox *history;
 /** Interpreter */
 QSInterpreter *interpreter;
};

} // namespace gui

#endif
