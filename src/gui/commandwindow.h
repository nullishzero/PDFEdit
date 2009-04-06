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
#ifndef __COMMANDWINDOW_H__
#define __COMMANDWINDOW_H__

#include "qtcompat.h"
#include <qwidget.h>
class Q_ComboBox;
class Q_TextEdit;
class QLineEdit;
class QSInterpreter;
class QSEditor;
class QSplitter;

namespace gui {

/**
 Class representing command window<br>
 The window allow editation of command and
 show output of previously executed commands
*/
class CommandWindow : public QWidget {
 Q_OBJECT
public:
 /** Source of command */
 enum cmd { CmdNone = 0, CmdHistory = 1, CmdLine = 2, CmdEditor = 4 };
 CommandWindow(QWidget *parent=0,const char *name=0);
 void addCommand(const QString &command);
 void addError(const QString &message);
 void addString(const QString &str);
 virtual bool eventFilter( QObject * o, QEvent * e );
 void reloadSettings();
 void saveWindowState();
 void restoreWindowState();
 void clearWindow();
 ~CommandWindow();
 void execute(enum cmd from);
public slots:
 void execute();
 void setHistorySize( int historySize );
 void setHistoryFile( const QString & historyFile );
 void setInterpreter( QSInterpreter * ainterpreter, QObject * context );
 void loadHistory();
 void saveHistory();

 void hideCmdHistory( bool hide = true );
 void hideCmdLine( bool hide = true );
 void hideCmdEditor( bool hide = true );
 void showCmdHistory( bool show = true );
 void showCmdLine( bool show = true );
 void showCmdEditor( bool show = true );
 bool isShownCmdHistory();
 bool isShownCmdLine();
 bool isShownCmdEditor();
private slots:
 void selectedHistoryItem( int );
signals:
 /** Signal emitted when command is executed from this command window */
 void commandExecuted(QString);
private:
 /** Console widget */
 Q_TextEdit *out;
 /** Commandline widget */
 QLineEdit *cmd;
 /** Script editor widget */
 QSEditor  *in;
 /** History widget */
 Q_ComboBox *history;
 /** Interpreter */
 QSInterpreter *interpreter;
 /** Vetical  splitter between (out + command line) and command editor */
 QSplitter *spl;
};

} // namespace gui

#endif
