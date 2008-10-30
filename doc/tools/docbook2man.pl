#!/usr/bin/perl -w
use strict;

my $x=1;

#Specific settings
my $appname="pdfedit";
my $longname="PDFedit";
my $desc="Editor for PDF files";

#Direcotory with script
my $selfdir=`dirname $0`;
$selfdir=~s/[\r\n]+//g;
#version
my $version=`$selfdir/../../getversion -v -r`;
if ($version eq '') {
 die "No version information - aborting";
}
$version=~s/^.*"(.*)".*[\r\n]*/$1/;

my $appnameu=uc($appname);
my $title='';
my $c='';

while (<>) {
 s/<[?!][^>]+>//gm; # doctype/xml/whatever
 s/<\/?(book|bookinfo|part)[^>]*>//gm;
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
while($c=~s|<sect1[^<>]*>\s*<title>Option\s*<option>([^<>]+)</option></title>\s*<para>\s*([\x00-\xff]+?)\s*</para>\s*</sect1>||m) {
 $optlist.=" [$1]";
 $opts.=".TP\n.I \"\\$1\"\n$2\n";
}

sub itable {
 my $r=shift;
 $r=~s/\s+/ /gm;
 $r=~s/\s*<title>([^<>]+)<\/title>/\n.B $1\n/gm;
 $r=~s/<\/row>//g;
 $r=~s/\s*<entry>/.B /g;
 $r=~s/<\/entry>/\n/g;
 $r=~s/<row>/\n/g;
 return "\n".$r."\n";
}

while($c=~s|<chapter id="([^\"]+)">\s*([\x00-\xff]+?)\s*</chapter>||m) {
 $x=$2;
 my $name1=$1;
 $name1=~s/^cmdline_//;
 $chaps.=".SH ".uc($name1)."\n";
 $x=~s/<title>(.*)<\/title>//gm;
 $x=~s/<para>//m;
 $x=~s/<\/para>//gm;
 $x=~s/\s+/ /gm;
 $x=~s/>\.+/>/gm;
 $x=~s/\s*<parameter>(.*?)<\/parameter>\s*/\n.B $1\n/gm;
 $x=~s/\s*<envar>(.*?)<\/envar>\s*/\n.B $1\n/gm;
 $x=~s/\s*<option>(.*?)<\/option>\s*/\n.B $1\n/gm;
 $x=~s/<para>/\n/gm;
 $x=~s/\n+/\n/gm;
 $x=~s/<\/?(tgroup|tbody)[^>]*>//gm;
 my $itable;
 while ($x=~s/\s*<informaltable[^>]*>(.*)<\/informaltable>\s*/itable($1)/em) {
 }
 $chaps.="$x\n";
}

$opts=~s/<\/?(tgroup|tbody)[^>]*>//gm;
while ($opts=~s/\s*<table[^>]*>(.*)<\/table>\s*/itable($1)/em) {
}

$opts=~s/\s*<parameter>(.*?)<\/parameter>\s*/\n.B $1\n/gm;
$opts=~s/\s*<envar>(.*?)<\/envar>\s*/\n.B $1\n/gm;
$opts=~s/\s*<option>(.*?)<\/option>\s*/\n.B $1\n/gm;

print $title;
print '.SH "SYNOPSIS"'."\n";
print "$appname [file(s)]$optlist\n";
print '.SH "OPTIONS"'."\n";
print $opts;
print $chaps;
$c=~s/ +$//g;
#possible rest of content - probably nothing useful
#print $c;
