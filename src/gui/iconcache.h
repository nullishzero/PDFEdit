#ifndef __ICONCACHE_H__
#define __ICONCACHE_H__

class QString;
class QPixmap;
class QIconSet;

namespace gui {

/**
 Class responsible for loading and caching icons<br>
 There can be multiple instances of this class at once,
 but all of them share common cache
 \brief Class caching loaded icons
*/
class IconCache {
public:
 IconCache();
 ~IconCache();
 QPixmap* getIcon(const QString &name);
 QIconSet* getIconSet(const QString &name);
private:
 QString getIconFile(const QString &name);
};

} // namespace gui

#endif
