#ifndef MIDEWINDOW_H
#define MIDEWINDOW_H

#include <QMainWindow>
#include <QString>

#include "ui_idewindow.h"

class QTreeWidgetItem;
class PreferencesBase;
class QSPreferencesContainer;
class QSProjectContainer;
class QSVariablesContainer;
class QDockWidget;
class QSOutputContainer;
class QSFindText;
class QSGotoLine;
class QSReplaceText;
class QSProject;
class QSScript;
class QTreeWidgetItem;
class QShowEvent;
class QHideEvent;

class IdeWindow : public QMainWindow, public Ui::IdeWindow
{
    Q_OBJECT

public:
    IdeWindow(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);
    ~IdeWindow();

    QSProject *project;

    virtual void editReplace( bool all );
    void init();
    virtual void setProject( QSProject * p );
    virtual void showEvent(QShowEvent *e);
    virtual void hideEvent(QHideEvent *e);
    virtual void setRunningState(bool);

public slots:
    virtual void scriptNew();
    virtual void scriptImport();
    virtual void scriptExport();
    virtual void scriptExit();
    virtual void scriptPrint();
    virtual void scriptRename();
    virtual void editUndo();
    virtual void editRedo();
    virtual void editCut();
    virtual void editCopy();
    virtual void editPaste();
    virtual void editFind();
    virtual void editFindNext();
    virtual void editFindPrev();
    virtual void editReplace();
    virtual void editReplaceAll();
    virtual void editSelectAll();
    virtual void editPreferences();
    virtual void editGotoLine();
    virtual void helpIndex();
    virtual void helpContents();
    virtual void helpAbout();
    virtual void addPage( QSScript * s );
    virtual void removePage( QSScript * s );
    virtual void removePage();
    virtual void showPage( QSScript * s );
    virtual void showFunction( QSScript * s, const QString & f );
    virtual void removeScript( QSScript * s );
    virtual void removeScript();
    virtual void projectRun();
    virtual void projectCall();
    virtual void projectStop();
    virtual void currentTabChanged(int);
    virtual void projectError( const QString & msg, const QString &, int line );
    virtual void scriptItemDoubleClicked( QTreeWidgetItem * i );
    virtual void enableEditActions( bool enable );
    virtual void enableProjectActions( bool enable );
    virtual void textChanged();
    virtual void savePreferences();
    virtual void projectChanged();
    virtual void evaluateProject();
    virtual void interpreterTimeout(int);

protected:
    int newVariable;

private:
    void initMenuAndToolBar();

private:
    int errorMode;
    bool hadError;
    PreferencesBase *qsaEditorSyntax;
    QSPreferencesContainer *preferencesContainer;
    QSProjectContainer *projectContainer;
    QSVariablesContainer *variablesContainer;
    QString runFunction;
    QDockWidget *outputContainerDock;
    QDockWidget *projectContainerDock;
    QDockWidget *variablesContainerDock;
    QSOutputContainer *outputContainer;
    QSFindText *findText;
    QSGotoLine *gotoLine;
    QSReplaceText *replaceText;

    QMenuBar *menubar;
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *searchMenu;
    QMenu *projectMenu;
    QMenu *windowMenu;
    QMenu *helpMenu;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *projectToolBar;

    QAction *fileNewAction;
    QAction *fileImportAction;
    QAction *fileExportAction;
    QAction *fileRenameAction;
    QAction *fileRemoveAction;
    QAction *fileCloseAction;
    QAction *filePrintAction;
    QAction *fileExitAction;
    QAction *editUndoAction;
    QAction *editRedoAction;
    QAction *editCutAction;
    QAction *editCopyAction;
    QAction *editPasteAction;
    QAction *editSelectAllAction;
    QAction *editPreferencesAction;
    QAction *editFindAction;
    QAction *editFindAgainAction;
    QAction *editFindAgainBWAction;
    QAction *editReplaceAction;
    QAction *editGotoLineAction;
    QAction *helpAboutAction;
    QAction *helpContentsAction;
    QAction *helpIndexAction;
    QAction *projectCallAction;
    QAction *projectRunAction;
    QAction *projectStopAction;
    QAction *projectEvaluateAction;


};

#endif
