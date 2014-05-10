#!/usr/bin/perl
# records 8-channel data received over serial port and logs to OLS format

$baud = 6000000;
$framerate = $baud/10;      # 8 bits + start/stop bits
$loops = $framerate/1024;   # sample 1s @1K bytes/buffer

# 1st argument must be port name
$port = $ARGV[0];

# open port in read/write mode
open(UART, "+<$port") or die "$port ", $!;
binmode UART;
open(LOG, ">log8.ols") or die $!;
print LOG ";Rate: $framerate\n";
print LOG ";Channels: 8\n";

# drain UART buffer of noise
$count = read (UART, $buf, 1024);
my ($buf, $count);
my $samplenum = 1;
while ($loops-- > 0){
    $count = read (UART, $buf, 1024);
    my @samples = unpack("C*",$buf);
    foreach $sample(@samples) {
        printf LOG "%02x\@%d\n", $sample, $samplenum++;
    }
}
close(UART) or die $!;
print "\n";
