use strict;


my $path = $ARGV[0];
my $root = $ARGV[1];
my @exts = split(' ', $ARGV[2]);


my @paths; 

readpaths($path, \@paths, 0);

if($root =~ /^[\\\/]/)
	{
	$root = substr($root, 1);
	}
	
exit if(length($root) == 0); # nothing to add	

foreach my $mm (@paths)
	{
	my @lines;
	open(FILE, $mm->{'PATH'}) or die "$mm: $!\n";
	while(<FILE>)
		{
			# 
		my $tops = "..\\" x $mm->{'LEVEL'};
		$tops .= 'epoc32';
		s/(^|\s|\;|\"|\')\\epoc32\\/$1$tops\\/gi;
		push(@lines, $_);
		}
	close(FILE)
	open(FILE, ">$mm->{'PATH'}") or die "$mm: $!\n";
	print FILE, @lines;
	close(FILE);
	}

sub readpaths
	{
	my ($root, $paths, $level) = @_;
	opendir(DIR, $root) or die "Cannot open $root $!\n";
	my @entries = readdir(DIR);
	closedir(DIR);
	foreach my $e (@entries)
		{
		next if $e eq '.' || $e eq '..';
		my $fullpath = "$root/$e";
		if(-d $fullpath)
			{
			readpaths($fullpath, $paths, $level + 1);
			}
		else
			{
			foreach my $ex (@exts)
				{
				if ($e =~ /\.$ex$/i)
					{
					my $item = {'PATH' => $fullpath, 'LEVEL' => $level}; 
					push(@$paths, $item) ;
					next;
					}
				}
			}
		}
	}



