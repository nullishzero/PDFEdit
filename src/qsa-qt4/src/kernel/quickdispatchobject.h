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

#ifndef QUICKDISPATCHOBJECT_H
#define QUICKDISPATCHOBJECT_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QByteArray>
#include <QVariantList>

class QuickInterpreter;
class QuickDispatchObjectFactoryPrivate;

class QSObjectFactory;
class QSWrapperFactory;

class QuickDispatchObjectFactory
{
public:
    QuickDispatchObjectFactory(QuickInterpreter *i);
    ~QuickDispatchObjectFactory();

    bool constructInterface(const QByteArray &className, void *ptr, QVector<QObject *> *result);

    bool constructInstance(const QString &className, const QVariantList &args,
                           QVector<QObject *> *result);

    QMap<QString,QObject*> staticDescriptors() const;
    QMap<QString,QString> instanceDescriptors() const;

    QStringList classes() const;

    void throwError(const QString &message) const;

    QuickInterpreter *interpreter() const { return ip; }

    void addObjectFactory(QSObjectFactory *factory);
    void addWrapperFactory(QSWrapperFactory *factory);

    void removeObjectFactory(QSObjectFactory *factory);
    void removeWrapperFactory(QSWrapperFactory *factory);

private:
    bool createInterface(const QByteArray &className, void *ptr, QVector<QObject *> *);
    bool createInstance(const QString &className,
			 const QList<QVariant> &args,
                         QVector<QObject *> *);

    void addInterface(QObject *iface, QVector<QObject *> *);
    void addObject(QObject *obj, QVector<QObject *> *);

private:
    QuickDispatchObjectFactoryPrivate *d;
    QuickInterpreter *ip;
    bool recurseBlock;
};


class QuickUnnamedObject : public QObject
{
    Q_OBJECT
public:
    QuickUnnamedObject(QObject *p = 0, const char *n = 0) : QObject(p)
    {
        setObjectName(QLatin1String(n));
    }
    QuickInterpreter* interpreter() const;
    void throwError(const QString &message) const;
};


class QuickPtrDispatchObject : public QuickUnnamedObject
{
    Q_OBJECT
public:
    QuickPtrDispatchObject(const char *n, void *p);
    ~QuickPtrDispatchObject();

    void* pointer() const { return ptr; }
    void setPointer(void *p);

public slots:
    bool isValid() const { return !!ptr; }

private:
    void *ptr;
};

#endif
