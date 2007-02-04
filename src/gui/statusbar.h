/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__

#include <qstatusbar.h>
#include <qstring.h>

class QLabel;
class QTimer;
class QColor;

namespace gui {

/**
 Class representing statusbar.<br>
 The statusbar contain tho areas, large message area on left
 and smaller informative area on right.
 \brief Editor statusbar
 */
class StatusBar : public QStatusBar {
 Q_OBJECT
public:
 StatusBar(QWidget *parent=0,const char *name=0);
 ~StatusBar();
public slots:
 void message(const QString &theMessage);
 void receiveInfoText(const QString &theMessage);
 void receiveWarnText(const QString &theMessage);
private slots:
 void timeOut();
private:
 void normCol();
 void warnCol();
private:
 /** Informational label on right */
 QLabel *info;
 /** Message label on left */
 QLabel *msgLabel;
 /** Store old message in case of replacement by timed warning */
 QString storedMsg;
 /** Timer to watch for message timeouts*/
 QTimer *tm;
};

} // namespace gui

#endif
