CREATE OR REPLACE FUNCTION set_divide_i4r(
    set1 int4range,
    set2 int4range
)
RETURNS int4range AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN (
        SELECT int4range(
            LEAST(p1, p2, p3, p4), 
            GREATEST(p1, p2, p3, p4),
            '[]'
        )
        FROM (
            SELECT 
                lower(set1) / lower(set2) as p1,
                lower(set1) / (upper(set2)-1) as p2,
                (upper(set1)-1) / lower(set2) as p3,
                (upper(set1)-1) / (upper(set2)-1) as p4
        ) calc
        -- ignore bounds that cross 0 because this is not possible to divide by.
        -- ignore [0,1) == [0,0]
        WHERE NOT(lower(set2) <= 0 AND upper(set2) > 0) 
          AND NOT (lower(set2) = 0 AND upper(set2) = 1)
    );
END;
$$ LANGUAGE plpgsql;