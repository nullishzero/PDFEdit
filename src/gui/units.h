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
		Units( const QString _defaultUnit=QString::null);
		virtual ~Units();

		bool setDefaultUnits( const QString dunits = QString::null);
		QString getDefaultUnits( ) const;
		QString getDescriptionForUnit( const QString _unit ) const;
		QString getUnitFromAlias( const QString _unit ) const;
		double convertUnits( double num, const QString fromUnits=QString::null, const QString toUnits=QString::null) const;
		double convertFromUnitsToPoint( double num, const QString & fromUnits ) const;
		double convertFromPointToUnits( double num, const QString & toUnits ) const;

		QStringList getAllUnits( ) const;
	protected:
		QMap<QString, double>	units;
		QMap<QString, QString>	aliases;
		QMap<QString, QString>	description;
		QString					defaultUnit;
};

} //namespace gui

#endif
