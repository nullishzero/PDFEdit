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
 Reference validator - checks if given String is valid Reference in given document
 Does validate only format, not if the reference exists (that would be too slow)
 Reference existence is validated on attempt to write it to the property
 @author Martin Petricek
*/

#include "refvalidator.h"
#include "qtcompat.h"
#include <qstring.h>
#include <utils/debug.h>
#include "util.h"

namespace gui {

/** Regular expression for validation */
QRegExp valid("\\d+,\\d+");

/** Regular expression for intermediate validation */
QRegExp ivalid("\\d*,?\\d*");


/** Constructor for this validator
@param parent Parent object
@param name name of this validator (not used, except debugging)
 */
RefValidator::RefValidator(QObject *parent,const char *name/*=0*/) : QValidator(parent,name) {
}

/** Try to "fix" the string to be valid. Does not guarantee that string will be valid after returning
 @param input String to fix (reference)
*/
void RefValidator::fixup(QString &input) const {
 input=input.simplifyWhiteSpace();
}

/** Try to validate the string as reference target
 @param input String to validate (reference)
 @param pos position of cursor (not changed)
 @return result of validation (Acceptable, Intermediate or Invalid)
*/
QValidator::State RefValidator::validate(QString &input,__attribute__((unused)) int &pos) const {
 if (valid.exactMatch(input)) {
  guiPrintDbg(debug::DBG_DBG,"Validate: " << Q_OUT(input) << " Acceptable");
  return Acceptable;
 }
 if (ivalid.exactMatch(input)) {
  guiPrintDbg(debug::DBG_DBG,"Validate: " << Q_OUT(input) << " Intermediate");
  return Intermediate;
 }
 guiPrintDbg(debug::DBG_DBG,"Validate: " << Q_OUT(input) << " Invalid");
 return Invalid;
}

} // namespace gui
