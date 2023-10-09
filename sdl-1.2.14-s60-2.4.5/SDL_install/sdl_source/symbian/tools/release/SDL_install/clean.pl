#
# Copyright Markus Mertama 2008
# Cleanup folders from tree 
#

use  strict;
use File::Path qw(rmtree);

my $root = $ARGV[0];

my @dirs;

for(my $i = 1; $i <= $#ARGV; $i++)
	{
	push(@dirs, $ARGV[$i]);
	}


my @paths;	
find_paths($root, \@paths);	

foreach my $pathn (@paths)
	{	
	rmtree($pathn);
	}
	
my $count = @paths;
print "$count folders removed\n";	
	
sub find_paths
	{
	my ($dir, $paths) = @_;
	my $fullPath = "$dir";
	opendir(DIR, $fullPath) or die "Cannot open $fullPath: $!\n";
	my @items = readdir(DIR);
	closedir(DIR);
	foreach my $i (@items)
			{
			my $found = 0;
			foreach my $dir (@dirs)
				{
				if(lc($i) eq lc($dir))
					{
					push(@$paths, "$fullPath/$i");
					$found = 1;
					}
				}
			if(!$found && !($i =~ /^\.\.?$/))
				{
				my $subpath =	"$dir/$i";
				find_paths($subpath, $paths) if -d "$subpath";
				}
			}
	}