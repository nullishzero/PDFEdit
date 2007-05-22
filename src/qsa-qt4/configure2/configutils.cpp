/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "configutils.h"

#include <qdebug.h>
#include <qcoreapplication.h>
#include <qobject.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qlibraryinfo.h>
#include <qprocess.h>
#include <qtextstream.h>
#ifdef Q_WS_WIN
#include <windows.h>
#endif
#ifdef Q_OS_UNIX
#include <stdlib.h>
#endif

QString *qtDir = 0;
QString *qsa_prefix = 0;

int errors = 0;
int warnings = 0;
bool autoShutdown = true;

bool qdoc_warning = false;
bool qmake_warning = false;
bool license_warning = false;

static QString *assistant = 0;

static QStringList static_messages;

QRegExp licenseKeyExtRegExp() {
    return QRegExp(QLatin1String("(.....?)-(....?)-(....?)-(....?)-(.....?)-(.....?)-(....)"));
};

void message(const QString &str)
{
    static_messages.append(str);
}

QStringList messages()
{
    return static_messages;
}

QString firstLicensePath()
{
    QStringList allPaths;
    allPaths << *qtDir + QLatin1String(".qt-license")
             << QDir::homePath() + QLatin1String("/.qt-license");
    for (int i = 0; i< allPaths.count(); ++i)
        if (QFile::exists(allPaths.at(i)))
            return allPaths.at(i);
    return QString();
}


QString licenseFile() {
    QString licenseFile = firstLicensePath();

    QFile f(licenseFile);
    if (f.exists() && f.open(QIODevice::ReadOnly))
        return QString::fromLatin1(f.readAll());

    message(QLatin1String("Failed to read license file: ") + licenseFile);
    return QString();
}

bool isUsLicense() {
    QString text = licenseFile();
    QRegExp re = licenseKeyExtRegExp();
    if (re.indexIn(text) >= 0) {
        QString key = re.cap(4);
        if (key.isEmpty())
            return false;
        char pos = key.at(0).toLatin1();
        return pos == 'G';
    }
    return false;
}

static bool execute(const QStringList &arguments)
{
    QProcess bp;

    QStringList args = arguments;
    QString app = args.takeFirst();

    bp.start(app, args);
    if (!bp.waitForFinished(-1))
        return false;

    return bp.exitCode() == 0;
}

static void runQMake(const QString &dir,
                     const QStringList &configs,
                     const QStringList &antiConfigs,
                     const QString &prefix,
                     const QString &target)
{
    QString old_dir = QDir::currentPath();
    QDir::setCurrent(dir);

    // make the top level Makefile
    QStringList args;
    args.append(QLatin1String("qmake"));
    if (!prefix.isEmpty())
        args.append(QLatin1String("QSA_INSTALL_PREFIX=") + prefix);
    if (!target.isNull()) {
        args.append(QLatin1String("-o"));
        args.append(target);
    }

#if defined Q_WS_MACX
    args << "-spec" << "macx-g++";
#endif

    if (configs.size() > 0 || antiConfigs.size() > 0) {
        args.append("-after");
        if (!configs.isEmpty())
            args.append( "CONFIG+=" + configs.join( " " ) );
        if (!antiConfigs.isEmpty())
            args << "CONFIG-=" + antiConfigs.join(" ");
    }

    if(!execute(args)) {
        warnings++;
        qmake_warning = true;
    }

    QDir::setCurrent(old_dir);
}


#if QT_VERSION > 0x040100
static void runQMakeRecursive(const QString &dir,
                              const QStringList &configs,
                              const QStringList &anti_configs,
                              const QString &prefix,
                              const QString &target)
{
    QDir d(dir);

    QStringList pro_files = d.entryList(QStringList() << "*.pro", QDir::Files);
    if (pro_files.size())
        runQMake(dir, configs, anti_configs, prefix, target);

    QStringList subdirs = d.entryList(QDir::Dirs);
    for (int i=0; i<subdirs.size(); ++i) {
        if (subdirs.at(i) == "." || subdirs.at(i) == "..")
            continue;
        runQMakeRecursive(d.absoluteFilePath(subdirs.at(i)), configs, anti_configs, prefix, target);
    }
}
#endif

