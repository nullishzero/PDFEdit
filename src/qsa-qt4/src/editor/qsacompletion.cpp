/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech AS. All rights reserved.
**
** This file is part of the QSA of the Qt Toolkit.
**
** For QSA Commercial License Holders (non-open source):
** 
** Licensees holding a valid Qt Script for Applications (QSA) License Agreement
** may use this file in accordance with the rights, responsibilities and
** obligations contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of the Licensing Agreement are
** not clear to you.
** 
** Further information about QSA licensing is available at:
** http://www.trolltech.com/products/qsa/licensing.html or by contacting
** info@trolltech.com.
** 
** 
** For Open Source Edition:  
** 
** This file may be used under the terms of the GNU General Public License
** version 2 as published by the Free Software Foundation and appearing in the
** file LICENSE.GPL included in the packaging of this file.  Please review the
** following information to ensure GNU General Public Licensing requirements
** will be met:  http://www.trolltech.com/products/qt/opensource.html 
** 
** If you are unsure which license is appropriate for your use, please review
** the following information:
** http://www.trolltech.com/products/qsa/licensing.html or contact the 
** sales department at sales@trolltech.com.

**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsacompletion.h"
#include "qsinterpreter.h"
#include "qsasyntaxhighlighter.h"
#include "qsdate_object.h"
#include "qsproject.h"
#include "quickbytearrayobject.h"
#include "quickclassparser.h"
#include "quickcolorobject.h"
#include "quickcoordobjects.h"
#include "quickfontobject.h"
#include "quickinterpreter.h"
#include "quickobjects.h"
#include "quickpixmapobject.h"

#include <qobject.h>
#include <qmetaobject.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3StrList>
#include <Q3ValueList>

#include "q3richtext_p.h"

#include <editor.h>

// #define QSA_COMPLETION_DEBUG

QVector<QObject *>* QSACompletion::interfaceObjects( const QSObject &o ) const
{
    return interpreter()->wrapperClass()->objectVector( &o );
}

// =============================================================================
// =============================================================================
// == Functions to set return completion information
// =============================================================================
// =============================================================================

struct Property
{
    QString name;
    QString type;
    bool operator==( const Property &p ) const {
	    return name == p.name && type == p.type;
    }
};

static void getSlots( const QMetaObject *meta, Q3ValueList<Property> &result,
		      bool super, bool withArgs, bool sigs )
{
    while (meta) {
        int nmethods = meta->methodCount();
        for (int i=0; i<nmethods; ++i) {
            const QMetaMethod m = meta->method(i);

            if (m.access() == QMetaMethod::Private)
                continue ;

            if ((m.methodType() == QMetaMethod::Slot && !sigs) ||
                (m.methodType() == QMetaMethod::Signal && sigs)) {
	            Property prop;
	            QString s = QLatin1String( m.signature() );
	            s = s.left( s.find( '(' ) );

                QList<QByteArray> parameterTypes = m.parameterTypes();
                QList<QByteArray> parameterNames = m.parameterNames();
                prop.type = m.typeName();
                QuickInterpreter::cleanType(prop.type);
	            if ( withArgs ) {
	                s += QLatin1String("(");

	                for ( int j = 0; j < parameterTypes.count(); ++j ) {
		                s += parameterTypes.at(j);
                        s += QLatin1String(" ");
		                s += QString::fromLatin1(parameterNames.at(j));
		                if ( j < parameterTypes.count() - 1 )
		                    s += QString::fromLatin1(",");
	                }
	                s += QString::fromLatin1(")");
	            }

	            prop.name = s;
	            if ( result.find( prop ) == result.end() )
	                result << prop;
            }
        }

        if (super)
            meta = meta->superClass();
        else
            meta = 0;
    }
}

static void addLayoutChildren( QObject *o, Q3ValueList<CompletionEntry> &res )
{
    QObjectList l = o->children();
    for (int i=0; i<l.size(); ++i) {
        QObject *o = l.at(i);
        CompletionEntry c;
        c.type = o->isWidgetType() ? "widget" : "object";
        c.text = o->objectName();
        c.postfix2 = o->metaObject()->className();
        if ( !c.postfix2.isEmpty() )
            c.postfix2.prepend( QString::fromLatin1(" : ") );
        res << c;
    }
}

static QStringList getArguments( const QString &s )
{
    QString str = s.mid( s.find( QString::fromLatin1("(") ) + 1, s.find( QString::fromLatin1(")") ) - 1 - s.find( QString::fromLatin1("(") ) );
    str = str.simplifyWhiteSpace();
    QStringList lst = QStringList::split( ',', str );
    QStringList res;
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	QString arg = *it;
	arg = arg.replace( QRegExp( QString::fromLatin1("const") ), QString::fromLatin1("") );
	arg = arg.replace( QRegExp( QString::fromLatin1("&") ), QString::fromLatin1("") );
	arg = arg.replace( QRegExp( QString::fromLatin1("*") ), QString::fromLatin1("") );
	arg = arg.simplifyWhiteSpace();
	res << arg;
    }
    return res;
}

