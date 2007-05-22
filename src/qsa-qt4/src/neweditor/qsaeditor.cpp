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

#include "qsaeditor.h"
#include "qsasyntaxhighlighter.h"
#include "parenmatcher.h"
#include "quickinterpreter.h"
#include "quickclassparser.h"
#include "quickobjects.h"
#include "qsenv.h"
#include "qsinterpreter.h"
#include "completionbox.h"
#include "conf.h"
#include <QDebug>
#include <QMetaProperty>
#include <QApplication>
#include <QFrame>
#include <QVBoxLayout>
#include <QTextLayout>

extern QuickInterpreter *get_quick_interpreter(QSInterpreter *);

extern int indentForBottomLine(const QStringList& program, QChar typedIn);
extern void setTabSize(int s);
extern void setIndentSize(int s);

QSAEditor::QSAEditor(QWidget *parent)
    : QTextEdit(parent), qsInterp(0)
{
    setLineWrapMode(NoWrap);
    QSyntaxHighlighter *hl = new QSASyntaxHighlighter(this);
    hl->setDocument(document());
    matcher = new ParenMatcher;
    connect(this, SIGNAL(cursorPositionChanged()), matcher, SLOT(matchFromSender()));
    // #### add completion entries from highlighter

    readSettings();
}

QSAEditor::~QSAEditor()
{
    delete matcher;
}

void QSAEditor::clearMarkerFormat(const QTextBlock &block, int markerId)
{
    QTextLayout *layout = block.layout();
    QList<QTextLayout::FormatRange> formats = layout->additionalFormats();
    bool formatsChanged = false;
    for (int i = 0; i < formats.count(); ++i)
        if (formats.at(i).format.hasProperty(markerId)) {
            formats[i].format.clearBackground();
            formats[i].format.clearProperty(markerId);
            if (formats.at(i).format.properties().isEmpty()) {
                formats.removeAt(i);
                --i;
            }
            formatsChanged = true;
        }

    if (formatsChanged)
        layout->setAdditionalFormats(formats);
}

void QSAEditor::readSettings()
{
    // ###

    ::setTabSize(Config::indentTabSize());
    ::setIndentSize(Config::indentIndentSize());
}

static QChar charFromCursor(QTextCursor cursor, QTextCursor::MoveOperation op)
{
    cursor.clearSelection();
    if (!cursor.movePosition(op, QTextCursor::KeepAnchor))
        return QChar();
    if (!cursor.hasSelection())
        return QChar();
    return cursor.selectedText().at(0);
}

// indentation

static int indentation(const QString &s)
{
    if (s.simplified().isEmpty())
        return 0;
    int i = 0;
    int ind = 0;
    while (i < s.length()) {
        QChar c = s.at(i);
        if (c == QLatin1Char(' '))
            ++ind;
        else if (c == QLatin1Char('\t'))
            ind += 8;
        else
            break;
        ++i;
    }
    return ind;
}

static void tabify(QString &s)
{
    if (!Config::indentKeepTabs())
        return;
    int i = 0;
    const int tabSize = Config::indentTabSize();
    forever {
        for (int j = i; j < s.length(); ++j) {
            if (s.at(j) != QLatin1Char(' ') && s.at(j) != QLatin1Char('\t')) {
                if (j > i) {
                    QString t  = s.mid(i, j - i);
                    int spaces = 0;
                    for (int k = 0; k < t.length(); ++k)
                        spaces += (t.at(k) == QLatin1Char(' ') ? 1 : tabSize);
                    s.remove(i, t.length());
                    int tabs = spaces / tabSize;
                    spaces = spaces - (tabSize * tabs);
                    QString tmp;
                    tmp.fill(QLatin1Char(' '), spaces);
                    if (spaces > 0)
                        s.insert(i, tmp);
                    tmp.fill(QLatin1Char('\t'), tabs);
                    if (tabs > 0)
                        s.insert(i, tmp);
                }
                break;
            }
        }
        i = s.indexOf(QLatin1Char('\n'), i);
        if (i == -1)
            break;
        ++i;
    }
}

