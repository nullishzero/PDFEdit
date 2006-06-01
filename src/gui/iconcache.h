#ifndef __ICONCACHE_H__
#define __ICONCACHE_H__

class QString;
class QPixmap;
class QIconSet;

namespace gui {

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
