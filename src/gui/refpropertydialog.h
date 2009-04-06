/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
#ifndef __REFPROPERTYDIALOG_H__
#define __REFPROPERTYDIALOG_H__

#include <qdialog.h>
#include <kernel/iproperty.h>
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
 RefPropertyDialog(boost::weak_ptr<CPdf> _pdf,IndiRef src, QWidget *parent=0);
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
 boost::weak_ptr<CPdf> pdf;
 /** Label to show errors/messages*/
 QLabel* infoText;
};

} // namespace gui

#endif
