/** @file
 ProgressBar - class representing Progress bar
*/
#include "progressbar.h"

namespace gui {

/**
 Constructor of ProgressBar
 @param parent Parent window of this dialog
 @param name Name of this window (used only for debugging)
 @param f Window flags
*/
ProgressBar::ProgressBar( QWidget* parent/*=0*/, const char* name/*=0*/, WFlags f/*=0*/ )
        :QProgressBar(parent, name, f)
{
        hide();
}

/**
 Constructor of ProgressBar
 @param totalSteps Number of total steps in progressbar (value for 100%)
 @param parent Parent window of this dialog
 @param name Name of this window (used only for debugging)
 @param f Window flags
*/
ProgressBar::ProgressBar( int totalSteps, QWidget* parent/*=0*/, const char* name/*=0*/, WFlags f/*=0*/ )
        :QProgressBar(totalSteps, parent, name, f)
{
        hide();
}

void ProgressBar::start()
{
        show();
}

void ProgressBar::finish()
{
        hide();
}

void ProgressBar::setMaxStep(int maxStep)
{
        setTotalSteps(maxStep);
}

void ProgressBar::update(int step)
{
        setProgress(step);
}

} // gui namespace

