/****************************************************************************
** $Id$
**
** Copyright (C) 2001-2006 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Script for Applications framework (QSA).
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding a valid Qt Script for Applications license may use
** this file in accordance with the Qt Script for Applications License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about QSA Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
*****************************************************************************/

#ifndef QSENGINE_H
#define QSENGINE_H

#include "qsobject.h"
#include "qstimeouttrigger.h"
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qobject.h>

class QSList;

class Global;
#ifdef QSDEBUGGER
class Debugger;
#endif

/**
 * @short ECMAScript interpreter
 */
class QUICKCORE_EXPORT QSEngine : public QObject {
  friend class QSEngineImp;
  friend class QSObject;
  friend class Global;
  Q_OBJECT
public:
  enum CheckMode {
      CheckNormal = 0,
      DisallowGlobal
  };

  /**
   * Create a new ECMAScript interpreter. You can later ask it to interprete
   * code by passing it via @ref #evaluate.
   */
  QSEngine( QObject *parent = 0, const char *name = 0 );
  /**
   *  Destructor
   */
  virtual ~QSEngine();
  /**
   * Force a "context switch". You usually do not need to do that,
   * evaluate() does it too.
   */
  void init();
  /**
   * Current level of recursive calls to this interpreter. 0 initially.
   */
  int recursion() const;
  /**
   * Asks the interpreter to evaluate a piece of code. If called more than
   * once the state (global variables, functions etc.) will be preserved
   * between each call.
   * @param code is a string containing the code to be executed.
   * @return True if the code was evaluated successfully, false if an error
   * occured.
   */
  bool evaluate(const char *code);
  /**
   * Same as above. Only differing in the arguments accepted.
   * @param code is a pointer to an Unicode string containing the code to
   * be executed.
   * @param length number of characters.
   */
  bool evaluate( const QSObject &thisV, const QString &code, int linezero=0 );
  /**
   * Added for convenience in case you have the code in available in
   * internal representation already.
   * @param code is an Unicode string containing the code to be executed.
   */
  bool evaluate(const QString &code);
  /**
   * Call the specified function directly, optionally passing args as a
   * list of arguments. Return value and treatment of errors is analog
   * to the evaluate() calls.
   */
  bool call( const QString &func, const QSList &args );
  bool call( QSObject *scope, const QString &func, const QSList &args );
  /**
   * Clear the interpreter's memory. Otherwise, function declarations
   * and global variables will be remembered after each invokation of
   * @ref QSEngine::evaluate.
   */
  void clear();
  /**
   * @return Return value from the last call to @ref evaluate(). Null if there
   * hasn't been any.
   */
  QSObject returnValue() const;

    void setReturnValue(const QSObject &v);

  /**
   * @return Return code from last call to @ref evaluate(). 0 on success.
   */
  int errorType() const;
  /**
   * @return Return line of last error. -1 if last call didn't have an error.
   */
  QValueList<uint> errorLines() const;
  /**
   * @return Error message from last call to @ref evaluate(). Empty string
   * if no error occured.
   */
  QStringList errorMessages() const;
  /**
   * Check the syntax of a piece of code. Return true if the code could be
   * parsed without errors, false otherwise. @ref errorLine() will tell you
   * approximately where the syntax error happened.
   */
  bool checkSyntax( const QString &code, int checkMode = CheckNormal,
		    bool deleteNodes = TRUE );
  /**
   * Adds a debug() function to the set of pre-defined properties.
   * debug(arg) tries to convert 'arg' to a string and prints the result
   * to stderr. If you want to debug self defined Host Objects this way
   * you should provide them with a toString() method that returns a string.
   */
  void enableDebug();

  QSEnv *env() const;
#ifdef QSDEBUGGER
    Debugger* debugger() const;
#endif

  void registerType( QSClass *c );

    void warn( const QString &msg, int l );
    void requestPackage( const QString &package, QString &err );

    void setTimeoutTriggerEnabled(bool enable);
    QSTimeoutTrigger *timeoutTrigger() const { return timer; }

signals:
    /*!
     * This signal is emitted on each non-fatal error \a msg at line \a l.
     */
    void warning( const QString &msg, int l );
    /*!
     * Emitted whenever the interpreter encountered an 'import'
     * statement asking for a \a package. Upon receiptong feed the
     * requested source to the interpreter. If you want to return
     * an error set \a err to a non-null string.
     */
    void packageRequest( const QString &package, QString &err );

public:
    // ### ugly
    QSEngineImp* imp() const { return rep; }

private:
  QSEngineImp *rep;
  // not implemented
  QSEngine( const QSEngine& );
  QSEngine operator=( const QSEngine& );
    QSTimeoutTrigger *timer;

#ifdef QSDEBUGGER
  friend class Debugger;
#endif
};

inline void QSEngine::setTimeoutTriggerEnabled(bool enable)
{
    if (enable && !timer) {
	timer = new QSTimeoutTrigger;
    } else if (!enable && timer) {
	delete timer;
	timer = 0;
    }
}

#endif
