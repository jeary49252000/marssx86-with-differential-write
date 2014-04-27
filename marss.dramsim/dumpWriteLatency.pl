#!/user/bin/perl

use warnings;
# usage: perl dumpWriteLatency.pl $dir $pattern
#    ex: perl dumpWriteLatency.pl log w160
opendir(DIR, $ARGV[0]) or die $!;
my @file_list= sort { $a cmp $b } readdir(DIR); 
$pattern = $ARGV[1] || '.';
while(my $file = shift  @file_list){
    if($file ne "." and $file ne ".."){
        #print $file.":\t".parseIPC($ARGV[0]."/".$file)."\n";
        # ptlsim log
        print $file.":\t".`sed -n \'/current average write latency/p\' $ARGV[0]/$file | awk \'{print \$6}\' | tail -1`."\n" if($file =~ m/$pattern/);
        # dramsim log
        #print $file.":\t".`sed -n \'/final average write latency/p\' $ARGV[0]/$file | awk \'{print \$6}\' | tail -1`."\n" if($file =~ m/$pattern/);
    }
}
closedir(DIR);

