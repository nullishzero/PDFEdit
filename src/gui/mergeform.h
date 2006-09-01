#ifndef __MERGEFORM_H__
#define __MERGEFORM_H__

#include <qvariant.h>
#include <qdialog.h>
#include <qfiledialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QLineEdit;
class QPushButton;
class QFrame;
class QListBox;
class QListBoxItem;

namespace gui {

/** Type for merge dialog result.
 * Contains 2 arrays, one for items, which are T typed and second keeps
 * an array of positions for items in original document. Each mergePositions
 * element corresponds to mergeItems element with same index.
 @brief Type for merge dialog result.
 */
template<typename T> class MergeArray
{
private:
        T * mergeItems;
        size_t * mergePositions;
        size_t length;

	/** Private constructor for empty array */
        MergeArray():length(0), mergeItems(NULL), mergePositions(NULL)
        {
        }
public:        
	/**
 	  Construct MergeArray with arrays of specified length
	  @param _length Length of item and position arrays
	 */
        MergeArray(size_t _length):length(_length)
        {
                mergeItems=new T[length+1];
                mergePositions=new size_t[length+1];
        }

	/** Destructor */
        ~MergeArray()
        {
                if(length>0)
                {
                        delete [] mergeItems;
                        delete [] mergePositions;
                }
        }

	/**
	 Initialize the contents of items array from given data
	 Array passed as parameter must have at least same number of elements as length of the MergeArray
	 @param items Items
	 */
        void initItems(T * items)
        {
                for(size_t i=0; i<length; ++i)
                        mergeItems[i]=items[i];
        }

	/**
	 Initialize the contents of position array from given data
	 Array passed as parameter must have at least same number of elements as length of the MergeArray
	 @param positions Item positions
	 */
        void initPositions(size_t * positions)
        {
                for(size_t i=0; i<length; ++i)
                        mergePositions[i]=positions[i];
        }

	/** Returns length of the array */
        size_t getLength()const
        {
                return length;
        }

	/** return array with items */
        T * getItems()
        {
                return mergeItems;
        }

	/** Returns array with item positions */
        size_t * getPositions()
        {
                return mergePositions;
        }
};



/** Dialog for merging documents.
 * <pre>
 * Usage:
 * 
 * // Create dialog instance
 * MergeDialog * dialog=new MergeDialog();
 *
 * // Inits original document (one which is currently opened) with its page 
 * // count.
 * dialog->initOriginal(pageCount);
 *
 * // Starts dialog as modal and does something if OK was pressed
 * if(dialog->exec()==QDialog::Accepted)
 * {
 *      // gets result of merging operation
 *      MergeDialog<int> * result=dialog->getResult();
 *
 *      // if result length is 0 - there is nothing to merge
 *      if(result->getLength()>0)
 *      {
 *              // result->getItems() returns an array of pages to be merged
 *              // with current document
 *              // result->getPositions() returns an array of positions for
 *              // those pages
 *      }
 *
 *      // result cleanup
 *      delete result;
 * }
 *
 * // dialog cleanup
 * dialog->destroyOpenFile();
 * delete dialog;
 * 
 * </pre>
 @brief Dialog for merging documents.
 */
class MergeDialog : public QDialog {
    Q_OBJECT

public:
    MergeDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~MergeDialog();

    QLabel* textLabel1;
    /** Editbox for typing in a filename*/
    QLineEdit* fileNameInput;
    /** button for invoking a dialog to pick the filename*/
    QPushButton* fileNameBtn;
    QPushButton* openBtn;
    QFrame* line1;
    QListBox* mergeList;
    QPushButton* addBtn;
    QPushButton* removeBtn;
    /** button to move current page one position up*/
    QPushButton* upBtn;
    /** button to move current page one position down*/
    QPushButton* downBtn;
    QListBox* fileList;
    /** Cancel button - dismiss dialog without doing anything*/
    QPushButton* cancelBtn;
    /** Ok button - accept result of dialog and merge pages*/
    QPushButton* okBtn;

    /** Returns result of merging.
     * 
     * Creates MergeArray instance (Note that caller is responsible for
     * deallocation) and fills it according current state of mergeList as
     * follows:
     * <br>
     * Collects all ListItems from mergeList with FROMFILE node data type. 
     * MergeArray's length is number of these items. Items arrray is then
     * initialized with ORIGINAL item position which is before FROMFILE
     * item (starting from 1 for items which are at the begining).
     * <br>
     * As a result items array contains page positions (in opened file) 
     * which should be merged and positions array contains page positions
     * from original file before which they should be inserted.
     * <br>
     * TODO - maybe use shared_ptr&lt;CPage&gt; as parameter...
     */
    MergeArray<int>* getResult();

    /** Initializes mergeList with pages from original document.
     * @param count Original document page count.
     *
     * Creates ListItems (for count pages) with ORIGINAL type for 
     * mergeList. These items represents pages from document where
     * we want to merg (insert pages from opened file).
     * <br>
     * Ordering or count of these items can't be changed.
     */
    void initOriginal( size_t count );

    /** Clean up for opened file.
     *
     * Destroys everything what was created during initFileList.
     */
    void destroyOpenFile();

    /**
     Return file name of document that contain pages to merge
     @return filename of selected document
    */
    QString fileName();

public slots:
    // Events handlers
    virtual void mergeList_currentChanged( QListBoxItem * item );
    virtual void fileList_currentChanged( QListBoxItem * );
    virtual void addBtn_clicked();
    virtual void removeBtn_clicked();
    virtual void upBtn_clicked();
    virtual void downBtn_clicked();
    virtual void openBtn_clicked();
    virtual void fileNameBtn_clicked();

protected:
    /** Fills fileList with ListItems corresponding to pdf document given by 
     * name.
     * @param fileName Pdf document name to merge with original one.
     *
     * Gets page count from given file and creates entries (ListItem instances
     * with NodeData with FROMFILE type) for fileList list box.
     * <br>
     * Method is called when openBtn is clicked.
     * return true in case of sucecss, false in case of failure
     */
    bool initFileList( QString & fileName );

    QGridLayout* MergeDialogLayout;
    QVBoxLayout* layout48;
    QHBoxLayout* layout27;
    QHBoxLayout* layout2;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QHBoxLayout* layout47;
    QVBoxLayout* layout4;
    QSpacerItem* spacer3;
    QHBoxLayout* layout6;
    QSpacerItem* spacer5;
    QSpacerItem* spacer6;

protected slots:
    virtual void languageChange();
};

} //namespace gui

#endif // __MERGEDIALOG_H__
