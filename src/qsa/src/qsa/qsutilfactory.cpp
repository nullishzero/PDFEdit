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

/*!
  \class QSUtilFactory qsutilfactory.h

  \brief The QSUtilFactory class extends QSA with classes to allow
  file access, directory traversal and executing external processes.

  To extend the interpreter with the Utililty Framework you must
  create an instance of the QSUtilFactory class and add it to the
  interpreter using the function QSInterpreter::addObjectFactory().
  For example:

  \code
  QSProject proj;
  QSInterpreter *ip = proj->interpreter();
  ip->addObjectFactory( new QSUtilFactory );
  \endcode

  The script extensions provided by the QSUtilFactory are
  described in detail in \link extensions-2.book Utility
  Framework \endlink
 */

/*!
  \enum QSUtilFactory::Utilities

  The Utilities enum can be used to enable and disable script access
  to the classes that the utility factory provides, thereby
  controlling the access that end users can have to the underlying
  system.

  \value None No classes are available. If no access is required then
  there is no point in adding the QSUtilFactory to the interpreter at
  all.

  \value File Gives access to the File class.

  \value Directory Gives access to the Directory class.

  \value Process Gives access to Process class.

  \value All Gives access to all the classes in availble in the
  factory. This is the default.

*/

/*!
  \fn QSUtilFactory::interpreter() const
  \reimp
*/

#include "qsinterpreter.h"
#include "qsutilfactory.h"

#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qobject.h>
#include <qprocess.h>
#include <qvariant.h>

#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
#include <qthread.h>
extern Q_EXPORT Qt::HANDLE qt_get_application_thread_id();
#endif

#if (QT_VERSION >= 0x030200)
#  define ERRORSTRING(x) (x)->errorString()
#else
#  define ERRORSTRING(x) "no further details"
#endif

class QSFileStatic : public QObject
{
    Q_OBJECT
    Q_OVERRIDE(QString name SCRIPTABLE false)
    Q_ENUMS(AccessMode)
public:
    enum AccessMode {
	ReadOnly  = IO_ReadOnly,
	WriteOnly = IO_WriteOnly,
	ReadWrite = IO_ReadWrite,
	Append    = IO_Append,
	Truncate  = IO_Truncate,
	Translate = IO_Translate
    };
    QSFileStatic(QSUtilFactory *i) : factory(i) { }

public slots:
    bool exists(const QString &fileName) { return QFile::exists(fileName);}
    void remove(const QString &fileName);
    void write(const QString &fileName, const QString &content);
    QString read(const QString &fileName);

    bool isFile(const QString &fileName) { return QFileInfo(fileName).isFile(); }
    bool isDir(const QString &fileName) { return QFileInfo(fileName).isDir(); }

private:
    QSUtilFactory *factory;
};

class QSFile : public QObject
{
    Q_OBJECT

    Q_OVERRIDE(QString name READ fileName)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString fullName READ absFileName)
    Q_PROPERTY(QString baseName READ baseName)
    Q_PROPERTY(QString extension READ extension)
    Q_PROPERTY(QString symLink READ symLink)

    Q_PROPERTY(bool exists READ exists)
    Q_PROPERTY(bool readable READ isReadable)
    Q_PROPERTY(bool writable READ isWritable)
    Q_PROPERTY(bool executable READ isExecutable)
    Q_PROPERTY(bool hidden READ isHidden)
    Q_PROPERTY(bool eof READ eof)

    Q_PROPERTY(QDateTime created READ created)
    Q_PROPERTY(QDateTime lastModified READ lastModified)
    Q_PROPERTY(QDateTime lastRead READ lastRead)

    Q_PROPERTY(int size READ size)

    Q_ENUMS(AccessMode)
public:
    enum AccessMode {
	ReadOnly  = IO_ReadOnly,
	WriteOnly = IO_WriteOnly,
	ReadWrite = IO_ReadWrite,
	Append    = IO_Append,
	Truncate  = IO_Truncate,
	Translate = IO_Translate
    };

public slots:
    void open(int accessMode);
    void close() { file->close(); }
    void remove();

    int readByte();
    QString read();
    QString readLine();
    QStringList readLines();

    void writeByte(int byte);
    void write(const QString &data, int length = -1);
    void writeLine(const QString &data) { write(data + QString::fromLatin1("\n")); }

