-- not sure if this works as expected. Still confused bc div returns fraction
CREATE OR REPLACE FUNCTION range_set_divide_{{RANGE_TYPE_ABBREV}}(
    set1 {{RANGE_TYPE}}[], 
    set2 {{RANGE_TYPE}}[]
)
RETURNS {{RANGE_TYPE}}[] AS $$
BEGIN
    IF array_length(set1, 1) IS NULL THEN
        RETURN set2;
    END IF;
    IF array_length(set2, 1) IS NULL THEN
        RETURN set1;
    END IF;

    RETURN ARRAY(
        SELECT {{RANGE_TYPE}}(
            {{LOWER_EXPR}},
            {{UPPER_EXPR}}
        )
        FROM (
            SELECT 
                lower(i) / lower(j) as p1,
                lower(i) / (upper(j)-1) as p2,
                (upper(i)-1) / lower(j) as p3,
                (upper(i)-1) / (upper(j)-1) as p4
            FROM unnest(set1) i, unnest(set2) j
            -- ignore bounds that cross 0 because this is not possible to divide by.
			-- ignore [0,1) == [0,0]
            WHERE NOT(lower(j) <= 0 AND upper(j) > 0) 
			  AND NOT (lower(j) = 0 AND upper(j) = 1)
        ) calc
    );
END;
$$ LANGUAGE plpgsql;