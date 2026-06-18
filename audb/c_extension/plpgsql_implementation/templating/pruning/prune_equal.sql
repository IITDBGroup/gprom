-- must compare cross product of ranges O(n^2)
CREATE OR REPLACE FUNCTION prune_eq(set1 int4range[], set2 int4range[], direction bool)
RETURNS int4range[] AS $$
DECLARE
  rv int4range[] := '{}';
  curr1 int4range;
  curr2 int4range;
BEGIN
  FOREACH curr1 IN ARRAY normalize_vals(set1) LOOP
    FOREACH curr2 IN ARRAY normalize_vals(set2) LOOP
      IF curr1 && curr2 THEN
        rv := rv || (curr1 * curr2);
      END IF;
    END LOOP;
  END LOOP;

  RETURN rv;
END;
$$ LANGUAGE plpgsql;



-- (1,3), (7,10) = (1,2) , (7,8)
-- (1,1), 7,7