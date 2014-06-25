use warnings;

use Scalar::Util qw(looks_like_number);

#    ex: perl dumpWriteLatency.pl log w160
# usage: perl dumpWriteLatency.pl $dir $pattern
opendir(DIR, $ARGV[0]) or die $!;
my @file_list= sort { $a cmp $b } readdir(DIR); 
$pattern = $ARGV[1] || '.';
while(my $file = shift  @file_list){
    if($file ne "." and $file ne ".."){
        if($file =~ m/$pattern/){
            $total_cycle = 0;
            for(my $b = 0; $b <=7; ++$b){
                $cycle = `sed -n \'/-Bursting Time (Bank $b)/p\' $ARGV[0]/$file | awk \'{print \$7}\' | tail -1`; 
                if(looks_like_number($cycle)){
                    $total_cycle += $cycle;
                }
            }
            if($total_cycle ne 0){
                print $file.":\t".($total_cycle/8)."\n";
            }
        } 
    }
}
closedir(DIR);

