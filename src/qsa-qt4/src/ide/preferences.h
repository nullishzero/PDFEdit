#ifndef MPREFERENCES_H
#define MPREFERENCES_H

#include <QMap>
#include "conf.h"
#include "ui_preferences.h"

class PreferencesBase : public QWidget, public Ui::PreferencesBase
{
    Q_OBJECT

public:
    PreferencesBase(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
    ~PreferencesBase();

public slots:
    virtual void init();
    virtual void destroy();
    virtual void colorClicked();
    virtual void reInit();
    virtual void save();
    virtual void updatePreview();
    virtual void boldChanged( bool b );
    virtual void elementChanged( const QString & element );
    virtual void familyChanged( const QString & f );
    virtual void italicChanged( bool b );
    virtual void setColorPixmap( const QColor & c );
    virtual void sizeChanged( int s );
    virtual void underlineChanged( bool b );

protected:
    QMap<QString, ConfigStyle> styles;
    ConfigStyle currentStyle;
    QString currentElement;

};

#endif
