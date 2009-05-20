/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
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

/**
 Delete current page in GUI
*/
function delCurrentPage() {
 removePage(document.getPagePosition(page()));
 rootReload();
 checkMenus();
}

/** Calculates number of first n elements lower than value from given
 * array.
 * @param positions Array of numbers.
 * @param value Value which to compare.
 * @return Number of positions array members which are lower than given
 * value.
 */
function calcDiff(positions, value, n)
{
        var diff=0;

	print(positions+" "+value+" "+n);
        for(j=0; j<n; ++j)
                if(positions[j]>0 && positions[j]<=value)
                        diff++;

	print(diff);
        return diff;
}

/** Merge given pages with current document.
 * @param pages Array of pages to merge with.
 * @param positions Array of positions for pages (see below).
 *
 * Inserts all pages from given array each to the position from given array.
 * positions members are positions of pages in original document where to
 * insert coresponding page. If positions array is shorter than pages, missing
 * members are calculated as successors of the highest positions member.
 * If pages array is shorther than positions array, positions's redundant
 * (those without page) are ignored.
 * <br>
 * Examples:
 * <pre>
 * // Current document has 3 pages (p1, p2, p3)
 * // We want to merge pp1, pp2, pp3 with current document and
 *
 * // 1.)
 * // pp1 should be inserted to the p1's position (currently 1st)
 * // pp2 should be inserted to the p2's position (currently 2nd)
 * // pp3 should be inserted to the p3's position (currently 3rd)
 * mergeWithPages([pp1,pp2,pp3], [1,2,3])
 * // or with skipped position parameters
 * mergeWithPages([pp1,pp2,pp3], [1,2])
 * // with same meaning as
 * mergeWithPages([pp1,pp2,pp3], [1,2])
 * // or
 * mergeWithPages([pp1,pp2,pp3], [1])
 * // or empty positions means from the begining of document
 * mergeWithPages([pp1,pp2,pp3]. [])
 *
 * // As a result:
 * // pp1, p1, pp2, p2, pp3, p3
 *
 * // 2.)
 * // pp1 should be inserted to the p3's position (currently 1st)
 * // pp2 should be inserted to the p2's position (currently 2nd)
 * // pp3 should be inserted to the p1's position (currently 3rd)
 * mergeWithPages([pp1,pp2,pp3], [3, 2, 1])
 *
 * // As a result:
 * // pp3, p1, pp2, p2, pp1, p3
 *
 * // 3.)
 * // merge pp1, pp2, pp3 behind current pages (join documents with
 * // current in front part.
 * mergeWithPages([pp1,pp2,pp3], [document.getPageCount()+1])
 *
 * // As a result:
 * // p1, p2 ,p3, pp1, pp2, pp3
 *
 * // 4.)
 * // merge pp1, pp2, pp3 before current pages (join documents with
 * // current in back part.
 * mergeWithPages([pp1,pp2,pp3], [1,1,1])
 *
 * // As a result:
 * // pp1, pp2 ,pp3, p1, p2, p3
 * </pre>
 */
function mergeWithPages(pages, positions)
{
        var maxPos=0;
        var pos;

        // stores all pages which have their position
        for(i=0;i<pages.length && i<positions.length; ++i)
        {
                pos=positions[i];

                // ignores all out of range positions
                if(pos<1)
                {
                        warn(pos+" "+"out of range");
                        continue;
                }

                // cheks maximum of positions for pages which don't have
                // their position
                if(maxPos<pos)
                        maxPos=pos;

                // given pos has to be recalculated because of previous
                // insertions. So all lower positions increments diff
                // for it.
                pos+=calcDiff(positions, pos, i);

                // inserts page to the current document to the calculated place
                document.insertPage(pages[i], pos);
        }

        // handles also pages which doesn't have their position
        // stores them behind last
        pos=maxPos+calcDiff(positions, maxPos+1, positions.length)+1;
        //System.println("maxPos="+maxPos+" pos="+pos);

        // gets editor progress bar
        progress_bar=progressBar();
        for(j=0;i<pages.length; ++i)
        {
                document.insertPage(pages[i], pos);

                // displays current state of progress.
                // document might use same progress observer, so we will
                // initializes total steps and make the bar visible just
                // to be sure
                progress_bar.show();
                progress_bar.setProgress(j+1, pages.legth);
		processEvents();
                // moves to next position with respect to above insertion
                pos+=2;
        }

        go();
}

/**
 Function that is invoked when slecting "Merge pages from other document" from menu/toolbar
*/
function mergeDocument() {
 var mresult=mergeDialog();
 if (!mresult) return;
 var pageNumbers=mresult[0];
 var pagePositions=mresult[1];
 var filename=mresult[2];
 var pages=[];
 var pdf2=loadPdf(filename);
 for (i=0;i<pageNumbers.length;i++) {
  pages[i]=pdf2.getPage(pageNumbers[i]);
 }
 mergeWithPages(pages,pagePositions);
 pdf2.unloadPdf();
 return mresult;
}
