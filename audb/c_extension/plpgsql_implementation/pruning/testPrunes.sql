CREATE OR REPLACE FUNCTION scalarComparisonFilter(ranges int4range[], x int, op text)
RETURNS int4range[] AS $$
DECLARE
    result int4range[] := '{}';
    r int4range;
    new_lower int;
    new_upper int;
BEGIN
    FOREACH r IN ARRAY ranges LOOP
        CASE op

            WHEN '<' THEN
                IF lower(r) < x THEN
                    new_upper := LEAST(upper(r), x);
                    result := result || int4range(lower(r), new_upper, '[)');
                END IF;
            WHEN '<=' THEN
                IF lower(r) <= x THEN
                    new_upper := LEAST(upper(r), x + 1);
                    result := result || int4range(lower(r), new_upper, '[)');
                END IF;
            WHEN '>' THEN
                IF upper(r) > x THEN
                    new_lower := GREATEST(lower(r), x + 1);
                    result := result || int4range(new_lower, upper(r), '[)');
                END IF;
            WHEN '>=' THEN
                IF upper(r) >= x THEN
                    new_lower := GREATEST(lower(r), x);
                    result := result || int4range(new_lower, upper(r), '[)');
                END IF;
            WHEN '=' THEN
                IF lower(r) <= x AND upper(r) > x THEN
                    result := result || int4range(x, x + 1, '[)');
                END IF;
            ELSE
                RAISE EXCEPTION 'Unsupported operator: %', op;
        END CASE;
    END LOOP;

    RETURN result;
END;
$$ LANGUAGE plpgsql;


create table tempMult (
	A int4range[],
	B int4range[],
	mult int4range
); 

--drop table if exists tempMult;
--insert into temp (A, B) values (array[int4range(1,9)], array[int4range(2,8)]), (array[int4range(0,2)], array[int4range(4,5)]);
--insert into tempMult (A, B, mult) values (array[int4range(1,9)], array[int4range(2,8)], int4range(0,1)), (array[int4range(0,2)], array[int4range(4,5)], int4range(1,3));

insert into tempMult (A, B, mult) 
values (array[int4range(2,7), int4range(9,12)], array[int4range(2,8), int4range(9,11)], int4range(0,1)), 
	   (array[int4range(0,2), int4range(5,11)], array[int4range(3,6), int4range(12,20)], int4range(0,1)),
	   (array[int4range(0,5), int4range(1,11)], array[int4range(2,16), int4range(12,20)], int4range(0,1));
		


select *
from tempMult
where range_less_than(a, array[int4range(3,4)]) is not false

select normalize_vals(a), normalize_vals(b)
from tempMult


select prune_gt(t.resA, 2), t.resb as b
from (
	select prune_lt(a, 3) as resA, b as resB
	from temp
	where range_set_equal(a, array[int4range(3,4)]) is not false
) t



select prune_gt(t.resA, 2), t.resb as b
from (
	select prune_lt(a, 3) as resA, b as resB
	from temp
	where range_set_equal(a, array[int4range(3,4)]) is not false
) t


select scalarComparisonFilter(A, 3, '=') 
from temp
where range_set_equal(a, array[int4range(3,4)]) is not false


CREATE OR REPLACE FUNCTION prune_lt(set1 int4range[], k int)
RETURNS int4range[] as $$
BEGIN
    RETURN (
		SELECT array_agg( int4range(lower(i), least(upper(i)-1, k), '[]') )
        FROM unnest(set1) i
        WHERE lower(i) < k
	);
END;
$$ LANGUAGE plpgsql;




select * from tempmult t  where range_set_equal(a,b) is not false