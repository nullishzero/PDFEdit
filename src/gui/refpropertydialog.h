/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __REFPROPERTYDIALOG_H__
#define __REFPROPERTYDIALOG_H__

#include <qdialog.h>
#include <iproperty.h>
class QString;
class QLabel;
namespace pdfopbjects {
 class CPdf;
}

namespace gui {

class RefProperty;

using namespace pdfobjects;

/**
 Dialog allowing to select reference target in some "interactive way"
 \brief Dialog to pick reference target
*/
class RefPropertyDialog : public QDialog {
Q_OBJECT
public:
 RefPropertyDialog(CPdf* _pdf,IndiRef src, QWidget *parent=0);
 ~RefPropertyDialog();
 QString getResult();
private slots:
 void maybeAccept();
 void setNum(const QString &text);
 void setGen(const QString &text);
private:
 void message(const QString &message);
 void error(const QString &message);
 bool check(bool sayIfOk=true);
private:
 /** Current value */
 IndiRef value;
 /** Pdf document with reference */
 CPdf* pdf;
 /** Label to show errors/messages*/
 QLabel* infoText;
};

} // namespace gui

#endif
