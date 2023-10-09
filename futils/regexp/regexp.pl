#use strict;

my $pat = $ARGV[0];
my $mat = $ARGV[1];

die "Inv" unless length($pat) && length($mat); 

my @cc = split(/[(]/, $pat);



my $count = $#cc + 1;

my $str = '$mat =~ /$pat/;';
for(my $i = 1; $i < $count; $i++)
	{
	$str.= "print \"\$$i \\n\";";
	}

eval $str;


