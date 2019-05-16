#!/usr/bin/perl
use Net::Telnet;
use File::Basename;

$numArgs = $#ARGV + 1;
if($numArgs != 1){
	   die( "Usage ./do_flash.pl [main.bin] \n");
}

print "$ARGV[0]\n";
print "$0\n";

$file = $ARGV[0];
$ip = "127.0.0.1";
$port = 4444;
$telnet = new Net::Telnet (
	Port => $port,
	Timeout=>20,
	Errmode=>'die',
	Prompt =>'/>/');

$telnet->open($ip);

print $telnet->cmd('reset halt');
print $telnet->cmd('flash probe 0');
print $telnet->cmd('stm32f4x mass_erase 0');
print $telnet->cmd('flash write_bank 0 '.$file.' 0');
print $telnet->cmd('reset run');
print $telnet->cmd('exit');

print "\n";
