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

/** Moves given page with given difference.
 * @param page Page to move.
 * @param diff Difference to add to given page position.
 * @return New positon of given page.
 *
 * New page is moved to page + diff position. Therefore negative diff value
 * means moving backwards and positive forward. If position would not change
 * does nothing.
 * <br>
 * If page + diff &lt; 1 resp. &gt; pageCount, page is moved to the 1st position
 * resp. behind last position.
 */
function movePage(page, diff)
{
        var currPos=document.getPagePosition(page);
        var storePos = currPos + diff;
        if(storePos<1)
                storePos=1;

        if(storePos == currPos)
                return currPos;

        document.removePage(currPos);
        document.insertPage(page, storePos)
        PageSpace.refresh(currPos);

        // reload
        go();
        reuturn storePos;
}

