 -- compute Minowski sum, O(n x m)
CREATE OR REPLACE FUNCTION range_set_add_i8r(
    set1 int8range[], 
    set2 int8range[]
)
RETURNS int8range[] AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN ARRAY(
        SELECT int8range(lower(i) + lower(j), upper(i)-1 + upper(j)-1, '[]')
        FROM unnest(set1) i, unnest(set2) j
    );
END;
$$ LANGUAGE plpgsql;
