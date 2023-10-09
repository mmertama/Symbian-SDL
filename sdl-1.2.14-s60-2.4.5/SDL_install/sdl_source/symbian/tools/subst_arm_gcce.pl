use strict;

my $pkgfile = $ARGV[0];
my $ext = $ARGV[1];
my $match = $ARGV[2];
my $rep = $ARGV[3];

my $pkgfile_gcce = $pkgfile;

$pkgfile_gcce =~ s/\.(\w+)$/$ext\.$1/i;

print "Created $pkgfile_gcce\n";

open(IN, $pkgfile) or die "Open in $!, $pkgfile\n";
open(OUT, ">$pkgfile_gcce") or die "Open out $!, $pkgfile_gcce\n";

while(<IN>)
	{
	s/$match/$rep/ig;
	print OUT;
	}

close(IN);
close(OUT);