public:
    QSFile(const QString &file, QSInterpreter *i);
    ~QSFile();

    QString fileName() const { return QFileInfo(*file).fileName(); }
    QString path() const { return QFileInfo(*file).dirPath(true); }
    QString absFileName() const { return QFileInfo(*file).absFilePath(); }
    QString baseName() const { return QFileInfo(*file).baseName(); }
    QString extension() const { return QFileInfo(*file).extension(); }
    QString symLink() const { QFileInfo f(*file); return f.isSymLink() ? f.readLink() : QString(); }

    bool exists() const { return file->exists(); }
    bool isReadable() const { return QFileInfo(*file).isReadable(); }
    bool isWritable() const { return QFileInfo(*file).isWritable(); }
    bool isExecutable() const { return QFileInfo(*file).isExecutable(); }
    bool isHidden() const { return QFileInfo(*file).isHidden(); }
    bool eof() const { return file->atEnd(); }

    QDateTime created() const { return QFileInfo(*file).created(); }
    QDateTime lastModified() const { return QFileInfo(*file).lastModified(); }
    QDateTime lastRead() const { return QFileInfo(*file).lastRead(); }

    int size() const { return file->size(); }

private:
    QFile *file;
    QSInterpreter *interpreter;
};


class QSDirStatic : public QObject
{
    Q_OBJECT
    Q_ENUMS(FilterSpec)
    Q_ENUMS(SortSpec)
    Q_OVERRIDE(QString name SCRIPTABLE false)
    Q_PROPERTY(QString current READ current WRITE setCurrent)
    Q_PROPERTY(QString home READ home)
    Q_PROPERTY(QString root READ root)
    Q_PROPERTY(QStringList drives READ drives)

public:
    enum FilterSpec {
	Dirs          	= 0x001,
	Files 		= 0x002,
	Drives 		= 0x004,
	NoSymLinks 	= 0x008,
	All 		= 0x007,
	TypeMask 	= 0x00F,
	Readable 	= 0x010,
	Writable 	= 0x020,
	Executable 	= 0x040,
	RWEMask 	= 0x070,
	Modified 	= 0x080,
	Hidden 		= 0x100,
	System 		= 0x200,
	AccessMask 	= 0x3F0
    };

    enum SortSpec {
	Name 		= 0x00,
	Time 		= 0x01,
	Size 		= 0x02,
	Unsorted 	= 0x03,
	SortByMask 	= 0x03,
	DirsFirst 	= 0x04,
	Reversed 	= 0x08,
	IgnoreCase 	= 0x10
    };

public slots:
    QString cleanDirPath(const QString &path) const { return QDir::cleanDirPath(path); }
    QString convertSeparators(const QString &path) const { return QDir::convertSeparators(path); }

public:
    QSDirStatic(QSUtilFactory *f) : factory(f) { };
    void setCurrent(const QString &path);
    QString current() const { return QDir::currentDirPath(); }
    QString home() const { return QDir::homeDirPath(); }
    QString root() const { return QDir::rootDirPath(); }
    QStringList drives() const;

private:
    QSUtilFactory *factory;
};

class QSDir : public QObject
{
    Q_OBJECT
    Q_ENUMS(FilterSpec)
    Q_ENUMS(SortSpec)
    Q_OVERRIDE(QString name READ name)
    Q_PROPERTY(QString path READ path WRITE setPath)
    Q_PROPERTY(QString absPath READ absPath)
    Q_PROPERTY(QString canonicalPath READ canonicalPath)
    Q_PROPERTY(bool readable READ isReadable)
    Q_PROPERTY(bool exists READ exists)
public:
    enum FilterSpec {
	Dirs          	= 0x001,
	Files 		= 0x002,
	Drives 		= 0x004,
	NoSymLinks 	= 0x008,
	All 		= 0x007,
	TypeMask 	= 0x00F,
	Readable 	= 0x010,
	Writable 	= 0x020,
	Executable 	= 0x040,
	RWEMask 	= 0x070,
	Modified 	= 0x080,
	Hidden 		= 0x100,
	System 		= 0x200,
	AccessMask 	= 0x3F0
    };

