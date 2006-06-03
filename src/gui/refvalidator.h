#ifndef __REFVALIDATOR_H__
#define __REFVALIDATOR_H__

#include <qvalidator.h> 
class QString;

namespace gui {

/**
 Reference validator - checks if given String is valid Reference in given document
 Does validate only format, not if the reference exists (that would be too slow)
 Reference existence is validated on attempt to write it to the property
*/
class RefValidator : public QValidator {
public:
 RefValidator(QObject *parent,const char *name=0);
 void fixup(QString &input) const;
 QValidator::State validate(QString &input,int &pos) const;
};

} // namespace gui

#endif
