#! /usr/bin/perl -w
# vim:set ft=perl ai si et ts=4 sts=4 sw=4 tw=0:
# THIS FILE IS A LIBRARY, AND MEANT TO BE 'required' BY OTHER SCRIPTS
use strict;
use Class::Struct yytbl_hdr => [ 
                    th_magic => '$', th_magic_offset => '$',
                    th_hsize => '$', th_hsize_offset => '$',
                    th_ssize => '$', th_ssize_offset => '$',
                    th_flags => '$', th_flags_offset => '$',
                    th_version => '$', th_version_offset => '$',
                    th_name => '$', th_name_offset => '$'
                 ];
use Class::Struct yytbl_data => [
                    td_id => '$', td_id_offset => '$',
                    td_flags => '$', td_flags_offset => '$',
                    td_hilen => '$', td_hilen_offset => '$',
                    td_lolen => '$', td_lolen_offset => '$',
                    td_data => '@', td_data_offset => '$'
                  ];

sub read_hdr {
    my $th = new yytbl_hdr;
    $th->th_magic_offset(tell FD);
    $th->th_magic(read32());
    $th->th_hsize_offset(tell FD);
    $th->th_hsize(read32());
    $th->th_ssize_offset(tell FD);
    my @a = unpack 'Nna*',readn($th->th_hsize - 8);
    $th->th_ssize($a[0]);
    $th->th_flags_offset($th->th_ssize_offset + 4);
    $th->th_flags($a[1]);
    $th->th_version_offset($th->th_flags_offset + 2);
    $th->th_version($a[2]);
    $th->th_name((split /\0/, $th->th_version)[1]);
    $th->th_version((split /\0/, $th->th_version)[0]);
    $th->th_name_offset($th->th_version_offset + length($th->th_version) + 1);

    return $th;
}
sub dump_hdr {
    my $th = shift;
    printf "%04X th_magic:   %08X\n",$th->th_magic_offset, $th->th_magic;
    printf "%04X th_hsize:   \%d\n",$th->th_hsize_offset, $th->th_hsize;
    printf "%04X th_ssize:   \%d\n",$th->th_ssize_offset, $th->th_ssize;
    printf "%04X th_flags:   \%d\n",$th->th_flags_offset, $th->th_flags;
    printf "%04X th_version: \%s\n",$th->th_version_offset, $th->th_version;
    printf "%04X th_name:    \%s\n",$th->th_name_offset, $th->th_name;
}


sub readn { my $s; read FD, $s, shift; return $s }
sub read32 { my $n; read FD, $n, 4; return unpack 'i', pack 'i', unpack 'N',$n }
sub read16 { my $n; read FD, $n, 2; return unpack 's', pack 's', unpack 'n',$n }
sub read8 { my $n; read FD, $n, 1;  return unpack 'c',$n }
sub readstruct {
    my $td=shift;
    my $r = $::TFLAGS{$td->td_flags & 0x07}->[1];
    return ($r->(), $r->());
}
sub getreader {
    my $td = shift;
    return \&readstruct if ($td->td_flags & 0x10);
    return  $::TFLAGS{$td->td_flags & 0x07}->[1];
}

sub read_table {
    my $td = new yytbl_data;
    printf "HERE: %04X\n", tell FD;
    $td->td_id_offset(tell FD);
    $td->td_id(read16());
    $td->td_flags_offset(tell FD);
    $td->td_flags(read16());
    $td->td_hilen_offset(tell FD);
    $td->td_hilen(read32());
    $td->td_lolen_offset(tell FD);
    $td->td_lolen(read32());
    $td->td_data_offset(tell FD);

    my $read = getreader($td);
    
    my $tot = $td->td_lolen * ($td->td_hilen?$td->td_hilen:1);
    $tot *=2 if $::TID{$td->td_id}eq 'YYTH_ID_TRANSITION';

    for(1..$tot){
        my @v = $read->($td);
        push @{$td->td_data}, @v;
    }
    my $pad = pad64(tell FD);
    while($pad-- > 0){ read8() }
    return $td;
}

sub dump_table {
    my $td = shift;
    my $max = shift;
    defined($max) or $max = 2**15;
    printf "%04X td_id:    \%d (\%s)\n", $td->td_id_offset, $td->td_id, $::TID{$td->td_id};
    printf "%04X td_flags: \%0x (\%s)\n",$td->td_flags_offset, $td->td_flags, "@{[TFLAGS($td->td_flags)]}";
    printf "%04X td_hilen: \%d\n", $td->td_hilen_offset, $td->td_hilen;
    printf "%04X td_lolen: \%d\n", $td->td_lolen_offset, $td->td_lolen;
    
    printf "%04X td_data: ", $td->td_data_offset;
    foreach( @{$td->td_data} ){
        last if $max-- == 0;
        print "$_, ";
    }
    print "\n";
    return $td;
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
	0x01 => 'YYTD_ID_ACCEPT' ,
	0x02 => 'YYTD_ID_BASE' ,
	0x03 => 'YYTD_ID_CHK' ,
	0x04 => 'YYTD_ID_DEF' ,
	0x05 => 'YYTD_ID_EC' ,
	0x06 => 'YYTD_ID_META' ,
	0x07 => 'YYTD_ID_NUL_TRANS' ,
	0x08 => 'YYTD_ID_NXT' ,
	0x09 => 'YYTD_ID_RULE_CAN_MATCH_EOL' ,
	0x0A => 'YYTD_ID_START_STATE_LIST' ,
	0x0B => 'YYTD_ID_TRANSITION',
	0x0C => 'YYTD_ID_ACCLIST');

    %::TFLAGS = (
	0x01 => ['YYTD_DATA8',\&read8] ,
	0x02 => ['YYTD_DATA16',\&read16] ,
	0x04 => ['YYTD_DATA32',\&read32] ,
	0x08 => ['YYTD_PTRANS',undef],
	0x10 => ['YYTD_STRUCT',\&readstruct]);
}

# CODE TO EXTRACT TABLES FROM GENERATED C CODE
sub extract_from_code {
    my $nms = qr/yy_(?:accept|base|chk|
                 def|ec|meta|NUL_trans|
                 nxt|rule_can_match_eol|
                 start_state_list|transition)/x;

    while(<FD>){
        next unless m/static\s+yyconst\s+.+yy_($nms)(\[\])?\[(\d+)\]\s*=/;
        my $n = $1;
        my $dim = defined($2)?2:1;
        # TODO 
    }
}

1
__END__