    enum SortSpec {
	Name 		= 0x00,
	Time 		= 0x01,
	Size 		= 0x02,
	Unsorted 	= 0x03,
	SortByMask 	= 0x03,
	DirsFirst 	= 0x04,
	Reversed 	= 0x08,
	IgnoreCase 	= 0x10
    };

public slots:
    QString filePath(const QString &file=QString::null) const;
    QString absFilePath(const QString &file=QString::null) const;
    void cd(const QString &dirName) const;
    void cdUp() const;

    QStringList entryList(const QString &filter,
			  int filterSpec=-1,
			  int sortSpec=-1) const;

    void mkdir(const QString &dirName = QString::null) const;
    void rmdir(const QString &dirName = QString::null) const;

    void mkdirs(const QString &dirName = QString::null) const;
    void rmdirs(const QString &dirName = QString::null) const;

    void remove(const QString &fileName) const;
    void rename(const QString &oldName, const QString &newName);

    bool fileExists(const QString &fileName) const { return dir->exists(fileName); }

    void setCurrent() const;

public:
    QSDir(const QString &path, QSInterpreter *ip) : dir(new QDir(path)), interpreter(ip) { }
    QString name() const { return dir->dirName(); }
    QString path() const { return dir->path(); }
    void setPath(const QString &newPath) const { dir->setPath(newPath); }
    QString absPath() const { return dir->absPath(); }
    QString canonicalPath() const { return dir->canonicalPath(); }

    bool isReadable() const { return dir->isReadable(); }
    bool exists() const { return dir->exists(); }

private:
    QDir *dir;
    QSInterpreter *interpreter;
};


class QSBlockingProcess : public QProcess
{
    Q_OBJECT

public:
    QSBlockingProcess()
    {
	connect(this, SIGNAL(readyReadStdout()), this, SLOT(readOut()));
	connect(this, SIGNAL(readyReadStderr()), this, SLOT(readErr()));
	connect(this, SIGNAL(processExited()), this, SLOT(exited()));
        connect(this, SIGNAL(wroteToStdin()), this, SLOT(closeStdin()));
        outUsed = errUsed = 0;
    }

public slots:
    void readOut()
    {
 	QByteArray rout = readStdout();
	if (outUsed + rout.size() > out.size())
	    out.resize(outUsed + rout.size());
	memcpy(out.data() + outUsed, rout, rout.size());
	outUsed += rout.size();
    }

    void readErr()
    {
 	QByteArray rerr = readStderr();
	if (errUsed + rerr.size() > err.size())
	    err.resize(errUsed + rerr.size());
	memcpy(err.data() + errUsed, rerr, rerr.size());
	errUsed += rerr.size();
    }

    void exited()
    {
	qApp->exit_loop();
    }

public:
    QByteArray out;
    QByteArray err;
    int outUsed;
    int errUsed;
};

class QSProcessStatic : public QObject
{
    Q_OBJECT
    Q_OVERRIDE(QString name SCRIPTABLE false)
    Q_PROPERTY(QString stdout READ readStdout)
    Q_PROPERTY(QString stderr READ readStderr)
public:
    QSProcessStatic(QSUtilFactory *f) :
	factory(f)
    {
    }

    QString readStdout() const { return QString(out); }
    QString readStderr() const { return QString(err); }

public slots:
    int execute(const QStringList &command, const QString &stdinBuffer=QString::null);
    int executeNoSplit(const QStringList &command, const QString &stdinBuffer = QString::null);
    int execute(const QString &command, const QString &stdinBuffer=QString::null)
    {
	return execute(QStringList(command), stdinBuffer);
    }

private:
    QString out, err;
    QSUtilFactory *factory;
};


class QSProcess : public QObject
{
    Q_OBJECT
    Q_OVERRIDE(QString name SCRIPTABLE false)
    Q_PROPERTY(QStringList arguments READ arguments WRITE setArguments)
    Q_PROPERTY(QString workingDirectory READ workingDirectory WRITE setWorkingDirectory)
    Q_PROPERTY(bool running READ running)
    Q_PROPERTY(int exitStatus READ exitStatus)

public:
    QSProcess(QSUtilFactory *f, const QStringList &args=QStringList());

