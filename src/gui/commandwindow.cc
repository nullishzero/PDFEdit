/** @file
 CommandWindow - class representing command window (editation of command and output of previous commands)
*/
#include "commandwindow.h"
#include "settings.h"
#include "util.h"
#include <utils/debug.h>
#include <iostream>
#include <qvbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qfile.h>
#include <qlistbox.h>
#include <qtextedit.h> 
#include <qlineedit.h>
#include <qcombobox.h>
#include <qseditor.h>
#include <qsinterpreter.h>
#include <qsplitter.h>
#include <qsizepolicy.h>

using namespace std;
using namespace util;

namespace gui {

/** Root settings key prefixing all settings defined in this class */
QString CMD = "gui/CommandLine/";
/** Settings key specifying if to show the commandline */
QString CMDSHOWHISTORY = "CmdShowHistory";
/** Settings key specifying if the commandline is enables for editing or just as history drop-down */
QString CMDSHOWLINE = "CmdShowLine";
/** Settings key specifying if to show the script editor */
QString CMDSHOWEDITOR = "CmdShowEditor";
/** Settings key identifying maximal size of history (in lines) */
QString HISTORYSIZE = "HistorySize";
/** Settings key identifying history file */
QString HISTORYFILE = "HistoryFile";
/** Settings key identifying history items separator */
QString HISTORYFILEITEMSEPARATOR = "HistoryFileItemSeparator";
/** Default history file */
QString DEFAULT__HISTORYFILE = ".pdfedit-history";
/** Default history items separator */
QString DEFAULT__HISTORYFILEITEMSEPARATOR = "<EndItem>";
/** Default maximal size of history (in lines) */
int DEFAULT__HISTORYSIZE = 10;
/** Default value of setting identified by key CMDSHOWHISTORY */
bool DEFAULT__CMDSHOWHISTORY = true;
/** Default value of setting identified by key CMDSHOWLINE */
bool DEFAULT__CMDSHOWLINE = true;
/** Default value of setting identified by key CMDSHOWEDITOR */
bool DEFAULT__CMDSHOWEDITOR = false;


/**
 constructor of CommandWindow, creates window and fills it with elements
 @param parent Parent widget
 @param name Name of the widget (used for debugging)
*/
CommandWindow::CommandWindow ( QWidget *parent/*=0*/, const char *name/*=0*/ ):QWidget(parent,name) {
 QBoxLayout * hl = new QHBoxLayout( this );
 spl=new QSplitter( Vertical, this, "spl" );
 hl->addWidget( spl );

 QVBox * l = new QVBox( spl );
 out = new QTextEdit( /*this*/ l );
 cmd = new QLineEdit( this , "CmdLine" );

 // init history
 history = new QComboBox( /*this*/ l, "CmdHistory" );
 history->setLineEdit( cmd );
 history->setEditable( true );
 history->setMaxCount( globalSettings->readNum( CMD + HISTORYSIZE, DEFAULT__HISTORYSIZE ) + 1 );
 
 // setting sizePolicy for ignoring width hint (some history items are too large)
 QSizePolicy spol ( QSizePolicy::Ignored, QSizePolicy::Preferred );
 history->setSizePolicy( spol );
 history->setMinimumHeight( history->sizeHint().height() );

 loadHistory();
// history->setInsertionPolicy( QComboBox::AtTop );
 history->setInsertionPolicy( QComboBox::NoInsertion );

 cmd->setText( "" );			//clear commandline
// history->setAutoCompletion( true );
 QObject::connect(cmd, SIGNAL(returnPressed()), this, SLOT(execute()));
// l->addWidget(out);
// l->addWidget(history);
 out->setTextFormat(LogText);
 out->setWrapPolicy(QTextEdit::AtWordOrDocumentBoundary);

 in = new QSEditor( spl , "CmdEditor" );
 in->textEdit()->setText("");
 in->textEdit()->installEventFilter( this );
 in->textEdit()->viewport()->installEventFilter( this );
 in->setFocus();
 interpreter = NULL;
// l->addWidget( in );

 connect( history, SIGNAL( activated(int) ), this, SLOT( selectedHistoryItem(int) ));
 reloadSettings();
}

/** Load/reload show/hide settings from global settings and apply them to this widget */
void CommandWindow::reloadSettings() {
 showCmdHistory(	globalSettings->readBool( CMD + CMDSHOWHISTORY, DEFAULT__CMDSHOWHISTORY ) );
 showCmdLine(		globalSettings->readBool( CMD + CMDSHOWLINE, DEFAULT__CMDSHOWLINE ) );
 showCmdEditor(		globalSettings->readBool( CMD + CMDSHOWEDITOR, DEFAULT__CMDSHOWEDITOR ) );
}

/**
 Set interpreter and context for script editor
 @param ainterpreter QSInterpreter instance
 @param context Context in which scripts are executed
*/
void CommandWindow::setInterpreter( QSInterpreter * ainterpreter, QObject * context ) {
	in->setInterpreter( ainterpreter, context );
	interpreter = ainterpreter;
}

/**
 Erase all text in the window
*/
void CommandWindow::clearWindow() {
	out->setText("");
}

bool CommandWindow::eventFilter( QObject *o, QEvent *e )
{
	if ( o != in->textEdit() && o != in->textEdit()->viewport() )
		return FALSE;
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent*)e;
		switch ( ke->key() ) {
			case Key_Return:
			case Key_Enter:
				if ( ke->state() & ControlButton ) {
					in->textEdit()->doKeyboardAction( QTextEdit::ActionReturn );
				} else {
					QString code = in->textEdit()->text();
					if ( code[ 0 ] == '?' )
						code = "debug(" + code.mid( 1 ) + ");";
					if ( !interpreter->checkSyntax( code ) ) {
						in->textEdit()->doKeyboardAction( QTextEdit::ActionReturn );
						return TRUE;
					}
					execute( CmdEditor );
				}
				return TRUE;
			case Key_Up:
				if ( ke->state() & ControlButton ) {
					if ( history->currentItem() > 0 ) {
						history->setCurrentItem( history->currentItem() - 1);
						in->textEdit()->setText( history->currentText() );
					}
					return TRUE;
				}
				break;
			case Key_Down:
				if ( ke->state() & ControlButton ) {
					if ( history->currentItem() < history->count() - 1 ) {
						history->setCurrentItem( history->currentItem() + 1);
						in->textEdit()->setText( history->currentText() );
					} else {
						in->textEdit()->clear();
					}
					return TRUE;
				}
				break;
			case Key_Escape:
				ke->ignore();
				return TRUE;
		}  // end of switch
	}
	return FALSE;
}

