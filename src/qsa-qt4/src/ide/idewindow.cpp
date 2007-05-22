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

#include "idewindow.h"
#include "preferences.h"
#include "ui_projectcontainer.h"
#include "ui_findtext.h"
#include "ui_gotoline.h"
#include "ui_outputcontainer.h"
#include "ui_preferencescontainer.h"
#include "ui_replacetext.h"

#include <qseditor.h>
#include <qsscript.h>
#include <qsproject.h>
#include <qsinterpreter.h>

#include <stdlib.h>
#include <QTimer>
#include <QStyle>
#include <QPrinter>
#include <QPainter>
#include <QBoxLayout>
#include <QShowEvent>
#include <QHideEvent>
#include <QTextStream>
#include <QPrintDialog>
#include <QTextEdit>
#include <QTreeWidget>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDockWidget>
#include <QHeaderView>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QSettings>

class QSProjectContainer: public QWidget, public Ui::QSProjectContainer
{
public:
    QSProjectContainer(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class QSFindText: public QDialog, public Ui::QSFindText
{
public:
    QSFindText(QWidget *parent = 0)
        : QDialog(parent)
    {
        setupUi(this);
    }
};

class QSGotoLine: public QDialog, public Ui::QSGotoLine
{
public:
    QSGotoLine(QWidget *parent = 0)
        : QDialog(parent)
    {
        setupUi(this);
    }
};

class QSOutputContainer: public QWidget, public Ui::QSOutputContainer
{
public:
    QSOutputContainer(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

class QSPreferencesContainer: public QDialog, public Ui::QSPreferencesContainer
{
public:
    QSPreferencesContainer(QWidget *parent = 0)
        : QDialog(parent)
    {
        setupUi(this);
    }
};

class QSReplaceText: public QDialog, public Ui::QSReplaceText
{
public:
    QSReplaceText(QWidget *parent = 0)
        : QDialog(parent)
    {
        setupUi(this);
    }
};


#ifndef QSA_NEW_EDITOR
#   include <Q3TextEdit>
#   include <Q3PaintDeviceMetrics>
#   include <Q3SimpleRichText>
#endif

static QTextEdit *debugoutput = 0;
static void (*qt_default_message_handler)(QtMsgType, const char *msg);

void debugMessageOutput(QtMsgType type, const char *msg)
{
    // So that we don't override others defaults...
    if (qt_default_message_handler) {
        qt_default_message_handler(type, msg);
    } else {
        fprintf(stderr, "%s\n", msg);
        fflush(stderr);
    }

    if (type != QtFatalMsg) {
        if (debugoutput)
            debugoutput->append(Qt::convertFromPlainText(msg) + QLatin1String("\n"));
    } else {
        fprintf(stderr, msg);
        abort();
    }
    QCoreApplication::flush();
}

static QIcon createIconSet(const QString &name)
{
    QIcon ic(QString::fromLatin1(":/images/") + name);
    return ic;
}

class ScriptListItem : public QTreeWidgetItem
{
    friend class IdeWindow;

public:
    ScriptListItem(QTreeWidget *parent, QSScript *script)
	  : QTreeWidgetItem(parent), qss(script)
    {
        setText(0, script->name());

        QString fname = (qss && qss->context() ?
                         QString::fromLatin1(":/images/scriptobject.png") :
                         QString::fromLatin1(":/images/script.png"));
        setIcon(0, QPixmap(fname));
    }

private:
    QSScript *qss;
};

IdeWindow::IdeWindow(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, fl)
{
    setObjectName(name);
    setupUi(this);
    init();
}

IdeWindow::~IdeWindow()
{
}

void IdeWindow::scriptNew()
{
    bool ok;
    QString qsn =
        QInputDialog::getText(this, tr("Input Script Name"),
                              tr("Script Name: "),
                              QLineEdit::Normal,
                              tr("Script%1.qs")
                                .arg(project->scripts().count() + 1),
                              &ok);

    if (ok && !qsn.isEmpty()) {
        QSScript *script = project->createScript(qsn);
        if (!script)
            return;
        addPage(script);
        enableEditActions(true);
        enableProjectActions(true);
    }
}


void IdeWindow::scriptImport()
{
    QStringList names = QFileDialog::getOpenFileNames(this, tr("Import Script"));
    for (QStringList::Iterator it = names.begin(); it != names.end(); ++it) {
        QFile file(*it);
        if (file.open(QIODevice::ReadOnly)) {
            QSScript *script = project->createScript(QFileInfo(file).fileName(), file.readAll());
            if (!script) return;
            addPage(script);
            enableEditActions(true);
            enableProjectActions(true);
        } else {
            QMessageBox::information(this, tr("Import script failed"),
                tr("The file '%1' is not readable and cannot be imported.").arg(*it),
                QMessageBox::Ok);

            return ;
        }
    }
}


void IdeWindow::scriptExport()
{
    QSScript *script = 0;
    QSEditor *activeEditor = static_cast<QSEditor *>(tabWidget->currentWidget());
    if (activeEditor) {
        script = activeEditor->script();
    } else {
        ScriptListItem *item = (ScriptListItem *)projectContainer->scriptsListView->currentItem();
        if (!item)
            return;
        script = item->qss;
    }
    QString name = QFileDialog::getSaveFileName(this,
                                                tr("Export Script"),
                                                script->name(),
                                                tr("Qt Script files(*.qs)"));

    QFile file(name);
    if (!name.isEmpty()) {
        if(!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Export script failed"),
                tr("The file '%1' could not be opened for\nwriting. Script '%2' was not exported.")
                .arg(name).arg(script->name()),
                QMessageBox::Ok);

            return ;
        }
        QSEditor *editor = project->editor(script);
        if (editor)
            editor->commit();
        QTextStream stream(&file);
        stream << script->code();
    }
}



void IdeWindow::scriptPrint()
{
    QSEditor *activeEditor = static_cast<QSEditor *>(tabWidget->currentWidget());
    if (!activeEditor)
        return ;

#if defined(QSA_NEW_EDITOR)
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);

    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (dlg->exec() == QDialog::Accepted) {
        activeEditor->textEdit()->document()->print(&printer);
    }
    delete dlg;
#else
    QString scriptName = activeEditor->script()->name();
    Q3TextEdit *te = activeEditor->textEdit();
    QString printString = te->text();

    // fix formatting
    printString.replace('&', QString::fromLatin1("&amp;"));
    printString.replace('<', QString::fromLatin1("&lt;"));
    printString.replace('>', QString::fromLatin1("&gt;"));
    printString.replace('\n', QString::fromLatin1("<br>\n"));
    printString.replace('\t', QString::fromLatin1("        "));
    printString.replace(' ', QString::fromLatin1("&nbsp;"));

    printString = QString::fromLatin1("<html><body>") + printString +
                  QString::fromLatin1("</body></html>");

    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);
    if (printer.setup(this)) {
        QPainter p(&printer);

        // Check that there is a valid device to print to.
        if (!p.device())
            return;

        Q3PaintDeviceMetrics metrics(p.device());
        int dpiy = metrics.logicalDpiY();
        int margin = (int) ((2/2.54)*dpiy); // 2 cm margins
        QRect body(margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin);
        QFont font(te->QWidget::font());
        font.setPointSize(10); // we define 10pt to be a nice base size for printing

        Q3SimpleRichText richText(printString, font,
            te->context(),
            te->styleSheet(),
            te->mimeSourceFactory(),
            body.height());
        richText.setWidth(&p, body.width());
        QRect view(body);
        int page = 1;
        do {
            richText.draw(&p, body.left(), body.top(), view, colorGroup());
            view.moveBy(0, body.height());
            p.translate(0 , -body.height());
            p.setFont(font);
            QString renderText = scriptName + QString::fromLatin1(", ") + QString::number(page);
            p.drawText(view.right() - p.fontMetrics().width(renderText),
                view.bottom() + p.fontMetrics().ascent() + 5,
                renderText);
            if (view.top()  >= richText.height())
                break ;
            printer.newPage();
            page++;
        } while (true);
    }
#endif
}


void IdeWindow::scriptExit()
{
    // ###
}

void IdeWindow::scriptRename()
{
    QSEditor *activeEditor = static_cast<QSEditor *>(tabWidget->currentWidget());
    if (!activeEditor)
        return ;

    QSScript *script = activeEditor->script();
    if (script->context()) {
        QMessageBox::information(this,
                                 tr("Rename script failed"),
                                 tr("Unable to rename object scripts"),
                                 QMessageBox::Ok);
        return ;
    }

    bool ok = false;
    QString newName = QInputDialog::getText(this, tr("Rename script: %1").arg(script->name()),
                                            tr("New script &Name: "),
                                            QLineEdit::Normal,
                                            QString::fromLatin1("Script%1.qs")
                                                .arg(project->scripts().count()),
                                            &ok);

    if (ok && !newName.isEmpty()) {
	    QSScript *newScript = project->createScript(newName, script->code());
	    if (!newScript) return;
            delete script;
	    addPage(newScript);
	    enableEditActions(true);
	    enableProjectActions(true);
    }
}

void IdeWindow::editUndo()
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor) return;
    editor->undo();
}


