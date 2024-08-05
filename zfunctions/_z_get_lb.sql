-- Returns [min, max]
CREATE OR REPLACE FUNCTION _z_get_lb(coefficients REAL[]) RETURNS REAL AS
$$
    DECLARE
        num_error_terms_expected INTEGER;
        z_min REAL;
    BEGIN
        num_error_terms_expected := cardinality(coefficients) - 1;
        z_min := coefficients[1];

        IF mod(num_error_terms_expected, 2) != 0 THEN
            RAISE EXCEPTION 'Error with provided zonotope array size!';
        END IF;

        -- RAISE NOTICE 'TERM 0: %x(e0)', coefficients[1];
        FOR i IN 2..cardinality(coefficients) BY 2 LOOP
            -- RAISE NOTICE 'TERM %: %x(e%)', i, coefficients[i], coefficients[i+1];
            z_min := z_min - coefficients[i];
        END LOOP;

        -- RAISE NOTICE 'LOWER BOUND: %', z_min;
        RETURN z_min;
    END; 

$$
LANGUAGE plpgsql;

-- Examples:
-- SELECT * FROM get_lb(ARRAY [1, 2, 2]);
-- SELECT * FROM get_lb(ARRAY [1, 1, 5]);
