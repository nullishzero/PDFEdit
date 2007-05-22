/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "preferences.h"

#include <QFontDatabase>
#include <QColorDialog>
#include <QSettings>
#include <QPalette>

PreferencesBase::PreferencesBase(QWidget* parent, const char* name, Qt::WFlags fl)
    : QWidget(parent, fl)
{
    setObjectName(name);
    init();
}

PreferencesBase::~PreferencesBase()
{
}

void PreferencesBase::init()
{
    setupUi(this);
    QFontDatabase fdb;
    comboFamily->addItems(fdb.families());
    listElements->setCurrentItem( listElements->item(0) );
    currentElement = "";
}

void PreferencesBase::destroy()
{

}

void PreferencesBase::colorClicked()
{
    QColor c = QColorDialog::getColor( currentStyle.color, this);
    if ( c.isValid() ) {
	currentStyle.color = c;
	setColorPixmap( c );
    }
}

void PreferencesBase::reInit()
{
    styles = Config::readStyles();
    currentElement = "";
    elementChanged( tr("Comment") );
    for ( int i = 0; i < comboFamily->count(); ++i ) {
	if (listElements->item(i)->text() == tr("Comment")) {
	    listElements->setCurrentItem(listElements->item(i));
	    break;
	}
    }
    checkWordWrap->setChecked( Config::wordWrap() );
    checkCompletion->setChecked( Config::completion() );
    checkParenMatching->setChecked( Config::parenMatching() );
    spinTabSize->setValue( Config::indentTabSize() );
    spinIndentSize->setValue( Config::indentIndentSize() );
    checkKeepTabs->setChecked( Config::indentKeepTabs() );
    checkAutoIndent->setChecked( Config::indentAutoIndent() );
}

void PreferencesBase::save()
{
    if ( !currentElement.isEmpty() ) {
	styles.remove( currentElement );
	styles.insert( currentElement, currentStyle );
	currentElement = "";
    }

    QSettings settings;
    Config::saveStyles( styles );
    Config::setWordWrap( checkWordWrap->isChecked() );
    Config::setCompletion( checkCompletion->isChecked() );
    Config::setParenMatching( checkParenMatching->isChecked() );
    Config::setIndentTabSize( spinTabSize->value() );
    Config::setIndentIndentSize( spinIndentSize->value() );
    Config::setIndentKeepTabs( checkKeepTabs->isChecked() );
    Config::setIndentAutoIndent( checkAutoIndent->isChecked() );
}

void PreferencesBase::updatePreview()
{
    editPreview->setFont( currentStyle.font );
    QPalette pal = editPreview->palette();
    pal.setColor( QPalette::Active, QPalette::Text, currentStyle.color );
    pal.setColor( QPalette::Active, QPalette::Foreground, currentStyle.color );
    editPreview->setPalette( pal );
}

void PreferencesBase::boldChanged( bool b )
{
    currentStyle.font.setBold( b );
    updatePreview();
}

void PreferencesBase::elementChanged( const QString &element )
{
    if ( !currentElement.isEmpty() ) {
	styles.remove( currentElement );
	styles.insert( currentElement, currentStyle );
	currentElement = "";
    }
    QMap<QString, ConfigStyle>::Iterator it = styles.find( element );
    if ( it == styles.end() )
	return;
    ConfigStyle s = *it;
    currentStyle = s;
    comboFamily->lineEdit()->setText( s.font.family() );
    spinSize->setValue( s.font.pointSize() );
    checkBold->setChecked( s.font.bold() );
    checkItalic->setChecked( s.font.italic() );
    checkUnderline->setChecked( s.font.underline() );
    setColorPixmap( s.color );
    currentElement = element;
    updatePreview();
}

void PreferencesBase::familyChanged( const QString &f )
{
    QString oldFamily = currentStyle.font.family();
    currentStyle.font.setFamily( f );
    if ( currentElement == tr("Standard") ) {
	for ( QMap<QString, ConfigStyle>::Iterator it = styles.begin(); it != styles.end(); ++it ) {
	    if ( (*it).font.family() == oldFamily )
		(*it).font.setFamily( f );
	}
    }
    updatePreview();
}

void PreferencesBase::italicChanged( bool b )
{
    currentStyle.font.setItalic( b );
    updatePreview();
}

void PreferencesBase::setColorPixmap( const QColor &c )
{
    QPixmap pm( 20, 20 );
    pm.fill( c );
    buttonColor->setIcon( pm );
    updatePreview();
}

void PreferencesBase::sizeChanged( int s )
{
    int oldSize = currentStyle.font.pointSize();
    currentStyle.font.setPointSize( s );
    if ( currentElement == tr("Standard") ) {
	for ( QMap<QString, ConfigStyle>::Iterator it = styles.begin(); it != styles.end(); ++it ) {
	    if ( (*it).font.pointSize() == oldSize )
		(*it).font.setPointSize( s );
	}
    }
    updatePreview();
}

void PreferencesBase::underlineChanged( bool b )
{
    currentStyle.font.setUnderline( b );
    updatePreview();
}
