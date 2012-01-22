#include "Search.h"
#include <string.h>

Search::Search(QWidget * parent) : QWidget(parent, Qt::Window)
{
	ui.setupUi(this);

	connect(this->ui.stopButton, SIGNAL(pressed()), this, SLOT(stop()));
	connect(this->ui.text, SIGNAL(returnPressed()),this, SLOT(next()));
	connect(this->ui.nextButton, SIGNAL(pressed()),this, SLOT(next()));
	connect(this->ui.prevButton, SIGNAL(pressed()),this, SLOT(prev()));
}
void Search::stop()
{
	emit stopSignal();
}
void Search::showEvent ( QShowEvent * event )
{
	this->ui.text->setFocus();
}
void Search::next()
{
	emit search(ui.text->text(),getFlags() | SearchForward);
}
void Search::setFlags( int flags )
{
	this->ui.caseSensitive->setChecked(flags&SearchCaseSensitive);
	this->ui.concate->setChecked(flags&SearchConcate);
	this->ui.regexp->setChecked(flags&SearchRegexp);
}
int Search::getFlags()
{
	int flags = 0;
	flags |= this->ui.caseSensitive->isChecked()? SearchCaseSensitive : 0;
	flags |= this->ui.concate->isChecked()? SearchConcate : 0;
	flags |= this->ui.regexp->isChecked()? SearchRegexp: 0;
	return flags;
}
void Search::prev()
{
	emit search(ui.text->text(),getFlags());
}
void Search::replace()
{
//	emit replaceTextSignal(this->ui.text->toPlainText(),this->ui.replacetext->toPlainText());
}


