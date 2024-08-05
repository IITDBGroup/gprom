-- Returns ...
CREATE OR REPLACE FUNCTION _z_op_add(lhs REAL[], rhs REAL[]) RETURNS REAL[] AS
$$
    DECLARE
        res REAL[];
    BEGIN
        res[1] := lhs[1] + rhs[1];

        IF mod(cardinality(lhs) - 1, 2) != 0 OR mod(cardinality(rhs) - 1, 2) != 0  THEN
            RAISE EXCEPTION 'Error with a provided zonotope array size!';
        END IF;
          
        FOR i IN 2..cardinality(lhs) BY 2 LOOP
            -- RAISE NOTICE 'res[%]', lhs[i+1];

            IF res[lhs[i+1]*2] IS NULL THEN
                res[lhs[i+1]*2] := 0;
            END IF;

            res[lhs[i+1]*2] := res[lhs[i+1]*2] + lhs[i];
            res[lhs[i+1]*2 + 1] := lhs[i+1];
        END LOOP;

        FOR i IN 2..cardinality(rhs) BY 2 LOOP
            -- RAISE NOTICE 'res[%]', rhs[i+1];

            IF res[rhs[i+1]*2] IS NULL THEN
                res[rhs[i+1]*2] := 0;
            END IF;

            res[rhs[i+1]*2] := res[rhs[i+1]*2] + rhs[i];
            res[rhs[i+1]*2 + 1] := rhs[i+1];
        END LOOP;

        res := array_remove(res, NULL);
        RETURN res;
    END; 

$$
LANGUAGE plpgsql;

-- Examples:
-- SELECT z_add(ARRAY[1,1,1], ARRAY[1,2,2]);
-- SELECT z_add(ARRAY[1,2,1,5,3], ARRAY[1,1,1,2,2,7,7]);
