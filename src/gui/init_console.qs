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
//PDFedit init script for console mode
//This script will be run at start

/* ==== load various needed functions ==== */
run( "pdfoperator.qs" );	//Functions for manipulation with pdf operators
run( "page.qs" );			//Page manipulation functions
run( "test.qs" );			//Testing functions

/* ==== Code to run on start ==== */

//Print welcome message
print("PDFedit "+version());

class FunctionList {
 var flist=[];
 var fnames=new Array();

 /** Get available 'registered' scripts */
 function FunctionList() {
  var spath=settings.read("path/script");
  var pth=spath.split(';');
  for(var i=0;i<pth.length;i++) {
   var pt=settings.expand(pth[i])
   var dir=new Dir(pt);
   var f=dir.entryList('*.qs');
   for(var x=0;x<f.length;x++) {
    process(pt,f[x]);
   }
  }
 }

 /** Check for existence of member memb in array arr */
 function isMember(arr,memb) {
  try {
   var n=arr[memb];
   return true;
  } catch (e) {
   return false;
  }
 }

 /** Get member memb from array arr, with default value defValue if member is not found */
 function getMember(arr,memb,defValue) {
  try {
   var n=arr[memb];
   return n;
  } catch (e) {
   return defValue;
  }
 }

 /** Process single script file */
 function process(pt,fn) {
  var f=new File(pt+"/"+fn);
  f.open(File.ReadOnly);
  var trim1=new RegExp("^\\s+");
  var trim2=new RegExp("\\s+$");
  var recx=[];
  while (!f.eof) {
   var line=f.readLine();
   if (!line.startsWith('//')) break;
   var dpos=line.find(':');
   if (dpos<0) break;
   var key=line.substring(2,dpos).replace(trim1,'').replace(trim2,'').toLowerCase();
   var value=line.substring(dpos+1).replace(trim1,'').replace(trim2,'');
   if (!key.length) break;
   recx[key]=value;
  }
  f.close();
  if (isMember(recx,"console")) {
   var name=recx["console"];
   if (isMember(flist,name)) return;//We have this already
   recx["filename"]=fn;
   recx["pathname"]=pt;
   flist[name]=recx;
   fnames.push(name);
  }
 }

 /** List available 'registered' scripts */
 function list() {
  var brief=false;
  if (arguments.length) brief=arguments[0];
  print(tr("Available functions:"));
  for(var i=0;i<fnames.length;i++) {
   var name=fnames[i];
   var recx=flist[name];
    //Something found -> add it
   desc=getMember(recx,'description','');
   params=getMember(recx,'parameters','');
   fname=getMember(recx,'filename','');
   print(" "+name);
   if (!brief) {
    if (desc.length) print("  "+tr("Description")+": "+desc);
    if (params.length) print("  "+tr("Parameters")+": "+params);
//    if (fname.length) print("  "+tr("Filename")+": "+fname);
   }
  }
 }

 /**
  Test if the name is unambiguous enough or if it is present at all.
  Return only if exactly one function matches, otherwise print error and exit application
  Return exact name of function
 */
 function getFunc(func) {
  var fn=func.toLowerCase();
  var cnt=0;
  var match=new Array();
  for(var i=0;i<fnames.length;i++) {
   var name=fnames[i];
   if (name.toLowerCase().startsWith(fn)) {
    cnt++;
    match.push(name);
   }
  }
  if (!cnt) {
   var msg=tr("No function found matching '%1'");
   print(msg.arg(func));
   functionList.list(true);
   exit(1);
  }
  if (cnt>1) {
   var msg=tr("Too many function found matching '%1'");
   print(msg.arg(func));
   print(tr("Matching functions")+":");
   exit(1);
  }
  return match[0];
 }

 /**
  Run function, given its full name
 */
 function runFunc(func) {
  print(tr("Running")+" "+func+" ("+flist[func].filename+")");
  run(flist[func].filename);
 }
}

/** 'default' console function */
function onConsoleStart() {
 // This is effectively launched after all other script,
 // effectively only if the scripts won't terminate application while executing
 // and only it they won't redefine this function with something else
 functionList=new FunctionList();
 func=takeParameter();
 if (!func) {
  print(tr("Usage:"));
  print(" pdfedit -console ["+tr("function name")+"] ["+tr("function parameter(s)")+"]");
  print(tr("First parameter is name of function to invoke (case insensitive) or its unambiguous part."));
  print(tr("Rest of parameters are passed to invoked function."));
  functionList.list();
  exit(1);
 }
 func=functionList.getFunc(func);
 functionList.runFunc(func);
}
