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
#ifndef __UNITS_H__
#define __UNITS_H__

#include <qstring.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qmap.h>

namespace gui {

/**
 Class for conversion between various length units
 @brief Units conversion
*/

class Units : protected QObject {
	public:
		/** Implicit constructor.
		 * @param _defaultUnit Unit for set as default. Default is set "pt".
		 */
		Units( const QString _defaultUnit=QString::null);
		/** Implicit destructor. */
		virtual ~Units();

		/** Method set default unit for conversion.
		 * @param dunits Unit for set as default. Nothing chenage, if not find in internal units.
		 *
		 * @return Return true if default unit is change to \a dunits. Otherwise return false.
		 */
		bool setDefaultUnits( const QString dunits = QString::null);
		/** Method return default conversion unit.
		 * @return Return defualt unit.
		 */
		QString getDefaultUnits( ) const;
		/** Method return description for unit.
		 * @param _unit Which unit is required description.
		 *
		 * @return Description for \a _unit. If nothing description found for \a _unit, return \a _unit.
		 */
		QString getDescriptionForUnit( const QString _unit ) const;
		/** Function return unit from its aliases or description.
		 * @param _unit Alias or description of unit.
		 *
		 * @return Return unit for alias \a _unit. If alias \a _unit is not find in internal aliases or
		 *		descrition, return \a _unit.
		 */
		QString getUnitFromAlias( const QString _unit ) const;
		/** Convert value from one unit to other.
		 * @param num Value for conversion.
		 * @param fromUnits Unit of value \a num.
		 * @param toUnits To which unit is required conversion of value \a num.
		 *
		 * If \a fromUnits or \a toUnits is not set, then default unit use.
		 *
		 * @return Return value \a num conversed to unit \a toUnits.
		 */
		double convertUnits( double num, const QString fromUnits=QString::null, const QString toUnits=QString::null) const;
		/** Conver value to point ("pt").
		 * @param num Value for conversion.
		 * @param fromUnits Unit of value \a num.
		 *
		 * If \a fromUnits is not set, then default unit use.
		 *
		 * @return Return value \a num conversed to points.
		 */
		double convertFromUnitsToPoint( double num, const QString & fromUnits ) const;
		/** Conver value from points ("pt") to some unite.
		 * @param num Value for conversion.
		 * @param toUnits To which unit is required conversion of value \a num.
		 *
		 * If \a toUnits is not set, then default unit use.
		 *
		 * @return Return value \a num conversed from points to units \a toUnits.
		 */
		double convertFromPointToUnits( double num, const QString & toUnits ) const;

		/** Function return description of all units internal defined.
		 * @return Return description of units.
		 */
		QStringList getAllUnits( ) const;
		/** Function return identifiers of all units internal defined.
		 * @return Return identifiers of units.
		 */
		QStringList getAllUnitIds ( ) const;
	protected:
		/** Units with its multipltiplicator for conversion to points. */
		QMap<QString, double>	units;
		/** Aliases defined for units.
		 * aliases[ \a alias ] = \a unit
		 *
		 * @see getUnitFromAlias
		 */
		QMap<QString, QString>	aliases;
		/** Description for units defined in \a units.
		 * description[ \a unit ] = \a description
		 *
		 * @see getAllUnits
		 * @see getDescriptionForUnit
		 */
		QMap<QString, QString>	description;
		/** Default unit for conversion of values.
		 * @see setDefaultUnits
		 */
		QString					defaultUnit;
};

} //namespace gui

#endif
