-- still not sure what these operators are for
CREATE OR REPLACE FUNCTION uncertain_not(a BOOLEAN)
RETURNS BOOLEAN as $$
BEGIN
    IF a IS TRUE THEN
        RETURN FALSE;
    ELSIF a IS FALSE THEN
        RETURN TRUE;
    ELSE
        RETURN NULL;
    END IF;
END;
$$ LANGUAGE plpgsql;

-- select logical_3vl_not(true, NULL);
-- select logical_3vl_not(TRUE, FALSE);
-- select logical_3vl_not(TRUE, TRUE);

