#!/usr/bin/perl

# 1st argument must be port name
$port = $ARGV[0];

# open port in read/write mode
open(UART, "+<$port") or die "$port ", $!;
binmode UART;
open(LOG, ">log.ols") or die $!;
print LOG ";Rate: 600000\n";
print LOG ";Channels: 8\n";
my ($buf, $count);
$count = read (UART, $buf, 256);
print ".";
my @samples = unpack("C*",$buf);
my $samplenum = 1;
foreach $sample(@samples) {
    printf LOG "%02x\@%d\n", $sample, $samplenum++;
}
close(UART) or die $!;
print "\n";