void IdeWindow::editRedo()
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor) return;
    editor->redo();
}


void IdeWindow::editCut()
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor) return;
    editor->cut();
}


void IdeWindow::editCopy()
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor) return;
    editor->copy();
}


void IdeWindow::editPaste()
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor) return;
    editor->paste();
}

void IdeWindow::editFindNext()
{
    findText->radioForward->setChecked(true);
    editFind();
}

void IdeWindow::editFindPrev()
{
    findText->radioBackward->setChecked(true);
    editFind();
}

void IdeWindow::editFind()
{
    if (findText->comboFind->currentText().isEmpty()) {
        findText->show();
        return;
    }

    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor)
        return;
    QString findString = findText->comboFind->currentText();
    findText->comboFind->addItem(findString);
    bool caseSensitive = findText->checkCase->isChecked();
    bool wholeWordsOnly = findText->checkWhole->isChecked();
    bool startAtCursor = !findText->checkStart->isChecked();
    bool forward = findText->radioForward->isChecked();
    editor->find(findString, caseSensitive, wholeWordsOnly, forward, startAtCursor);
}


void IdeWindow::editReplace()
{
    editReplace(false);
}


void IdeWindow::editReplaceAll()
{
    editReplace(true);
}


void IdeWindow::editReplace(bool all)
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor)
        return;
    QString findString = replaceText->comboFind->currentText();
    replaceText->comboFind->addItem(findString);
    QString replaceString = replaceText->comboReplace->currentText();
    replaceText->comboFind->addItem(replaceString);
    bool caseSensitive = replaceText->checkCase->isChecked();
    bool wholeWordsOnly = replaceText->checkWhole->isChecked();
    bool startAtCursor = !replaceText->checkStart->isChecked();
    bool forward = replaceText->radioForward->isChecked();
    editor->replace(findString, replaceString, caseSensitive,
		            wholeWordsOnly, forward, startAtCursor, all);
}