static void indentLine(QTextBlock block, int newIndent)
{
    QString indentString;
    indentString.fill(QLatin1Char(' '), newIndent);
    indentString.append(QLatin1String("a"));
    tabify(indentString);
    indentString.remove(indentString.length() - 1, 1);
    newIndent = indentString.length();

    QTextCursor cursor(block);
    cursor.movePosition(QTextCursor::StartOfBlock);

    QString blockText = block.text();
    int i = 0;
    while (!blockText.isEmpty()
           && (blockText.at(i) == QLatin1Char(' ') || blockText.at(i) == QLatin1Char('\t')))
        ++i;

    if (i > 0) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, i);
        cursor.removeSelectedText();
    }

    if (block.length() == 1)
        cursor.insertText(QLatin1String(" "));

    if (!indentString.isEmpty())
        cursor.insertText(indentString);

    /*
    while ( p->length() > 0 && ( p->at( 0 )->c == ' ' || p->at( 0 )->c == '\t' ) ) {
        ++oldIndent;
        p->remove( 0, 1 );
    }
    if ( p->string()->length() == 0 )
        p->append( QString::fromLatin1(" ") );
    if ( !indentString.isEmpty() )
        p->insert( 0, indentString );
        */
}

static void indent(QTextBlock block, const QStringList &previousCode)
{
    int indent = indentForBottomLine(previousCode, QChar::Null);
    indentLine(block, indent);
}

static void indent(QTextDocument *doc, QTextBlock block)
{
    QStringList code;
    QTextBlock currentBlock = block;
    block = doc->begin();

    while (block.isValid()) {
        code << block.text();

        if (block == currentBlock)
            break;
        block = block.next();
    }

    indent(currentBlock, code);
}

static void indent(QTextDocument *doc, const QTextCursor &cursor)
{
    if (cursor.hasSelection()) {
        QTextBlock block = doc->findBlock(cursor.selectionStart());
        QTextBlock last = doc->findBlock(cursor.selectionEnd());
        do {
            indent(doc, block);
            block = block.next();
        } while (block.isValid() && block < last);
    } else {
        indent(doc, cursor.block());
    }
}
static void indentDocument(QTextDocument *doc)
{
    QStringList allCode;
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next())
        allCode << block.text();

    QTextBlock block = doc->begin();
    for (int line = 0; line < allCode.count() && block.isValid(); ++line, block = block.next()) {
        const QStringList previousCode = allCode.mid(0, line + 1);
        indent(block, previousCode);
    }
}

void QSAEditor::keyPressEvent(QKeyEvent *e)
{
    QTextCursor cursor = textCursor();
    const QChar leftChar = charFromCursor(cursor, QTextCursor::PreviousCharacter);

    if (e->key() == Qt::Key_Tab) {
        QString leftText = cursor.block().text().left(cursor.position() - cursor.block().position());
        if (cursor.hasSelection() || leftText.simplified().isEmpty()) {
            indent(document(), cursor);
            e->accept();
            return;
        }
    }

    if ((e->key() == Qt::Key_Period
         && (cursor.atBlockStart() || leftChar != QLatin1Char('.'))
        )
        || (e->key() == Qt::Key_Greater
            && !cursor.atBlockStart()
            && leftChar == QLatin1Char('-'))
       ) {
        doObjectCompletion();
    }
    QTextEdit::keyPressEvent(e);
}

QSEnv *QSAEditor::env() const
{
    return interpreter()->env();
}

QuickInterpreter *QSAEditor::interpreter() const
{   
#if defined ( QSA_COMPLETION_DEBUG )
    if( !qsInterp ) printf( "QSAEditor::interpreter(), using defaultInterpreter\n" );
#endif
    return qsInterp ? get_quick_interpreter( qsInterp )
                    : get_quick_interpreter( QSInterpreter::defaultInterpreter() );
}

// =============================================================================
// =============================================================================
// == Parser to resolve assignments in Qt Script Code
// =============================================================================
// =============================================================================

static QString resolveValue(const QString &value, const QVector<QPair<QString, QString> > &assignments)
{
    for (int i = 0; i < assignments.count(); ++i)
        if (assignments.at(i).first == value)
            return assignments.at(i).second;
    return QString::null;
}

