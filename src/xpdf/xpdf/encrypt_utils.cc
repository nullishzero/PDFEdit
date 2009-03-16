#include "xpdf/encrypt_utils.h"

SecurityHandler * checkEncryptionCred(XRef * xref, GString *ownerPassword, GString *userPassword, GBool & result)
{
  Object encrypt;
  GBool encrypted;
  SecurityHandler *secHdlr = NULL;

  xref->getTrailerDict()->dictLookup("Encrypt", &encrypt);
  if ((encrypted = encrypt.isDict())) {
    if ((secHdlr = SecurityHandler::make(xref, &encrypt))) {
      if (secHdlr->checkEncryption(ownerPassword, userPassword)) {
	result = gTrue;
      } else {
	// authorization failed
	result = gFalse;
      }
    } else {
      // couldn't find the matching security handler
      result = gFalse;
      if(ownerPassword)
        gfree(ownerPassword);
      if(userPassword)
        gfree(userPassword);
    }
  } else {
    // document is not encrypted
    result = gTrue;
    if(ownerPassword)
      gfree(ownerPassword);
    if(userPassword)
      gfree(userPassword);
  }
  encrypt.free();
  return secHdlr;
}


void setEncryptionCred(XRef * xref, SecurityHandler * secHandler)
{
	if(!secHandler)
		return;

	// authorization succeeded
       	xref->setEncryption(secHandler->getPermissionFlags(),
			    secHandler->getOwnerPasswordOk(),
			    secHandler->getFileKey(),
			    secHandler->getFileKeyLength(),
			    secHandler->getEncVersion(),
			    secHandler->getEncAlgorithm());
}