void QSACompletion::completeQSObject( QSObject &obj, Q3ValueList<CompletionEntry> &res, bool includeMembers )
{
    QStringList funcs = interpreter()->functionsOf( obj, true, true, includeMembers );
    QStringList::Iterator it;
    for ( it = funcs.begin(); it != funcs.end(); ++it ) {
	    CompletionEntry c;
	    c.type = "function";
	    c.text = *it;
	    c.prefix = "";
	    c.postfix2 = "";
	    res << c;
    }
    QStringList vars = interpreter()->variablesOf( obj, true, true, includeMembers );
    for ( it = vars.begin(); it != vars.end(); ++it ) {
	    CompletionEntry c;
	    c.type = "variable";
	    c.text = *it;
	    c.prefix = "";
	    c.postfix2 = "";
	    res << c;
    }
    QStringList classes = interpreter()->classesOf( obj );
    for ( it = classes.begin(); it != classes.end(); ++it ) {
	    CompletionEntry c;
	    c.type = "class";
	    c.text = *it;
	    c.prefix = "";
	    c.postfix2 = "";
	    res << c;
    }
}

void QSACompletion::completeQMetaObject( const QMetaObject *meta,
					 const QString &,
					 Q3ValueList<CompletionEntry> &res,
					 int flags,
					 QSObject &obj )
{
    QMap<QString, bool> propMap;
    bool includeSuperClass = (flags & IncludeSuperClass) == IncludeSuperClass;

    // properties
    const QMetaObject *m = meta;
    int num = m->propertyCount();

    for ( int j = 0; j < num; ++j ) {
        const QMetaProperty mp = m->property( j );
        if ( propMap.find( QString::fromLatin1(mp.name()) ) != propMap.end() )
            continue;
        CompletionEntry c;
        propMap.replace(QLatin1String(mp.name()), false);
        c.type = QLatin1String("property");
        c.text = mp.name();
        c.prefix = QString();
        c.postfix2 = mp.typeName();
        QuickInterpreter::cleanType( c.postfix2 );
        if ( !c.postfix2.isEmpty() )
            c.postfix2.prepend( QLatin1String(" : ") );
        res.append( c );
    }

    if ( includeSuperClass && obj.isValid() && !obj.isUndefined() ) {
        QStringList vars = interpreter()->variablesOf( obj, true );
        QStringList::iterator it;
        for ( it = vars.begin(); it != vars.end(); ++it ) {
            CompletionEntry c;
            c.type = QLatin1String("variable");
            c.text = *it;
            c.prefix = QString();
            c.postfix2 = QString();
            res << c;
        }
    }

    // functions
    Q3ValueList<Property> lst;
    Q3ValueList<Property>::Iterator pit;
    getSlots( meta, lst, includeSuperClass, false, false );
    for ( pit  = lst.begin(); pit != lst.end(); ++pit ) {
        CompletionEntry c;
        c.type = QLatin1String("function");
        c.text = (*pit).name;
        c.postfix = QLatin1String("()");
        c.postfix2 = (*pit).type;
        if ( !c.postfix2.isEmpty() )
            c.postfix2.prepend( QString::fromLatin1(" : ") );
        res << c;
    }
    if ( includeSuperClass && obj.isValid() && !obj.isUndefined() ) {
        QStringList funcs = interpreter()->functionsOf( obj, true, true, true );
        QStringList::Iterator it;
        for ( it = funcs.begin(); it != funcs.end(); ++it ) {
            CompletionEntry c;
            c.type = QLatin1String("function");
            c.text = *it;
            c.prefix = QString();
            c.postfix2 = QString();
            res << c;
        }
    }

    // enum values

    m = meta;
    for (int k=0; k<m->enumeratorCount(); ++k) {
        QMetaEnum me = m->enumerator(k);
        for (int l=0; l<me.keyCount(); ++l) {
            CompletionEntry c;
            c.type = QLatin1String("enum");
            c.text = QLatin1String(me.key(l));
            c.prefix = QString();
            c.postfix2 = QLatin1String(me.name());

            if (!c.postfix2.isEmpty())
                c.postfix2.prepend( QString::fromLatin1(" : ") );
            res << c;
        }
    }

    if ( includeSuperClass && obj.isValid() && !obj.isUndefined() ) {
        QStringList classes = interpreter()->classesOf( obj );
        QStringList::Iterator it;
        for ( it = classes.begin(); it != classes.end(); ++it ) {
            CompletionEntry c;
            c.type = QLatin1String("class");
            c.text = *it;
            c.prefix = QString();
            c.postfix2 = QString();
            res << c;
        }
    }
}

