#! /usr/bin/env perl
# vim:set autoindent smartindent expandtab tabstop=4 softtabstop=4 shiftwidth=4 textwidth=0:
# Author and scapegoat if this breaks: Millaway
use Fcntl qw(:seek);
use File::Copy;
use strict;
$^W = 1; # same as perl -w
my $USAGE = q{
    Usage:
        create-test.pl [-p] [-+] TEST_NAME

        Creates a new flex test named "TEST_NAME" in the directory
        TEST_NAME/. Modifies configure.in.

        OPTIONS:

          -p  include a skeleton bison parser
          -+  make a  C++ scanner

};
sub abort { print shift,"\n"; exit 1}

my $skel= "TEMPLATE";
my $testname = undef;
my ($parser,$cplusplus) = (0,0);

while(defined ($_=shift)){
    /^-p$/  && do{ $parser = 1; next};
    /^-\+$/ && do{ $cplusplus = 1; next};
    /^-/    && abort "$0: Unrecognized option '$_'.\n$USAGE";
    $testname = $_;
    last;
}

abort($USAGE) unless @ARGV == 0 && defined $testname;

# Some simple safe-guards.
$testname =~ m/^test-\w+/ or abort "Testname must begin with 'test-'. Aborting.";
-e $testname              and abort "File '$testname' already exists. Refusing to overwrite.";
-e $skel && -d $skel      or abort "Can't find directory '$skel/'. Aborting.";

# create the test dir
print "mkdir $testname\n";
mkdir $testname or abort "Unable to create directory '$testname/': $!";

# shallow copy the skeleton directory, replacing special keywords here and there.
opendir SKEL, $skel or abort "Could not read directory $skel: $!";

foreach my $file ( grep { !/^\.+$/ && -f "$skel/$_" } readdir SKEL ){

    next if $file eq 'parser.y' && !$parser;

    print "cp  $skel/$file  $testname/$file\n";
    open IN, "< $skel/$file" or abort "Unable to read file $skel/$file: $!";
    open OUT, "> $testname/$file" or abort "Unable to write file $testname/$file: $!";
    my $did_cpp_option=0;
    my $did_parser_objs=0;

    while(<IN>){
        s/\bTEMPLATE\b/$testname/g;

        # add %option c++
        $cplusplus
            && /^\%option\s+.+$/
            && !$did_cpp_option
            &&  do { $_ .= "\%option c++\n"; $did_cpp_option=1 };

        $cplusplus && s/scanner\.c/scanner\.cc/g;

        # change $(CC) to $(CXX)
        BEGIN{ $::CC = qr/^(?<=\t)\$\(CC\)/;
               $::CXX = quotemeta '$(CXX)';
              }
        $cplusplus && s/$::CC/$::CXX/o;

        # fix the makefile to include the parser.
        $parser
            && !$did_parser_objs
            && s/^(OBJS\s*=\s*scanner\.o)/$1 parser.o/
            && ($did_parser_objs=1);

        print OUT;
    }

    close IN;
    close OUT;
}
closedir SKEL;

if (-e "$testname/cvsignore" ){
    print "mv $testname/cvsignore $testname/.cvsignore...\n";
    move("$testname/cvsignore", "$testname/.cvsignore")
        or abort "Could not rename $testname/.cvsignore";
}

# modify configure.in...
print "Modifying configure.in (appending to TESTDIRS and AC_OUTPUT)...\n";

# We slurp it all into memory. We can do this because configure.in is small.
# If we move the tests/ dir to automake then revisit this.
open CONF, "+< configure.in" or abort "Could not read 'configure.in': $!";
my $entire_conf = do{ local $/; <CONF> };

# - Add test-directory to "TESTDIRS" variable
#   This is complex because we append to it, we avoid creating long lines, and we
#   preserve indenting. Hey, that's what perl is for.
#   Otherwise we could just do:  s/TESTDIRS="/$&$testname /;
$entire_conf =~ s/(TESTDIRS\s*=\s*".+)             #  (everything up to last line)
                  ^(\s*)((?:test-[\w-]+\s*)+)"     #  (indenting)(last line)
                 /length("$2$3$testname") > 80
                      ? qq($1$2$3 \\\n$2$testname")
                      : qq($1$2$3 $testname")
                 /xmsge;


# - Add test-directory/Makefile to "AC_OUTPUT" variable
#   This is complex. See above.
$entire_conf =~ s/(AC_OUTPUT\s*\(.+)                      # (everything up to last line)
                  ^(\s*)((?:test-[\w-]+\/Makefile\s*)+)\) # (indenting)(last line)
                 /length("$2$3$testname\/Makefile") > 80
                       ? "$1$2$3 \\\n$2$testname\/Makefile)"
                       : "$1$2$3 $testname\/Makefile)"
                 /xmsge;

# rewind the file and save the changes
seek CONF, 0, SEEK_SET;
print CONF $entire_conf;
close CONF;

print <<MSG;

*********************************************************************
"$testname/" has been created. It contains a generic test.
Now you must:
 1. Edit "$testname/scanner.@{[$cplusplus?'cc':'c']}" to customize your test.
 2. Make an entry in the README, describing your test.
 3. Run 'autoconf' to rebuild 'configure'.
*********************************************************************

MSG

__END__

