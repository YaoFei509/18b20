#!/usr/bin/perl
use Time::Local;
use DBI;

my $database='yfhome';
my $server  ='yfhomeserver.local'; 
my $username='www';
my $password='www';

my $dsn = "DBI:mysql:database=$database:host=$server";
my $dbh = DBI->connect($dsn, $username, $password) || die "Can't connect.\n";

while(<>) {
	chop;
	@line = split('\t');
	next if $line[2] == "";

	$t = time;
	if ( $t % 60 == 0 )  {  
	$sql = qq[ INSERT home_temp VALUES  (0,$t,"$line[0]",$line[1], "$line[2]" ) ];
	$sth    = $dbh->prepare($sql) || die "DBI error with connect to database.\n";
	$result = $sth->execute       || die "DBI error with execute.\n";
	}
    }
$dbh->disconnect;
