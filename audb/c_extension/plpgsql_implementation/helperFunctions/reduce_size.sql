CREATE OR REPLACE FUNCTION reduce_size(vals int4range[], numRangesKeep int)
RETURNS int4range[] AS $$
-- RETURNS text[] AS $$
DECLARE
    sortedRanges int4range[];
	currNumRanges int := NULL;
    distance int := NULL;
	index int := NULL;
    prev int4range;
    curr int4range;

BEGIN
	currNumRanges := array_length(vals, 1);
	IF vals IS NULL OR currNumRanges IS NULL OR numRangesKeep < 1 THEN
        RETURN '{}';
	ELSIF currNumRanges <= numRangesKeep THEN
		RETURN vals;
    END IF;

    sortedRanges := sort(vals);
    currNumRanges := array_length(sortedRanges, 1);
	
	WHILE currNumRanges > numRangesKeep LOOP
		prev := sortedRanges[1];
	    FOR i IN 2..currNumRanges LOOP
			curr := sortedRanges[i];
			
			-- get the smallest gap O(n) each time
			IF distance is NULL or abs(range_distance(prev, curr)) < distance THEN
				distance := range_distance(prev, curr);	
				index := i-1;
			END IF;
			prev := curr; 
	    END LOOP;
		
		sortedRanges[index] := int4range(
								least(lower(sortedRanges[index]), lower(sortedRanges[index+1])), 
								greatest(upper(sortedRanges[index]), upper(sortedRanges[index+1])) 
							);			
		
		FOR i in index+1..currNumRanges-1 LOOP
			sortedRanges[i] := sortedRanges[i+1];
		END LOOP;
		
		  := currNumRanges-1;
		distance := NULL;	
	END LOOP;
    
    RETURN sortedRanges;
END;
$$ LANGUAGE plpgsql;