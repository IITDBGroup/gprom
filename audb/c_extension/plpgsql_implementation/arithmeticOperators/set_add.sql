-- reference
-- running into issue with range bounds and using upper/lower which 
-- return the actual int at the bound, and not the last int within the bound
-- select lower(int4range(3,4)), upper(int4range(3,4));                         // 3|4
-- select lower(int4range(6,8, '[)')), upper(int4range(6,8,'[)'));              // 6|8

CREATE OR REPLACE FUNCTION set_add(set1 int4range, set2 int4range)
RETURNS int4range AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN 
        int4range(lower(set1) + lower(set2), 
        upper(set1)-1 + upper(set2)-1,          --subtract 2 bc exclusive upper bound
        '[]'
    );
END;
$$ LANGUAGE plpgsql;