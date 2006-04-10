#ifndef __REFVALIDATOR_H__
#define __REFVALIDATOR_H__

#include <qvalidator.h> 
#include <qstring.h> 

class RefValidator : public QValidator {
public:
 RefValidator(QObject *parent,const char *name=0);
 void fixup(QString &input) const;
 QValidator::State validate(QString &input,int &pos) const;
};

#endif
