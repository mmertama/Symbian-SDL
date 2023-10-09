use strict;
my $makefile = $ARGV[0];
my $rule = $ARGV[1];
my $handler = $ARGV[2];
my $passparam = join(' ', splice(@ARGV, 3));

open(MAKE, $makefile) or die "$makefile error: $!\n";

my @targets;
my %assoc;
my %rules;
my $cur_open = 0;

while(<MAKE>)
	{
	chomp;
	if(/^#*\s*$/)
		{
		$cur_open = 0;
		next;	
		}
	if($cur_open && /\s*(.*)/)
		{
		my $ref = $rules{$cur_open};
		push(@$ref, $1);
		next;
		}
	if(/^\s*(.*?)\s*=\s*(.*)/)
		{
		$assoc{$1} = $2;	
		next;
		}
	
	if(/^\s*(.*?)\s*:\s*(.*)/)
			{
			$cur_open = $1;
			Alias(\$cur_open);
			push(@targets, $cur_open);
			$rules{$cur_open} = [$2];
			next;
			}		
	}
	
close(MAKE);

my $global_calls = 0; 
my $result = Recurse_Dependencies(\$rule);

		
sub Alias
	{
	my $name = shift;
	if($$name =~ /\$\((.*?)\)/)
		{
		my $res = $assoc{$1};
		if(defined($res))
			{
			$$name = "$`$res$'";
			Alias($name);
			}
		}	
	}		

sub Dependencies
	{
	my $target = shift;
	my $r = $rules{$$target};
  my @out;
	return \@out unless defined($r);
	my $count = @$r;
	if($count)
		{
		my $item = $r->[0]; 
		Alias(\$item);
		@out = split(' ', $item);	
		}
	return \@out;	
	}
	
sub Commands
	{
	my $target = shift;
	my $r = $rules{$$target};
	my @out;
	return \@out unless defined($r);
	my $count = @$r;
	for(my $i = 1;  $i < $count; $i++)
		{
		my $item = $r->[$i];	
		Alias(\$item);
		push(@out, $item);
		}	
	return \@out;		
	}	

sub Recurse_Dependencies
	{
	my $r = shift;
	my $dep = Dependencies($r);
	my $count = @$dep;
	my @files;
	foreach my $item (@$dep)
		{
		push (@files, $item) if 0 == Recurse_Dependencies(\$item);
		}
	my $cmds = Commands($r);
	my $filelst = join(' ', @files);
	foreach my $cmd (@$cmds)
		{
		$cmd =~ s/\$\?/$filelst/g;
		$cmd =~ s/\$\@/$$r/g;
		++$global_calls;
		system("$handler $cmd $passparam #$global_calls") == 0 or die "failed: $cmd $passparam\n";
		}
	return $count;
	}

	