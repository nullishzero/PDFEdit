#!/usr/bin/perl
use strict;

#Generate list of keyboard shortcuts from the menu file

#template file into which the shortcut table will be inserted
my $srcfile=$ARGV[0];
#optional translation file
my $tr=$ARGV[1];


#Read template
my $srccont='';
open IF, "<$srcfile";
while (my $l=<IF>) {
 $srccont.=$l;
}
close IF;

#Read translation, if specified
my $ctx='';
my $src='';
my %hash=();
if ($tr) {
 open TR, "<$tr";
 while (my $l=<TR>) {
  if ($l=~/<name>([^<>]+)<\/name>/) {
   $ctx=$1;
  } elsif ($l=~/<source>([^<>]+)<\/source>/) {
   $src=$ctx."/".$1;
  } elsif ($l=~/<translation>([^<>]+)<\/translation>/) {
   my $trx=$1;
   $trx=~s/&amp;//g;
   $hash{$src}=$trx;
  }
 }
 close TR;
}

#read input (file with settings, containing all menu items)
my %out=();
while (<STDIN>) {
 if (/^items\/[^=]+=\s*item\s+([^,]*)\s*,\s*([^,]*),\s*([^, ]+)/) {
  #Item with shortcut found
  my $kbd=$3;
  my $action=$1;
  my $actionk=$action;
  $actionk=~s/&/&amp;/g;
  $actionk="gui::Settings/".$actionk;
  $action=~s/&//g;
  my $trac=$action;
  #Try to translate
  if (length($hash{$actionk})) {
   $trac=$hash{$actionk};
  }
  my $kbdkey=$kbd;
  $kbdkey=~s/\s+//mg;
  $kbdkey=~s/^([a-z]+)\+([a-z])$/$1+ $2/i;
  $out{$kbdkey}.="<row><entry>$kbd</entry><entry>$trac</entry></row>";
 }
}

my $outs='';
foreach my $i (sort keys %out) {
 $outs.=$out{$i};
}

#replace result in template
$srccont=~s/<KBDTABLE\/>/$outs/mg;

#write template
print $srccont;
