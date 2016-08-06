#!/usr/bin/perl
use DBI;

my $database='';
my $server  ='';
my $username='www';
my $password='www';

my $dsn = "DBI:mysql:database=$database:host=$server";
my $dbh = DBI->connect($dsn, $username, $password) || die "Can't connect.\n";

$|=1;
my $dsid = "";

print "Content-type: image/png\n\n";

@locs = ("Up Room", "Up Out", "Fish", "Down Room");

$k = 0;
$procid = $$;
@tmpfile = ();

$nowtime = time;

foreach $loc (@locs) {

    $sql = qq[ SELECT id from ds18b20 where location = "$loc" ];

    $sth = $dbh->prepare($sql) || die "DBI error with connect to database.\n";
    $result = $sth->execute  || die "DBI error with execute.\n";

    if ($sth->rows) {
	@row = $sth->fetchrow;
	$dsid = $row[0];
    } 

    $sth->finish;

    next if $dsid == "";

    $sql = qq[ SELECT DATE_FORMAT(from_unixtime(time), '%m/%e %T') as mtime, temperature from home_temp  where location = "$dsid" and row_id > 96370000 order by time desc limit 2880 ];
    
    $sth    = $dbh->prepare($sql) || die "DBI error with connect to database.\n";
    $result = $sth->execute       || die "DBI error with execute.\n";

    $tmpfile[$k] = "/tmp/tmpdata$procid$k.dat";
    open(OUTPUT, ">$tmpfile[$k]") || die ("Can't write data file");
    if ($sth->rows) {
	$nums = $sth->{'NUM_OF_FIELDS'};
	while (@row = $sth->fetchrow) {
	    for ($i=0; $i< $nums ; $i++ ) {
		print OUTPUT "$row[$i]\t";
	    }
	    print OUTPUT "\n";
	}
    }
    $sth->finish;
    close(OUTPUT);
    $k++;
} #foreach

    $imgfile = "/tmp/tmp$procid.png";
    $gnuplot = "|/usr/bin/gnuplot";

# use GNUPLOT to draw the figure
    open(GNUPLOT, $gnuplot);
    print GNUPLOT <<GEND;
	set term pngcairo enhanced size 2048,1536
	set output  "$imgfile"
	set title   "72小时以来的室温"
	set xlabel  "日期 时间"
	set timefmt "%m/%d %H:%M:%S"
	set xdata   time
	set ylabel  "摄氏度 {/Symbol \260}C"
	set grid  
	plot "$tmpfile[0]" using 1:3 title "实测" , "$tmpfile[1]" using 1:3 title "户外" with step, "$tmpfile[2]" using 1:3 title "鱼缸" w step, "$tmpfile[3]" using 1:3 title "客厅" w step
GEND

    close(GNUPLOT);
    unlink($tmpfile[0]);
    unlink($tmpfile[1]);
    unlink($tmpfile[2]);
    unlink($tmpfile[3]);
    
# send the image data to client
    open(INPUT, "<$imgfile");
    while(<INPUT>) {
	print;
    }
    close(INPUT);
    unlink($imgfile);

$dbh->disconnect;
