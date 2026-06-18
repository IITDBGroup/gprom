CREATE OR REPLACE FUNCTION range_set_subtract(set1 int4range[], set2 int4range[])
RETURNS int4range[] AS $$
BEGIN
    -- want to return null, bc its a valid range value; in every world, this val is null
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;
    
    RETURN ARRAY(
        -- return inclusive range to account for result = [x,x] == 'empty'. instead convert to [x, x+1)
        SELECT int4range(l, u, '[]')
        FROM (
            SELECT (lower(i) - (upper(j)-1)) as l, 
                   ((upper(i)-1) - lower(j)) as u
            FROM unnest(set1) i, unnest(set2) j
        ) calc
        WHERE l <= u
    );
END;
$$ LANGUAGE plpgsql;



-----------------------------------
-- SELECT (range_set_subtract(
--    ARRAY[int4range(1,4), int4range(3,6), int4range(6,8)],
--    ARRAY[int4range(2,3), int4range(5,9)]
--  ));


--  SELECT (range_set_subtract(
--    ARRAY[int4range(1,4), int4range(6,9)],
--    ARRAY[int4range(2,5), int4range(4,8)]
--  ));

--  SELECT (range_set_subtract(
--    ARRAY[int4range(1,4)],
--    ARRAY[int4range(2,5)]
--  ));
-----------------------------------