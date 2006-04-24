#!/usr/bin/perl

use strict;

# Directory where source files are located
my $srcdir="../../src/gui";

#convert function definition C++ -> QSA
sub convert_definition {
 my $def=shift;
 #convert some C++ types to QSA types
 $def=~s/virtual\s+//;
 $def=~s/size_t\s+/int /;
 $def=~s/QString\s+/string /;
 $def=~s/QS(Page|ContentStream|Dict)\s*\*\s*/\1 /g;
 #trim unnecessary blank characters
 $def=~s/^\s+//;
 $def=~s/;\s*[\r\n]+$//;
 return $def;
}

sub get_doc {
 my $name=shift;
 open X,"<$srcdir/$name";
 my $classname='';
 my $ancestor='';
 my $classdesc='';
 my $out='';
 while(my $l=<X>) {
  $l=~s/^\s+//;
  $l=~s/[\r\n]+$//;
  if ($l=~/class\s+QS(\w+)\s+:\s+public\s+QS(\w+)/) {
   # found class name
   $classname=$1;
   $ancestor=$2;
  } elsif ($l=~/class\s+QS(\w+)\s+:\s+public\s+QObject/) {
   # found class name
   $classname=$1;
   $ancestor='';
  }
  # Start of multiline comment -read more lines
  while ($l=~/\/\*[\-=]([^*]+|\*[^\/])*$/m) {
   my $le=<X>;
   #normalize whitespace, concatenate to one line
   $le=~s/^\s+//;
   $l.=" ";
   $l.=$le;
   $l=~s/[\r\n]+$//;
  }
  if ($l=~m|/\*=\s*(.*?)\s*\*/|) {
   # found class description comment
   $classdesc=$1;
  }
  # look for documentational comment
  if ($l=~m|/\*\-\s*(.*?)\s*\*/|) {
   # found function documentation comment
   # next line is function definition
   my $cmt=$1;
   my $def=<X>;
   $def=convert_definition($def);
   #Look for function name
   $def=~/^\S+\s+([a-zA-Z0-9_]+)/;
   my $func=$1;
   #add function definition and description
   $out.=<<EOF;
   <sect2 id=\"${classname}.${func}\">
    <title><funcsynopsis>$def</funcsynopsis></title>
    $cmt
   </sect2>
EOF
  }
 }
 close X;
 #add header and footer 
 my $anc='';
if ($ancestor) {
$anc=<<EOF;
   <para>
    Ancestor type: <link linkend="type_$ancestor">$ancestor</link>
   </para>
EOF
}

 $out=<<EOF;
  <sect1 id="type_${classname}">
   <title>$classname</title>
$anc   <para>
    $classdesc
   </para>
$out  </sect1>
EOF
 return $out;
}

while (<>) {
 # parse xml file, all comments in form <!--TYPE: filename.h ...] --> are replaced
 # by documentation generated from that header
 if (/<!--TYPE:\s*([a-zA-Z0-9_\.\-]+)\s*-->/) {
  $_=get_doc($1);
 }
 print;
}
