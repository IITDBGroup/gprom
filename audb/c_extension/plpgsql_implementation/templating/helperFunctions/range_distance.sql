-------------
CREATE OR REPLACE FUNCTION range_distance(r1 int4range, r2 int4range)
RETURNS integer AS $$
BEGIN
    IF r1 @> r2 OR r1 <@ r2 THEN
        RETURN 0;
    ELSIF upper(r1) < lower(r2) THEN
        RETURN lower(r2) - upper(r1);
    ELSE
        RETURN lower(r1) - upper(r2);
    END IF;
END;
$$ LANGUAGE plpgsql;