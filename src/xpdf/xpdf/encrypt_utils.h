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
// ownerPassword and userPassword may be NULL meaning an empty password.
// If the check is successfull result is set to gTrue, gFalse otherwise.
// Return value:
// handler == NULL && result == false
// 	- security handler not found - cannot check credentials
// handler == NULL && result == true
// 	- document is not encrypted
// handler != NULL
// 	- credentials checked and result keeps the status
SecurityHandler * checkEncryptionCred(XRef * xref,
		GString *ownerPassword, GString *userPassword,
		GBool & result);

#endif

