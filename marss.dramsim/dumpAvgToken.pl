use warnings;

use Scalar::Util qw(looks_like_number);

#    ex: perl dumpWriteLatency.pl log w160
# usage: perl dumpWriteLatency.pl $dir $pattern
opendir(DIR, $ARGV[0]) or die $!;
my @file_list= sort { $a cmp $b } readdir(DIR); 
$pattern = $ARGV[1] || '.';
while(my $file = shift  @file_list){
    if($file ne "." and $file ne ".."){
        #print $file.":\t".parseIPC($ARGV[0]."/".$file)."\n";
        # ptlsim log
        if($file =~ m/$pattern/){
            $set_count = `sed -n \'/Differential wirte - # set bits:/p\' $ARGV[0]/$file | awk \'{print \$7}\' | tail -1`; 
            $reset_count = `sed -n \'/Differential wirte - # reset bits:/p\' $ARGV[0]/$file | awk \'{print \$7}\' | tail -1`; 
            $write_count = `sed -n \'/Marss total write count/p\' $ARGV[0]/$file | awk \'{print \$6}\' | tail -1`; 
            if(looks_like_number($set_count) and looks_like_number($reset_count) and looks_like_number($write_count)){
                print $file.":\t".(($set_count+$reset_count)/$write_count)."\n";
            }
        } 
        # dramsim log
        #print $file.":\t".`sed -n \'/final average read latency/p\' $ARGV[0]/$file | awk \'{print \$6}\' | tail -1`."\n" if($file =~ m/$pattern/);
    }
}
closedir(DIR);

