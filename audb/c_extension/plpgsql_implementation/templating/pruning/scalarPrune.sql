CREATE OR REPLACE FUNCTION scalarPrune(vals int4range[], x int, op text)
RETURNS int4range[] AS $$
DECLARE
    result int4range[] := '{}';
    curr int4range;
    new_lower int;
    new_upper int;
BEGIN
    FOREACH curr IN ARRAY vals LOOP
        CASE op
            WHEN '<' THEN
                IF lower(curr) < x THEN
                    new_upper := LEAST(upper(curr), x);
                    result := result || int4range(lower(curr), new_upper, '[)');
                END IF;
            WHEN '<=' THEN
                IF lower(curr) <= x THEN
                    new_upper := LEAST(upper(curr), x + 1);
                    result := result || int4range(lower(curr), new_upper, '[)');
                END IF;
            WHEN '>' THEN
                IF upper(curr) > x THEN
                    new_lower := GREATEST(lower(curr), x + 1);
                    result := result || int4range(new_lower, upper(curr), '[)');
                END IF;
            WHEN '>=' THEN
                IF upper(curr) >= x THEN
                    new_lower := GREATEST(lower(curr), x);
                    result := result || int4range(new_lower, upper(curr), '[)');
                END IF;
            WHEN '=' THEN
                IF lower(curr) <= x AND upper(curr) > x THEN
                    result := result || int4range(x, x + 1, '[)');
                END IF;
            ELSE
                RAISE EXCEPTION 'Unsupported operator: %', op;
        END CASE;
    END LOOP;

    RETURN result;
END;
$$ LANGUAGE plpgsql;