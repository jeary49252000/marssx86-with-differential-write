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

# usage: perl dumpSetResetInfo.pl $dir
#    ex: perl dumpSetResetInfo.pl log
opendir(DIR, $ARGV[0]) or die $!;
my @file_list= sort { $a cmp $b } readdir(DIR); 
while(my $file = shift  @file_list){
    if($file ne "." and $file ne ".."){
        $w =  parser($ARGV[0]."/".$file,"Differential wirte - # write requests:\t","\n");
        $success_rate = sprintf("%.3f",1 - parser($ARGV[0]."/".$file,"Differential wirte - # write with page fault / # total write:\t","\n"));
        $set = parser($ARGV[0]."/".$file,"Differential wirte - # set bits:\t","\n");
        $reset = parser($ARGV[0]."/".$file,"Differential wirte - # reset bits:\t","\n");
        $set_ratio = sprintf("%.3f",$set/($set+$reset));
        print STDOUT "$file:\t$w\t[$success_rate]\t( $set / $reset = $set_ratio)\n";
    }
}
closedir(DIR);

