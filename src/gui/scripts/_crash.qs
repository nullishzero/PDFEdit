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
//Crash test for QS script

function _crash() {
 //Operator crashes
 x=createEmptyOperator();
 y=x.getName();
 x.iterator();

 //Put any code that crashes script here for future semi-automatic testing

 print('SUCCESS');
}

function _crash_treeitem() {
 try {
  parentType=firstSelectedItem().parent().itemtype();
  if (parentType!="Dict" && parentType!="Array") throw "Parent of selected tree item must be Dict/Array";
 } catch (e) {
  print("Error"+e);
  return;
 }
 x=firstSelectedItem();
 if (!x.valid()) throw "!x.valid()";
 x.remove();
 if (x.valid()) throw "x.valid()";
 print('SUCCESS');
}
