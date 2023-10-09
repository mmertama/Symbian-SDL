use strict;
open(IN, $ARGV[0]);
open(OUT, ">$ARGV[1]");
print OUT "struct ccontent_type\n";
print OUT "\t{\n";
print OUT "\tconst char* const name;\n";
print OUT "\tconst int ord;\n";
print OUT "\t};\n\n";

print OUT "const ccontent_type G_ordinals[] =\n"; 
print OUT "\t{\n";
my $count = 0;
while(<IN>)
	{
	if(/(\w*)\s@\s(\d*)/)
		{
		print OUT ",\n" if $count > 0;
		$count++;
		print OUT "\t\t{\"$1\",$2}";		
		}
	}

print OUT "\t};\n\n";

print OUT "#define G_ordinals_count (sizeof( G_ordinals) / sizeof(ccontent_type))\n\n";



