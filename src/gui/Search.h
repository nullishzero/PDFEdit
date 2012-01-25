/** \brief Search.h defines class for handling search input */
#ifndef ___SEARCH___
#define ___SEARCH___

#include <QWidget>
#include <QString>
#include "ui_search.h"
#include <string>
#include "typedefs.h"

/** \brief main class creating requests for searching */
/** this class will emit the signal for searching with corresponding flags */
class Search : public QWidget 
{
	Q_OBJECT

	Ui::Search ui;

public:
	/** \brief constructor */
	Search(QWidget * widget);
	/** \brief construct flags with wcchi there was request to search */
	int getFlags();
	/** set flags */
	void setFlags(int flags);
protected:
	/** on shoe event, the window should stay active */
	void showEvent ( QShowEvent * event );
public slots:
	/** this methos is called when there should be string search forward */
	void next();
	/** this methos is called when there should be string search backward */
	void prev();
	/** this was not implemented*/
	void replace();
	/** this generates the stop signal that stopn threadt in actual page */
	void stop();
signals:
	/** this was not used */
	void replaceTextSignal(QString a1,QString a2);
	/** emits the signal gor searching with appropriate flags */
	void search(QString,int); //flags
	/** stops the searching */
	void stopSignal();
};

#endif
