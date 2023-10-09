use strict;


my $count = @ARGV;
my $docomp = 0;
my $target;
my @libs;
my @files;
my $uid = 0;
my $targetdir;
my $pkgfile;
my $bldinffile;
my $srcdir;
my @userinc;
my @sysinc;
my @exclude;
my $mathlibneed = 0;
my $gllibneed = 0;
my $sdk_root = "\\";

my $ii = -1;

while(1)
	{
	++$ii;
	last unless $ii < $count;
	
	if($ARGV[$ii] =~ /\@CC\@/)
		{
		$docomp = 1;
		next;
		}
	if($docomp)
		{
		if($ARGV[$ii] eq "-o")
			{
			++$ii;
			$target = $ARGV[$ii];
			next;
			}
		if($ARGV[$ii] eq "\@CFLAGS\@")
			{
			next;
			}
		if($ARGV[$ii] eq "\@LIBS\@")
			{
			push(@libs, "euser.lib");
			push(@libs, "sdl.lib");
			push(@libs, "sdlexe.lib");
			next;
			}
		if($ARGV[$ii] eq "\@MATHLIB\@")
			{
			$mathlibneed = 1;
			next;
			}
		if($ARGV[$ii] eq "\@GLLIB\@")
			{
			$gllibneed = 1;
			next;
			}
		if($ARGV[$ii] eq "-uid")
			{
			++$ii;
			$uid += hex($ARGV[$ii]);
			next;
			}	
		if($ARGV[$ii] eq "-targetdir")
			{
			++$ii;
			$targetdir = $ARGV[$ii];
			next;
			}	
		if($ARGV[$ii] eq "-sdkroot")
			{
			++$ii;
			$sdk_root = $ARGV[$ii];
			$sdk_root = substr($sdk_root, 2) if $sdk_root =~ /^[A-Za-z]\:/;
			$sdk_root .= "\\" unless $sdk_root =~ /[\\\/]$/;
			next;
			}	
		if($ARGV[$ii] eq "-pkgfile")
			{
			++$ii;
			$pkgfile = $ARGV[$ii];
			next;
			}		
		if($ARGV[$ii] eq "-bldinffile")
			{
			++$ii;
			$bldinffile = $ARGV[$ii];
			next;
			}		
		if($ARGV[$ii] eq "-uinc")
			{
			++$ii;
			push(@userinc, split(/;/, $ARGV[$ii]));
			next;
			}			
		if($ARGV[$ii] eq "-sinc")
			{
			++$ii;
			push(@sysinc, split(/;/, $ARGV[$ii]));
			next;
			}			
		if($ARGV[$ii] eq "-excl")
			{
			++$ii;
			push(@exclude, split(/;/, $ARGV[$ii]));
			next;
			}				
		if($ARGV[$ii] =~ /^#(\d+)/)
			{
			$uid += $1;
			next;
			}			
		if($ARGV[$ii] eq "-srcdir")
			{
			++$ii;
			$srcdir = $ARGV[$ii];
			next;	
			}
		if($ARGV[$ii] eq "-lib")
			{
			++$ii;
			push(@libs, split(/;/, $ARGV[$ii]));
			next;
			}
		if($ARGV[$ii] eq "-mathlib")
			{
			++$ii;
			push(@libs, split(/;/, $ARGV[$ii])) if $mathlibneed;
			next;
			}
		if($ARGV[$ii] eq "-gllib")
			{
			++$ii;
			push(@libs, split(/;/, $ARGV[$ii])) if $gllibneed;
			next;
			}	
		push(@files, $ARGV[$ii]);
		}
	}



my $name;
if($target =~ /(.*?)\@EXE\@/)
	{
	$name = $1; 
	}
	
foreach my $exl (@exclude)
	{
	exit if($name =~ /$exl/);	
	}

my $mmpname = "$targetdir\\$name.mmp";


open(MMP, ">$mmpname") or die "cannot open mmpfile; $mmpname\n";

print MMP "\n\n";

print MMP "TARGET $name.exe\n";
print MMP "TARGETTYPE exe\n";
printf MMP "UID 0 0x%x\n", $uid;
print MMP "CAPABILITY ReadUserData WriteUserData\n";
print MMP "EPOCHEAPSIZE 1000000 20000000\n\n";

print MMP "USERINCLUDE ";

foreach my $uinc (@userinc)
	{	
	print MMP "$uinc ";
	}
print MMP "\n";
print MMP "SYSTEMINCLUDE \\epoc32\\include "; 

foreach my $sinc (@sysinc)
	{	
	$sinc = substr($sinc, 2) if $sinc =~ /^[A-Za-z]\:/;
	print MMP "$sinc ";
	}
	
print MMP "\n";

my $currentpath;
foreach my $filepath (@files)
	{
	my $curfile;
	$filepath =~ s/\@srcdir\@/$srcdir/;
	if($filepath =~ /(.*?)[\/]+(.*)$/)
		{
		if($currentpath ne $1)
			{
			$currentpath = $1;
			print MMP "SOURCEPATH $currentpath\n";
			}
		$curfile = $2;
		}
	else
		{
		$curfile = $filepath;
		}
		
	print MMP "SOURCE $curfile\n";
	}

print MMP "\n";	

print MMP "SOURCEPATH .\n";
print MMP "START RESOURCE ";
print MMP "$name";
print MMP "_reg.rss\n";
print MMP "TARGETPATH \\private\\10003a3f\\apps\n";
print MMP "END\n\n";


	
foreach my $lib (@libs)
	{	
	print MMP "LIBRARY $lib\n";
	}

print MMP "\n";	

print MMP "STATICLIBRARY sdlmain.lib\n\n";

close(MMP);

my $regname = "$targetdir\\$name" . "_reg.rss";

open(REG, ">$regname") or die "cannot open regfile; $regname\n";

print REG "#include <appinfo.rh>\n\n";


print REG "UID2 KUidAppRegistrationResourceFile\n";
printf REG "UID3 0x%x\n\n", $uid;


print REG "RESOURCE APP_REGISTRATION_INFO\n";
print REG "\t{\n";
print REG "\tapp_file = \"$name\";\n";
print REG "\tgroup_name = \"SDL test\";\n";
print REG "\t}\n";

close REG;

open (BLD, ">>$bldinffile") or die "cannot open bld.inf; $bldinffile: $!\n";
print BLD "$name.mmp\n";
close BLD;

open (PKG, ">>$pkgfile") or die "cannot open pkg file; $pkgfile: $!\n";
print PKG "\"" . $sdk_root . "epoc32\\release\\armv5\\urel\\$name.exe\"-\"!:\\sys\\bin\\$name.exe\"\n";
print PKG "\"" . $sdk_root . "epoc32\\data\\z\\private\\10003a3f\\apps\\$name";
print PKG "_reg.rsc\"-\"!:\\private\\10003a3f\\import\\apps\\$name";
print PKG "_reg.rsc\"\n\n";
close PKG;






