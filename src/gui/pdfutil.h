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
#ifndef __PDFUTIL_H__
#define __PDFUTIL_H__
/** @file
 \brief PDF manipulation utility functions header
*/

#include <kernel/cannotation.h>
#include <kernel/cobject.h>
#include <kernel/cpdf.h>
#include <kernel/iproperty.h>
class QString;
class QWidget;

namespace util {

using namespace pdfobjects;

QString getTypeId(PropertyType typ);
QString getTypeId(IProperty *obj);
QString getTypeId(boost::shared_ptr<IProperty> obj);
QString getTypeName(PropertyType typ);
QString getTypeName(IProperty *obj);
QString getTypeName(boost::shared_ptr<IProperty> obj);
IndiRef getRef(IProperty *ref);
IndiRef getRef(boost::shared_ptr<IProperty> ref);
bool isRefValid(boost::shared_ptr<CPdf> pdf, IndiRef ref);
bool isSimple(IProperty* prop);
bool isSimple(boost::shared_ptr<IProperty> prop);
boost::shared_ptr<IProperty> dereference(boost::shared_ptr<IProperty> obj);
bool saveCopy(boost::shared_ptr<CPdf> obj, const QString &name, QString *error);
boost::shared_ptr<IProperty> getObjProperty(boost::shared_ptr<CDict> obj, const QString &name);
boost::shared_ptr<IProperty> getObjProperty(boost::shared_ptr<CArray> obj, const QString &name);
boost::shared_ptr<IProperty> recursiveProperty(boost::shared_ptr<CDict> obj, const QString &name);
boost::shared_ptr<IProperty> recursiveProperty(boost::shared_ptr<CArray> obj, const QString &name);
QString propertyPreview(boost::shared_ptr<IProperty> obj);
QString annotType(CAnnotation::AnnotType at);
QString annotType(boost::shared_ptr<CAnnotation> anot);
QString annotTypeName(boost::shared_ptr<CAnnotation> anot);
boost::shared_ptr<CPdf> openPdfWithFallback(const QString &filename, CPdf::OpenMode mode);

//Password-related functions
boost::shared_ptr<CPdf> getPdfInstance(QWidget *parent, const QString &filename, CPdf::OpenMode mode, bool askPassword=true);
bool setPdfPassword(boost::shared_ptr<CPdf> pdf, const QString &pass);

} // namespace util

#endif
