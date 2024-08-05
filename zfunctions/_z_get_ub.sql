-- Returns [min, max]
CREATE OR REPLACE FUNCTION _z_get_ub(coefficients REAL[]) RETURNS REAL AS
$$
    DECLARE
        num_error_terms_expected INTEGER;
        z_max REAL;
    BEGIN
        num_error_terms_expected := cardinality(coefficients) - 1;
        z_max := coefficients[1];

        IF mod(num_error_terms_expected, 2) != 0 THEN
            RAISE EXCEPTION 'Error with provided zonotope array size!';
        END IF;

        -- RAISE NOTICE 'TERM 0: %x(e0)', coefficients[1];
        FOR i IN 2..cardinality(coefficients) BY 2 LOOP
            -- RAISE NOTICE 'TERM %: %x(e%)', i, coefficients[i], coefficients[i+1];
            z_max := z_max + coefficients[i];
        END LOOP;

        -- RAISE NOTICE 'UPPER BOUND: %', z_max;
        RETURN z_max;
    END; 

$$
LANGUAGE plpgsql;

-- Examples:
-- SELECT * FROM get_ub(ARRAY [1, 2, 2]);
-- SELECT * FROM get_ub(ARRAY [1, 1, 5]);
