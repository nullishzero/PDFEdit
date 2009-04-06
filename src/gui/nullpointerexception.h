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
#ifndef __NULLPOINTEREXCEPTION_H__
#define __NULLPOINTEREXCEPTION_H__

#include <exception>

class QString;

namespace gui {

/**
 Exception raised when attempting to call some wrapper function
 methods, while the objest wrapped inside is a NULL pointer, meaning that the
 operation would result in null pointer dereference (and program crash), if
 they would be executed.
 \brief Exception raised when encountering NULL pointer when not expected
*/
class NullPointerException : public std::exception {
public:
 NullPointerException(const QString &className,const QString &methodName);
 QString exceptionClassName() const;
 QString exceptionMethod() const;
 QString message() const;
 virtual const char* what() const throw();
 ~NullPointerException() throw();
protected:
 /** source of exception - class */
 QString _class;
 /** source of exception - method */
 QString _method;
 /** Exception message */
 QString _message;
};

} //namespace gui

#endif
