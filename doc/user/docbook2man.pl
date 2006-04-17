#!/usr/bin/perl -w
use strict;

my $x=1;

#Specific settings
my $appname="pdfedit";
my $longname="PDF Editor";
my $desc="Editor for PDF files";

#version
my $version=`grep 'define VERSION' ../../src/gui/version.h`;
$version=~s/^.*"(.*)".*[\r\n]*/$1/;

my $appnameu=uc($appname);
my $title='';
my $c='';

while (<>) {
 s/<[?!][^>]+>//gm; # doctype/xml/whatever
 s/<\/?(book|bookinfo)[^>]*>//gm;
 s/^\s+//gm;
 s/[\r\n]+//gm;
 if ($x) {
  if (s/<title>(.*)<\/title>/.TH $appname 1 "" "$longname $version" "$longname"\n.SH "NAME"\n$appname \\- $desc\n/g) {$x=0;$title=$_;$_='';}
 }
 $c.=" ".$_;
}

my $opts='';
my $optlist='';
my $chaps='';
while($c=~s|<sect1>\s*<title>Option\s*<option>([^<>]+)</option></title>\s*<para>\s*([\x00-\xff]+?)\s*</para>\s*</sect1>||m) {
 $optlist.=" [$1]";
 $opts.=".TP\n.I \"\\$1\"\n$2\n";
}

sub itable {
 my $r=shift;
 $r=~s/\s+/ /gm;
 $r=~s/<\/row>//g;
 $r=~s/\s*<entry>/.B /g;
 $r=~s/<\/entry>/\n/g;
 $r=~s/<row>/\n/g;
 return "\n".$r."\n";
}

while($c=~s|<chapter id="([^\"]+)">\s*([\x00-\xff]+?)\s*</chapter>||m) {
 $chaps.=".SH ".uc($1)."\n";
 $x=$2;
 $x=~s/<title>(.*)<\/title>//gm;
 $x=~s/<\/?para>//gm;
 $x=~s/\s+/ /gm;
 $x=~s/\s*<variable>(.*)<\/variable>\s*/\n.B $1\n/gm;
 $x=~s/<\/?(tgroup|tbody)[^>]*>//gm;
 my $itable;
 while ($x=~s/\s*<informaltable[^>]*>(.*)<\/informaltable>\s*/itable($1)/em) {
 }
 $chaps.="$x\n";
}

print $title;
print '.SH "SYNOPSIS"'."\n";
print "$appname [file(s)]$optlist\n";
print '.SH "OPTIONS"'."\n";
print $opts;
print $chaps;
print $c;
