#!/usr/bin/perl
use Time::Local;
use DBI;
use Device::SerialPort;

my $port = Device::SerialPort->new("$ARGV[0]") || die ("Can't open serial port\n");

my $database='yfhome';
my $server  ='yfhomeserver.local'; 
my $username='www';
my $password='www';

my $dsn = "DBI:mysql:database=$database:host=$server";
my $dbh = DBI->connect($dsn, $username, $password) || die "Can't connect.\n";

$t = time;
$port->baudrate(9600);
$port->write("T");

sleep 1; #wait for serialport!

($cnt, $_) = $port->read(255);
chop;
chop;

@line = split(/\t/);


$sql = qq[ INSERT home_temp VALUES  (0,$t,0, $line[1], "$line[0]") ];

print "$sql\n";

$sth    = $dbh->prepare($sql) || die "DBI error with connect to database.\n";
$result = $sth->execute       || die "DBI error with execute.\n";

$dbh->disconnect;
undef $port;
