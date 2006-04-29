#ifndef __ICONCACHE_H__
#define __ICONCACHE_H__

#include <qmap.h>

class QStringList;
class QString;
class QPixmap;
class QIconSet;

namespace gui {

/** Icon Cache type: mapping from icon name to pixmap */
typedef QMap<QString, QPixmap*> IconCacheData;
/** Icon Sets Cache type: mapping from icon name to icon set */
typedef QMap<QString, QIconSet*> IconSetsData;

class IconCache {
public:
 IconCache();
 ~IconCache();
 QPixmap* getIcon(const QString &name);
 QIconSet* getIconSet(const QString &name);
private:
 QString getIconFile(const QString &name);
 /** List with paths to application icons */
 QStringList iconPath;
 /** Cache storing loaded icons */
 IconCacheData iconCache;
 /** Cache storing created icons sets*/
 IconSetsData setCache;
};

} // namespace gui

#endif