    QStringList arguments() const { return process->arguments(); }
    void setArguments(const QStringList &arguments) { process->setArguments(arguments); }
    QString workingDirectory() const { return process->workingDirectory().absPath(); }
    void setWorkingDirectory(const QString &workingDirectory) { process->setWorkingDirectory(workingDirectory); }

    bool running() const { return process->isRunning(); }
    int exitStatus() const { return process->exitStatus(); }

public slots:
    void start(QStringList *env=0);
    void launch(const QString &buf, QStringList *env=0);

    QString readStdout() { return process->readStdout(); }
    QString readStderr() { return process->readStderr(); }

    bool canReadLineStdout() { return process->canReadLineStdout(); }
    bool canReadLineStderr() { return process->canReadLineStderr(); }

    QString readLineStdout() { return process->readLineStdout(); }
    QString readLineStderr() { return process->readLineStderr(); }

    void tryTerminate() const { process->tryTerminate(); }
    void kill() const { process->kill(); }
    void writeToStdin(const QString &buffer) { process->writeToStdin(buffer); }
    void closeStdin() { process->closeStdin(); }

signals:
    void readyReadStdout();
    void readyReadStderr();
    void processExited();
    void wroteToStdin();
    void launchFinished();

private:
    QProcess *process;
    QSUtilFactory *factory;
};



/*! Instantiates the QSUtilFactory and enables the script classes
  specified in \a enableFlags. \a enableFlags is a bitmask of the
  \l Utilities enum.

  The QSUtilityFactory must be added to an interpreter before its
  classes can be used

  \sa QSInterpreter::addObjectFactory(), Utilities
*/
QSUtilFactory::QSUtilFactory(uint enableFlags)
{
    if ((enableFlags&File) != 0)
	registerClass(QString::fromLatin1("File"), QString::fromLatin1("QSFile"),
                      new QSFileStatic(this));
    if ((enableFlags&Directory) != 0)
	registerClass(QString::fromLatin1("Dir"), QString::fromLatin1("QSDir"),
                      new QSDirStatic(this));
    if ((enableFlags&Process) != 0) {
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
	if (qt_get_application_thread_id() != QThread::currentThread()) {
	    qWarning("QSUtilFactory::QSUtilFactory(), "
		     "Process class cannot be used in non GUI thread");
	} else
#endif
	{
	    registerClass(QString::fromLatin1("Process"), QString::fromLatin1("QSProcess"),
                          new QSProcessStatic(this));
	}
    }
}

/*!
  \reimp
*/
QObject *QSUtilFactory::create( const QString &name,
				const QSArgumentList &args,
				QObject * )
{
    int argCount = args.size();

    // File construction..
    if (name == QString::fromLatin1("File")) {
	if (argCount == 0) {
	    interpreter()->throwError(QString::fromLatin1("Cannot construct File, "
                                                          "missing argument filename"));
	    return 0;
	}
	QSArgument fn = args[0];
	if (fn.type() != QSArgument::Variant && fn.variant().type() != QVariant::String) {
	    interpreter()->throwError(QString::fromLatin1("Cannot construct File, "
                                                          "Filename must be of type string"));
	    return 0;
	}
	return new QSFile(args[0].variant().toString(), interpreter());
    }

    // Dir construction
    else if (name == QString::fromLatin1("Dir")) {
	if (argCount >= 1) {
	    QSArgument fn = args[0];
	    if (fn.type() != QSArgument::Variant && fn.variant().type() != QVariant::String) {
		interpreter()->throwError(QString::fromLatin1("Cannot construct Dir object, "
                                                              "Filename must be of type string"));
		return 0;
	    }
	    return new QSDir(args[0].variant().toString(), interpreter());
	} else {
	    return new QSDir(QDir::currentDirPath(), interpreter());
	}
    }

    else if (name == QString::fromLatin1("Process")) {
#if defined (QT_THREAD_SUPPORT) && QT_VERSION >= 0x030300
	if (qt_get_application_thread_id() != QThread::currentThread()) {
	    qWarning("QSUtilFactory::create(), "
		     "Process class cannot be used in non GUI thread");
	    return 0;
	}
#endif
	if (argCount>=1) {
	    QStringList commands;
	    for (int i=0; i<argCount; ++i) {
		if (args[i].variant().type() == QVariant::StringList)
		    commands += args[i].variant().toStringList();
		else
		    commands += args[i].variant().toString();
	    }
	    return new QSProcess(this, commands);
	} else {
	    return new QSProcess(this);
	}
    }

    else {
	interpreter()->throwError(QString::fromLatin1("QSUtilfactory::create(), don't know how to create: ")
				  + name);
	return 0;
    }
}

