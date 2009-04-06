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
 InvalidMenuException
 - class for exception raised when found some error in menu definitions
 @author Martin Petricek
*/

#include <qstring.h>
#include "invalidmenuexception.h"

namespace gui {

/**
 Default constructor of exception
 @param message Exception message to pass
*/
InvalidMenuException::InvalidMenuException(const QString &message){
 msg=message;
}

/**
 Return exception message passed in constructor of this exception
 The message usually explain what item is invaild and why
 @return exception message
*/
QString InvalidMenuException::message() const {
 return msg;
}

/** default destructor */
InvalidMenuException::~InvalidMenuException() throw() {
}

/**
 Overloaded what() from std::exception<br>
 For getting usable error message if the exception is not handled
 */
const char* InvalidMenuException::what() const throw(){
 return msg.ascii();
}


} // namespace gui
