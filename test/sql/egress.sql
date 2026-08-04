SELECT pgch_version() IS NOT NULL AS loaded;
SHOW pg_clickhouse.egress_interface;

CREATE ROLE regress_egress_user;
SET ROLE regress_egress_user;
SET pg_clickhouse.egress_interface = 'lo';
RESET ROLE;
DROP ROLE regress_egress_user;
