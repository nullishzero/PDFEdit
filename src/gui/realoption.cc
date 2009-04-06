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
/** @file
 RealOption - class for widget containing one editable setting of type float
 (internally represented as a double)<br>
 Used as one item in option window
 @author Martin Petricek
*/

#include "realoption.h"
#include <qvalidator.h>
#include <qlineedit.h>

namespace gui {

/** Default constructor of RealOption item
 @param parent parent Option Editor containing this control
 @param _key Key in settings for this option
 @param _defValue Default value if option not found
 */
RealOption::RealOption(const QString &_key/*=0*/,QWidget *parent/*=0*/,double _defValue/*=0*/)
 : StringOption (_key,parent) {
 ed->setText(QString::number(_defValue));
 ed->setValidator(new QDoubleValidator(ed));
}

/** default destructor */
RealOption::~RealOption() {
}

} // namespace gui
