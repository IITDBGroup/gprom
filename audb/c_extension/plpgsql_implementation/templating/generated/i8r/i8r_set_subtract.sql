CREATE OR REPLACE FUNCTION set_subtract_i8r(
    set1 int8range,
    set2 int8range
)
RETURNS int8range AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;
    
    RETURN int8range(
        lower(set1) - (upper(set2)-1),
        (upper(set1)-1) - lower(set2),
		'[]'
    );
END;