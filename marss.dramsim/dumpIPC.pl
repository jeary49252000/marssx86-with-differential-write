#!/user/bin/perl

use warnings;

sub parseIPC{
    my $file = shift;
    open FILE, "<$file" or die $!;
    
    my $cycle = "";
    my $buf = "";
    while(<FILE>){
        $buf = $_;
        if($buf=~m/(insns\/cyc: .*?\))/){
            $buf = substr($1, length("insns\/cyc: "),
                length($1)-length("insns\/cyc: ")-length("\("));
            $cycle = sprintf("%.03lf",$buf);
        }
    }
    close FILE;
    return $cycle;
}

# usage: perl dumpIPC.pl $dir
#    ex: perl dumpIPC.pl log
opendir(DIR, $ARGV[0]) or die $!;
my @file_list= sort { $a cmp $b } readdir(DIR); 
while(my $file = shift  @file_list){
    if($file ne "." and $file ne ".."){
        print $file.":\t".parseIPC($ARGV[0]."/".$file)."\n";
    }
}
closedir(DIR);