/********************************************************************************
 * class QSFileStatic
 */

void QSFileStatic::write(const QString &fileName, const QString &content)
{
    QFile file(fileName);
    if (!file.open(IO_WriteOnly))
	factory->interpreter()->throwError(QString::fromLatin1("Could open file '") + fileName + QString::fromLatin1("' for writing"));
    else if (!file.writeBlock(content.local8Bit(), content.length()) == content.length())
	factory->interpreter()->throwError(QString::fromLatin1("Failed to write file"));
}

QString QSFileStatic::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.exists()) {
	factory->interpreter()->throwError(QString::fromLatin1("File '%1' does not exist").arg(fileName));
	return QString();
    } else if (!file.open(IO_ReadOnly)) {
	factory->interpreter()->throwError(QString::fromLatin1("Could not read file '%1'").arg(fileName));
	return QString();
    }
    return QString(file.readAll());
}

void QSFileStatic::remove(const QString &file)
{
    QFile rmfile(file);
    if (!rmfile.remove(file)) {
	factory->interpreter()->throwError(QString::fromLatin1("File %1 could not be removed: %2")
					   .arg(file).arg(ERRORSTRING(&rmfile)));
    }
}

/********************************************************************************
 * class QSFile
 */
QSFile::QSFile(const QString &fileName, QSInterpreter *i)
    : file(new QFile(fileName)), interpreter(i)
{
}

QSFile::~QSFile()
{
    delete file;
}


void QSFile::open(int accessMode)
{
    if (accessMode == Append)
	accessMode |= WriteOnly;
    if (!file->open(accessMode)) {
	interpreter->throwError(QString::fromLatin1("Could not open file '%1':  %2")
				.arg(file->name()).arg(ERRORSTRING(file)));
    }
}

QString QSFile::read()
{
    if (!file->isOpen()) {
	interpreter->throwError(QString::fromLatin1("File '") + file->name() + QString::fromLatin1("' is not open"));
	return QString::null;
    }
    return QString(file->readAll());
}

QString QSFile::readLine()
{
    QString buffer;
    int read = file->readLine(buffer, 16384);
    if (read == -1) {
	interpreter->throwError(QString::fromLatin1("Failed to read line from file '") + file->name() + QString::fromLatin1("': ")
				+ ERRORSTRING(file));
	return QString::null;
    }
    return buffer;
}


QStringList QSFile::readLines()
{
    return QStringList::split(QString::fromLatin1("\n"), read());
}

void QSFile::write(const QString &data, int length)
{
    if (length<0)
	length = data.length();
    int written = file->writeBlock(data.local8Bit(), length);
    if (written != length) {
	interpreter->throwError(QString::fromLatin1("Failed to write file '%1': %2")
				.arg(file->name()).arg(ERRORSTRING(file)));
    }
}

void QSFile::remove()
{
    if (!file->remove()) {
	interpreter->throwError(QString::fromLatin1("Failed to remove file '%1': %2")
				.arg(file->name()).arg(ERRORSTRING(file)));
    }
}

int QSFile::readByte()
{
    int c = file->getch();
    if (c == -1) {
	interpreter->throwError(QString::fromLatin1("Failed to read byte from file '%1': %2")
				.arg(file->name()).arg(ERRORSTRING(file)));
    }
    return c;
}

void QSFile::writeByte(int byte)
{
    int c = file->putch(byte);
    if (c == -1) {
	interpreter->throwError(QString::fromLatin1("Failed to write byte to file '%1': %2")
				.arg(file->name()).arg(ERRORSTRING(file)));
    }
}

