-- not sure if this works as expected. Still confused bc div returns fraction
CREATE OR REPLACE FUNCTION range_set_divide_i8r(
    set1 int8range[], 
    set2 int8range[]
)
RETURNS int8range[] AS $$
BEGIN
    IF array_length(set1, 1) IS NULL THEN
        RETURN set2;
    END IF;
    IF array_length(set2, 1) IS NULL THEN
        RETURN set1;
    END IF;

    RETURN ARRAY(
        SELECT int8range(
            ((LEAST(p1, p2, p3, p4)))::int,
            ((GREATEST(p1, p2, p3, p4)) + 1)::int
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