void IdeWindow::editSelectAll()
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    editor->selectAll();
}


void IdeWindow::editPreferences()
{
    qsaEditorSyntax->reInit();
    preferencesContainer->show();
}


void IdeWindow::editGotoLine()
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    gotoLine->hide();
    editor->setFocus();
    int val = gotoLine->spinLine->value();
    editor->gotoLine(val);
}


void IdeWindow::helpIndex()
{
    // ###
}


void IdeWindow::helpContents()
{
    // ###
}


void IdeWindow::helpAbout()
{
    QMessageBox box(this);
    box.setText(QString::fromLatin1("<center><img src=\":/images/splash.png\">"
		 "<p>Version " QSA_VERSION_STRING  "</p>"
		 "<p>Copyright (C) 2001-$THISYEAR$ Trolltech AS. All rights reserved.</p>"
		 "</center><p></p>"
		 "<p>QSA Commercial Edition license holders: This program is"
		 " licensed to you under the terms of the QSA Commercial License"
		 " Agreement. For details, see the file LICENSE that came with"
		 " this software distribution.</p><p></p>"
		 "<p>QSA Free Edition users: This program is licensed to you"
		 " under the terms of the GNU General Public License Version 2."
		 " For details, see the file LICENSE.GPL that came with this"
		 " software distribution.</p><p>The program is provided AS IS"
		 " with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF"
		 " DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE."
                                     "</p>")
		);
    box.setWindowTitle(tr("About QSA Workbench"));
    box.setIcon(QMessageBox::NoIcon);
    box.exec();
}


void IdeWindow::addPage(QSScript *s)
{
    Q_ASSERT(s);
    QSEditor *editor = project->createEditor(s, 0, 0);
    tabWidget->addTab(editor, s->name());
    int idx = tabWidget->indexOf(editor);
    tabWidget->setCurrentIndex(idx);
    enableEditActions(true);
}


void IdeWindow::removePage(QSScript *s)
{
    QSEditor *editor = project->editor(s);
    if (!editor)
        return;
    editor->commit();
    tabWidget->removeTab(tabWidget->indexOf(editor));
    delete editor;
    enableEditActions(tabWidget->count() > 0);
}


void IdeWindow::removePage()
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor)
        return;
    editor->commit();
    tabWidget->removeTab(tabWidget->indexOf(editor));
    delete editor;
    enableEditActions(tabWidget->count() > 0);
}


void IdeWindow::showPage(QSScript *s)
{
    QSEditor *editor = project->editor(s);
    int idx = -1;
    if (editor && (idx = tabWidget->indexOf(editor)) != -1) {
	    tabWidget->setCurrentIndex(idx);
	    return;
    }
    addPage(s);
}


void IdeWindow::showFunction(QSScript *s, const QString &f)
{
    showPage(s);
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor)
        return;
    editor->find("function " + f, false, true, true, false);
}

