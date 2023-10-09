use strict;

my $msg = $ARGV[0];
my $header = $ARGV[1];
my $wait = $ARGV[2];

my $LEN = 76;

my @ln;
my $ww = "";
my @words = split(' ',$msg);
foreach my $w (@words)
	{
	if(length($ww) + length($w) < ($LEN - 5))
		{
		$ww .= "$w ";
		}
	else
		{
		push(@ln, $ww);
		$ww = $w;
		}
	}
#substr($ww, 0, -1)
push(@ln, $ww);

line	($header, '*');
line  ("", " ");
foreach my $l (@ln)
	{
	line  ($l, " ");
	}
line  ("", " ");
line  ("", "*");

sleep($wait);

sub line
	{
	my ($txt, $pad) = @_;
	if(length($txt) & 0x1)
		{
		$txt .= $pad;
		}
	my $len = $LEN - length($txt);
	print "*";
	print $pad x ($len / 2);
	print $txt;
	print $pad x ($len / 2);
	print "*\n";
	}







