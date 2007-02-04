/*                                                                              
 * PDFedit - free program for PDF document manipulation.                        
 * Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko, 
 *                                              Miroslav Jahoda,       
 *                                              Jozef Misutka, 
 *                                              Martin Petricek                                             
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit                                                                      
 */ 
/** @file
 ProgressBar - class representing Progress bar
*/
#include "progressbar.h"

namespace gui {

void ProgressBar::start()
{
        if(!progressBar)
                return;
        progressBar->show();
        update(0);
}

void ProgressBar::finish()
{
        if(!progressBar)
                return;
        progressBar->hide();
}

void ProgressBar::setMaxStep(int maxStep)
{
        if(!progressBar)
                return;
        progressBar->setTotalSteps(maxStep);
}

void ProgressBar::update(int step)
{
        if(!progressBar)
                return;
        progressBar->setProgress(step);
}

} // gui namespace

