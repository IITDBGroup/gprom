 -- compute Minowski sum, O(n x m)
CREATE OR REPLACE FUNCTION range_set_add_{{RANGE_TYPE_ABBREV}}(
    set1 {{RANGE_TYPE}}[], 
    set2 {{RANGE_TYPE}}[]
)
RETURNS {{RANGE_TYPE}}[] AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN ARRAY(
        SELECT {{RANGE_TYPE}}(lower(i) + lower(j), upper(i)-1 + upper(j)-1, '[]')
        FROM unnest(set1) i, unnest(set2) j
    );
END;
$$ LANGUAGE plpgsql;