void QSACompletion::completeQObject(const QVector<QObject *> &objects,
                                    const QString &object,
                                    Q3ValueList<CompletionEntry> &res)
{
    for ( int i = 0; i < objects.count(); i++ ) {
        QObject *qobj = objects[ i ];
        if ( !qobj )
            continue;
        // children
        QObjectList clist;
        if ( qobj == qApp )
            clist = interpreter()->topLevelObjects() != 0 ?
                    *((QObjectList*)interpreter()->topLevelObjects()) :
                    QObjectList();
        else
            clist = qobj->children();

        if ( !clist.isEmpty() ) {
            for (int ci = 0; ci<clist.size(); ++ci) {
                const QObject *o = clist.at(ci);
                CompletionEntry c;
                c.type = o->isWidgetType() ? "widget" : "object";
                c.text = o->objectName();
                c.postfix2 = o->metaObject()->className();
                if ( !c.postfix2.isEmpty() )
                    c.postfix2.prepend( QString::fromLatin1(" : ") );
                res << c;
            }
        }

        QSObject qsobj = interpreter()->wrap( qobj );
        int flags = 0;
        if ( i == 0 )
            flags |= IncludeSuperClass;
        completeQMetaObject( qobj->metaObject(),
                             object,
                             res,
                             flags,
                             qsobj
                             );

    }
}

// =============================================================================
// =============================================================================
// == Parser to resolve assignments in Qt Script Code
// =============================================================================
// =============================================================================

QString QSACompletion::resolveValue( const QString &value, const Q3ValueList<QPair<QString, QString> > &assignments ) const
{
    for ( Q3ValueList<QPair<QString, QString> >::ConstIterator it = assignments.begin(); it != assignments.end(); ++it ) {
	if ( (*it).first == value )
	    return (*it).second;
    }
    return QString::null;
}

QString qsa_strip_open_parenthesis(const QString &str, char type) {
    int pos = str.lastIndexOf(type);
    if (pos >= 0)
        return str.mid(pos + 1);
    return str;
}

QString qsa_strip_down(const QString &str, char start, char stop)
{
    // Match inside paranthesis
    QString s = str;
    int pos = str.lastIndexOf(start);
    if (pos >= 0) {
        int end = str.indexOf(stop, pos+1);
        if (end < 0)
            s = str.mid(pos+1);
    } else {
        s = str;
    }

    s = qsa_strip_open_parenthesis(s, start);
    return s;
}

QString QSACompletion::resolveFullyQualifiedValue(const QString &value,
                                                  const Q3ValueList<QPair<QString, QString> > &assignments ) const
{
    QString val = value;

    // Only resolve the last statement.
    int pos = val.findRev(';');
    if (pos > 0)
        val = val.mid(pos+1);

    val = qsa_strip_down(val, '(', ')');
    val = qsa_strip_down(val, '{', '}');

    QStringList l = QStringList::split( '.', val);
    QString valuePart;
    for ( QStringList::const_iterator vit = l.begin(); vit != l.end(); ++vit ) {
	    if ( !valuePart.isNull() )
	        valuePart += QString::fromLatin1(".");
	    valuePart += (*vit).left( (*vit).find( '(' ) );
	    QString replacedValue;
	    while ( ( replacedValue = resolveValue( valuePart, assignments ) ) != QString::null )
	        valuePart = replacedValue;
    }

    return valuePart;
}

#define APPEND_PARSED_CHAR( s ) \
do { \
	if ( s == LeftHandSide ) \
	    leftHandBuffer += c; \
	else if ( s == RightHandSight ) \
	    rightHandBuffer += c; \
} while ( false )


