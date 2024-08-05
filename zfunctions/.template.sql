-- Returns ...
CREATE OR REPLACE FUNCTION func(parameter NUMERIC) RETURNS NUMERIC AS
$$
    DECLARE
        var NUMERIC;
    BEGIN
        RETURN 0;
    END; 

$$
LANGUAGE plpgsql;

-- Examples:
-- 
