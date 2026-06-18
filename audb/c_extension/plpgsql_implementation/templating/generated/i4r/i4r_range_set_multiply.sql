-- take cartesian product and returns largest range possible
CREATE OR REPLACE FUNCTION range_set_multiply_i4r(
    set1 int4range[], 
    set2 int4range[]
)
RETURNS int4range[] AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN ARRAY(
        SELECT int4range(
            ((LEAST(p1, p2, p3, p4)))::int,
            ((GREATEST(p1, p2, p3, p4)) + 1)::int,
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