Q3ValueList<QPair<QString, QString> > QSACompletion::parseAssignments( const QString &code ) const
{
    QChar c, last;
    enum State { LeftHandSide, RightHandSight, Comment, String, Parentheses } state = LeftHandSide;
    int parenCount = 0;
    State lastState = LeftHandSide; // initialize to make compilers happy
    QChar ignoreEnd[2];
    QString leftHandBuffer, rightHandBuffer;
    Q3ValueList<QPair<QString, QString> > assignments;

    for ( int i = 0; i < code.length(); ++i ) {
	    last = c;
	    c = code[ (int) i];

	    if ( state == Comment || state == String || state == Parentheses ) {
	        if ( state == String )
		        APPEND_PARSED_CHAR( lastState );
	            if ( c == '(' && state == Parentheses ) {
		        parenCount++;
		        continue;
	        }
	        if ( !ignoreEnd[1].isNull() ) {
		        if ( last == ignoreEnd[0] && c == ignoreEnd[1] )
		            state = (state == String ? lastState : LeftHandSide);
	        } else if ( c == ignoreEnd[0] ) {
		        if ( state == Parentheses ) {
		            parenCount--;
		            if ( parenCount > 0 )
			        continue;
		        }
    		    state = ( (state == String || state == Parentheses) ? lastState : LeftHandSide );
	        }
	        continue;
        }

	    if ( c == '*' && last == '/' ) {
	        state = Comment;
	        ignoreEnd[0] = '*';
	        ignoreEnd[1] = '/';
	        leftHandBuffer = QString::null;
	        rightHandBuffer = QString::null;
	        continue;
	    } else if ( c == '/' && last == '/' ) {
	        state = Comment;
	        ignoreEnd[0] = '\n';
	        ignoreEnd[1] = QChar::Null;
	        leftHandBuffer = QString::null;
	        rightHandBuffer = QString::null;
	        continue;
	    } else if ( c == '\"' ) {
	        lastState = state;
	        state = String;
	        ignoreEnd[0] = '\"';
	        ignoreEnd[1] = QChar::Null;
	        APPEND_PARSED_CHAR( lastState );
	        continue;
	    } else if ( c == '\'' ) {
	        lastState = state;
	        state = String;
	        ignoreEnd[0] = '\'';
	        ignoreEnd[1] = QChar::Null;
	        APPEND_PARSED_CHAR( lastState );
	        continue;
	    } else if ( c == '(' ) {
	        lastState = state;
	        state = Parentheses;
	        ignoreEnd[0] = ')';
	        ignoreEnd[1] = QChar::Null;
	        parenCount = 1;
	        continue;
	    }

        if ( last.isSpace() ) {
    	    if ( i > 1 && code[ (int)(i-2) ] != '.' && c != '=' && c != ';' && c != '{' && c != '}' && c != '(' && c != ')' ) {
		        if ( state == LeftHandSide )
		            leftHandBuffer = QString::null;
		        else if ( state == RightHandSight )
		            rightHandBuffer = QString::null;
	        }
        }


	    if ( c == ';' || c == '{' || c == '}' ) {
	        if ( state == LeftHandSide ) {
    		    leftHandBuffer = QString::null;
	        } else if ( state == RightHandSight ) {
		        rightHandBuffer = rightHandBuffer.replace( QRegExp( QString::fromLatin1("\\s") ), QString::fromLatin1("") );
		        leftHandBuffer = leftHandBuffer.replace( QRegExp( QString::fromLatin1("\\s") ), QString::fromLatin1("") );
		        QPair<QString, QString> p;
		        p.first = leftHandBuffer;
		        p.second = rightHandBuffer;
		        assignments.prepend( p );
		        leftHandBuffer = QString::null;
		        rightHandBuffer = QString::null;
		        state = LeftHandSide;
		        continue;
	        }
        }

	    if ( c == '=' ) {
	        if ( last == '!' || last == '=' ) {
		        leftHandBuffer = QString::null;
		        rightHandBuffer = QString::null;
		        state = LeftHandSide;
		        continue;
	        }
	        if ( state == RightHandSight ) {
		        leftHandBuffer = QString::null;
		        rightHandBuffer = QString::null;
		        state = LeftHandSide;
	        } else if ( state == LeftHandSide ) {
		        state = RightHandSight;
	        }
    	    continue;
        }

	    APPEND_PARSED_CHAR( state );
    }

    for ( Q3ValueList<QPair<QString, QString> >::Iterator it = assignments.begin(); it != assignments.end(); ++it ) {
	    QString key = (*it).first;
	    QString value = (*it).second;
	    QStringList l = QStringList::split( '.', value );
	    QString valuePart;
	    for ( QStringList::ConstIterator vit = l.begin(); vit != l.end(); ++vit ) {
	        if ( !valuePart.isNull() )
		        valuePart += QString::fromLatin1(".");
	        valuePart += *vit;
	        QString replacedValue;
	        int counter = 0;
	        while ( ( replacedValue = resolveValue( valuePart, assignments ) ) != QString::null ) {
		        if( ++counter > 1000 ) // Avoid recursion...
		            return Q3ValueList<QPair<QString,QString> >();
    		    valuePart = replacedValue;
	        }
	        (*it).second = valuePart;
        }
    }

    return assignments;
}

// =============================================================================
// =============================================================================
// == Function to return the code which should be parsed for assignments
// =============================================================================
// =============================================================================

QString QSACompletion::functionCode() const
{
    Q3TextParagraph *p = curEditor->textCursor()->paragraph();
    QString funcName;
    int pos;
    int braceCount = -1;
    QString s;
    while ( p && ( pos = p->string()->toString().find( QString::fromLatin1("function") ) ) == -1 ) {
	s = p->string()->toString();
	braceCount += s.count( '{' );
	braceCount -= s.count( '}' );
	p = p->prev();
    }
    if ( p ) {
	s = p->string()->toString();
	braceCount += s.count( '{' );
	braceCount -= s.count( '}' );
    }

    if ( p && pos != -1 && braceCount >= 0 ) {
	funcName = p->string()->toString().mid( pos + 9 ).simplifyWhiteSpace();
	funcName = funcName.left( funcName.find( '(' ) );
    }

    QuickClassParser parser;
    parser.parse( curEditor->text() );
    Q3ValueList<QuickClass> classes = parser.classes();
    bool global = funcName.isNull() || !p;
    QString code;
    for ( Q3ValueList<QuickClass>::ConstIterator it = classes.begin(); it != classes.end(); ++it ) {
	if ( (*it).type == QuickClass::Global ) {
	    for ( QStringList::ConstIterator vit = (*it).variables.begin(); vit != (*it).variables.end(); ++vit )
		code += *vit + QString::fromLatin1(";\n");
	    code += QString::fromLatin1("\n");
	    if ( global )
		break;
	}

	if ( global )
	    continue;

	for ( QList<LanguageInterface::Function>::ConstIterator fit = (*it).functions.begin();
	      fit != (*it).functions.end(); ++fit ) {
	    if ( (*fit).name.left( (*fit).name.find( '(' ) ) == funcName ) {
		if ( (*it).type != QuickClass::Global ) {
		    for ( QStringList::ConstIterator vit = (*it).variables.begin(); vit != (*it).variables.end(); ++vit )
			code += *vit + QString::fromLatin1(";\n");
		    code += QString::fromLatin1("\n");
		}
		code += QString::fromLatin1("\n") + (*fit).body + QString::fromLatin1("\n");
		break;
	    }
	}
    }

    return code;
}

