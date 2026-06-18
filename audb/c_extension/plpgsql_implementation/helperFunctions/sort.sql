CREATE OR REPLACE FUNCTION sort(ranges int4range[])
RETURNS int4range[] AS $$
DECLARE
    sorted_ranges int4range[];
BEGIN
    SELECT array_agg(vals ORDER BY lower(vals), upper(vals))
    INTO sorted_ranges
    FROM unnest(ranges) AS vals;
    
    RETURN sorted_ranges;
END;
$$ LANGUAGE plpgsql;