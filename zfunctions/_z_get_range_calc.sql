-- Returns [min, max, calculated]
CREATE OR REPLACE FUNCTION _z_get_range_calc(coefficients REAL[], error_terms REAL[]) RETURNS REAL[] AS
$$
    DECLARE
        num_error_terms_expected INTEGER;
        num_error_terms_provided INTEGER;
        z_min REAL;
        z_max REAL;
        calculated REAL;
        e REAL;
    BEGIN
        num_error_terms_expected := cardinality(coefficients) - 1;
        num_error_terms_provided := cardinality(error_terms);
        z_min := coefficients[1];
        z_max := coefficients[1];
        calculated := coefficients[1];

        RAISE NOTICE 'TOTAL ERROR TERMS EXPECTED: %', mod(num_error_terms_expected, 2);
        IF mod(num_error_terms_expected, 2) != 0 THEN
            RAISE EXCEPTION 'Error with provided zonotope array size!';
        END IF;

        FOREACH e IN ARRAY error_terms LOOP
            IF abs(e) > 1 THEN
                RAISE EXCEPTION 'All error terms must be in the range [-1, 1]!';
            END IF;
        END LOOP;

        -- RAISE NOTICE 'TERM 0: %x(e0)', coefficients[1];
        FOR i IN 2..cardinality(coefficients) BY 2 LOOP
            IF (coefficients[i] > num_error_terms_provided) THEN
                RAISE EXCEPTION 'Could not find associated error term for e%', coefficients[i+1];
            END IF;

            -- RAISE NOTICE 'TERM %: %x(e%)', i, coefficients[i], coefficients[i+1];
            z_min := z_min - coefficients[i];
            z_max := z_max + coefficients[i];
            calculated := calculated + (coefficients[i] * error_terms[coefficients[i + 1]]);
        END LOOP;

        RETURN ARRAY [z_min, z_max, calculated];
    END; 

$$
LANGUAGE plpgsql;

-- Examples:
-- SELECT * FROM get_range_calc(ARRAY [2,3,1,2,2], ARRAY [1, 1, 1]);
