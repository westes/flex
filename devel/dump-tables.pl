#! /usr/bin/perl -w
# vim:set ft=perl ai si et ts=4 sts=4 sw=4 tw=0:
# USAGE: dump-tables.pl  FILE [max-data]
use strict;

my $dir = $0;
$dir =~ s:/[^/]+$::;
push @INC, $dir;
require "tables.pl";

@ARGV == 1 || @ARGV == 2 || die "\nUSAGE: dump-tables.pl FILE [max-data]\n";
open FD, $ARGV[0] or die "$ARGV[0]: $!";

dump_hdr(read_hdr());
while(!eof FD){
   dump_table(read_table(),$ARGV[1]);
}
close FD;

1
__END__

