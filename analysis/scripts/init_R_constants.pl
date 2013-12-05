#!/usr/bin/perl 
use strict; 
use warnings; 
# the purpose of this file is to load the constants.h file into an R lib, setting the values of global variable to there values. 

my $infile = "../../constants.h"; 
my $outfile  = "r-constants-h.R"; 

open my $in, '<', $infile;  
open my $out, '>', $outfile; 

while(defined(my $line = <$in>)){
next if $line !~ m/define/; 
my @def = split ' ', $line; 
print $out $def[1]." <<- ".$def[2]."\n"; 


}

close($in); 
close($out); 


