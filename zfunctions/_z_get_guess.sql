-- Returns a trivial guess for the given zonotope
CREATE OR REPLACE FUNCTION _z_get_guess(coefficients REAL[]) RETURNS REAL AS
$$
    BEGIN
        IF (cardinality(coefficients) < 1) THEN
          RAISE EXCEPTION 'zonotope malformated! Size is less than 1';
        END IF;

        -- Returns the trivial guess, that being the center
        -- TODO: This should be a consideration for improvements in the future if possible
        RETURN coefficients[1];
    END; 

$$
LANGUAGE plpgsql;

-- Examples:
-- SELECT * FROM _z_get_guess(ARRAY [1, 2, 1, 3, 2, 1, 5]);
