CREATE OR REPLACE FUNCTION prune_lt(set1 int4range[], set2 int4range[], direction bool)
RETURNS int4range[] AS $$
DECLARE
  rv int4range[] := '{}';
  curr int4range;
  norm1 int4range[];
  norm2 int4range[];
  minA int;
  maxB int;
  pruned int4range;
BEGIN
  norm1 := normalize_vals(set1);
  norm2 := normalize_vals(set2);

  -- A < B; A = A n [-inf, max(B)-1]
  IF direction = FALSE THEN 
    -- not sure what to do on CASE when A < NULL
		IF array_length(norm2,1) is NULL THEN
      RETURN norm1;
    END IF;

    SELECT max(upper(ranges)) INTO maxB
    FROM unnest(norm2) as ranges
    WHERE upper(ranges) is not NULL;

	  -- nothing to prune above
    IF maxB IS NULL THEN
      RETURN rv;  
    END IF;

    FOREACH curr IN ARRAY norm1 LOOP
      pruned := curr * int4range(NULL, maxB);
      IF NOT isempty(pruned) THEN
        rv := rv || pruned;  
      END IF;
    END LOOP;  
  -- A < B; B = B n [min(A)+1, inf]
  ELSE
    -- not sure what to do on CASE when A < NULL
		IF array_length(norm2,1) is NULL THEN
      RETURN norm1;
    END IF;

    SELECT min(lower(ranges)) INTO minA
    FROM unnest(norm1) as ranges
    WHERE lower(ranges) is not NULL;

    -- nothing to prune above
    IF minA IS NULL THEN
      RETURN rv;  
    END IF;

    FOREACH curr IN ARRAY norm2 LOOP
      pruned := curr * int4range(minA+1, NULL);
      IF NOT isempty(pruned) THEN
        rv := rv || pruned;  
      END IF;
    END LOOP;  
  END IF;
  RETURN rv;
END;
$$ LANGUAGE plpgsql;