/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
 *                                              Miroslav Jahoda,
 *                                              Jozef Misutka,
 *                                              Martin Petricek
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __ANNOTDIALOG_H__
#define __ANNOTDIALOG_H__

#include "qtcompat.h"
#include <qvariant.h>
#include <qdialog.h>
#include "qspage.h"
#include "kernel/static.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QTabWidget;
class QWidget;
class Q_GroupBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QComboBox;
class QFrame;

namespace gui {

/**
 Dialog for adding annotation to documents.
 Dialog expect to receive the annotation rectangle in constructor,
 it will solicit the annotation data from user and add the annotation to page,
 if user presses Ok
 @brief Dialog for adding annotations
*/

class AnnotDialog : public QDialog {
    Q_OBJECT

private:
    /** Annotation rectangle */
    libs::Rectangle rect;
    /** Page in which the annotation will be added */
    QSPage & page;
public:
    AnnotDialog(QSPage & p, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~AnnotDialog();


    void setRectangle(double xl, double yl, double width, double height);

public slots:
    virtual void fillStates( const QString & current );
    virtual void createAnnot();

protected:
    /** OK button to accept the dialog */
    QPushButton* okBtn;
    /** Cancel button to cancel the dialog */
    QPushButton* cancelButton;
    QTabWidget* tabWidget3;
    QWidget* tab;
    Q_GroupBox* groupBox1;
    QCheckBox* open;
    QLabel* textLabel2;
    QLineEdit* textContent;
    QLabel* textLabel3;
    QComboBox* iconName;
    QLabel* textLabel4_2;
    QComboBox* state;
    QLabel* textLabel5_2;
    QComboBox* stateModel;
    QWidget* tab_2;
    QFrame* frame3;
    QLabel* textLabel2_3;
    QLineEdit* linkContent;
    QLabel* textLabel1;
    QLineEdit* destination;
    QLabel* textLabel2_2;
    QComboBox* highLight;

    QGridLayout* AnnotDialogLayout;
    QHBoxLayout* layout8;
    QSpacerItem* spacer8;
    QSpacerItem* spacer7;
    QSpacerItem* spacer9;
    QGridLayout* tabLayout;
    QGridLayout* groupBox1Layout;
    QVBoxLayout* layout21_2;
    QHBoxLayout* layout16_2;
    QHBoxLayout* layout18_2;
    QHBoxLayout* layout19_2;
    QHBoxLayout* layout20_2;
    QGridLayout* tabLayout_2;
    QGridLayout* frame3Layout;
    QVBoxLayout* layout17;
    QSpacerItem* spacer4;
    QVBoxLayout* layout12;
    QHBoxLayout* layout16_2_2;
    QHBoxLayout* layout8_2;
    QHBoxLayout* layout9;

    void createTextAnnot();
    void createLinkAnnot();

protected slots:
    virtual void languageChange();

};

} // namespace gui

#endif
