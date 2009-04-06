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
#ifndef __QTCOMPAT_H__
#define __QTCOMPAT_H__

/**
 @file
 Compatibility fixes to allow Qt3 code to work in Qt4
*/
#include <qglobal.h>

#if defined(QT_VERSION) && QT_VERSION >= 0x040000

 // QT4 or newer
#define QT4 1

#include <Qt>

/** qt3/qt4 compatibility typedef */
typedef Qt::WindowFlags WFlags;

/** Macro working in QT3 and QT4, allowing to send QString to debugging output */
#define Q_OUT(x) (x.toUtf8().data())

//Type aliases
#define Q_ButtonGroup		Q3ButtonGroup
#define Q_ComboBox		Q3ComboBox
#define Q_Dict			Q3Dict
#define Q_DictIterator		Q3DictIterator
#define Q_GroupBox		Q3GroupBox
#define Q_List			QList
#define Q_ListBox		Q3ListBox
#define Q_ListBoxItem		Q3ListBoxItem
#define Q_ListBoxText		Q3ListBoxText
#define Q_ListView		Q3ListView
#define Q_ListViewItem		Q3ListViewItem
#define Q_ListViewItemIterator	Q3ListViewItemIterator
#define Q_Painter		Q3Painter
#define Q_PopupMenu		Q3PopupMenu
#define Q_PtrCollection		Q3PtrCollection
#define Q_PtrDict		Q3PtrDict
#define Q_PtrDictIterator	Q3PtrDictIterator
#define Q_PtrList		Q3PtrList
#define Q_PtrListIterator	Q3PtrListIterator
#define Q_ScrollView		Q3ScrollView
#define Q_TextBrowser		Q3TextBrowser
#define Q_TextEdit		Q3TextEdit
#define QStrList		QList<QByteArray>
#define QMenuData		QMenu
#define QCString		QByteArray
#define QMemArray		QVector
#define IbeamCursor		IBeamCursor
#define TheWheelFocus		Qt::WheelFocus

//Include aliases
#define QBUTTONGROUP	<Q3ButtonGroup>
#define QBYTEARRAY	<QByteArray>
#define QCOMBOBOX	<Q3ComboBox>
#define QDICT		<Q3Dict>
#define QGROUPBOX	<Q3GroupBox>
#define QICON		<QIcon>
#define QLISTBOX	<Q3ListBox>
#define QLISTVIEW	<Q3ListView>
#define QPOPUPMENU	<Q3PopupMenu>
#define QPTRDICT	<Q3PtrDict>
#define QPTRLIST	<Q3PtrList>
#define QPTRCOLLECTION	<Q3PtrCollection>
#define QLIST		<QList>
#define QSCROLLVIEW	<Q3ScrollView>
#define QSTRLIST	<QList>
#define QTEXTBROWSER	<Q3TextBrowser>
#define QTEXTEDIT	<Q3TextEdit>

#else

 // QT3
#define QT3 1

/** Macro working in QT3 and QT4, allowing to send QString to debugging output */
#define Q_OUT(x) (x.utf8())

//Type aliases
#define Q_ButtonGroup		QButtonGroup
#define Q_ComboBox		QComboBox
#define Q_Dict			QDict
#define Q_DictIterator		QDictIterator
#define Q_GroupBox		QGroupBox
#define Q_List			QValueList
#define Q_ListBox		QListBox
#define Q_ListBoxItem		QListBoxItem
#define Q_ListBoxText		QListBoxText
#define Q_ListView		QListView
#define Q_ListViewItem		QListViewItem
#define Q_ListViewItemIterator	QListViewItemIterator
#define Q_Painter		QPainter
#define Q_PopupMenu		QPopupMenu
#define Q_PtrCollection		QPtrCollection
#define Q_PtrDict		QPtrDict
#define Q_PtrDictIterator	QPtrDictIterator
#define Q_PtrList		QPtrList
#define Q_PtrListIterator	QPtrListIterator
#define Q_ScrollView		QScrollView
#define Q_TextBrowser		QTextBrowser
#define Q_TextEdit		QTextEdit
//Classes renamed between Qt3 and Qt4 -> using Qt4 names
#define QIcon			QIconSet
//Similarity aliases
#define QAbstractButton		QButton
#define TheWheelFocus		QWidget::WheelFocus

//Include aliases
#define QBUTTONGROUP	<qbuttongroup.h>
#define QBYTEARRAY	<qcstring.h>
#define QCOMBOBOX	<qcombobox.h>
#define QDICT		<qdict.h>
#define QGROUPBOX	<qgroupbox.h>
#define QICON		<qiconset.h>
#define QLISTBOX	<qlistbox.h>
#define QLISTVIEW	<qlistview.h>
#define QPOPUPMENU	<qpopupmenu.h>
#define QPTRDICT	<qptrdict.h>
#define QPTRLIST	<qptrlist.h>
#define QPTRCOLLECTION	<qptrcollection.h>
#define QLIST		<qvaluelist.h>
#define QSCROLLVIEW	<qscrollview.h>
#define QSTRLIST	<qstrlist.h>
#define QTEXTBROWSER	<qtextbrowser.h>
#define QTEXTEDIT	<qtextedit.h>

#endif

#endif
