CREATE OR REPLACE FUNCTION set_add_{{RANGE_TYPE_ABBREV}}(
    set1 {{RANGE_TYPE}}, 
    set2 {{RANGE_TYPE}}
)
RETURNS {{RANGE_TYPE}} AS $$
BEGIN
    IF set1 IS NULL OR set2 IS NULL THEN
        RETURN NULL;
    END IF;

    RETURN 
        {{RANGE_TYPE}}(
        lower(set1) + lower(set2), 
        upper(set1)-1 + upper(set2)-1,          --subtract 2 bc exclusive upper bound
        '[]'
    );
END;
$$ LANGUAGE plpgsql;