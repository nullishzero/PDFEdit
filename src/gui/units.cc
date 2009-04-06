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
 Units - conversion between various length units
*/

#include "units.h"

namespace gui {

//  ------------------------------------------------------
//  --------------------   Units   -----------------------
//  ------------------------------------------------------
Units::Units( const QString _defaultUnit/*=QString::null*/ ) : QObject() {
	units["pt"]		= 1;					// 1 Point
	units["in"]		= 72 /*1 / 0.013837*/;			// 1 Point == 0.013837 inch
	units["mil"]	= 0.001 * units["in"];	// 1 mil == 0.001 inch
	units["hand"]	= 4 * units["in"];		// 1 hand == 4 inches
	units["ft"]		= 12 * units["in"];		// 12 inches == 1 foot
	units["li"]		= 0.66 * units["ft"];	// 1 link == 0.66 foot
	units["yd"]		= 3 * units["ft"];		// 3 feet == 1 yard
	units["fm."]	= 6 * units["ft"];		// 1 fathom == 6 feet
	units["rd"]		= 16.5 * units["ft"];	// 16.5 feet == 1 rod
	units["ch"]		= 66 * units["ft"];		// 1 chain == 66 feet
	units["fur."]	= 10 * units["ch"];		// 1 furlong == 10 chains
	units["cable"]	= 720 * units["ft"];	// 1 cable's length == 720 feet
	units["mi"]		= 8 * units["fur."];		// 1 mile == 8 furlongs
	units["land"]	= 3 * units["mi"];		// 1 league == 3 miles
	aliases[	"point"]	= "pt";
	aliases[tr("point")]	= "pt";		description["pt"]	= tr("pt : point");
	aliases[	"inch"]		= "in";
	aliases[	"inches"]	= "in";
	aliases[tr("inch")]		= "in";
	aliases[tr("inches")]	= "in";		description["in"]	= tr("in : inch");

	description["mil"]	= tr("mil : milli-inch");

	aliases[	"hands"]	= "hand";
	aliases[tr("hands")]	= "hand";
	aliases[tr("hand")]		= "hand";	description["hand"]	= tr("hand : hand");

	aliases[	"foot"]		= "ft";
	aliases[	"feet"]		= "ft";
	aliases[tr("foot")] 	= "ft";
	aliases[tr("feet")] 	= "ft";		description["ft"]	= tr("ft : foot");

	aliases[	"link"]		= "li";
	aliases[	"links"]	= "li";
	aliases[tr("link")]		= "li";
	aliases[tr("links")]	= "li";		description["li"]	= tr("li : link");

	aliases[	"yard"]		= "yd";
	aliases[	"yards"]	= "yd";
	aliases[tr("yard")]		= "yd";
	aliases[tr("yards")]	= "yd";		description["yd"]	= tr("yd : yard");

	aliases[	"rod"]		= "rd";
	aliases[	"rods"]		= "rd";
	aliases[tr("rod")]		= "rd";
	aliases[tr("rods")]		= "rd";
	aliases[	"pole"]		= "rd";
	aliases[	"poles"]	= "rd";
	aliases[tr("pole")]		= "rd";
	aliases[tr("poles")]	= "rd";
	aliases[	"perch"]	= "rd";
	aliases[	"perches"]	= "rd";
	aliases[tr("perch")]	= "rd";
	aliases[tr("perches")]	= "rd";		description["rd"]	= tr("rd : rod (pole, perch)");

	aliases[	"fathom"]	= "fm.";
	aliases[	"fathoms"]	= "fm.";
	aliases[tr("fathom")]	= "fm.";
	aliases[tr("fathoms")]	= "fm.";	description["fm."]	= tr("fm. : fathom");

	aliases[	"chain"]	= "ch";
	aliases[	"chains"]	= "ch";
	aliases[tr("chain")]	= "ch";
	aliases[tr("chains")]	= "ch";		description["ch"]	= tr("ch : chain");

	aliases[	"furlong"]	= "fur.";
	aliases[	"furlongs"]	= "fur.";
	aliases[tr("furlong")]	= "fur.";
	aliases[tr("furlongs")]	= "fur.";	description["fur."]	= tr("fur. : furlong");

	aliases[tr("cable")]	= "cable";	description["cable"]	= tr("cable : cable");

	aliases[	"mile"]		= "mi";
	aliases[	"miles"]	= "mi";
	aliases[tr("mile")]		= "mi";
	aliases[tr("miles")]	= "mi";		description["mi"]	= tr("mi : mile");

	aliases[	"lands"]	= "land";
	aliases[tr("land")]		= "land";
	aliases[tr("lands")]	= "land";
	aliases[	"league"]	= "land";
	aliases[	"leagues"]	= "land";
	aliases[tr("league")]	= "land";
	aliases[tr("leagues")]	= "land";
	description["land"]	= tr("land : land (league)");

	units["m"] = units["in"] / 0.0254;		// 1 inch = 2.54 cm
	units["Ym"] = 1e24 * units["m"];		// Y (yotta)	meter
	units["Zm"] = 1e21 * units["m"];		// Z (zetta)	meter
	units["Em"] = 1e18 * units["m"];		// E (exa)	meter
	units["Pm"] = 1e15 * units["m"];		// P (peta)	meter
	units["Tm"] = 1e12 * units["m"];		// T (tera)	meter
	units["Gm"] = 1e9 * units["m"];			// G (giga)	meter
	units["Mm"] = 1e6 * units["m"];			// M (mega)	meter
	units["km"] = 1e3 * units["m"];			// k (kilo)	meter
	units["hm"] = 1e2 * units["m"];			// h (hecto)	meter
	units["dam"] = 10 * units["m"];			// da (deka)	meter
	units["dm"] = 0.1 * units["m"];			// d (deci)	meter
	units["cm"] = 1e-2 * units["m"];		// c (centi)	meter
	units["mm"] = 1e-3 * units["m"];		// m (milli)	meter
	units["um"] = 1e-6 * units["m"];		// u (micro)	meter
	units["nm"] = 1e-9 * units["m"];		// n (nano)	meter
	units["pm"] = 1e-12 * units["m"];		// p (pico)	meter
	units["fm"] = 1e-15 * units["m"];		// f (femto)	meter
	units["am"] = 1e-18 * units["m"];		// a (atto)	meter
	units["zm"] = 1e-21 * units["m"];		// z (zepto)	meter
	units["ym"] = 1e-24 * units["m"];		// y (yocto)	meter

	description["m"]	= tr("m : meter");
	description["Ym"]	= tr("Ym : yottameter");
	description["Zm"]	= tr("Zm : zettameter");
	description["Em"]	= tr("Em : exameter");
	description["Pm"]	= tr("Pm : petameter");
	description["Tm"]	= tr("Tm : terameter");
	description["Gm"]	= tr("Gm : gigameter");
	description["Mm"]	= tr("Mm : megameter");
	description["km"]	= tr("km : kilometer");
	description["hm"]	= tr("hm : hectometer");
	description["dam"]	= tr("dam : decameter");
	description["dm"]	= tr("dm : decimeter");
	description["cm"]	= tr("cm : centimeter");
	description["mm"]	= tr("mm : millimeter");
	description["um"]	= tr("um : micrometer");
	description["nm"]	= tr("nm : nanometer");
	description["pm"]	= tr("pm : picometer");
	description["fm"]	= tr("fm : femtometer");
	description["am"]	= tr("am : attometer");
	description["zm"]	= tr("zm : zeptometer");
	description["ym"]	= tr("ym : yoctometer");

	units["A"] = 0.1 * units["nm"];			// 1 angstrom = 0.1 nm	
	description["A"]	= tr("A : angstrom");

	units["AU"] = 149597870691.0 * units["m"];		// AU (astronomical unit)
	units["ly"] = 9460528404879.0 * units["km"];	// Light year
	units["pc"] = 3.26156378 * units["ly"];			// Parsec

	aliases[	"astronomical unit"]	= "AU";
	aliases[	"astronomical units"]	= "AU";
	aliases[tr("astronomical unit")]	= "AU";
	aliases[tr("astronomical units")]	= "AU";
	description["AU"]	= tr("AU : astronomical unit");

	aliases["light year"]			= "ly";
	aliases["light years"]			= "ly";
	aliases[tr("light year")]		= "ly";
	aliases[tr("light years")]		= "ly";
	description["ly"]	= tr("ly : light year");

	aliases["parsec"]			= "pc";
	aliases[tr("parsec")]			= "pc";
	description["pc"]	= tr("pc : parsec");

	units["n.m."] = 1.852 * units["km"];	// 1 nautical mile == 1.852 km;

	aliases["nautical mile"]		= "n.m.";
	aliases["nautical miles"]		= "n.m.";
	aliases[tr("nautical mile")]		= "n.m.";
	aliases[tr("nautical miles")]		= "n.m.";
	description["n.m."]	= tr("n.m. : nautical mile");

	defaultUnit = "pt";
	setDefaultUnits( _defaultUnit );
}

Units::~Units()
	{}
QStringList Units::getAllUnits( ) const {
	QMap<double,QString>					h_map;
	QMap<QString,double>::ConstIterator		it = units.begin();
	for ( ; it != units.end() ; ++it )
		if (description.contains( it.key() ))
			h_map[ it.data() ] = description[ it.key() ];
		else
			h_map[ it.data() ] = it.key();

	return h_map.values();
}

QStringList Units::getAllUnitIds( ) const {
	QMap<double,QString>					h_map;
	QMap<QString,double>::ConstIterator		it = units.begin();
	for ( ; it != units.end() ; ++it )
		h_map[ it.data() ] = it.key();

	return h_map.values();
}

bool Units::setDefaultUnits( const QString dunits/*=QString::null*/ )
	{
		QString h_unit = getUnitFromAlias( dunits );

		if (! h_unit.isEmpty()) {
			defaultUnit = h_unit;
			return true;
		}

		return false;
	}

QString Units::getUnitFromAlias( const QString _unit ) const
	{
		if (_unit.isNull()) {
			return "pt";
		}
		if (units.contains( _unit )) {
			return _unit;
		}
		if (aliases.contains( _unit )) {
			return aliases[ _unit ];
		}
		if (description.values().contains( _unit )) {
			return description.keys()[description.values().findIndex( _unit )];
		}

		return QString::null;
	}

QString Units::getDescriptionForUnit( const QString _unit ) const
	{
		QString h_unit = getUnitFromAlias( _unit );
		if (description.contains( h_unit ))
			return description[ h_unit ];
		// else
		return _unit;
	}

QString Units::getDefaultUnits( ) const
	{
		return defaultUnit;
	}

double Units::convertFromUnitsToPoint( double num, const QString & fromUnits ) const
	{
		bool hu_unit;
		bool ha_unit;
		if (fromUnits.isNull())
			return num * units[ defaultUnit ];
		if (! (	(hu_unit = units.contains( fromUnits )) ||
				(ha_unit = aliases.contains( fromUnits )) ||
				description.values().contains( fromUnits ) ))
			return num;
		// else
		if (hu_unit)
			return num * units[ fromUnits ];
		else if (ha_unit)
			return num * units[ aliases[ fromUnits ] ];
		else
			return num * units[ description.keys()[description.values().findIndex( fromUnits )] ];
	}

double Units::convertFromPointToUnits( double num, const QString & toUnits ) const
	{
		bool hu_unit;
		bool ha_unit;
		if (toUnits.isNull())
			return num / units[ defaultUnit ];
		if (! (	(hu_unit = units.contains( toUnits )) ||
				(ha_unit = aliases.contains( toUnits )) ||
				description.values().contains( toUnits ) ))
			return num;
		// else
		if (hu_unit)
			return num / units[ toUnits ];
		else if (ha_unit)
			return num / units[ aliases[ toUnits ] ];
		else
			return num / units[ description.keys()[description.values().findIndex( toUnits )] ];
	}

double Units::convertUnits( double num, const QString fromUnits/*=QString::null*/, const QString toUnits/*=QString::null*/ ) const
	{
		return convertFromPointToUnits( convertFromUnitsToPoint( num, fromUnits ), toUnits );
	}

} //namespace gui
