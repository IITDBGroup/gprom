-- take cartesian product and returns largest range possible
CREATE OR REPLACE FUNCTION range_set_multiply(set1 int4range[], set2 int4range[])
RETURNS int4range[] AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN ARRAY(
        SELECT int4range(LEAST(p1, p2, p3, p4), GREATEST(p1, p2, p3, p4), '[]')
        FROM (
            SELECT 
                lower(i) * lower(j) as p1,
                lower(i) * (upper(j)-1) as p2,
                (upper(i)-1) * lower(j) as p3,
                (upper(i)-1) * (upper(j)-1) as p4
            FROM unnest(set1) i, unnest(set2) j
        ) calc
    );
END;
$$ LANGUAGE plpgsql;



-----------------------------------
-- SELECT (range_set_multiply(
--    ARRAY[int4range(1,4), int4range(3,6), int4range(6,8)],
--    ARRAY[int4range(2,3), int4range(5,9)]
--  ));


--  SELECT (range_set_multiply(
--    ARRAY[int4range(1,4), int4range(6,9)],
--    ARRAY[int4range(2,5), int4range(4,8)]
--  ));

--  SELECT (range_set_multiply(
--    ARRAY[int4range(1,4)],
--    ARRAY[int4range(2,5)]
--  ));
-----------------------------------