static QString qsa_strip_down(const QString &str, char start, char stop)
{
    // Match inside paranthesis
    int pos = str.lastIndexOf(start);
    if (pos > 0) {
        int end = str.indexOf(stop, pos+1);
        if (end < 0)
            return str.mid(pos+1);
        else // Cannot resolve function return types, so abort.
            return QString();
    }
    return str;
}

static QString resolveFullyQualifiedValue(const QString &value,
                                              const QVector<QPair<QString, QString> > &assignments )
{
    QString val = value;

    // Only resolve the last statement.
    int pos = val.lastIndexOf(';');
    if (pos > 0)
        val = val.mid(pos+1);

    val = qsa_strip_down(val, '(', ')');
    val = qsa_strip_down(val, '{', '}');

    QStringList l = val.split('.');
    QString valuePart;
    for ( QStringList::const_iterator vit = l.begin(); vit != l.end(); ++vit ) {
	    if ( !valuePart.isNull() )
	        valuePart += QString::fromLatin1(".");
	    valuePart += (*vit).left( (*vit).indexOf( '(' ) );
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


static QVector<QPair<QString, QString> > parseAssignments(const QString &code)
{
    QChar c, last;
    enum State { LeftHandSide, RightHandSight, Comment, String, Parentheses } state = LeftHandSide;
    int parenCount = 0;
    State lastState = LeftHandSide; // initialize to make compilers happy
    QChar ignoreEnd[2];
    QString leftHandBuffer, rightHandBuffer;
    QVector<QPair<QString, QString> > assignments;

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

    for ( QVector<QPair<QString, QString> >::Iterator it = assignments.begin(); it != assignments.end(); ++it ) {
	    QString key = (*it).first;
	    QString value = (*it).second;
	    QStringList l = value.split('.');
	    QString valuePart;
	    for ( QStringList::ConstIterator vit = l.begin(); vit != l.end(); ++vit ) {
	        if ( !valuePart.isNull() )
		        valuePart += QString::fromLatin1(".");
	        valuePart += *vit;
	        QString replacedValue;
	        int counter = 0;
	        while ( ( replacedValue = resolveValue( valuePart, assignments ) ) != QString::null ) {
		        if( ++counter > 1000 ) // Avoid recursion...
		            return QVector<QPair<QString,QString> >();
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

QString QSAEditor::functionCode() const
{
    QTextBlock block = textCursor().block();
    QString funcName;
    int pos;
    int braceCount = -1;
    QString s;
    while (block.isValid() && (pos = block.text().indexOf(QLatin1String("function"))) == -1) {
        s = block.text();
        braceCount += s.count(QLatin1Char('{'));
        braceCount -= s.count(QLatin1Char('}'));
        block = block.previous();
    }
    if (block.isValid()) {
        s = block.text();
        braceCount += s.count( '{' );
        braceCount -= s.count( '}' );
    }

    if (block.isValid() && pos != -1 && braceCount >= 0) {
        funcName = block.text().mid(pos + 9).simplified();
        funcName = funcName.left(funcName.indexOf(QLatin1Char('(')));
    }

    QuickClassParser parser;
    parser.parse(toPlainText());
    QList<QuickClass> classes = parser.classes();
    const bool global = funcName.isNull() || !block.isValid();
    QString code;
    for (QList<QuickClass>::ConstIterator it = classes.begin(); it != classes.end(); ++it) {
        if ((*it).type == QuickClass::Global) {
            for (QStringList::ConstIterator vit = (*it).variables.begin(); vit != (*it).variables.end(); ++vit)
                code += *vit + QString::fromLatin1(";\n");
            code += QString::fromLatin1("\n");
            if (global)
                break;
        }

        if (global)
            continue;

        for (QList<LanguageInterface::Function>::ConstIterator fit = (*it).functions.begin();
             fit != (*it).functions.end(); ++fit) {
            if ((*fit).name.left( (*fit).name.indexOf( '(' ) ) == funcName) {
                if ((*it).type != QuickClass::Global) {
                    for (QStringList::ConstIterator vit = (*it).variables.begin(); vit != (*it).variables.end(); ++vit)
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

QSCompletionObject QSAEditor::queryObject(const QString &object)
{
    QStringList l = object.split('.');

    QSCompletionObject ctx( env()->currentScope() );
    if ( context )
	    ctx = interpreter()->wrap( context );

    bool end = false;
    bool first = true;
    for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ) {
	    QString s = *it;
	    first = it == l.begin();
	    ++it;
	    end = it == l.end();

	    if ( first && s == QString::fromLatin1("this") ) {
	        if ( end ) {
		        if ( context )
		            return QSCompletionObject( interpreter()->wrap( context ) );
		        return QSCompletionObject( env()->currentScope() );
	        }
	        continue;
	    }

	    ctx.resolve();
	    QSCompletionObject obj = queryCompletionObject( ctx, s );
	    if ( obj.isNull() && first && context ) {
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

QSCompletionObject QSAEditor::queryCompletionObject(QSCompletionObject &ctx,
                                                        const QString &property) const
{
    QString s = property;

    // this is always an Array
    if ( s.indexOf( '[' ) != -1 )
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

QSObject QSAEditor::queryQSObject( QSObject &ctx, const QString &property ) const
{
    QString s = property;

    QSObject ctxValue = ctx.get( s );
    if ( ctxValue.isA( env()->typeClass() ) ) {
	return ctxValue;
    }

    if ( s.indexOf( '[' ) != -1 )
	return QSArray( env() );
    else if ( s == QString::fromLatin1("false") || s == QString::fromLatin1("true") )
	return env()->createBoolean( false );
    else if ( s[0] == '\'' || s[0] == '\"' )
	return env()->createString( QString::fromLatin1("") );
    return ctxValue;
}

QSObject QSAEditor::vTypeToQSType( const QString &type ) const
{
    QSObject qstype = env()->globalObject().get( type.mid( 1 ) );
    if( qstype.objectType() && qstype.objectType()->valueType() == TypeClass ) {
	    return qstype;
    }

    QVariant::Type t = QVariant::nameToType( type.toLatin1().constData() );
    switch ( t ) {
    case QVariant::Time:
    case QVariant::DateTime:
	    return env()->globalObject().get( QString::fromLatin1("Date") );
    case QVariant::StringList:
    case QVariant::List:
    case QVariant::Map:
	    return env()->globalObject().get( QString::fromLatin1("Array") );
    case QVariant::ByteArray:
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


QSObject QSAEditor::queryQSObject( const QMetaObject *meta, const QString &property, bool /*includeSuperClass*/ ) const
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
	        n = n.left(n.indexOf('('));
	        if ( property != n )
		        continue;

            return vTypeToQSType(mm.typeName());
        }
    }

    return env()->createUndefined();
}

QSObject QSAEditor::queryQSObject( const QVector<QObject *> &objects, const QString &property ) const
{
    for ( int i = 0; i < objects.count(); i++ ) {
	const QMetaObject *mo = objects[i]->metaObject();
	QSObject qso = queryQSObject( mo, property, i == 0 );
	if ( !qso.isNull() && !qso.isUndefined() )
	    return qso;
    }
    return env()->createUndefined();
}

const QMetaObject *QSAEditor::queryQMetaObject( const QMetaObject *meta,
						      const QString &property,
						      bool /*includeSuperClass*/ ) const
{
    const QMetaObject *m = meta;
    for (int i=0; i<m->methodCount(); ++i) {
        QMetaMethod mm = m->method(i);
        if (mm.methodType() == QMetaMethod::Slot) {
            QString n = QLatin1String(mm.signature());
            n = n.left(n.indexOf('('));

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

const QMetaObject *QSAEditor::queryQMetaObject( const QVector<QObject *> &objects, const QString &property ) const
{
    for ( int i = 0; i < objects.count(); i++ ) {
	    const QMetaObject *mo = objects[i]->metaObject();
	    const QMetaObject *meta = queryQMetaObject( mo, property, i == 0 );
	    if ( meta )
	        return meta;
    }
    return 0;
}

QVector<QObject *> QSAEditor::queryQObject( const QVector<QObject *> &objects, const QString &property ) const
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

QVector<QObject *>* QSAEditor::interfaceObjects( const QSObject &o ) const
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

static void getSlots( const QMetaObject *meta, QList<Property> &result,
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
	            s = s.left(s.indexOf('('));

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
	            if (result.indexOf(prop) == -1)
	                result << prop;
            }
        }

        if (super)
            meta = meta->superClass();
        else
            meta = 0;
    }
}

void QSAEditor::completeQSObject( QSObject &obj, QVector<CompletionEntry> &res, bool includeMembers )
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

void QSAEditor::completeQMetaObject( const QMetaObject *meta,
					 const QString &,
					 QVector<CompletionEntry> &res,
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
        propMap[QLatin1String(mp.name())] = false;
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
    QList<Property> lst;
    QList<Property>::Iterator pit;
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

void QSAEditor::completeQObject(const QVector<QObject *> &objects,
                                    const QString &object,
                                    QVector<CompletionEntry> &res)
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

QString QSAEditor::cppClassForScript( const QString &className ) const
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
    QMap<QString,QString>::ConstIterator cppName = names.find( className );
    if( cppName != names.end() )
	return *cppName;
    return QString::null;
}

const QMetaObject *QSAEditor::locateMetaObject( const QString &name ) const
{
    QString cppName = cppClassForScript( name );
    if (!cppName.isNull()) {
        extern const QMetaObject *qsa_query_meta_object(const QByteArray &name);
        return qsa_query_meta_object(name.toLatin1());
    }

    return 0;
}

void QSAEditor::doObjectCompletion()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
    QString objectName = cursor.selectedText();
    if (objectName.endsWith(QLatin1Char('-')))
        objectName.chop(1);

    objectName = objectName.simplified();

    QString object = resolveFullyQualifiedValue(objectName, parseAssignments(functionCode()));

    bool assumedStatic = false;
    QSCompletionObject o;
    if(objectName == object) {
        QSObject stobj = env()->globalObject().get(object);
        if(stobj.isValid() && stobj.objectType()->valueType() == TypeClass) {
#if defined ( QSA_COMPLETION_DEBUG )
            printf(" -> assuming static\n");
#endif
            o = stobj;
            assumedStatic = true;
        }
    }
    if(o.type == QSCompletionObject::TNull) {
        o = queryObject(object);
    }

#if defined ( QSA_COMPLETION_DEBUG )
    printf(" -> type is: %d\n", o.type);
#endif

    o.resolve();
    if (o.isNull())
        return;

    QVector<CompletionEntry> res;

    QSObject nullObject;
    switch (o.type) {
    case QSCompletionObject::TQSObject:
#if defined ( QSA_COMPLETION_DEBUG )
        printf(" -> objectType is: %s\n", o.qsobj.objectType()->name().latin1());
#endif
        if(o.qsobj.objectType()->name() == QString::fromLatin1("FactoryObject")){
            QSObject sinst = ( (QSFactoryObjectProxy*) o.qsobj.objectType() )->staticInstance();
            if(!sinst.isValid())
                return;
            QSWrapperShared *shared = (QSWrapperShared*) sinst.shVal();
            completeQObject( shared->objects, object, res );
            break;
        }
        completeQSObject(o.qsobj, res, !assumedStatic);
        break;
    case QSCompletionObject::TQMetaObject:
        completeQMetaObject(o.meta, object, res, IncludeSuperClass, nullObject);
        break;
    case QSCompletionObject::TQObject:
        completeQObject(o.qobj, object, res);
        break;
    case QSCompletionObject::TNull:
        break;
    }

    if (!res.isEmpty()) {
        QFrame *f = new QFrame(0, Qt::Popup);
        f->setAttribute(Qt::WA_DeleteOnClose);

        QWidget *box = new CompletionBox(this, res);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(box);
        layout->setMargin(0);
        f->setLayout(layout);

        f->move(mapToGlobal(cursorRect().bottomLeft()));
        f->show();
        box->setFocus();
    }
}


