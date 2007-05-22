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

#ifdef QSDEBUGGER

#include "qsdebugger.h"
#include "qsengine.h"
#include "qsenv.h"
#include "qsinternal.h"
#include "qsnodes.h"
#include <qstring.h>

Debugger::Debugger( QSEngine *engine )
  : eng( 0 ),
    sid( -1 )
{
  attach( engine );
}

Debugger::~Debugger()
{
  detach();
}

void Debugger::attach( QSEngine *e )
{
  dmode = Disabled;
  if (e) {
    if (!eng || e->rep != eng->rep) {
      eng = e;
      eng->rep->attachDebugger(this);
    }
  } else {
    eng = 0L;
  }
  reset();
}

QSEngine *Debugger::engine() const
{
  return eng;
}

QSEnv* Debugger::env() const
{
    return eng->env();
}

void Debugger::detach()
{
  reset();
  if (!eng)
    return;
  eng->rep->attachDebugger(0L);
  eng = 0L;
}

void Debugger::setMode(Mode m)
{
  dmode = m;
}

Debugger::Mode Debugger::mode() const
{
  return dmode;
}

// supposed to be overriden by the user
bool Debugger::stopEvent()
{
  return true;
}

void Debugger::callEvent(const QString &, const QString &)
{
}

void Debugger::returnEvent()
{
}

void Debugger::reset()
{
  l = -1;
}

int Debugger::freeSourceId() const
{
  return eng ? eng->rep->sourceId()+1 : -1;
}

bool Debugger::setBreakpoint(int id, int line)
{
  if (!eng)
      return false;
  return eng->rep->setBreakpoint(id, line, true);
}

bool Debugger::deleteBreakpoint(int id, int line)
{
  if (!eng)
    return false;
  return eng->rep->setBreakpoint(id, line, false);
}

void Debugger::clearAllBreakpoints(int id)
{
  if (!eng)
    return;
  eng->rep->setBreakpoint(id, -1, false);
}

bool Debugger::validBreakpoint( const QString &code, int line )
{
    Q_ASSERT(!"Debugger::validBreakpoint is not reentrant and deletes all nodes... Evil!!");
    if ( code.isEmpty() || line < 0 )
	return false;
    // can't check with invalid syntax

    QSEngine e;

    if ( !e.checkSyntax( code, QSEngine::CheckNormal, false ) ) {
	return false;
    }
    // try to set a breakpoint
    Q_ASSERT(!"Unsupported action...");
//     bool b = QSNode::setBreakpoint( /*QSNode::firstNode()*/ 0, e.rep->sourceId(), line, true );
    // free the temporary source id
    return false /* b */;
}

QString Debugger::varInfo( const QString & )
{
  if (!eng)
    return QString();

  qWarning("Debugger::varInfo(), should not be called...\n");

#if 0

  int dot = ident.find('.');
  if (dot < 0)
      dot = ident.length();
  QString sub = ident.mid(0, dot);
  QSObject obj;
  // resolve base
  if (sub == QString::fromLatin1("||Global||")) {
      obj = env()->globalObject();
  } else if (sub == QString::fromLatin1("||Activation||")) {
      obj = env()->currentScope();
  } else if (sub == QString::fromLatin1("this")) {
      obj = env()->thisValue();
  } else {
      obj = env()->resolveValue( ident );
      if ( !obj.isValid() )
	  return QString();
  }
  // look up each part of a.b.c.
  while (dot < (int)ident.length()) {
    int olddot = dot;
    dot = ident.find('.', olddot+1);
    if (dot < 0)
      dot = ident.length();
    sub = ident.mid(olddot+1, dot-olddot-1);
    obj = obj.get(sub);
    if (!obj.isDefined())
      break;
  }

#endif
  return QString();
}

bool Debugger::setVar(const QString &ident, const QSObject &value)
{
  if (!eng || ident == QString::fromLatin1("this"))
    return false;
  int dot = ident.indexOf('.');
  QString sub = dot > 0 ? ident.mid(0, dot) : ident;
  QSObject base;
  // resolve base
  if (sub == QString::fromLatin1("||Global||")) {
    base = env()->globalObject();
  } else if (sub == QString::fromLatin1("||Activation||")) {
    base = env()->currentScope();
  } else if (sub == QString::fromLatin1("this")) {
    base = env()->thisValue();
  } else {
      return false;
#if 0 // ###
    const QSList *chain = Context::current()->pScopeChain();
    QSListIterator scope = chain->begin();
    while (scope != chain->end()) {
      if (scope->hasProperty(sub)) {
	base = *scope;
	break;
      }
      scope++;
    }
    if (scope == chain->end())
      return false;
#endif
  }
  // look up each part of a.b.c.
  while (dot > 0) {
    int olddot = dot;
    dot = ident.indexOf('.', olddot+1);
    if (dot < 0) {
      sub = ident.mid(olddot+1);
      break;
    }
    sub = ident.mid(olddot+1, dot-olddot-1);
    if (!base.hasProperty(sub))
      return false;
    base = base.get(sub);
  }

  base.put(sub, value);

  return true;
}

// called from the scripting engine each time a statement node is hit.
bool Debugger::hit(int line, bool breakPoint)
{
  l = line;
  if (!eng)
    return true;

  if (!breakPoint && ( mode() == Continue || mode() == Disabled ) )
      return true;

  bool ret = stopEvent();
  eng->init();	// in case somebody used a different interpreter meanwhile
  return ret;
}

void Debugger::clear()
{

}

#endif
