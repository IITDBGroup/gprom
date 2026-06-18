 -- compute Minowski sum, O(n x m)
CREATE OR REPLACE FUNCTION range_set_add_i4r(
    set1 int4range[], 
    set2 int4range[]
)
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