void IdeWindow::initMenuAndToolBar()
{
    menubar = new QMenuBar(this);
    setMenuBar(menubar);
    fileMenu = menubar->addMenu(tr("&File"));
    editMenu = menubar->addMenu(tr("&Edit"));
    searchMenu = menubar->addMenu(tr("&Search"));
    projectMenu = menubar->addMenu(tr("&Project"));
    windowMenu = menubar->addMenu(tr("&Window"));
    helpMenu = menubar->addMenu(tr("&Help"));

    fileNewAction = new QAction(tr("&New Script"), this);
        connect(fileNewAction, SIGNAL(triggered(bool)), this, SLOT(scriptNew()));
        fileNewAction->setShortcut(QKeySequence(tr("Ctrl+N")));
        fileNewAction->setIcon(createIconSet("scriptnew.png"));
        fileMenu->addAction(fileNewAction);

    fileImportAction = new QAction(tr("&Import Script"), this);
        connect(fileImportAction, SIGNAL(triggered(bool)), this, SLOT(scriptImport()));
        fileImportAction->setShortcut(QKeySequence(tr("Ctrl+I")));
        fileImportAction->setIcon(createIconSet("fileopen.png"));
        fileMenu->addAction(fileImportAction);
    fileExportAction = new QAction(tr("&Export Script"), this);
        connect(fileExportAction, SIGNAL(triggered(bool)), this, SLOT(scriptExport()));
        fileExportAction->setShortcut(QKeySequence(tr("Ctrl+E")));
        fileExportAction->setEnabled(false);
        fileExportAction->setIcon(createIconSet("filesave.png"));
        fileMenu->addAction(fileExportAction);
        fileMenu->addSeparator();
    fileRenameAction = new QAction(tr("&Rename Script"), this);
        connect(fileRenameAction, SIGNAL(triggered(bool)), this, SLOT(scriptRename()));
        fileRenameAction->setShortcut(QKeySequence(tr("Ctrl+R")));
        fileMenu->addAction(fileRenameAction);
    fileRemoveAction = new QAction(tr("&Remove Script"), this);
        connect(fileRemoveAction, SIGNAL(triggered(bool)), this, SLOT(removeScript()));
        fileRemoveAction->setShortcut(QKeySequence(tr("Ctrl+D")));
        fileRemoveAction->setEnabled(false);
        fileRemoveAction->setIcon(createIconSet("editdelete.png"));
        fileMenu->addAction(fileRemoveAction);
    fileCloseAction = new QAction(tr("Close &Tab"), this);
        connect(fileCloseAction, SIGNAL(triggered(bool)), this, SLOT(removePage()));
        fileCloseAction->setShortcut(QKeySequence(tr("Ctrl+F4")));
        fileCloseAction->setEnabled(false);
        fileMenu->addAction(fileCloseAction);
        fileMenu->addSeparator();
    filePrintAction = new QAction(tr("&Print Script"), this);
        connect(filePrintAction, SIGNAL(triggered(bool)), this, SLOT(scriptPrint()));
        filePrintAction->setShortcut(QKeySequence(tr("Ctrl+P")));
        fileMenu->addAction(filePrintAction);
        fileMenu->addSeparator();
    fileExitAction = new QAction(tr("E&xit"), this);
        connect(fileExitAction, SIGNAL(triggered(bool)), this, SLOT(scriptExit()));
        connect(fileExitAction, SIGNAL(triggered(bool)), this, SLOT(close()));
        fileMenu->addAction(fileExitAction);

    editUndoAction = new QAction(tr("&Undo"), this);
        connect(editUndoAction, SIGNAL(triggered(bool)), this, SLOT(editUndo()));
        editUndoAction->setShortcut(QKeySequence(tr("Ctrl+Z")));
        editUndoAction->setEnabled(false);
        editUndoAction->setIcon(createIconSet("undo.png"));
        editMenu->addAction(editUndoAction);
    editRedoAction = new QAction(tr("&Redo"), this);
        connect(editRedoAction, SIGNAL(triggered(bool)), this, SLOT(editRedo()));
        editRedoAction->setShortcut(QKeySequence(tr("Ctrl+Y")));
        editRedoAction->setEnabled(false);
        editRedoAction->setIcon(createIconSet("redo.png"));
        editMenu->addAction(editRedoAction);
        editMenu->addSeparator();
    editCutAction = new QAction(tr("&Cut"), this);
        connect(editCutAction, SIGNAL(triggered(bool)), this, SLOT(editCut()));
        editCutAction->setShortcut(QKeySequence(tr("Ctrl+X")));
        editCutAction->setEnabled(false);
        editCutAction->setIcon(createIconSet("editcut.png"));
        editMenu->addAction(editCutAction);
    editCopyAction = new QAction(tr("C&opy"), this);
        connect(editCopyAction, SIGNAL(triggered(bool)), this, SLOT(editCopy()));
        editCopyAction->setShortcut(QKeySequence(tr("Ctrl+C")));
        editCopyAction->setEnabled(false);
        editCopyAction->setIcon(createIconSet("editcopy.png"));
        editMenu->addAction(editCopyAction);
    editPasteAction = new QAction(tr("&Paste"), this);
        connect(editPasteAction, SIGNAL(triggered(bool)), this, SLOT(editPaste()));
        editPasteAction->setShortcut(QKeySequence(tr("Ctrl+V")));
        editPasteAction->setEnabled(false);
        editPasteAction->setIcon(createIconSet("editpaste.png"));
        editMenu->addAction(editPasteAction);
    editSelectAllAction = new QAction(tr("Select &All"), this);
        connect(editSelectAllAction, SIGNAL(triggered(bool)), this, SLOT(editSelectAll()));
        editSelectAllAction->setShortcut(QKeySequence(tr("Ctrl+A")));
        editSelectAllAction->setEnabled(false);
        editMenu->addAction(editSelectAllAction);
        editMenu->addSeparator();
    editPreferencesAction = new QAction(tr("Preferences..."), this);
        connect(editPreferencesAction, SIGNAL(triggered(bool)), this, SLOT(editPreferences()));
        editMenu->addAction(editPreferencesAction);

    editFindAction = new QAction(tr("&Find"), this);
        editFindAction->setShortcut(QKeySequence(tr("Ctrl+F")));
        editFindAction->setEnabled(false);
        editFindAction->setIcon(createIconSet("searchfind.png"));
        searchMenu->addAction(editFindAction);
    editFindAgainAction = new QAction(tr("Find &Next"), this);
        connect(editFindAgainAction, SIGNAL(triggered(bool)), this, SLOT(editFindNext()));
        editFindAgainAction->setShortcut(QKeySequence(tr("F3")));
        editFindAgainAction->setEnabled(false);
        searchMenu->addAction(editFindAgainAction);
    editFindAgainBWAction = new QAction(tr("Find &Previous"), this);
        connect(editFindAgainBWAction, SIGNAL(triggered(bool)), this, SLOT(editFindPrev()));
        editFindAgainBWAction->setShortcut(QKeySequence(tr("Shift+F3")));
        editFindAgainBWAction->setEnabled(false);
        searchMenu->addAction(editFindAgainBWAction);
    editReplaceAction = new QAction(tr("&Replace"), this);
        editReplaceAction->setShortcut(QKeySequence(tr("Ctrl+H")));
        editReplaceAction->setEnabled(false);
        searchMenu->addAction(editReplaceAction);
        searchMenu->addSeparator();
    editGotoLineAction = new QAction(tr("&Goto Line"), this);
        editGotoLineAction->setShortcut(QKeySequence(tr("Ctrl+G")));
        editGotoLineAction->setEnabled(false);
        searchMenu->addAction(editGotoLineAction);

    projectCallAction = new QAction(tr("&Call Function"), this);
        connect(projectCallAction, SIGNAL(triggered(bool)), this, SLOT(projectCall()));
        projectCallAction->setShortcut(QKeySequence(tr("F5")));
        projectCallAction->setEnabled(false);
        projectCallAction->setIcon(createIconSet("play.png"));
        projectMenu->addAction(projectCallAction);
    projectRunAction = new QAction(tr("Call &Previous"), this);
        connect(projectRunAction, SIGNAL(triggered(bool)), this, SLOT(projectRun()));
        projectRunAction->setShortcut(QKeySequence(tr("F6")));
        projectRunAction->setEnabled(false);
        projectRunAction->setIcon(createIconSet("playprev.png"));
        projectMenu->addAction(projectRunAction);
    projectStopAction = new QAction(tr("&Kill Interpreter"), this);
        connect(projectStopAction, SIGNAL(triggered(bool)), this, SLOT(projectStop()));
        projectStopAction->setShortcut(QKeySequence(tr("F8")));
        projectStopAction->setEnabled(false);
        projectStopAction->setIcon(createIconSet("stop.png"));
        projectMenu->addAction(projectStopAction);
        projectMenu->addSeparator();
    projectEvaluateAction = new QAction(tr("&Evaluate Project"), this);
        connect(projectEvaluateAction, SIGNAL(triggered(bool)), this, SLOT(evaluateProject()));
        projectEvaluateAction->setShortcut(QKeySequence(tr("F7")));
        projectEvaluateAction->setEnabled(false);
        projectMenu->addAction(projectEvaluateAction);

    helpAboutAction = new QAction(tr("&About"), this);
        connect(helpAboutAction, SIGNAL(triggered(bool)), this, SLOT(helpAbout()));
        helpAboutAction->setIcon(createIconSet("qsa.png"));
        helpMenu->addAction(helpAboutAction);
    helpContentsAction = new QAction(tr("&Contents..."), this);
        helpContentsAction->setIcon(createIconSet("help.png"));
        connect(helpContentsAction, SIGNAL(triggered(bool)), this, SLOT(helpContents()));
    helpIndexAction = new QAction(tr("&Index..."), this);
        connect(helpIndexAction, SIGNAL(triggered(bool)), this, SLOT(helpIndex()));

    fileToolBar = new QToolBar(tr("File"));
    fileToolBar->setObjectName("toolbar_file");
        fileToolBar->addAction(fileNewAction);
        fileToolBar->addAction(fileImportAction);
        fileToolBar->addAction(fileExportAction);
        fileToolBar->addAction(fileRemoveAction);
        addToolBar(fileToolBar);

    editToolBar = new QToolBar(tr("Edit"));
    editToolBar->setObjectName("toolbar_edit");
        editToolBar->addAction(editUndoAction);
        editToolBar->addAction(editRedoAction);
        editToolBar->addSeparator();
        editToolBar->addAction(editCutAction);
        editToolBar->addAction(editCopyAction);
        editToolBar->addAction(editPasteAction);
        addToolBar(editToolBar);

    projectToolBar = new QToolBar(tr("Project"));
    projectToolBar->setObjectName("toolbar_edit");
        projectToolBar->addAction(projectCallAction);
        projectToolBar->addAction(projectRunAction);
        projectToolBar->addAction(projectStopAction);
        addToolBar(projectToolBar);
}


