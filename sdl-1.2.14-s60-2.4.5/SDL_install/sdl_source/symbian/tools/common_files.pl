
use strict;

my $bldfile = $ARGV[0];
my $pkgfile = $ARGV[1];
my $sdk_root = $ARGV[2];
$sdk_root = substr($sdk_root, 2) if $sdk_root =~ /^[A-Za-z]\:/;
$sdk_root .= "\\" unless $sdk_root =~ /\\$/;

open(BLD, ">$bldfile") or die "Cannot open $bldfile: $!\n";

my $exp_source = "..\\..\\..\\test";
my $exp_target = "\\EPOC32\\winscw\\c\\Private";
my @pkgdata;

print BLD "PRJ_PLATFORMS\n";
print BLD "\n\n";
print BLD "PRJ_EXPORTS\n";
doexp("sample.bmp",	"e000101c");
doexp("sample.wav", "e0001003");
doexp("icon.bmp",		"e0001004");
doexp("sample.bmp",	"e0001006");
doexp("moose.dat", "e0001013");
doexp("sample.bmp", "e0001014");
doexp("sail.bmp", 	"e0001015");
doexp("icon.bmp", 	"e0001018");
print BLD "\n\n";
print BLD "PRJ_MMPFILES\n";
close(BLD);

open(PKG, ">$pkgfile") or die "Cannot open $pkgfile: $!\n";
print PKG ";Package header\n\n";

print PKG ";Languages\n";
print PKG "&EN\n";
print PKG ";Header\n\n";


print PKG "#{\"SDL Test\"}, (0xE0001000), 1, 0, 0\n\n";
print PKG "%{\"SDL-EN\"} : \"SDL\"\n\n";
print PKG "*\"\\certificates\\selfsigned.key\", \"\\certificates\\selfsigned.cer\"\n\n";
print PKG "[0x101F7961], 0, 0, 0, {\"Series60ProductID\"}\n";
print PKG ";Installed files\n";

print "Common file created\n";

foreach my $pkgln (@pkgdata)
	{
	print PKG $pkgln;
	}

print PKG "\n\n";

close(PKG);

sub doexp
	{
	my ($s, $t) = @_;
	print BLD "$exp_source\\$s $exp_target\\$t\\$s\n";
	push(@pkgdata, "\"$exp_source\\$s\"-\"!:\\Private\\$t\\$s\"\n");
	}





