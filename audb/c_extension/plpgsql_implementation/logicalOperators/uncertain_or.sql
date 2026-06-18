-- still not sure what these operators are for
CREATE OR REPLACE FUNCTION uncertain_or(a BOOLEAN, b BOOLEAN)
RETURNS BOOLEAN as $$
BEGIN
    IF a IS TRUE OR b IS TRUE THEN
        RETURN TRUE;
    ELSIF a IS FALSE AND b IS FALSE THEN
        RETURN FALSE;
    ELSE
        RETURN NULL;    
    END IF;
END;
$$ LANGUAGE plpgsql;

-- select logical_3vl_or(true, NULL);
-- select logical_3vl_or(TRUE, FALSE);
-- select logical_3vl_or(NULL, FALSE);
-- select logical_3vl_or(FALSE, FALSE);
-- select logical_3vl_or(TRUE, TRUE);

