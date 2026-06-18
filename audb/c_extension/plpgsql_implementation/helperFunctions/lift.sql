CREATE OR REPLACE FUNCTION lift(x integer)
RETURNS int4range as $$
BEGIN
    RETURN int4range(x,x,'[]');
END
$$ LANGUAGE plpgsql;