# extract all table types from various scanners. We try to get the ones
# that are declared outside '{'.
# hopefully we'll hit them all.
OUTFILE=/tmp/EXTRACT-FLEX-SYMS.$$
echo > $OUTFILE

for s in  \
	yy_trans_info \
	yy_NUL_trans \
	yy_accept \
	yy_base \
	yy_chk \
	yy_def \
	yy_ec \
	yy_meta \
	yy_nxt \
	yy_rule_can_match_eol \
	yy_rule_linenum \
	yy_start_state_list \
	yy_state_type \
	yy_transition
do
	perl -ne \
	'
		BEGIN{
		   	$s = qq('$s');
		}
		s/\b(short|long)\s+int\b/$1/;
		s/\b(static|(yy)?const)\b\s+//g;
		next unless m/((?:struct\s*)?\w+)\W+$s\s*((?:\[\s*\d*\s*\])+)\s*=/;
		$type=$1;
		$arr = $2;
		$arr =~ s/\d+//g;
		$a{$s}->{$type . $arr}= $_;
		
		END{
			for(sort keys %a){
				print values %{$a{$_}}
			}
		}
	' \
	tests/test-*/*.c  >> $OUTFILE
done

grep -E 'typedef.*yy_state_type'  tests/test-*/*.c | 
		gawk -F ':' '{print $2}' |
		sort |
		uniq >> $OUTFILE

pcregrep  '(\w+)[\s*]*yy_(verify|nxt)\s*;'  tests/test-*/*.c |
		perl -pe 's/^.*?\.c?[\-\:]//;' |
		sort |
		uniq >> $OUTFILE

cat $OUTFILE
rm -f $OUTFILE

