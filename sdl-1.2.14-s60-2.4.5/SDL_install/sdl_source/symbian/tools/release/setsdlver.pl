use strict;

my @val;

if($ARGV[1] =~ /(\d+)\.(\d+)\.(\d+)/)
	{
	push(@val, $1);
	push(@val, $2);
	push(@val, $3);
	}
elsif($ARGV[1] =~ /(\d+)\.(\d+)/)
	{
	push(@val, $1);
	push(@val, $2);
	push(@val, 0);
	}
else
	{
	die("Invalid version number!!\n\n");
	}
	


open(IN, $ARGV[0]) or die "$ARGV[0], $!\n";

my @lines;

while(<IN>)
	{
	if(/#\{\"(.*?)\"\},\(0xF01F3D69\),(\d+),(\d+),(\d+),TYPE=(\S+)+/)
		{
		push (@lines, "#\{\"$1\"\},\(0xF01F3D69\),$val[0],$val[1],$val[2],TYPE=$5\n");
		}
	else
		{
		push (@lines, $_);
		}
	}

close(IN);

open(OUT, ">$ARGV[0]") or die "$!\n";
print OUT @lines;
#print @lines;
close(OUT);
