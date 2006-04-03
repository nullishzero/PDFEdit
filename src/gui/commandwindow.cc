/** @file
 CommandWindow - class representing command window (editation of command and output of previous commands)
*/
#include "commandwindow.h"
#include "settings.h"
#include "util.h"
#include <iostream>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qlistbox.h>

using namespace std;

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
 history->setMaxCount( 10 );		// TODO from settings
 loadHistory();
 history->setInsertionPolicy( QComboBox::AtTop );
 cmd->setText( "" );			//clear commandline
// history->setAutoCompletion( true );
//todo: subclass qlineedit, add history ... 
 QObject::connect(cmd, SIGNAL(returnPressed()), this, SLOT(execute()));
 l->addWidget(out);
 l->addWidget(history);
 out->setTextFormat(LogText);
 out->setWrapPolicy(QTextEdit::AtWordOrDocumentBoundary);
}

/** Load command history */
void CommandWindow::loadHistory() {
	QFile file( ".pdfedit-history" );	// TODO from settings
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		QString line;
		while ( !stream.atEnd() ) {
			line = stream.readLine();	// line of text excluding '\n'
			history->insertItem( line );
		}
		file.close();
	} else {
		addString( "Cannot open pdfedit-history to read!!!\n" );	// TODO tr
	}
}
/** Save current command history */
void CommandWindow::saveHistory() {
	QFile file( ".pdfedit-history" );	// TODO from settings
	if ( file.open( IO_WriteOnly ) ) {
		if (history->listBox()->firstItem() != NULL) {
			QTextStream stream( &file );
			for ( QListBoxItem * it = history->listBox()->firstItem(); it != NULL; it = it->next() )
				stream << it->text() << "\n";
		}
		file.close();
	} else {
		addString( "Cannot open pdfedit-history to write!!!" );	// TODO tr
	}
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
	saveHistory();
}
