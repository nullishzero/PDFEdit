#ifndef __STATICSETTINGS_H__
#define __STATICSETTINGS_H__

#include <qmap.h>
#include <qstring.h>

class QStringList;

namespace gui {
/**
 Class managing static settings, loading them from file
 \brief Loading static settings from file
 */
class StaticSettings {
public:
 QStringList entryList(const QString &prefix);
 bool tryLoad(const QString &dirName,const QString &fileName);
 bool tryLoad(const QString &fileName);
 QString readEntry(const QString &key,const QString &defValue=QString::null);
 StaticSettings();
 ~StaticSettings();
private:
 /** Map containing all settings */
 QMap<QString,QString> set;
};

} // namespace gui

#endif
