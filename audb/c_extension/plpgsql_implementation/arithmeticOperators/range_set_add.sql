 -- compute Minowski sum, O(n x m)
CREATE OR REPLACE FUNCTION range_set_add(set1 int4range[], set2 int4range[])
RETURNS int4range[] AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN ARRAY(
        SELECT int4range(lower(i) + lower(j), upper(i)-1 + upper(j)-1, '[]')
        FROM unnest(set1) i, unnest(set2) j
    );
END;
$$ LANGUAGE plpgsql;



-----------------------------------
-- SELECT (range_set_add(
--   ARRAY[int4range(1,4), int4range(3,6), int4range(6,8)],
--   ARRAY[int4range(2,3), int4range(5,9)]
-- ));


-- SELECT (range_set_add(
--   ARRAY[int4range(1,4), int4range(6,9)],
--   ARRAY[int4range(2,5), int4range(4,8)]
-- ));

-- SELECT (range_set_add(
--   ARRAY[int4range(1,4)],
--   ARRAY[int4range(2,5)]
-- ));
-----------------------------------