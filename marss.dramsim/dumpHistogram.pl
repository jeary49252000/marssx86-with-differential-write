#!/user/bin/perl

use warnings;

sub parser{
    my $file = shift;
    my $prefix = shift;
    my $suffix = shift;
    local *FILE;
    open FILE, "<$file" or die $!;
    
    my $target = "";
    my $buf = "";
    while(<FILE>){
        $buf = $_;
        if($buf=~m/($prefix.*?$suffix)/){
            $buf = substr($1, length($prefix),
                length($1)-length($prefix)-length($suffix));
            $target = $buf;
        }
    }
    close FILE;
    return $target;
}

# usage: perl dumpHistogram.pl $dir
#    ex: perl dumpHistogram.pl log
opendir(DIR, $ARGV[0]) or die $!;
my @file_list= sort { $a cmp $b } readdir(DIR); 
while(my $file = shift  @file_list){
    if($file ne "." and $file ne ".."){
        local *FILE;
        open FILE, "+>$ARGV[0]/$file.csv" or die $!;
        #print parser($ARGV[0]."/".$file,"Differential wirte - histogram of changed bits:\t","\n");
        print FILE parser($ARGV[0]."/".$file,"Differential wirte - histogram of changed bits:\t","\n");
        close FILE;
    }
}
closedir(DIR);

