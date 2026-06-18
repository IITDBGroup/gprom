CREATE OR REPLACE FUNCTION set_subtract_i4r(
    set1 int4range,
    set2 int4range
)
RETURNS int4range AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;
    
    RETURN int4range(
        lower(set1) - (upper(set2)-1),
        (upper(set1)-1) - lower(set2),
		'[]'
    );
END;