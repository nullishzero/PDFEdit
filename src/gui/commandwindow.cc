/** @file
 CommandWindow - class representing command window (editation of command and output of previous commands)
*/
#include "commandwindow.h"
#include "settings.h"
#include "util.h"
#include <utils/debug.h>
#include <iostream>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qlistbox.h>
#include <qtextedit.h> 
#include <qlineedit.h>
#include <qcombobox.h>

namespace gui {

using namespace std;

QString CMD = "gui/CommandLine/";
QString HISTORYSIZE = "HistorySize";
QString HISTORYFILE = "HistoryFile";
QString DEFAULT__HISTORYFILE = ".pdfedit-history";
int DEFAULT__HISTORYSIZE = 10;

/** constructor of CommandWindow, creates window and fills it with elements, parameters are ignored */
CommandWindow::CommandWindow ( QWidget *parent/*=0*/, const char *name/*=0*/ ):QWidget(parent,name) {
 QBoxLayout * l = new QVBoxLayout( this );
 QPushButton * p = new QPushButton( this );
 out = new QTextEdit( this );
 cmd = new QLineEdit( this );
 // init history
 history = new QComboBox( this );
 history->setLineEdit( cmd );
 history->setEditable( true );
 history->setMaxCount( globalSettings->readNum( CMD + HISTORYSIZE, DEFAULT__HISTORYSIZE ) + 1 );
 loadHistory();
// history->setInsertionPolicy( QComboBox::AtTop );
 history->setInsertionPolicy( QComboBox::NoInsertion );
 cmd->setText( "" );			//clear commandline
// history->setAutoCompletion( true );
//todo: subclass qlineedit, add history ... 
 QObject::connect(cmd, SIGNAL(returnPressed()), this, SLOT(execute()));
 l->addWidget(out);
 l->addWidget(history);
 out->setTextFormat(LogText);
 out->setWrapPolicy(QTextEdit::AtWordOrDocumentBoundary);
}

void CommandWindow::setHistorySize( int historySize ){
	printDbg(debug::DBG_DBG,"Write historySize");
	globalSettings->write( CMD + HISTORYSIZE, historySize );
	history->setMaxCount( historySize + 1 );
}
void CommandWindow::setHistoryFile( const QString & historyFile ){
	printDbg(debug::DBG_DBG,"Write historyFile");
	globalSettings->write( CMD + HISTORYFILE, historyFile );
}

/** Load command history */
void CommandWindow::loadHistory() {
	QFile file( globalSettings->readExpand( CMD + HISTORYFILE, DEFAULT__HISTORYFILE ) );
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		QString line;
		while ( !stream.atEnd() ) {
			line = stream.readLine();	// line of text excluding '\n'
			history->insertItem( line );
		}
		file.close();
		if (history->text( 0 ) != "") {
			history->insertItem("",0);
		}
		return ;
	}
	history->insertItem("");
	printDbg(debug::DBG_DBG,"Cannot open pdfedit-history to read!!!");
}
/** Save current command history */
void CommandWindow::saveHistory() {
	QFile file( globalSettings->readExpand( CMD + HISTORYFILE, DEFAULT__HISTORYFILE ) );
	if ( file.open( IO_WriteOnly ) ) {
		if (history->listBox()->firstItem() != NULL) {
			QTextStream stream( &file );
			for ( QListBoxItem * it = history->listBox()->firstItem(); it != NULL; it = it->next() )
				stream << it->text() << "\n";
		}
		file.close();
		return ;
	}
	printDbg(debug::DBG_DBG,"Cannot open pdfedit-history to read!!!");
}
/** Execute and clear current command */
void CommandWindow::execute() {
 QString command = cmd->text();
// addCommand(command);			//add to console
 cmd->setText("");			//clear commandline
 history->insertItem( command, 1 );
 history->setCurrentItem(-1);
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
	saveHistory();
}

} // namespace gui
