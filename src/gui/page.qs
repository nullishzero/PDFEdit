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
 * <br>
 * Current (visible) page is changed to moved position.
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
        document.insertPage(page, storePos);
        PageSpace.refresh(storePos);

        // reload
        go();
        return storePos;
}

/** Inserts given page to given position to current document.
 * @param page Page instance.
 * @param pos Position where to store given page.
 * @return Inserted page position.
 * 
 * Note that given page can be from different document.
 * <br>
 * If given pos &lt; 1 resp. &gt; page count, it will be stored before first
 * page, resp. behind last page.
 * <br>
 * Current (visible) page is changed to given pos.
 */
function insertPage(page, pos)
{
        document.insertPage(page, pos);

        PageSpace.refresh(pos);

        // reload
        go();
        return pos;
}

/** Removes page from given position.
 * @param pos Page position.
 * @return Removed page position.
 *
 * Checks whether given position is in range. If not, warning dialog is 
 * displayed, otherwise delegate to document.removePage. 
 * <br>
 * Current (visible) page is changed to given pos (or lowered to page count).
 */
function removePage(pos)
{
        if(pos < 1 || pos > document.getPageCount())
        {
               warn(pos+" "+tr("out of range")); 
               return -1;
        }

        document.removePage(pos);

        var displayPos=(pos<=document.getPageCount())?pos:document.getPageCount();

        PageSpace.refresh(displayPos);
        go();

        return displayPos;
}