/********************************************************************************
 * class QSDirStatic
 */
QStringList QSDirStatic::drives() const
{
    const QFileInfoList *lst = QDir::drives();
    QStringList driveNames;
    for (QPtrListIterator<QFileInfo> it(*lst); it.current(); ++it) {
	driveNames << it.current()->absFilePath();
    }
    return driveNames;
}

void QSDirStatic::setCurrent(const QString &dir)
{
    if (!QDir::setCurrent(dir))
	factory->interpreter()->throwError(QString::fromLatin1("Failed to set current directory to %1")
					   .arg(dir));
}


/********************************************************************************
 * class QSDir
 */
void QSDir::rename(const QString &oldName, const QString &newName)
{
    if (!dir->rename(QDir::convertSeparators(oldName), QDir::convertSeparators(newName)))
	interpreter->throwError(QString::fromLatin1("Failed to rename '%1' to '%2'").arg(oldName).arg(newName));
}

void QSDir::remove(const QString &name) const
{
    if (!dir->remove(name))
	interpreter->throwError(QString::fromLatin1("Failed to remove '%1'").arg(name));
}

void QSDir::rmdir(const QString &name) const
{
    if (!dir->rmdir(name.isEmpty() ? dir->absPath() : name))
	interpreter->throwError(QString::fromLatin1("Failed to remove directory '%1'").arg(name));
}

void QSDir::mkdir(const QString &name) const
{
    if (!dir->mkdir(name.isEmpty() ? dir->absPath() : name))
	interpreter->throwError(QString::fromLatin1("Failed to make directory '%1'").arg(name));
}

static bool qs_rm_dirs(QDir *dir)
{
    if (!dir->exists())
	return FALSE;

    QStringList files = dir->entryList(QDir::Files | QDir::Hidden | QDir::System);
    for (QStringList::ConstIterator fit = files.begin(); fit!=files.end(); ++fit)
	if (!dir->remove(dir->filePath(*fit)))
	    return FALSE;

    QStringList dirs = dir->entryList(QDir::Dirs | QDir::Hidden | QDir::System);
    for (QStringList::ConstIterator it = dirs.begin(); it!=dirs.end(); ++it) {
	if ((*it) == QString::fromLatin1(".") || (*it) == QString::fromLatin1(".."))
	    continue;
	dir->cd(*it);
	bool del = qs_rm_dirs(dir);
	dir->cdUp();
	if (!del)
	    return del;
    }

    if (!dir->rmdir(dir->absPath()))
	return FALSE;

    return TRUE;
}

void QSDir::rmdirs(const QString &name) const
{
    bool usesTemp = false;
    QDir *tmpdir;
    if (name.isEmpty()) {
	tmpdir = dir;
    } else {
	tmpdir = new QDir( QFileInfo(name).isRelative() ? dir->filePath(name) : name );
	usesTemp = true;
    }
    if (!qs_rm_dirs(tmpdir))
	    interpreter->throwError(QString::fromLatin1("Failed to remove directory structure: '%1'")
				    .arg(tmpdir->absPath()));
    if (usesTemp)
	delete tmpdir;
}


void QSDir::mkdirs(const QString &name) const
{
    // Use current, absolute or relative depending on name.
    QString finalDir;
    if (name.isEmpty())
	finalDir = dir->absPath();
    else if (QFileInfo(name).isRelative())
	finalDir = dir->filePath(name);
    else
	finalDir = name;

    // Split up
    finalDir = finalDir.replace(QString::fromLatin1("\\"), QString::fromLatin1("/"));
    QStringList dirs = QStringList::split(QString::fromLatin1("/"), finalDir);
    QStringList::ConstIterator it = dirs.begin();
    QDir tmpdir(finalDir.startsWith(QString::fromLatin1("/")) ? QString::fromLatin1("/") + *it : *it);
    ++it;
    do {
	if (it == dirs.end())
	    break;
	if (!tmpdir.exists(*it)) {
	    if (!tmpdir.mkdir(*it)) {
		interpreter->throwError(QString::fromLatin1("Failed to create directory %1")
					.arg(tmpdir.filePath(*it)));
		return;
	    }
	}
	tmpdir.cd(*it);
	++it;
    } while (it != dirs.end());
}


