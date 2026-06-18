-- still not sure what these operators are for
CREATE OR REPLACE FUNCTION uncertain_and(a BOOLEAN, b BOOLEAN)
RETURNS BOOLEAN as $$
BEGIN
    IF a IS FALSE OR b IS FALSE THEN
        RETURN FALSE;
    ELSIF a IS TRUE AND b IS TRUE THEN
        RETURN TRUE;
    ELSE
        RETURN NULL;
    END IF;
END;
$$ LANGUAGE plpgsql;

-- select logical_3vl_and(true, NULL);
-- select logical_3vl_and(TRUE, FALSE);
-- select logical_3vl_and(TRUE, TRUE);

