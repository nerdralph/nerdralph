#!/usr/bin/perl
# records 2-channel data received over serial port and logs to OLS format

$baud = 6000000;
$framerate = $baud/10;      # 8 bits + start/stop bits
$samplerate = $framerate*4; # 4 2-bit samples/frame
$loops = $framerate/1024;   # sample 1s @1K bytes/buffer

# 1st argument must be port name
$port = $ARGV[0];

my ($buf, $count);
# open port in read/write mode
open(UART, "+<$port") or die "$port ", $!;
binmode UART;
# drain UART buffer of noise
$count = read (UART, $buf, 1024);
open(TMPFILE, ">logger.tmp") or die "logger.tmp ", $!;
binmode TMPFILE;
my $i = $loops;
while ($i-- > 0){
    $count = read (UART, $buf, 1024);
    print TMPFILE $buf;
}
close(TMPFILE) or die $!;
close(UART) or die $!;

open(TMPFILE, "<logger.tmp") or die "logger.tmp ", $!;

open(LOG, ">log2.ols") or die $!;
print LOG ";Rate: $samplerate\n";
print LOG ";Channels: 2\n";

my $samplenum = 1;
while ($loops-- > 0){
    $count = read (TMPFILE, $buf, 1024);
    my @samples = unpack("C*",$buf);
    foreach $sample(@samples) {
        printf LOG "%01x\@%d\n", ($sample & 192) >> 6, $samplenum++;
        printf LOG "%01x\@%d\n", ($sample & 48) >> 4, $samplenum++;
        printf LOG "%01x\@%d\n", ($sample & 12) >> 2, $samplenum++;
        printf LOG "%01x\@%d\n", ($sample & 3), $samplenum++;
    }
}
close(TMPFILE) or die $!;
print "\n";