// =============================================================================
// =============================================================================
// == QSACompletion
// =============================================================================
// =============================================================================

QSACompletion::QSACompletion( Editor *e )
    : EditorCompletion( e ),
      thisObject( 0 ),
      qsInterp( 0 )
{
    int i = 0;
    while ( QString::fromLatin1(QSASyntaxHighlighter::keywords[ i ]) != QString::null )
	    addCompletionEntry( QString::fromLatin1(QSASyntaxHighlighter::keywords[ i++ ]), 0, false );
}

extern QuickInterpreter *get_quick_interpreter( QSInterpreter * );

QSEnv *QSACompletion::env() const
{
#if defined ( QSA_COMPLETION_DEBUG )
    if( !qsInterp ) printf( "QSACompletion::interpreter(), using defaultInterpreter\n" );
#endif
    return qsInterp ? get_quick_interpreter( qsInterp )->env()
	: get_quick_interpreter( QSInterpreter::defaultInterpreter() )->env();
}

QuickInterpreter *QSACompletion::interpreter() const
{
#if defined ( QSA_COMPLETION_DEBUG )
    if( !qsInterp ) printf( "QSACompletion::interpreter(), using defaultInterpreter\n" );
#endif
    return qsInterp ? get_quick_interpreter( qsInterp )
	: get_quick_interpreter( QSInterpreter::defaultInterpreter() );
}

bool QSACompletion::doObjectCompletion( const QString &obj )
{
    QString object = resolveFullyQualifiedValue( obj, parseAssignments( functionCode() ) );

#if defined ( QSA_COMPLETION_DEBUG )
    printf( "QSACompletion::doObjectCompletion(): %s -> %s\n",
	    obj.latin1(), object.latin1() );
#endif

    bool assumedStatic = false;
    QSCompletionObject o;
    if( obj == object ) {
        QSObject stobj = env()->globalObject().get( obj );
        if( stobj.isValid() && stobj.objectType()->valueType() == TypeClass ) {
#if defined ( QSA_COMPLETION_DEBUG )
    	    printf( " -> assuming static\n" );
#endif
            o = stobj;
            assumedStatic = true;
        }
    }
    if( o.type == QSCompletionObject::TNull ) {
        o = queryObject( object );
    }

#if defined ( QSA_COMPLETION_DEBUG )
    printf( " -> type is: %d\n", o.type );
#endif

    o.resolve();
    if ( o.isNull() ) {
        return false;
    }

    Q3ValueList<CompletionEntry> res;

    QSObject nullObject;
    switch ( o.type ) {
    case QSCompletionObject::TQSObject:
#if defined ( QSA_COMPLETION_DEBUG )
        printf( " -> objectType is: %s\n", o.qsobj.objectType()->name().latin1() );
#endif
        if( o.qsobj.objectType()->name() == QString::fromLatin1("FactoryObject") ) {
            QSObject sinst = ( (QSFactoryObjectProxy*) o.qsobj.objectType() )->staticInstance();
            if( !sinst.isValid() ) {
                return false;
            }
            QSWrapperShared *shared = (QSWrapperShared*) sinst.shVal();
            completeQObject( shared->objects, object, res );
            break;
        }
        completeQSObject( o.qsobj, res, !assumedStatic );
        break;
    case QSCompletionObject::TQMetaObject:
        completeQMetaObject( o.meta, object, res, IncludeSuperClass, nullObject );
        break;
    case QSCompletionObject::TQObject:
        completeQObject( o.qobj, object, res );
        break;
    case QSCompletionObject::TNull:
        break;
    }

    if ( !res.isEmpty() )
	showCompletion( res );

    return true;
}

