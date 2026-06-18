-- take cartesian product and returns largest range possible
CREATE OR REPLACE FUNCTION range_set_multiply_{{RANGE_TYPE_ABBREV}}(
    set1 {{RANGE_TYPE}}[], 
    set2 {{RANGE_TYPE}}[]
)
RETURNS {{RANGE_TYPE}}[] AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN ARRAY(
        SELECT {{RANGE_TYPE}}(
            {{LOWER_EXPR}},
            {{UPPER_EXPR}},
            '[]'
        )
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