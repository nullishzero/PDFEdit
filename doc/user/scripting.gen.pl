#!/usr/bin/perl

use strict;

my $srcdir="../../src/gui";

sub get_doc {
 my $name=shift;
 open X,"<$srcdir/$name";
 my $classname='';
 my $classdesc='';
 my $out='';
 while(my $l=<X>) {
  $l=~s/^\s+//;
  $l=~s/[\r\n]+$//;
  if ($l=~/class\s+QS(\w+)\s+:\s+public\s+QS/) {
   $classname=$1;
   print STDERR "Classname=$1\n";
  }
  if ($l=~m|/\*=\s*(.*?)\s*\*/|) {
   $classdesc=$1;
   print STDERR "Classdesc=$1\n";
  }
  # look for documentational comment
  if ($l=~m|/\*\-\s*(.*?)\s*\*/|) {
   #next line is function definition
   my $cmt=$1;
   print STDERR "Comment=$1\n";
   my $def=<X>;
   $def=~s/QString(\s+)/string\1/;
   $def=~s/QSPage*(\s+)/Page\1/;
   $def=~s/^\s+//;
   $def=~s/;\s*[\r\n]+$//;
   print STDERR "Def=$def\n";
   $def=~/^\S+\s+([a-zA-Z0-9_]+)/;
   my $func=$1;
   $out.=<<EOF;
   <sect2 id=\"${classname}.${func}\">
    <title><funcsynopsis>$def</funcsynopsis></title>
    $cmt
   </sect2>
EOF
  }
 }
 close X;
 $out=<<EOF;
  <sect1 id="type_${classname}">
   <title>$classname</title>
   <para>
    $classdesc
   </para>
$out  </sect1>
EOF
 return $out;
}

while (<>) {
 if (/<!--TYPE:\s*([a-zA-Z0-9_\.\-]+)\s*-->/) {
  print STDERR "$1\n";
  $_=get_doc($1);
 }
 print;
}