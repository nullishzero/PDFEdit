//Crash test for QS script

function _crash() {
 //Operator crashes
 x=createEmptyOperator();
 y=x.getName();
 x.iterator();

 //Put any code that crashes script here for future semi-automatic testing 
}