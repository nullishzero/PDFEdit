#ifndef _ENCRYPT_UTILS_H_
#define _ENCRYPT_UTILS_H_

#include "goo/gtypes.h"
#include "goo/GString.h"
#include "xpdf/SecurityHandler.h"
#include "xpdf/XRef.h"

// Sets given security handler to the xref
// Skipps if secHandler is NULL
void setEncryptionCred(XRef * xref, SecurityHandler * secHandler);

// Checks credentials for given file.
// ownerPassword and userPassword may be NULL meaining an empty password.
// If the check is successfull result is set to gTrue, gFalse otherwise.
// Return value is NULL if credentials are not correct or if ecryption
// is not used. Otherwise it points to the correct security handler.
SecurityHandler * checkEncryptionCred(XRef * xref,
		GString *ownerPassword, GString *userPassword,
		GBool & result);

#endif