Q3ValueList<QStringList> QSACompletion::functionParameters( const QString &f,
							     QChar &separator,
							     QString &,
							     QString &postfix )
{
     Q3ValueList<QStringList> l;

    separator = ',';

    // some hardcoded ones
    if ( f == QString::fromLatin1("startTimer") ) {
	QStringList args;
	args << QString::fromLatin1("interval : Number");
	args << QString::fromLatin1("callback : Function");
	postfix = QString::fromLatin1(" : Number");
	l << args;
	return l;
    } else if ( f == QString::fromLatin1("killTimer") ) {
	QStringList args;
	args << QString::fromLatin1("id : Number");
	l << args;
	return l;
    } else if ( f == QString::fromLatin1("connect") ) {
	QStringList args;
	args << QString::fromLatin1("sender : QObject");
	args << QString::fromLatin1("signal : String");
	args << QString::fromLatin1("receiver : QObject");
	args << QString::fromLatin1("slot : String");
	l << args;
	args.clear();
	args << QString::fromLatin1("sender : QObject");
	args << QString::fromLatin1("signal : String");
	args << QString::fromLatin1("receiver : Function");
	l << args;
	return l;
    }

    QString object = f;
    int pnt = object.findRev( '.' );
    if ( pnt == -1 ) {
	if ( !thisObject )
	    return l;
	object.prepend( QString::fromLatin1("this.") );
	pnt = object.findRev( '.' );
    }

    QString returnType;
    QString func = object.mid( pnt + 1 );
    object = resolveFullyQualifiedValue( object.left( pnt ), parseAssignments( functionCode() ) );
    QSCompletionObject obj = queryObject( object );
    if( obj.qsobj.isValid()
        && obj.qsobj.objectType()->name() == QString::fromLatin1("FactoryObject") ) {
	obj = ( (QSFactoryObjectProxy*) obj.qsobj.objectType() )
	      ->staticInstance();
	if( !obj.qsobj.isValid() )
	    return Q3ValueList<QStringList>();
    }

    obj.resolve();
    if ( obj.isNull() || obj.type == QSCompletionObject::TQSObject ) {
	return l;
    }

    Q3ValueList<Property> res2;
    int i;

    switch ( obj.type ) {
    case QSCompletionObject::TQObject:
	for ( i = obj.qobj.size() - 1; i >= 0; --i )
	    getSlots( obj.qobj[i]->metaObject(), res2, i == 0, true, false );
	break;
    case QSCompletionObject::TQMetaObject:
	getSlots( obj.meta, res2, true, true, false );
	break;
    default:
	break;
    }

    QStringList funcs;
    for ( Q3ValueList<Property>::Iterator it = res2.begin(); it != res2.end(); ++it ) {
	if ( (*it).name.left( (*it).name.find( '(' ) ) == func ) {
	    returnType = (*it).type;
	    funcs << (*it).name;
	}
    }

    if ( funcs.isEmpty() ) {
	if ( f[ 0 ] == '\"' ) {
	    Q3TextCursor *cursor = curEditor->textCursor();
	    QString line = cursor->paragraph()->string()->toString();
	    if ( line.findRev( QString::fromLatin1("connect("), cursor->index() ) != -1 ) {
		int i = line.findRev( ',' );
		static QString legalChars = QString::fromLatin1("abcdefghijklmnopqrstuvwxyzABSCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.");
		int end;
		for ( end = i; end >= 0; --end ) {
		    if ( legalChars.find( line[ end ] ) != -1 )
			break;
		}
		int begin;
		for ( begin = end; begin >= 0; --begin ) {
		    if ( legalChars.find( line[ begin ] ) == -1 ) {
			begin++;
			break;
		    }
		}
		QString obj_str = line.mid( begin, end - begin + 1 );
		func = f;
		func.remove( 0, 1 );
		func = func.left( func.find( QString::fromLatin1("(") ) );
		obj = queryObject( obj_str );
		obj.resolve();
		if ( obj.isNull() || obj.type == QSCompletionObject::TQSObject )
		    return Q3ValueList<QStringList>();
		Q3ValueList<Property> res2;
		int j;

		switch ( obj.type ) {
		case QSCompletionObject::TQObject:
		    for ( j = obj.qobj.size() - 1; j >= 0; --j ) {
			getSlots( obj.qobj[j]->metaObject(), res2, j == 0, true, false );
			getSlots( obj.qobj[j]->metaObject(), res2, j == 0, true, true );
		    }
		    break;
		case QSCompletionObject::TQMetaObject:
		    getSlots( obj.meta, res2, true, true, false );
		    getSlots( obj.meta, res2, true, true, true );
		    break;
		default:
		    break;
		}

		for ( Q3ValueList<Property>::Iterator it = res2.begin(); it != res2.end(); ++it ) {
		    if ( (*it).name.left( (*it).name.find( '(' ) ) == func ) {
			returnType = (*it).type;
			funcs << (*it).name;
		    }
		}
		if ( funcs.isEmpty() )
		    return Q3ValueList<QStringList>();
	    }
	} else {
	    return Q3ValueList<QStringList>();
	}
    }

    for ( QStringList::Iterator fit = funcs.begin(); fit != funcs.end(); ++fit ) {
	QString fun = *fit;
	QStringList argList = getArguments( fun );

	QStringList res;

	for ( QStringList::Iterator it2 = argList.begin(); it2 != argList.end(); ++it2 ) {
	    QStringList l = QStringList::split( ' ', *it2 );
	    QString type = l[ 0 ];
	    QuickInterpreter::cleanType( type );
	    if ( l.count() > 1 )
		type = l[ 1 ] + QString::fromLatin1(" : ") + type;
	    res << type;
	}

	if ( !returnType.isEmpty() )
	    postfix = QString::fromLatin1(" : ") + returnType;

	l << res;
    }

    return l;
}

