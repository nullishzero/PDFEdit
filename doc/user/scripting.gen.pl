#!/usr/bin/perl

use strict;

# Directory where source files are located
my $srcdir="../../src/gui";

my %used_func=();

#convert function definition C++ -> QSA
sub convert_definition {
 my $def=shift;
 #convert some C++ types to QSA types
 $def=~s/(virtual|const)\s+//g;		# keywords - have no meaning in QSA
 $def=~s/&//g;				# references - the same
 $def=~s/size_t\s+/int /g;		# size_t -> int
 $def=~s/=(QString::null|NULL|false)//g;# default null/false - remove
 $def=~s/QString\s+/string /g;		# QString -> string
 $def=~s/QStringList\s+/string[] /g;	# QStringList -> string[]
 $def=~s/Q(ByteArray|Color|Rect|Variant)\s+/\1 /g;# QT types
 $def=~s/QS(Pdf|PdfOperator|TreeItem|PdfOperatorIterator|Annotation|PdfOperatorStack|IPropertyArray|Menu|Page|ContentStream|IProperty|Dict|CObject)\s*\*\s*/\1 /g;	# QSCObjects ....
 #trim unnecessary blank characters
 $def=~s/^\s+//;
 $def=~s/;\s*[\r\n]+$//;
 if ($def=~s|/\*\s*Variant\s*=\s*([A-Za-z0-9_\[\]]+)\s*\*/||) { #Is there specification "what the variant is"?
  my $variantReplacement=$1;
  $def=~s/Variant/$variantReplacement/;
 }
 return $def;
}

sub get_doc {
 my $name=shift;
 my $typ=shift;
 open X,"<$srcdir/$name";
 my $classname='';
 my $ancestor='';
 my $classdesc='';
 my $out='';
 my $dot='';
 while(my $l=<X>) {
  $l=~s/^\s+//;
  $l=~s/[\r\n]+$//;
  if ($typ eq 'base') {
   $classname='';$dot='';
   $ancestor='';
  } elsif ($l=~/class\s+QS(\w+)\s+:\s+public\s+QS(\w+)/) {
   # found class name
   $classname=$1;$dot=".";
   $ancestor=$2;
  } elsif ($l=~/class\s+QS(\w+)\s+:\s+public\s+QObject/) {
   # found class name
   $classname=$1;$dot=".";
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
   my $funcid=$classname.$dot.$func;
   if ($used_func{$funcid}) {
    $used_func{$funcid}++;
    $funcid.="__".$used_func{$funcid};
   } else {
    $used_func{$funcid}=1;
   }
   $cmt=~s/<br\/?>/\n/mg;
   $out.=<<EOF;
   <sect2 id=\"$funcid\">
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

 if ($typ eq 'base') {
  $out=~s/<sect2/<sect1/gm;
  $out=~s/<\/sect2/<\/sect1/gm;
 } else {
 $out=<<EOF;
  <sect1 id="type_${classname}">
   <title>$classname</title>
$anc   <para>
    $classdesc
   </para>
$out  </sect1>
EOF
}
 return $out;
}

while (<>) {
 # parse xml file, all comments in form <!--TYPE: filename.h ...] --> are replaced
 # by documentation generated from that header
 if (/<!--TYPE:\s*([a-zA-Z0-9_\.\-]+)\s*-->/) {
  $_=get_doc($1,'type');
 }
 if (/<!--BASETYPE:\s*([a-zA-Z0-9_\.\-]+)\s*-->/) {
  $_=get_doc($1,'base');
 }
 #slightly disformat the result to discourage accidental editing or generated XML
 s/ +/ /mg;
 print;
}