/**
 Set maximal history size and write it to settings
 @param historySize maximal history size (in lines)
*/
void CommandWindow::setHistorySize( int historySize ){
	guiPrintDbg(debug::DBG_INFO,"Write historySize");
	globalSettings->write( CMD + HISTORYSIZE, historySize );
	history->setMaxCount( historySize + 1 );
}

/**
 Set file to store history and write it to settings
 @param historyFile file fopr storing history
*/
void CommandWindow::setHistoryFile( const QString & historyFile ){
	guiPrintDbg(debug::DBG_INFO,"Write historyFile");
	globalSettings->write( CMD + HISTORYFILE, historyFile );
}

/** Load command history */
void CommandWindow::loadHistory() {
	QString enditem = globalSettings->readExpand( CMD + HISTORYFILEITEMSEPARATOR, DEFAULT__HISTORYFILEITEMSEPARATOR );
	QFile file( globalSettings->readExpand( CMD + HISTORYFILE, DEFAULT__HISTORYFILE ) );
	history->clear();
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );
		stream.setEncoding( QTextStream::UnicodeUTF8 );
		QString h_line, line;
		while ( !stream.atEnd() ) {
			line = "";
			h_line = stream.readLine();	// line of text excluding '\n'
			while ((!stream.atEnd()) && (h_line.right( enditem.length() ).compare(enditem) != 0) ) {
				if (h_line.right(1).compare(" ") == 0)
						h_line.truncate( h_line.length() -1 );
				if (!line.isEmpty())
					line += "\n";
				line += h_line;
				h_line = stream.readLine();
			}
			if (h_line.right( enditem.length() ).compare(enditem) == 0) {
				h_line.truncate( h_line.length() - enditem.length() );
			}
			if (h_line.right(1).compare(" ") == 0)
					h_line.truncate( h_line.length() -1 );
			if ((! line.isEmpty()) && (! h_line.isEmpty()))
				line += "\n";
			if (! h_line.isEmpty())
				line += h_line;
			history->insertItem( line );
		}
		file.close();
		if ((history->count() == 0) || (history->text( 0 ) != "")) {
			history->insertItem("",0);
		}
		return ;
	}
	history->insertItem("");
	guiPrintDbg(debug::DBG_INFO,"Cannot open pdfedit-history to read!!!");
}
/** Save current command history */
void CommandWindow::saveHistory() {
	QString enditem = globalSettings->readExpand( CMD + HISTORYFILEITEMSEPARATOR, DEFAULT__HISTORYFILEITEMSEPARATOR );
	QFile file( globalSettings->readExpand( CMD + HISTORYFILE, DEFAULT__HISTORYFILE ) );
	if ( file.open( IO_WriteOnly ) ) {
		if (history->listBox()->firstItem() != NULL) {
			QTextStream stream( &file );
			stream.setEncoding( QTextStream::UnicodeUTF8 );
			for ( QListBoxItem * it = history->listBox()->firstItem(); it != NULL; it = it->next() ) {
				stream << it->text().replace( '\n', QString(" \n") ) << enditem << "\n" ;
			}
		}
		file.close();
		return ;
	}
	guiPrintDbg(debug::DBG_INFO,"Cannot open pdfedit-history to read!!!");
}
/**
 Execute and clear current command
 @param from Source of the command
 */
