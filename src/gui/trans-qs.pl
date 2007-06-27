#!/usr/bin/perl
#
# PDFedit - free program for PDF document manipulation.
# Copyright (C) 2006, 2007  PDFedit team:      Michal Hocko,
#                                              Miroslav Jahoda,
#                                              Jozef Misutka,
#                                              Martin Petricek
#
# Project is hosted on http://sourceforge.net/projects/pdfedit
#

while (<>) {
 while (s/tr\("([^"]+)"\)//) {
  print "QT_TRANSLATE_NOOP(\"QObject\",\"$1\")\n";
 }
 while (s/tr\("([^"]+)","([^"]+)"\)//) {
  print "QT_TRANSLATE_NOOP(\"QObject\",\"$1\",\"$2\")\n";
 }
 while (s/tr\('([^']+)'\)//) {
  print "QT_TRANSLATE_NOOP(\"QObject\",\"$1\")\n";
 }
 while (s/tr\('([^']+)','([^']+)'\)//) {
  print "QT_TRANSLATE_NOOP(\"QObject\",\"$1\",\"$2\")\n";
 }
}
