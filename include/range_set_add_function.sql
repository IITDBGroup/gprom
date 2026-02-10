-- Range Set Add Function for GProM Custom Operator
-- This function handles addition of two-dimensional integer arrays
-- Usage: range_set_add(int[][], int[][]) returns int[][]

CREATE OR REPLACE FUNCTION range_set_add(l int[][], r int[][]) 
RETURNS int[][] AS $$
DECLARE
    result int[][];
    i int;
    j int;
BEGIN
    -- Initialize result array
    result := '{}';

    -- double for loop to iterate through all elements of l and r
    FOR i IN array_lower(l, 1)..array_upper(l, 1) LOOP
        FOR j IN array_lower(r, 1)..array_upper(r, 1) LOOP
            result := result || ARRAY[ARRAY[
                    l[i][1] + r[j][1],
                    l[i][2] + r[j][2]
                ]];
        END LOOP;
    END LOOP;

    RETURN result;
END;
$$ LANGUAGE plpgsql;

-- Example usage and test cases
-- Test the function with sample data
/*
SELECT range_set_add(
    ARRAY[ARRAY[1,2], ARRAY[3,4]], 
    ARRAY[ARRAY[5,6], ARRAY[7,8]]
) as result;

-- Expected result: {{6,8},{8,10},{8,10},{10,12}}
*/ 