void runQMake(const QStringList &configs, const QStringList &anti_configs, const QString &prefix)
{
#if QT_VERSION > 0x040100
    runQMakeRecursive(QLatin1String("src"), configs, anti_configs,
                      prefix, QLatin1String("Makefile.qsa"));
    runQMakeRecursive(QLatin1String("examples"), configs, anti_configs,
                      prefix, QLatin1String("Makefile.qsa"));
#endif
    runQMake(QLatin1String("."), configs, anti_configs, prefix, QLatin1String("Makefile.qsa"));
}



void copyQSAHeaders()
{
    // install qmake feature file
    mkDir(*qtDir + QLatin1String("mkspecs/features"));
    copy(QLatin1String("src/qsa/qsa.prf"), *qtDir + QLatin1String("mkspecs/features/qsa.prf"));

    // install header files
    static const char *headers[11] = {
        "qsa/qsaglobal.h",
        "qsa/qsobjectfactory.h",
        "qsa/qswrapperfactory.h",
        "qsa/qseditor.h",
        "qsa/qsproject.h",
        "qsa/qsinterpreter.h",
        "qsa/qsinputdialogfactory.h",
        "qsa/qsutilfactory.h",
        "qsa/qsscript.h",
        "qsa/qsconfig.h",
        "ide/qsworkbench.h"
    };
    for (int j = 0; j < int(sizeof(headers) / sizeof(char*)); ++j) {
        QString header = QLatin1String(headers[j]);
        QString target = header.mid(header.indexOf(QLatin1Char('/')) + 1);
        symLink(QLatin1String("src/") + header, *qtDir + QLatin1String("include/") + target);
    }
}


static void removeContentFile(const QString &name)
{
    QStringList lst;
    lst.append(*assistant);
    // Assistant works differently in 3.2
    lst.append(QLatin1String("-removeContentFile"));
    lst.append(QDir::convertSeparators(name));
    execute(lst);
}


static void addContentFile(const QString &name)
{
    removeContentFile(name);
    QStringList lst;
    lst.append(*assistant);
    // Assistant works differently in 3.2
    lst.append(QLatin1String("-addContentFile"));
    lst.append(QDir::convertSeparators(name));
    if(!execute(lst)) {
        warnings++;
        qdoc_warning = true;
    }
}


void installDocs()
{
    if (!assistant) {
        assistant = new QString(QLatin1String("assistant"));
#ifdef Q_OS_MACX
        QStringList dirs = QString::fromLocal8Bit(qgetenv("PATH")).split(QLatin1Char(':'));
        QStringList::const_iterator it;
        for (it = dirs.constBegin(); it != dirs.constEnd(); ++it) {
            QString a = *it + QLatin1String("/assistant.app/Contents/MacOS/assistant");
            if (QFile::exists(a)) {
                *assistant = a;
                break;
            }
        }
#endif
    }

    // install documentation
    QString html = QLatin1String("doc/html/");
    addContentFile(html + QLatin1String("qsa.dcf"));
}


void mkDir(const QString &dir)
{
    QDir current;
    current.mkdir(dir);
}


void copy(const QString &source, const QString &dest)
{
    QString s = QDir::convertSeparators(source);
    QString d = QDir::convertSeparators(dest);
#ifdef Q_OS_UNIX
    system("cp " + QFile::encodeName(s) + " " + QFile::encodeName(d));
    system("chmod +w " + QFile::encodeName(d));
#else
    QT_WA(
    {
        if (!CopyFileW((TCHAR*) s.utf16(), (TCHAR*) d.utf16(), false)) {
            message("Failed to copy file: " + s);
            errors++;
        }
        if (!SetFileAttributesW((TCHAR*) d.utf16(), FILE_ATTRIBUTE_NORMAL)) {
            message("Failed to set file attributes to normal");
            errors++;
        }
    }, {
        if (!CopyFileA(QFile::encodeName(s), QFile::encodeName(d), false)) {
            message("Failed to copy file: " + s);
            errors++;
        }
        if (!SetFileAttributesA(QFile::encodeName(d), FILE_ATTRIBUTE_NORMAL)) {
            message("Failed to set file attributes to normal");
            errors++;
        }
    });
#endif
}


void symLink(const QString &source, const QString &dest)
{
#ifdef Q_OS_UNIX
    QString s = QDir::convertSeparators(source);
    QFileInfo info(s);
    s = info.absoluteFilePath();
    QString d = QDir::convertSeparators(dest);
    system("rm -f " + QFile::encodeName(d));
    system("ln -s " + QFile::encodeName(s) + " " + QFile::encodeName(d));
#else
    copy(source, dest);
#endif
}


