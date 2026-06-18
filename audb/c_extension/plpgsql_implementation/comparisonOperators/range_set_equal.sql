CREATE OR REPLACE FUNCTION range_set_equal(set1 int4range[], set2 int4range[])
RETURNS boolean AS $$
DECLARE
    norm1 int4range[] := '{}';
    norm2 int4range[] := '{}';
BEGIN
    norm1 := normalize_range(set1);
    norm2 := normalize_range(set2);

    IF array_length(norm1, 1) IS NULL AND array_length(norm2, 1) IS NULL THEN
        RETURN TRUE;
    END IF;
    IF array_length(norm1, 1) IS NULL OR array_length(norm2, 1) IS NULL THEN
        RETURN FALSE;
    END IF;

	-- vacuously true if a=b=c=d
    IF array_length(norm1,1) = 1 AND array_length(norm1,1) = array_length(norm2,1) THEN
        if (lower(norm1[1]) = upper(norm1[1])-1
			AND upper(norm1[1])-1 = lower(norm2[1])
			AND lower(norm2[1]) = upper(norm2[1])-1) THEN
			RETURN TRUE;
		END IF;
    END IF;

    -- any overlap = NULL rv
    FOR i IN 1..array_length(norm1, 1) LOOP
        FOR j IN 1..array_length(norm2, 1) LOOP
            IF norm1[i] && norm2[j] THEN
                RETURN NULL;
            END IF;
        END LOOP;
    END LOOP;

    -- otherwise, disjoint and unequal
    RETURN FALSE;
END;
$$ LANGUAGE plpgsql;