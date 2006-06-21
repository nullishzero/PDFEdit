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