void IdeWindow::init()
{
    initMenuAndToolBar();

    int s = 16;
    QToolButton *closeTabButton = new QToolButton(tabWidget);
    closeTabButton->setAutoRaise(true);
    closeTabButton->setFixedSize(s, s);
    closeTabButton->setIcon(QIcon(style()->standardPixmap(QStyle::SP_TitleBarCloseButton)));
    QObject::connect(closeTabButton, SIGNAL(clicked()), this, SLOT(removePage()));
    closeTabButton->setToolTip(tr("Close tab"));
    tabWidget->setCornerWidget(closeTabButton, Qt::TopRightCorner);

    tabWidget->removeTab(0);

    QObject::connect(tabWidget, SIGNAL(currentChanged(int)),
		             this, SLOT(currentTabChanged(int)));

    projectContainerDock = new QDockWidget(this);
    projectContainerDock->setObjectName("dock_project");
    projectContainer = new QSProjectContainer(projectContainerDock);
    projectContainerDock->setWidget(projectContainer);
    projectContainerDock->setWindowTitle(tr("Project Scripts"));
    projectContainer->scriptsListView->header()->hide();
    addDockWidget(Qt::LeftDockWidgetArea, projectContainerDock);
    projectContainerDock->show();

#ifdef QS_VARIABLE_WATCH
    variablesContainerDock = new QDockWidget(this);
    variablesContainer = new QSVariablesContainer(variablesContainerDock, 0, false);
    variablesContainerDock->setResizeEnabled(true);
    addDockWidget(Qt::RightDockWidgetArea, variablesContainerDock);
    variablesContainerDock->setWidget(variablesContainer);
    variablesContainerDock->setWindowTitle(tr("Variables"));
    variablesContainerDock->setFixedExtentWidth(150);
    //variablesContainer->variablesListView->header()->hide();
    variablesContainer->show();
#endif

    outputContainerDock = new QDockWidget(this);
    outputContainerDock->setObjectName("dock_output");
    outputContainer = new QSOutputContainer(outputContainerDock);
    addDockWidget(Qt::BottomDockWidgetArea, outputContainerDock);
    outputContainerDock->setWidget(outputContainer);
    outputContainerDock->setWindowTitle(tr("Output"));
    outputContainerDock->show();

    QObject::connect(projectContainer->scriptsListView,
		             SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		             this, SLOT(scriptItemDoubleClicked(QTreeWidgetItem *)));

    findText = new QSFindText(this);
    QObject::connect(editFindAction, SIGNAL(triggered(bool)), findText, SLOT(show()));
    QObject::connect(findText->pushFind, SIGNAL(clicked()), this, SLOT(editFind()));

    replaceText = new QSReplaceText(this);
    QObject::connect(editReplaceAction, SIGNAL(triggered(bool)), replaceText, SLOT(show()));
    QObject::connect(replaceText->pushReplace, SIGNAL(clicked()), this, SLOT(editReplace()));
    QObject::connect(replaceText->pushReplaceAll, SIGNAL(clicked()), this, SLOT(editReplaceAll()));
    gotoLine = new QSGotoLine(this);
    QObject::connect(editGotoLineAction, SIGNAL(triggered(bool)), gotoLine, SLOT(show()));
    QObject::connect(gotoLine->pushGoto, SIGNAL(clicked()), this, SLOT(editGotoLine()));

    preferencesContainer = new QSPreferencesContainer(this);
    QBoxLayout *preferencesLayout = new QBoxLayout(QBoxLayout::Down, preferencesContainer->frame);
    qsaEditorSyntax = new PreferencesBase(preferencesContainer->frame);
    preferencesLayout->addWidget(qsaEditorSyntax);
    QObject::connect(preferencesContainer->pushOk, SIGNAL(clicked()),
                     this, SLOT(savePreferences()));


    windowMenu->addActions(createPopupMenu()->actions());

    // restore any old message handlers first...
    if (debugoutput)
        qInstallMsgHandler(0);
    debugoutput = outputContainer->textEdit;
    qt_default_message_handler = qInstallMsgHandler(debugMessageOutput);

    setWindowIcon(QPixmap(QString::fromLatin1(":/images/qsa.png")));

    enableEditActions(false);
}


