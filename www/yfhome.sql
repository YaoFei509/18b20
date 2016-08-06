use yfhome;

DROP TABLE IF EXISTS home_temp;
create table home_temp (
		row_id 		INT AUTO_INCREMENT NOT NULL PRIMARY KEY,
		time            INT,
		pkg_time	TIME,
		temperature     FLOAT,
		location	char(32)
		);

CREATE INDEX  row_idx on home_temp(row_id);
CREATE INDEX  time on home_temp(time);

use yfhome;

drop table  if exists ds18b20;

create table ds18b20 (
	id char(24) not null PRIMARY KEY,
	location  char(32)
);

INSERT INTO ds18b20 values ("28:80:6A:23:05:00:00:95","Up Out");
INSERT INTO ds18b20 values ("28:E1:81:23:05:00:00:CB","Up Room");
INSERT INTO ds18b20 values ("28:FF:90:0D:64:14:03:6C","Down Room");
INSERT INTO ds18b20 values ("28:FF:4E:37:64:14:03:C3","Fish");