void QSACompletion::setContext( QObject *this_ )
{
    thisObject = this_;
}

QSObject QSACompletion::vTypeToQSType( const QString &type ) const
{
    QSObject qstype = env()->globalObject().get( type.mid( 1 ) );
    if( qstype.objectType() && qstype.objectType()->valueType() == TypeClass ) {
	    return qstype;
    }

    QVariant::Type t = QVariant::nameToType( type.latin1() );
    switch ( t ) {
    case QVariant::Time:
    case QVariant::DateTime:
	    return env()->globalObject().get( QString::fromLatin1("Date") );
    case QVariant::StringList:
    case QVariant::List:
    case QVariant::Map:
	    return env()->globalObject().get( QString::fromLatin1("Array") );
    case QVariant::CString:
	    return env()->globalObject().get( QString::fromLatin1("String") );
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Double:
	    return env()->globalObject().get( QString::fromLatin1("Number") );
    case QVariant::Bool:
	    return env()->globalObject().get( QString::fromLatin1("Boolean") );
    default:
	    break;
    }
    return env()->createUndefined();
}

QString QSACompletion::cppClassForScript( const QString &className ) const
{
#ifdef QSA_COMPLETION_DEBUG
    printf("QSACompletion::cppClassForScript( %s ), interpreter: %p\n",
	   className.latin1(), qsInterp);
#endif
    QuickInterpreter *ip = get_quick_interpreter( qsInterp
						  ? qsInterp
						  : QSInterpreter::defaultInterpreter() );
    if( !ip )
	return QString::null;

    QMap<QString,QString> names = ip->dispatchObjectFactory()->instanceDescriptors();
    return names.value(className);
}

const QMetaObject *QSACompletion::locateMetaObject( const QString &name ) const
{
    QString cppName = cppClassForScript( name );
    if (!cppName.isNull()) {
        extern const QMetaObject *qsa_query_meta_object(const QByteArray &name);
        const QMetaObject *meta = qsa_query_meta_object(cppName.toLatin1());
        return meta;
    }

    return 0;
}

QSCompletionObject QSACompletion::queryObject( const QString &object )
{
    QStringList l = QStringList::split( '.', object );

    QSCompletionObject ctx( env()->currentScope() );
    if ( thisObject )
	    ctx = interpreter()->wrap( thisObject );

    bool end = false;
    bool first = true;
    for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ) {
	    QString s = *it;
	    first = it == l.begin();
	    ++it;
	    end = it == l.end();

	    if ( first && s == QString::fromLatin1("this") ) {
	        if ( end ) {
		        if ( thisObject )
		            return QSCompletionObject( interpreter()->wrap( thisObject ) );
		        return QSCompletionObject( env()->currentScope() );
	        }
	        continue;
	    }

	    ctx.resolve();
	    QSCompletionObject obj = queryCompletionObject( ctx, s );
	    if ( obj.isNull() && first && thisObject ) {
	        ctx = env()->currentScope();
	        obj = queryCompletionObject( ctx, s );
	    }

	    obj.resolve();
	    if ( end ) {
	        return obj;
            }
	    else if ( !obj.isNull() )
	        ctx = obj;
	    else
	        return QSCompletionObject();
    }

    return QSCompletionObject();
}

QSCompletionObject QSACompletion::queryCompletionObject(QSCompletionObject &ctx,
                                                        const QString &property) const
{
    QString s = property;

    // this is always an Array
    if ( s.find( '[' ) != -1 )
    	return QSArray( env() );

    const QMetaObject *haveAsMeta = 0;
    if ( ctx.type == QSCompletionObject::TQSObject &&
	     ctx.qsobj.isA( env()->globalClass() ) ) {
	    haveAsMeta = locateMetaObject( s );
    }

    QSCompletionObject obj;

    if ( ctx.type == QSCompletionObject::TQSObject ) {
        obj = queryQSObject( ctx.qsobj, property );
        if ( obj.isNull() )
            obj.resolve();
    }

    if ( ctx.type == QSCompletionObject::TQMetaObject ) {
	    obj = queryQMetaObject( ctx.meta, property, true );
	    if ( obj.isNull() )
	        obj = queryQSObject( ctx.meta, property, true );
    }

    if ( ctx.type == QSCompletionObject::TQObject ) {
	    obj = queryQObject( ctx.qobj, property );
	    if ( obj.isNull() )
	        obj = queryQMetaObject( ctx.qobj, property );
	    if ( obj.isNull() )
	        obj = queryQSObject( ctx.qobj, property );
    }

    if ( obj.isNull() && ctx.type == QSCompletionObject::TQSObject &&
     	 ctx.qsobj.isA( env()->globalClass() ) ) {
	    const QMetaObject *meta = locateMetaObject( s );
	    if ( meta )
	        return QSCompletionObject( meta );
    }

    if ( haveAsMeta ) {
	    QSCompletionObject o = obj;
	    o.resolve();
	    if ( o.type == QSCompletionObject::TQSObject )
	        return haveAsMeta;
    }

    return obj;
}

