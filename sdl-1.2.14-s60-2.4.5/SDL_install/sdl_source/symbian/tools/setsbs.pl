#http://developer.symbian.org/wiki/Symbian_Build_System_v2/KitSetUp
use strict;

my $raptorfile = $ARGV[0] . "epoc32\\sbs_config";

$raptorfile =~ s/\\/\//g;

mkdir($raptorfile) unless(-d $raptorfile);


$raptorfile .= "/os_properties.xml";

die "$raptorfile already exits\n" if (-e $raptorfile);

open(FILE, ">$raptorfile") or die "Bad: Cannot open $raptorfile $!\n";

print FILE	"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
print FILE 	"<build xmlns=\"http://symbian.com/xml/build\"  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://symbian.com/xml/build ../../schema/build/2_0.xsd\">\n";
print FILE "\n\n";
print FILE "<!-- variants and alias go here -->\n";
print FILE "\n\n";


print FILE "\t<var name=\"root.changes\">\n";
print FILE "\t\t<set name=\'POSTLINKER_SUPPORTS_WDP\' value=\'\'/>\n";
print FILE "\t\t<set name=\'SUPPORTS_STDCPP_NEWLIB\' value=\'\'/>\n";
print FILE "\t\t<set name=\'RVCT_PRE_INCLUDE\' value=\'\$(EPOCINCLUDE)/rvct2_2/rvct2_2.h\'/>\n";
print FILE "\t\t<set name=\'VARIANT_HRH\' value=\'\$(EPOCINCLUDE)/variant/Symbian_OS.hrh\'/>\n";
print FILE "\t</var>\n";
print FILE "\n\n";
print FILE "</build>\n";

close(FILE);	
	
