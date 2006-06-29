// This header file is not included anywhere,
// but is parsed by doxygen, so it is used to add documentation

/**
 \mainpage PDF Editor programmer documentation

 \section intro_sec Introduction

 PDF Editor consist of two logical parts - Kernel and GUI.<br>
 Also, there are utility classes that are used by both of them.

 \section kernel_sec Kernel

 Kernel part is responsible for manipulation with PDF files and provides API that is used by GUI

 \section gui_sec GUI

 Graphical user interface.

 \section util_sec Utility classes

 Various utility classes.<br>
 They include configuration parser, delinearizator, object storage and iterator classes
 and classes to aid with debugging
  
*/

//Namespace documentation

/**
 GUI namespace<br>
 This namespace contain all user-interface related classes
 (widgets, tree items, scripting wrappers and others)
 \brief Graphical user interface namespace
*/
namespace gui {}
/**
 Utility namespace<br>
 Contains various static utility functions, used in GUI
 \brief Utility namespace
*/
namespace util {}

/** Namespace for configuration specific stuff. 
 */
 namespace configuration {}

/** Namespace for debug functions and related stuff. 
 */
 namespace debug {}

/** TODO namespace filters       */
 namespace filters {}
/** TODO namespace iterator      */
 namespace iterator {}
 
/** Namespace for generic observer stuff.
 */
 namespace observer {}
 
/** Namespace for rules manager generic classes.
 */
 namespace rulesmanager {}

/** TODO namespace xpdf          */
 namespace xpdf {}
