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
#ifndef __INVALIDMENUEXCEPTION_H__
#define __INVALIDMENUEXCEPTION_H__

#include <exception>

class QString;

namespace gui {

/**
 Exception raised when program encountered some error while loading
 menu or toolbar - in menu or toolbar item definitions
 \brief Exception raised when encountering invalid menu item
*/
class InvalidMenuException : public std::exception {
public:
 InvalidMenuException(const QString &message);
 ~InvalidMenuException() throw();
 QString message() const;
 const char* what() const throw();
protected:
 /** exception message*/
 QString msg;
};

} //namespace gui

#endif
