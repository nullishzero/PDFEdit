#!/usr/bin/perl

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
