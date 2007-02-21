#!/usr/bin/perl

use strict;

#omit output
my $omit=0;
my $code=0;
my @ttc=();

sub add_tag {
 my $tg=shift;
 close_tags($tg);
 push @ttc,$tg;
}

sub close_tags {
 my $regex=shift;
 my $tags='';
 while (@ttc) {
  my $last=$ttc[@ttc-1];
  if ($last=~/^$regex$/) {
   $tags.='</'.$last.'>';
   pop @ttc;
   next;
  }
  last;
 }
 return $tags;
}

my %tagx=();
$tagx{'c'}='code';
$tagx{'e'}='emphasis';

sub munge {
 my $par=shift;
 $par=~s/[^a-zA-Z0-9]/_/g;
 return $par;
}

my %taken_sections=();

sub uniq {
 my $par=shift;
 if ($taken_sections{$par}) {
  my $i=0;
  while ($taken_sections{$par.'_'.$i}) {
   $i++;
  }
  $par=$par.'_'.$i;
 }
 $taken_sections{$par}=1;
 return $par;
}

sub tag {
 my $tg=shift;
 my $param=shift;
 if ($tg eq 'omit')	{ $omit=1; return ''; }
 if ($tg eq 'endomit')	{ $omit=0; return ''; }
 if ($tg eq 'code')	{ $code=1; return '<programlisting>'; }
 if ($tg eq 'endcode')	{ $code=0; return '</programlisting>'; }
 if ($tg=~/^(chapter|section(\d+))$/)	{
  my $t='';
  if ($1 eq 'chapter') {$t=close_tags('.*'); }
  else {my $n=$2;$t=close_tags('sect['.$n.'-9]'); $tg=~s/^section/sect/;}
  add_tag($tg);
  my $idsect='p_'.munge($param);
 $idsect=uniq($idsect);
  return $t.'<'.$tg.' id="'.$idsect.'"><title>'.$param.'</title>';
 }
 if ($tg=~/^(c|e)(\{?)$/)	{
  my $cn=$tagx{$1};
  if ($2 eq '{') {
   $param="<$cn>$param</$cn>";
  } else {
   $param=~s/^(\S+)/<$cn>$1<\/$cn>/;
  }
  return $param;
 }
 if ($tg eq 'list')	{ return '<itemizedlist spacing="compact">'; }
 if ($tg eq 'endlist')	{ return close_tags('listitem|para').'</itemizedlist>'; }
 if ($tg eq 'l')	{
  #link to somewhere ... probably to somewhere out of the docs, so not supported
  return $param;
 }
 if ($tg eq 'img')	{
  #Image
  $param=~s/^\s+//;
  $param=~s/\s+$//;
  return '<mediaobject><imageobject><imagedata fileref="images/'.$param.'" /></imageobject></mediaobject>';
 }
 if ($tg eq 'i')	{
  my $t=close_tags('listitem|para');
  add_tag('listitem');
  add_tag('para');
  return $t.'<listitem><para>'.ptags($param);
 }
 die ("Unknown tag \\$tg\n");
}

if (!($ARGV[0] eq 'no')) {
print <<EOF;
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE chapter PUBLIC "-//OASIS//DTD DocBook XML V4.2//EN" "http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd">
EOF
}

sub ptags {
 my $l=shift;
 while ($l=~s/\\([a-z0-9_]+\{)([^}]*)\}/tag($1,$2)/em) {;}
 while ($l=~s/\\([a-z0-9_]+)(\s+|$)(.*)/tag($1,$3)/em) {;}
 return $l;
}

while (<STDIN>) {
 s/\r//g;
 s/&/&amp;/g;
 s/</&lt;/g;
 s/>/&gt;/g;
 $_=ptags($_);
 next if ($omit);
 print;
}
print close_tags('.*');