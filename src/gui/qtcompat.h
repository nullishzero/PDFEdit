/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
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
#define Q_PtrCollection		Q3PtrCollection
#define Q_PtrDict		Q3PtrDict
#define Q_PtrDictIterator	Q3PtrDictIterator
#define Q_PtrList		Q3PtrList
#define Q_PtrListIterator	Q3PtrListIterator

//Include aliases
#define QBUTTONGROUP	<Q3ButtonGroup>
#define QCOMBOBOX	<Q3ComboBox> 
#define QDICT		<Q3Dict>
#define QGROUPBOX	<Q3GroupBox> 
#define QLISTBOX	<Q3ListBox> 
#define QPTRDICT	<Q3PtrDict>
#define QPTRLIST	<Q3PtrList>
#define QPTRCOLLECTION	<Q3PtrCollection>
#define QLIST		<QList>

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
#define Q_PtrCollection		QPtrCollection
#define Q_PtrDict		QPtrDict
#define Q_PtrDictIterator	QPtrDictIterator
#define Q_PtrList		QPtrList
#define Q_PtrListIterator	QPtrListIterator

//Include aliases
#define QBUTTONGROUP	<qbuttongroup.h>
#define QCOMBOBOX	<qcombobox.h> 
#define QDICT		<qdict.h>
#define QGROUPBOX	<qgroupbox.h> 
#define QLISTBOX	<qlistbox.h> 
#define QPTRDICT	<qptrdict.h>
#define QPTRLIST	<qptrlist.h>
#define QPTRCOLLECTION	<qptrcollection.h>
#define QLIST		<qvaluelist.h>

#endif

#endif
