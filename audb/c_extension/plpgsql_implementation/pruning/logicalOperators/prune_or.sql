CREATE OR REPLACE FUNCTION prune_or(set1 int4range[], set2 int4range[])
RETURNS int4range[] as $$
DECLARE 
    rv int4range[] := '{}';
BEGIN
    rv := set1 || set2;
    return rv;
END;
$$ LANGUAGE plpgsql;