-- Metrics helpers for USET pruning experiments (ic_sum / R ratio).
-- Loaded during docker postgres init; safe to re-run.

CREATE OR REPLACE FUNCTION int_to_range_set(x int4)
RETURNS int4range[]
LANGUAGE SQL IMMUTABLE STRICT AS $$
  SELECT ARRAY[lift_scalar(x)]::int4range[];
$$;

CREATE OR REPLACE FUNCTION normalize_vals(s int4range[])
RETURNS int4range[]
LANGUAGE SQL IMMUTABLE STRICT AS $$
  SELECT set_normalize(s);
$$;

CREATE OR REPLACE FUNCTION int_count_range(r int4range)
RETURNS bigint
LANGUAGE SQL IMMUTABLE STRICT AS $$
  SELECT (upper(r) - lower(r))::bigint;
$$;

CREATE OR REPLACE FUNCTION int_count_set(s int4range[])
RETURNS bigint
LANGUAGE SQL IMMUTABLE STRICT AS $$
  SELECT COALESCE(SUM(upper(x) - lower(x)), 0)::bigint FROM unnest(s) AS t(x);
$$;

CREATE OR REPLACE FUNCTION ic_sum_result(s int4range[])
RETURNS bigint
LANGUAGE SQL IMMUTABLE STRICT AS $$
  SELECT int_count_set(s);
$$;