void IdeWindow::removeScript(QSScript *s)
{
    delete s;
    enableProjectActions(projectContainer->scriptsListView->topLevelItemCount() > 0);
    enableEditActions(tabWidget->count() > 0);
}


void IdeWindow::removeScript()
{
    ScriptListItem *item = (ScriptListItem *)projectContainer->scriptsListView->currentItem();
    if (!item)
	    return;
    QSScript *script = item->qss;
    delete script;
    enableProjectActions(projectContainer->scriptsListView->topLevelItemCount() > 0);
    enableEditActions(tabWidget->count() > 0);
}

void IdeWindow::setRunningState(bool running)
{
    projectStopAction->setEnabled(running);
    projectRunAction->setEnabled(!running);
    projectCallAction->setEnabled(!running);
    projectEvaluateAction->setEnabled(!running);
}

void IdeWindow::projectRun()
{
    hadError = false;
    project->commitEditorContents();
    QStringList functions = project->interpreter()->functions();
    if (hadError)
	    return;
    bool ok = true;
    if (runFunction.isNull()
	    || runFunction == QString::fromLatin1("")
	    || functions.indexOf(runFunction) == -1) {
	    runFunction = QInputDialog::getItem(this, tr("Call function"),
                                            tr("Function: "), functions,
					                        functions.indexOf(runFunction),
					                        false, &ok);
    }
    if (ok && !runFunction.isEmpty()) {
	    setRunningState(true);
	    project->interpreter()->call(runFunction);
	    setRunningState(false);
    }
}