void CommandWindow::execute( enum cmd  from ) {
 QString command;
 if (from != CmdEditor)
	command = cmd->text();
 else
	command = in->textEdit()->text();
 cmd->setText("");			//clear commandline
 in->textEdit()->clear();
 history->insertItem( command, 1 );
 history->setCurrentItem(0);

 emit commandExecuted(command);		//execute command via signal
}

void CommandWindow::selectedHistoryItem( int ) {
	in->textEdit()->setText( history->currentText() );
}

void CommandWindow::hideCmdHistory( bool hide ) {
	 showCmdHistory( ! hide );
}
void CommandWindow::hideCmdLine( bool hide ) {
	 showCmdLine( ! hide );
}
void CommandWindow::hideCmdEditor( bool hide ) {
	 showCmdEditor( ! hide );
}
void CommandWindow::showCmdHistory( bool show ) {
	if (show)
		history->show();
	else
		history->hide();
}
void CommandWindow::showCmdLine( bool show ) {
	if (show)
		cmd->show();
	else
		cmd->hide();
}
void CommandWindow::showCmdEditor( bool show ) {
	if (show)
		in->show();
	else
		in->hide();
}
bool CommandWindow::isShownCmdHistory() {
	 return history->isShown();
}
bool CommandWindow::isShownCmdLine() {
	 return cmd->isShown();
}
bool CommandWindow::isShownCmdEditor() {
	 return in->isShown();
}

/**
 Add command executed from menu or any source to be echoed to command window
 @param command Command to add
*/
void CommandWindow::addCommand(const QString &command) {
 out->append("<b>&gt; </b>"+htmlEnt(command));
 consoleLog("> "+command,globalSettings->readExpand("path/console_log"));
}

/**
 Add string to be echoed to command window
 @param str String to add
*/
void CommandWindow::addString(const QString &str) {
 out->append(htmlEnt(str));
 consoleLog(str,globalSettings->readExpand("path/console_log"));
}

/**
 Add error message to be echoed to command window
 @param message Error message to add
*/
void CommandWindow::addError(const QString &message) {
 out->append("<font color=red>! </font>"+htmlEnt(message));
 consoleLog("! "+message,globalSettings->readExpand("path/console_log"));
}

/** Saves command state to application settings*/
void CommandWindow::saveWindowState() {
 globalSettings->saveSplitter(spl,"spl_cmd"); 
}


/** Restores window state from application settings */
void CommandWindow::restoreWindowState() {
 globalSettings->restoreSplitter(spl,"spl_cmd"); 
}
/** default destructor */
CommandWindow::~CommandWindow() {
	saveHistory();
}

} // namespace gui
