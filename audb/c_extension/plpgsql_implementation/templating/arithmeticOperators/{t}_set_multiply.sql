-- MIGHT NEED TO MODIFY LOGIC FOR NON INT4RANGE
-- take cartesian product and returns largest range possible
CREATE OR REPLACE FUNCTION set_multiply_{{RANGE_TYPE_ABBREV}}(
    set1 {{RANGE_TYPE}}, 
    set2 {{RANGE_TYPE}}
)
RETURNS {{RANGE_TYPE}} AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN (
        SELECT {{RANGE_TYPE}}(LEAST(p1, p2, p3, p4), GREATEST(p1, p2, p3, p4), '[]')
        FROM (
            SELECT 
                lower(set1) * lower(set2) as p1,
                lower(set1) * (upper(set2)-1) as p2,
                (upper(set1)-1) * lower(set2) as p3,
                (upper(set1)-1) * (upper(set2)-1) as p4
        ) calc
    );
END;
$$ LANGUAGE plpgsql;