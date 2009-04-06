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
var childcount;

/** Count and list child items in entire tree. Open all tree items on the way */
function _count_childs() {
 maxc=1000;
 if (arguments.length) {
  maxc=arguments[0];
 }
 childcount=0;
 _count_childs_run(treeRoot(),'',maxc);
 print(childcount+" items");
}

function _count_childs_run(r,s,maxc) {
 if (childcount>maxc) return;
 childcount++;
 r.setOpen(true);
 var ch=r.getChildNames();
 print(s+r.id()+"   "+childcount);
 if (!ch) return;//No childs
 for(var i=0;i<ch.length;i++) {
  _count_childs_run(r.child(ch[i]),s+" ",maxc);
 }
}

function _count_selection() {
 treeItem=firstSelected();
 while (treeItem) {
  print(treeItem.type());
  treeItem=nextSelected();
 }
}

function _count_selection_other(x) {
 treeItem=firstSelected(x);
 while (treeItem) {
  print(treeItem.type());
  treeItem=nextSelected();
 }
}
