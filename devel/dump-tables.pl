#! /usr/bin/perl -w
# vim:set ft=perl ai si et ts=4 sts=4 sw=4 tw=0:
# USAGE: dump-tables.pl  FILE
use strict;

@ARGV == 1 || die "\nUSAGE: dump-tables.pl FILE\n";
open FD, $ARGV[0] or die "$ARGV[0]: $!";

my ($magic,$hsize) = (read32(),read32());
my $rest = readn($hsize - 8);
my ($ssize,$flags,$version,$name) = unpack 'Nna*', $rest;
($version,$name) = split /\0/, $version;

printf "th_magic:   %08X\n", $magic;
print  "th_hsize:   $hsize\n";
print  "th_ssize:   $ssize\n";
print  "th_flags:   $flags\n";
print  "th_version: $version\n";
print  "th_name:    $name\n";

my $bytes = $hsize;
while(!eof FD){
    dump_table();
}


sub readn { my $s; read FD, $s, shift; return $s }
sub read32 { my $n; read FD, $n, 4; return unpack 'i', pack 'i', unpack 'N',$n }
sub read16 { my $n; read FD, $n, 2; return unpack 's', pack 's', unpack 'n',$n }
sub read8 { my $n; read FD, $n, 1;  return unpack 'c',$n }

sub dump_table {
    my ($id,$flags,$hi,$lo) = (read16(),read16(),read32(),read32());
    my $max = 40;
    print "td_id:    $id ($::TID{$id})\n".
          "td_flags: $flags (@{[TFLAGS($flags)]})\n".
          "td_hilen: $hi\n".
          "td_lolen: $lo\n";

    my $read = $::TFLAGS{$flags}->[1];
    
    my $tot = $lo * ($hi?$hi:1);
    $tot *=2 if $::TID{$id}eq 'YYT_ID_TRANSITION';

    my @d;
    for(1..$tot){
        my $v = $read->();
        if($max-- > 0){
            push @d, $v;
        }
    }
    print "td_data: ";
    print join ', ', @d;
    my $pad = pad64(tell FD);
    print "\npadding: $pad\n";
    while($pad-- > 0){ read8() }
    return tell FD;
}

sub TFLAGS {
    my @s;
    my $f = shift;
    foreach(keys %::TFLAGS){
        if ($f & $_){
            push @s, $::TFLAGS{$_}->[0]
        }
    }
    return join '|', @s;
}

sub pad64{ return ((8-((shift)%8))%8) }

BEGIN {
    %::TID = ( 
	0x01 => 'YYT_ID_ACCEPT' ,
	0x02 => 'YYT_ID_BASE' ,
	0x03 => 'YYT_ID_CHK' ,
	0x04 => 'YYT_ID_DEF' ,
	0x05 => 'YYT_ID_EC' ,
	0x06 => 'YYT_ID_META' ,
	0x07 => 'YYT_ID_NUL_TRANS' ,
	0x08 => 'YYT_ID_NXT' ,
	0x09 => 'YYT_ID_RULE_CAN_MATCH_EOL' ,
	0x0A => 'YYT_ID_START_STATE_LIST' ,
	0x0B => 'YYT_ID_TRANSITION');

    %::TFLAGS = (
	0x01 => ['YYT_DATA8',\&read8] ,
	0x02 => ['YYT_DATA16',\&read16] ,
	0x04 => ['YYT_DATA32',\&read32] ,
	0x08 => ['YYT_PTRANS',sub{}]);
}

1
__END__

