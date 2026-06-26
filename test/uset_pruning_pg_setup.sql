-- Optional setup for scalar USET pruning tests (table r / r_interval).
CREATE EXTENSION IF NOT EXISTS i4r_audb_extension;

\i prune_effect_metrics.sql

DROP TABLE IF EXISTS r;
CREATE TABLE r (a int, b int, u_r int);
INSERT INTO r (a, b, u_r) VALUES
  (3, 5, 1), (3, 10, 2), (3, 100, 3), (3, 7, 10),
  (3, 3, 4), (3, 2, 5), (1, 5, 6), (4, 9, 7),
  (2, 8, 8), (-1, 10, 9), (0, 3, 11), (3, 4, 12);

DROP TABLE IF EXISTS r_interval;
CREATE TABLE r_interval (a int4range[], b int4range[], u_r int);
INSERT INTO r_interval VALUES
  (ARRAY['[3,4)'::int4range], ARRAY['[5,6)'::int4range], 1),
  (ARRAY['[3,4)'::int4range], ARRAY['[10,11)'::int4range], 2);
