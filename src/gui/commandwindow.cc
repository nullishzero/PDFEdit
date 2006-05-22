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
#include <qseditor.h>
#include <qsinterpreter.h>

using namespace std;
using namespace util;

namespace gui {

QString CMD = "gui/CommandLine/";
QString CMDMODE = "CmdMode";
QString HISTORYSIZE = "HistorySize";
QString HISTORYFILE = "HistoryFile";
QString HISTORYFILEITEMSEPARATOR = "HistoryFileItemSeparator";
QString DEFAULT__HISTORYFILE = ".pdfedit-history";
QString DEFAULT__HISTORYFILEITEMSEPARATOR = "<EndItem>";
int DEFAULT__HISTORYSIZE = 10;
int DEFAULT__CMDMODE = CommandWindow::CmdHistory | CommandWindow::CmdLine;

/** constructor of CommandWindow, creates window and fills it with elements, parameters are ignored */
CommandWindow::CommandWindow ( QWidget *parent/*=0*/, const char *name/*=0*/ ):QWidget(parent,name) {
 QBoxLayout * l = new QVBoxLayout( this );
 out = new QTextEdit( this );
 cmd = new QLineEdit( this , "CmdLine" );

 // init history
 history = new QComboBox( this, "CmdHistory" );
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
//todo: subclass qlineedit, add history ... 
 QObject::connect(cmd, SIGNAL(returnPressed()), this, SLOT(execute()));
 l->addWidget(out);
 l->addWidget(history);
 out->setTextFormat(LogText);
 out->setWrapPolicy(QTextEdit::AtWordOrDocumentBoundary);

 in = new QSEditor( this , "CmdEditor" );
 in->textEdit()->installEventFilter( this );
 in->textEdit()->viewport()->installEventFilter( this );
 in->setFocus();
 interpreter = NULL;
 l->addWidget( in );

 connect( history, SIGNAL( activated(int) ), this, SLOT( selectedHistoryItem(int) ));

 setCmdWindowMode( globalSettings->readNum( CMD + CMDMODE, DEFAULT__CMDMODE ) );
}

QSize CommandWindow::minimumSizeHint() const {
	QSize pom = this->QWidget::minimumSizeHint();
	pom.setWidth (std::max( out->minimumSizeHint().width(), in->minimumSizeHint().width() ));
	pom.setWidth (std::max( pom.width(), cmd->minimumSizeHint().width() ));

	return pom;
}

void CommandWindow::setInterpreter( QSInterpreter * ainterpreter, QObject * context ) {
	in->setInterpreter( ainterpreter, context );
	interpreter = ainterpreter;
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

void CommandWindow::setHistorySize( int historySize ){
	guiPrintDbg(debug::DBG_INFO,"Write historySize");
	globalSettings->write( CMD + HISTORYSIZE, historySize );
	history->setMaxCount( historySize + 1 );
}
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
/** Execute and clear current command */
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

int CommandWindow::getCmdWindowMode() {
	int h = CmdNone;
	if (history->isShown())
		h |= CmdHistory;
	if (cmd->isShown())
		h |= CmdLine;
	if (in->isShown())
		h |= CmdEditor;
	return h;
}

void CommandWindow::setCmdWindowMode( int mode ) {
	if (mode & CmdHistory)
		history->show();
	else
		history->hide();

	if (mode & CmdLine)
		cmd->show();
	else
		cmd->hide();

	if (mode & CmdEditor)
		in->show();
	else
		in->hide();
}
/** Add command executed from menu or any source to be echoed to command window */
void CommandWindow::addCommand(const QString &command) {
 out->append("<b>&gt; </b>"+htmlEnt(command));
 consoleLog("> "+command,globalSettings->readExpand("path/console_log"));
}

/** Add string to be echoed to command window */
void CommandWindow::addString(const QString &str) {
 out->append(htmlEnt(str));
}

/** Add error message to be echoed to command window */
void CommandWindow::addError(const QString &message) {
 out->append("<font color=red>! </font>"+htmlEnt(message));
 consoleLog("! "+message,globalSettings->readExpand("path/console_log"));
}

/** default destructor */
CommandWindow::~CommandWindow() {
	saveHistory();
	globalSettings->write( CMD + CMDMODE, getCmdWindowMode() );
}

} // namespace gui