uint convertor(const QString &list)
{
    static const unsigned char checksum[] = {
        0x61, 0x74, 0x18, 0x10, 0x06, 0x74, 0x76, 0x0b, 0x02, 0x7b,
            0x78, 0x18, 0x65, 0x72, 0x06, 0x76, 0x6d, 0x1f, 0x01, 0x75,
            0x7e, 0x79, 0x65, 0x01, 0x03, 0x06, 0x6c, 0x6e, 0x18, 0x14,
            0x8f, 0x75, 0x6a, 0x7a, 0x18, 0x7b, 0x76, 0x01, 0x1f, 0x7b,
            0x65, 0x72, 0x06, 0x06, 0x74, 0x76, 0x1f, 0x61, 0x03, 0x6a
    };

    uint length = 0;
    int temp = list.length();
    while (temp > 0) {
        temp--;
        uint alpha = 0x58;
        int currentIndex = 0;
        for (;;) {
            if ((uint)list.at(temp).toLatin1() == alpha) {
                length -= (length << 5) + currentIndex;
                break;
            }
            alpha ^= (uchar)checksum[currentIndex];
            if ((uchar)checksum[currentIndex] == 0x8f)
                return checksum[currentIndex] ^ 0x80;
            ++currentIndex;
        }
        length = uint(-int(length));
        if ((uint) (alpha - 0x8a) < 6)
            length += checksum[alpha - 0x8a];
    }
    return length;
}


bool checkLicense()
{
    enum LicenseType { None,
                       GPL,
                       Evaluation,
                       Console,
                       DesktopLight,
                       Desktop,
                       Universal,
                       Internal,
                       Preview };

    LicenseType qsaLicense = None;
    LicenseType qtLicense = None;
    bool usVersion = false;

    QString qtEdition = QLibraryInfo::licensedProducts();
    if (qtEdition == QLatin1String("Trolltech")) {
        qtLicense = Internal;
    } else if (qtEdition == QLatin1String("Evaluation")) {
        qtLicense = Evaluation;
    } else if (qtEdition == QLatin1String("Desktop")) {
        qtLicense = Desktop;
    } else if (qtEdition == QLatin1String("DesktopLight")) {
        qtLicense = DesktopLight;
    } else if (qtEdition == QLatin1String("Console")) {
        qtLicense = Console;
    } else if (qtEdition == QLatin1String("Universal")) {
        qtLicense = Universal;
    } else if (qtEdition == QLatin1String("OpenSource")) {
        qtLicense = GPL;
    } else if (qtEdition == QLatin1String("Preview")) {
        qtLicense = Preview;
    } else {
        message(QLatin1String("\nQSA is not compatible with your Qt edition\n"));
        return false;
    }

    switch (qtLicense) {
        case Desktop:
        case DesktopLight:
        case Console:
        case Universal:
            usVersion = isUsLicense();
            break ;
        default:
            break ;
    };

    if(qtLicense != Internal) {
        if (QFile::exists(QLatin1String("LICENSE.GPL"))) {
            qsaLicense = GPL;
        } else if (QFile::exists(QLatin1String("LICENSE.PREVIEW"))) {
            qsaLicense = Preview;
        } else if (QFile::exists(QLatin1String("LICENSE.EVAL"))) {
            qsaLicense = Evaluation;
        } else {
            if (!QFile::exists(QLatin1String("LICENSE")))
                copy(usVersion ? QLatin1String(".LICENSE-US") : QLatin1String(".LICENSE"),
                        QLatin1String("LICENSE"));
            if (QFile::exists(QLatin1String("LICENSE"))) {
                QString text = licenseFile();
                if(text.isEmpty()) {
                    message(QLatin1String("Failed to determine Qt license information"));
                    return false;
                }
                QRegExp re = licenseKeyExtRegExp();
                if (re.indexIn(text)) {
                    QString cap = re.cap(1);
                    char id = (cap.length() >= 5) ? cap.at(4).toLatin1() : 0;
                    qsaLicense = (id == 'U' || id == 'C' || id == 'E' || id == '7') ? Desktop : None;
                }
            }
        }
    }

    if (qtLicense == Internal
        || qtLicense == qsaLicense
        || (qtLicense == Universal && qsaLicense != GPL)
        || (qsaLicense == Evaluation && (qtLicense == Desktop || qtLicense == Universal))
        || qsaLicense == Preview
        || (qtLicense == Evaluation && qsaLicense == Desktop)  // Qt/QSA in full source eval
        || qtLicense == Preview)
        {
        return true;
    } else if ((qtLicense == Desktop || qtLicense == DesktopLight
                || qtLicense == Console || qtLicense == Universal) &&
               qsaLicense == GPL) {
        warnings++;
        message(QLatin1String("\nThis QSA Free Edition is licensed under the GNU General "
                "Public License.\nNote that any derivatives of this software "
                "must also be licensed under\nthe GNU GPL.\n\n"
                "See http://www.trolltech.com/gpl for more information or\n"
                "contact sales@trolltech.com for a commercial edition of QSA.\n"));
        return true;
    } else {
        message(QLatin1String("\nThe required license information cannot be found"
                "\nor the information is conflicting!\n"));
        QString str = QString::fromLatin1("Error code: %1%2\n\n").arg(qtLicense).arg(qsaLicense);
        message(str);
        return false;
    }
    return false;
}


