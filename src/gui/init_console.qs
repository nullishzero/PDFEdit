//PDF Editor init script for console mode
//This script will be run at start

/* ==== load various needed functions ==== */
run( "pdfoperator.qs" );	//Functions for manipulation with pdf operators
run( "page.qs" );			//Page manipulation functions
run( "test.qs" );			//Testing functions

/* ==== Code to run on start ==== */

//Print welcome message
print("PDF Editor "+version());