QSObject QSACompletion::queryQSObject( QSObject &ctx, const QString &property ) const
{
    QString s = property;

    QSObject ctxValue = ctx.get( s );
    if ( ctxValue.isA( env()->typeClass() ) ) {
	return ctxValue;
    }

    if ( s.find( '[' ) != -1 )
	return QSArray( env() );
    else if ( s == QString::fromLatin1("false") || s == QString::fromLatin1("true") )
	return env()->createBoolean( false );
    else if ( s[0] == '\'' || s[0] == '\"' )
	return env()->createString( QString::fromLatin1("") );
    return ctxValue;
}

QSObject QSACompletion::queryQSObject( const QMetaObject *meta, const QString &property, bool /*includeSuperClass*/ ) const
{
    int propertyIndex = -1;
    const QMetaObject *m = meta;
    propertyIndex = m->indexOfProperty(property.toLatin1().constData());

    if (propertyIndex >= 0) {
        QMetaProperty mp = m->property(propertyIndex);
        QSObject o = vTypeToQSType( QString::fromLatin1(mp.typeName()) );
	    if ( !o.isNull() && !o.isUndefined() )
	        return o;
    }

    m = meta;
    for (int i=0; i<m->methodCount(); ++i) {
        QMetaMethod mm = m->method(i);

        if (mm.methodType() == QMetaMethod::Slot) {
            QString n = QLatin1String(mm.signature());
	        n = n.left( n.find( '(' ) );
	        if ( property != n )
		        continue;

            return vTypeToQSType(mm.typeName());
        }
    }

    return env()->createUndefined();
}

QSObject QSACompletion::queryQSObject( const QVector<QObject *> &objects, const QString &property ) const
{
    for ( int i = 0; i < objects.count(); i++ ) {
	const QMetaObject *mo = objects[i]->metaObject();
	QSObject qso = queryQSObject( mo, property, i == 0 );
	if ( !qso.isNull() && !qso.isUndefined() )
	    return qso;
    }
    return env()->createUndefined();
}

const QMetaObject *QSACompletion::queryQMetaObject( const QMetaObject *meta,
						      const QString &property,
						      bool /*includeSuperClass*/ ) const
{
    const QMetaObject *m = meta;
    for (int i=0; i<m->methodCount(); ++i) {
        QMetaMethod mm = m->method(i);
        if (mm.methodType() == QMetaMethod::Slot) {
            QString n = QLatin1String(mm.signature());
            n = n.left(n.find('('));

            if (property != n)
                continue ;

            QByteArray retType(mm.typeName());

            if (retType.count('*') == 1) {
                extern const QMetaObject *qsa_query_meta_object(const QByteArray &name);
                return qsa_query_meta_object(qsa_strip_stars(retType));
            }
        }
    }

    return 0;
}

const QMetaObject *QSACompletion::queryQMetaObject( const QVector<QObject *> &objects, const QString &property ) const
{
    for ( int i = 0; i < objects.count(); i++ ) {
	    const QMetaObject *mo = objects[i]->metaObject();
	    const QMetaObject *meta = queryQMetaObject( mo, property, i == 0 );
	    if ( meta )
	        return meta;
    }
    return 0;
}

QVector<QObject *> QSACompletion::queryQObject( const QVector<QObject *> &objects, const QString &property ) const
{
    QSObject obj = interpreter()->wrap( objects[0] );

    if ( obj.isA( interpreter()->wrapperClass() ) ) {
	QSMember m;
	if ( interpreter()->wrapperClass()->member( &obj, property, &m ) ) {
	    QSObject o = interpreter()->wrapperClass()->fetchValue( &obj, m );
	    if ( o.isA( interpreter()->wrapperClass() ) )
		return *interfaceObjects( o );
	} else if ( interpreter()->applicationClass()->member( &obj, property, &m ) ) {
	    QSObject o = interpreter()->applicationClass()->fetchValue( &obj, m );
	    if ( o.isA( interpreter()->wrapperClass() ) )
		return *interfaceObjects( o );
	}
    }

    return QVector<QObject *>();
}

// =============================================================================
// =============================================================================
// == QSCompletionObject
// =============================================================================
// =============================================================================

void QSCompletionObject::resolve()
{
    if ( type == TQSObject ) {
	QuickInterpreter *ip = QuickInterpreter::fromEnv( qsobj.env() );
	if( qsobj.isA( ip->wrapperClass() ) ) {
	    type = TQObject;
	    qobj = *ip->wrapperClass()->objectVector( &qsobj );
	}
    }
}

bool QSCompletionObject::isNull() const
{
    return ( type == TNull ||
	     type == TQSObject && (qsobj.isNull() || qsobj.isUndefined()) ||
	     type == TQMetaObject && !meta ||
	     type == TQObject && qobj.isEmpty() );
}
