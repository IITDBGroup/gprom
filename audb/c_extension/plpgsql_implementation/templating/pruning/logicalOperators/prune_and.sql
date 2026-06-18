CREATE OR REPLACE FUNCTION prune_and(set1 int4range[], set2 int4range[])
RETURNS int4range[] as $$
DECLARE 
    rv := '{}'
BEGIN
    select A * B
    into rv
    from unnest set1 as A, unnest set2 as B
    where not isempty(A*B);
    
    return rv;
END;
$$ LANGUAGE plpgsql;