QStringList QSDir::entryList(const QString &filter, int filterSpec, int sortSpec) const
{
    return dir->entryList(filter, filterSpec, sortSpec);
}


void QSDir::cd(const QString &dirName) const
{
    if (!dir->cd(dirName))
	interpreter->throwError(QString::fromLatin1("Failed to change directory to %1").arg(dirName));
}

void QSDir::cdUp() const
{
    if (!dir->cdUp())
	interpreter->throwError(QString::fromLatin1("Failed to change to parent directory"));
}

QString QSDir::filePath(const QString &file) const
{
    return dir->filePath(file);
}

QString QSDir::absFilePath(const QString &file) const
{
    return dir->absFilePath(file);
}

void QSDir::setCurrent() const
{
    if (!QDir::setCurrent(dir->absPath())) {
	interpreter->throwError(QString::fromLatin1("Failed to set '%1' to current directory")
				.arg(dir->absPath()));
    }
}


/********************************************************************************
 * class QSProcessStatic
 */
int QSProcessStatic::execute(const QStringList &command, const QString &stdinBuffer)
{
    QStringList commands = command;
    if (commands.size() == 1 && commands[0].contains(' ')>=1) {
        commands = QStringList::split(QString::fromLatin1(" "), commands[0]);
    }
    out = err = QString::null;
    QSBlockingProcess pl;
    pl.setArguments(commands);
    if (!pl.start()) {
	factory->interpreter()->throwError(QString::fromLatin1("Failed to run process: '%1'")
					   .arg(command.join(QString::fromLatin1(" "))));
        return -1;
    }
    if (!stdinBuffer.isEmpty()) {
	pl.writeToStdin(stdinBuffer);
    }
    Q_ASSERT(qApp);
    qApp->enter_loop();
    int retCode = pl.exitStatus();
    out = QString::fromLatin1(pl.out, pl.outUsed);
    err = QString::fromLatin1(pl.err, pl.errUsed);
    return retCode;
}

int QSProcessStatic::executeNoSplit(const QStringList &command, const QString &stdinBuffer)
{
    out = err = QString::null;
    QSBlockingProcess pl;
    pl.setArguments(command);
    if (!pl.start()) {
	factory->interpreter()->throwError(QString::fromLatin1("Failed to run process: '%1'")
					   .arg(command.join(QString::fromLatin1(" "))));
        return -1;
    }
    if (!stdinBuffer.isEmpty()) {
	pl.writeToStdin(stdinBuffer);
    }
    Q_ASSERT(qApp);
    qApp->enter_loop();
    int retCode = pl.exitStatus();
    out = QString::fromLatin1(pl.out, pl.outUsed);
    err = QString::fromLatin1(pl.err, pl.errUsed);
    return retCode;
}

/********************************************************************************
 * class QSProcess
 */
QSProcess::QSProcess(QSUtilFactory *f, const QStringList &args)
    : factory(f)
{
    process = args.size() == 0 ? new QProcess(this) : new QProcess(args, this);
    connect(process, SIGNAL(readyReadStdout()), this, SIGNAL(readyReadStdout()));
    connect(process, SIGNAL(readyReadStderr()), this, SIGNAL(readyReadStderr()));
    connect(process, SIGNAL(processExited()), this, SIGNAL(processExited()));
    connect(process, SIGNAL(wroteToStdin()), this, SIGNAL(wroteToStdin()));
    connect(process, SIGNAL(launchFinished()), this, SIGNAL(launchFinished()));
}

void QSProcess::start(QStringList *env)
{
    if (!process->start(env)) {
	factory->interpreter()->throwError(QString::fromLatin1("Failed to start process: '%1'")
					   .arg(process->arguments().join(QString::fromLatin1(" "))));
    }
}

void QSProcess::launch(const QString &buf, QStringList *env)
{
    if (!process->launch(buf, env)) {
	factory->interpreter()->throwError(QString::fromLatin1("Failed to launch process: '%1'")
					   .arg(process->arguments().join(QString::fromLatin1(" "))));
    }
}


#include "qsutilfactory.moc"
