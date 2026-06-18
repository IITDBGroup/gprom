-- might need work for non int4range types regarding the return statement
CREATE OR REPLACE FUNCTION range_set_subtract_{{RANGE_TYPE_ABBREV}}(
    set1 {{RANGE_TYPE}}[], 
    set2 {{RANGE_TYPE}}[]
)
RETURNS {{RANGE_TYPE}}[] AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;
    
    RETURN ARRAY(
        SELECT {{RANGE_TYPE}}(l, u, '[]')
        FROM (
            SELECT (lower(i) - (upper(j)-1)) as l, 
                   ((upper(i)-1) - lower(j)) as u
            FROM unnest(set1) i, unnest(set2) j
        ) calc
        WHERE l <= u
    );
END;
$$ LANGUAGE plpgsql;