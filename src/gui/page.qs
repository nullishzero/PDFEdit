/* === Page changing functions === */

/** Return property from dictionary of current page */
function pageProperty(propName) {
 if (isPageAvaliable())
	return page().getDictionary().property(propName).ref();
 else
 	warn(tr("No page selected!"));
}

/**
 Return property from dictionary of current page,
 adding it with default value if property is not found
*/
function pagePropertyDef(propName,defValue) {
 if (isPageAvaliable())
	return page().getDictionary().propertyDef(propName,defValue).ref();
 else
 	warn(tr("No page selected!"));
}
