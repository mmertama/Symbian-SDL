use  strict;
use File::Copy::Recursive qw(dircopy);

my $target = $ARGV[0];
my $root = $ARGV[1];

my @copydirs;

for(my $i = 2; $i <= $#ARGV; $i++)
	{
	push(@copydirs, $ARGV[$i]);
	}


my @paths;	
find_bldpaths($root, \@paths);	

foreach my $pathn (@paths)
	{
	my $newpath = substr($pathn, length($root));
	$newpath = "$target/$newpath";
	print "$pathn -> $newpath\n";
	dircopy($pathn, $newpath);
	}
	
sub find_bldpaths
	{
	my ($dir, $bldpaths) = @_;
	my $fullPath = "$dir";
	opendir(DIR, $fullPath) or die "Cannot open $fullPath: $!\n";
	my @items = readdir(DIR);
	closedir(DIR);
	foreach my $i (@items)
			{
			my $found = 0;
			foreach my $dir (@copydirs)
				{
				if(lc($i) eq lc($dir))
					{
					push(@$bldpaths, "$fullPath/$i");
					$found = 1;
					}
				}
			if(!$found && !($i =~ /^\.\.?$/))
				{
				my $subpath =	"$dir/$i";
				find_bldpaths($subpath, $bldpaths) if -d "$subpath";
		#		print "$subpath\n";
				}
			}
	}