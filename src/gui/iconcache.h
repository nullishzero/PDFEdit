/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
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
