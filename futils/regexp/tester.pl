


use strict;
my $linecount = 0;
my $ok = 0;

while(<>)
	{
	chomp;
	next if length($_) <= 0;
	system("debug\\regexp $_ > tmprs1");
	test($_);
	$linecount++;
	open(R1, "tmprs1") or die "t_error: $!";
	open(R2, "tmprs2") or die "t_error: $!";;
	my @r1 = <R1>;
	my @r2 = <R2>;
	close R1;
	close R2;


	my $count1 = $#r1 + 1;
	my $count2 = $#r2 + 1;
	my $count = max($count1, $count2);
	
	for(my $i = 0; $i < $count; $i++)
		{
		my $out1  = $r1[$i];
		chomp($out1);
		my $out2 = $r2[$i]; 
		chomp($out2);

		unless($out1 eq $out2)
			{
			print "t_error $linecount\: $_ \n->\n\"$out1\" p:\n\"$out2\"\n\n";
			$ok--;
			}
			
		}
	print ".";	
	}

my $isok = $linecount + $ok;

print "\n$linecount tests ran, $isok ok\n";


sub cmdLine
	{
	my $line = shift;
	chomp($line);
	my @args = split(/(")/, $line);
	my @list;
	my $cc = 0;
	foreach my $arg (@args)
		{
		#	print "--$arg--";
		if($arg =~ /"/)
			{
			if($cc)
				{
				$cc = 0
				}
			else
				{
				$cc = 1;
				}
		#	print "($cc)";
			}
		else
			{
			unless($cc)	
				{
				my @oo = split(/(\s+)/, $arg);
				foreach my $oos (@oo)
					{
					push(@list, $oos) if $oos =~ /\S+/;
					}
				}
			else
				{
				push(@list, $arg) if $arg =~ /\S+/;
				}	
			}
		}
	return @list;
	}

sub test
	{
	my $line = shift();
# print "c:$line\n";

	my @cmd = cmdLine($line);

#	print "->$cmd[1]<-\n";

#	print "\n";

#	foreach my $f (@cmd)
#		{
#		print "C: >$f<\n";
#		}


	my $opt;
	my $sub;
	if(defined($cmd[2]))
		{
		if($cmd[2] =~ /-([a-z]+)/)
			{
			$opt = $1;
			}
		else
			{
			$sub = $cmd[2];
			}
		if(defined($cmd[3]))
			{
			if($cmd[3] =~ /-([a-z]+)/)
				{
				$opt = $1;
				}
			else
				{
				$sub = $cmd[3];
				}
			}
		}
	
	my $str = "open(TEMP, \">tmprs2\") or print \"t_error\";\n";
	$str .= 'if ($cmd[1] =~ /$cmd[0]/';
	$str .= "$opt" if(defined ($opt));
	$str .= ")\n";

	$str .= "{\nprint TEMP \"Matched\\n\";";
	
	my @cc = split(/[(]/, $cmd[0]);
	my $count = $#cc + 1;

	for(my $i = 1; $i < $count; $i++)
		{
		$str .= 'my $dd = $';
		$str .= "$i;"; 
		$str .= 'if(chs($dd))';
		$str.= "{\nprint TEMP \"\$dd\\n\"}else {\nprint TEMP \"-empty-\";}";
		}
	$str .= "}";
	$str .= "else{\nprint TEMP \"No Match\\n\";}";

#	print "\n\n$str";

	if(defined($sub))
		{
		$str .= 'my $s = $cmd[1];';
		$str .= "\n";
		$str .= 'my $c = $s =~ s/$cmd[0]/';
		$str .= "$sub/";
		$str .= "$opt" if(defined ($opt));
		

		$str .= ";\n";
		$str .= "print TEMP \"";
		$str .= '$c $s';
		$str .= "\\n\"";
		$str .= 'if $c > 0;';
	
		}
	$str .= "close (TEMP);";
#	if ($linecount == 13){
#	print $str ;
#	exit;	}

	eval $str;
	print $@ if defined $@;
	}

sub chs
	{
	my $n = shift;
	my @p = split('', $n);
	foreach my $nn (@p)
		{
		my $i = ord($nn);
		my $j = ord(' ');
#		print ">$i $j $nn<";
		return 1 if $i > $j;
		}
	return 0;
	}

sub max
	{
	my ($a, $b) = @_;
	return $a if($a > $b);
	return $b;
	}


