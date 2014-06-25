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
            $read_count = `sed -n \'/Marss total read count/p\' $ARGV[0]/$file | awk \'{print \$6}\' | tail -1`; 
            $write_count = `sed -n \'/Marss total write count/p\' $ARGV[0]/$file | awk \'{print \$6}\' | tail -1`; 
            $total_inst_count = `sed -n \'/Stopped after/p\' $ARGV[0]/$file | awk \'{print \$5}\' | tail -1`; 
            if(looks_like_number($total_inst_count)){
            print $file.":\t".($read_count*1000/$total_inst_count)."\t(read)\n";
            print $file.":\t".($write_count*1000/$total_inst_count)."\t(write)\n";
        }
        } 
        # dramsim log
        #print $file.":\t".`sed -n \'/final average read latency/p\' $ARGV[0]/$file | awk \'{print \$6}\' | tail -1`."\n" if($file =~ m/$pattern/);
    }
}
closedir(DIR);