void IdeWindow::projectCall()
{
    hadError = false;
    project->commitEditorContents();
    QStringList functions = project->interpreter()->functions();
    if (hadError)
        return;
    bool ok = false;
    runFunction = QInputDialog::getItem(this, tr("Call function"),
        tr("Function: "), functions,
        functions.indexOf(runFunction),
        false, &ok);
    if (ok && !runFunction.isEmpty()) {
        setRunningState(true);
        project->interpreter()->call(runFunction);
        setRunningState(false);
#ifdef QS_VARIABLE_WATCH
        variablesContainer->updateScope(project->interpreter());
#endif
    }
}


void IdeWindow::currentTabChanged(int idx)
{
    QWidget *w = tabWidget->widget(idx);
    QSEditor *editor = (QSEditor*)w;
    QList<QTreeWidgetItem *> items =
        projectContainer->scriptsListView->findItems(editor->script()->name(), 0);

    ScriptListItem *item = 0;
    if (items.count() > 0)
        item = (ScriptListItem *)items[0];

    if (item != 0)
        projectContainer->scriptsListView->setCurrentItem(item);
    textChanged();
}


void IdeWindow::projectError(const QString &msg, const QString &file, int line)
{
    hadError = true;

    QStringList error;

    error << QString::fromLatin1("<pre><font color=red><b>Error:</b></font> ") << file
        << QString::fromLatin1(" : <font color=blue>") << QString::number(line)
        << QString::fromLatin1("</font>\n") << QString::fromLatin1("<i>") << msg
        << QString::fromLatin1("</i>\n");

    QString stackTrace = project->interpreter()->stackTraceString();
    if(stackTrace.size()) {
        error << tr("Callstack:\n");
        error << tr("  ") << stackTrace.replace(QString::fromLatin1("\n"),
            QString::fromLatin1("\n  "));
    }

    error << QString::fromLatin1("</pre>");

    debugoutput->append(error.join(QString::fromLatin1("")));

    QSScript *script = project->script(file);
    if (script) {
        showPage(script);
        QSEditor *editor = project->editor(script);
        editor->setErrorMark(line);
    }
}