bool writeQSConfig(bool buildIde, bool buildEditor, bool buildNewEditor)
{
    QFile file(QLatin1String("src/qsa/qsconfig.h"));
    if(!file.open(QIODevice::WriteOnly)) {
        message(QLatin1String("Failed to open 'src/qsa/qsconfig.h' for writing."));
        return false;
    }
    QTextStream txt(&file);
    txt << "// This file is autogenerated by QSA configure, do not modify it!\n"
        << "#ifndef QS_CONFIG_H\n"
        << "#define QS_CONFIG_H\n"
        << "\n";
    if (!buildIde || !buildEditor)
        txt << "#define QSA_NO_IDE\n";
    if (!buildEditor)
        txt << "#define QSA_NO_EDITOR\n";
    if (buildNewEditor)
        txt << "#define QSA_NEW_EDITOR\n";
    txt << "\n"
        << "#endif\n";
    return true;
}

void rmDirRecursive(const QDir &dir)
{
    QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Drives | QDir::System | QDir::Hidden);
    if (!list.isEmpty()) {
        for (int i=0; i<list.size(); ++i) {
            const QFileInfo &fi = list.at(i);
            if((fi.fileName() != QLatin1String(".")) && (fi.fileName() != QLatin1String(".."))){
                if(fi.isDir())
                    rmDirRecursive(QDir(fi.absoluteFilePath()));
                else
                    QFile::remove(fi.absoluteFilePath());
            }
        }
    }
    // Remove this dir as well
    dir.rmdir(dir.absolutePath());
}

bool writeExtraConfigs(const QStringList &s)
{
    QFile file(QLatin1String(".qmake.cache"));
    if (!file.open(QIODevice::Text | QIODevice::Append)) {
        message(QLatin1String("Failed write configuration to '.qmake.cache'"));
        return false;
    }

    QTextStream stream(&file);
    stream << "CONFIG += " << s.join(QLatin1String(" ")) << endl;
    return true;
}


const char *contents_qsa_prf =
"# This file is automatically generated, don't touch"
"\n"
"\nCONFIG		+= qt warn_on"
"\n"
"\ncontains(QT_CONFIG, release):contains(QT_CONFIG, debug) {"
"\n    # Qt was configued with both debug and release libs"
"\n    CONFIG += debug_and_release build_all"
"\n}"
"\n"
"\nCONFIG(debug, debug|release) {"
"\n    unix:LIBS += -lqsa_debug"
"\n    else:LIBS += -lqsad"
"\n} else {"
"\n    LIBS += -lqsa"
"\n}"
"\n"
"\n"
"\n!shared {"
"\n	DEFINES += QSA_NO_DLL"
"\n}"
"\n";

bool writeQsaPrfFile(bool include_qt3_support)
{
    QFile file("src/qsa/qsa.prf");
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        message("Failed to open file '" + file.fileName() + "' for writing");
        return false;
    }

    QTextStream stream(&file);
    stream << contents_qsa_prf;

    if (include_qt3_support)
        stream << "mac:QT += qt3support" << endl;

    if (qsa_prefix && !qsa_prefix->isEmpty()) {
        stream << "INCLUDEPATH += " << QString(*qsa_prefix).replace("\\", "/") << endl;
    }

    stream.flush();

    return true;
}