void IdeWindow::scriptItemDoubleClicked(QTreeWidgetItem *i)
{
    ScriptListItem *item = (ScriptListItem *)i;
    showPage(item->qss);
}



void IdeWindow::enableEditActions(bool enable)
{
    filePrintAction->setEnabled(enable);
    fileExportAction->setEnabled(enable);
    fileCloseAction->setEnabled(enable);
    editCutAction->setEnabled(enable);
    editCopyAction->setEnabled(enable);
    editPasteAction->setEnabled(enable);
    editFindAction->setEnabled(enable);
    editFindAgainAction->setEnabled(enable);
    editFindAgainBWAction->setEnabled(enable);
    editReplaceAction->setEnabled(enable);
    editGotoLineAction->setEnabled(enable);
    editSelectAllAction->setEnabled(enable);
#if (QT_VERSION >= 0x030200)
    tabWidget->cornerWidget(Qt::TopRightCorner)->setShown(enable);
#endif
}


void IdeWindow::enableProjectActions(bool enable)
{
    fileRemoveAction->setEnabled(enable);
    projectRunAction->setEnabled(enable);
    projectCallAction->setEnabled(enable);
    projectEvaluateAction->setEnabled(enable);
    projectStopAction->setEnabled(false);
}


void IdeWindow::textChanged()
{
    QSEditor *editor = (QSEditor *)tabWidget->currentWidget();
    if (!editor) return;
    editUndoAction->setEnabled(editor->isUndoAvailable());
    editRedoAction->setEnabled(editor->isRedoAvailable());
}


void IdeWindow::setProject(QSProject *p)
{
    Q_ASSERT(p != 0);
    project = p;
    QObject::connect(project, SIGNAL(editorTextChanged()),
        this, SLOT(textChanged()));
    projectChanged();
    QList<QSScript *> scripts = project->scripts();
    if (tabWidget->count() == 0 && scripts.count())
        addPage(scripts.first());
    connect(project->interpreter(), SIGNAL(error(const QString &, const QString &, int)),
        this, SLOT(projectError(const QString &, const QString &, int)));
    connect(project, SIGNAL(projectChanged()), this, SLOT(projectChanged()));
    errorMode = project->interpreter()->errorMode();
    project->interpreter()->setTimeoutInterval(250);
    connect(project->interpreter(), SIGNAL(timeout(int)), this, SLOT(interpreterTimeout(int)));

#ifdef QS_VARIABLE_WATCH
    connect(project, SIGNAL(projectEvaluated()),
        variablesContainer, SLOT(updateScope()));
#endif
}


void IdeWindow::savePreferences()
{
    qsaEditorSyntax->save();
    QList<QSEditor *> editors = project->editors();
    for (int i=0; i<editors.size(); ++i) {
        QSEditor *editor = editors[i];
        editor->readSettings();
    }
}

void IdeWindow::projectChanged()
{
    QList<QSScript *> scripts = project->scripts();
    projectContainer->scriptsListView->clear();
    foreach (QSScript *script, scripts) {
        ScriptListItem *item = new ScriptListItem(projectContainer->scriptsListView, script);
        Q_UNUSED(item);
    }
    enableProjectActions(scripts.count());

    projectContainer->scriptsListView->sortByColumn(0);
    projectContainer->scriptsListView->sortByColumn(0);
}

void IdeWindow::evaluateProject()
{
    if(project) {
        project->commitEditorContents();
        QTimer::singleShot(0, project, SLOT(evaluate()));
    }
}

void IdeWindow::hideEvent(QHideEvent *)
{
    QSettings settings(QLatin1String("Trolltech"), QLatin1String("QSA"));
    QByteArray ba = saveState();
    settings.setValue(QLatin1String("QSWorkbenchSettings"), ba);

    if (project)
        project->interpreter()->setErrorMode((QSInterpreter::ErrorMode)errorMode);
}

void IdeWindow::showEvent(QShowEvent *)
{
    QSettings settings(QLatin1String("Trolltech"), QLatin1String("QSA"));
    QByteArray ba = settings.value(QLatin1String("QSWorkbenchSettings")).toByteArray();
    if (!ba.isEmpty())
        restoreState(ba);

    if (project)
        project->interpreter()->setErrorMode(QSInterpreter::Nothing);
}

void IdeWindow::projectStop()
{
    if (project)
        project->interpreter()->stopExecution();
}



void IdeWindow::interpreterTimeout(int)
{
    qApp->processEvents();
}
