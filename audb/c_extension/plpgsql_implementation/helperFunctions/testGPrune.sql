--------------------------
--select *
--from r
--where a+2 = a*2
--------------------------

select *
from borisExample
where range_set_equal(
	range_set_multiply(a, array[lift(2)]),
	range_set_add(a, array[lift(2)])
) is not false;
--a                 |b          |mult |
--------------------+-----------+-----+
--{"[1,3)","[4,35)"}|{"[2,10)"} |[1,2)|
--{"[1,3)"}         |{"[10,20)"}|[1,2)|
--{"[1,5)"}         |{"[1,3)"}  |[1,2)|

select
	prune_and(
		range_set_divide(
			prune_eq(
				range_set_multiply(a, array[lift(2)]), 
				range_set_add(a, array[lift(2)]),
				False
			), array[lift(2)]
		),
		range_set_subtract(
			prune_eq(
				range_set_multiply(a, array[lift(2)]), 
				range_set_add(a, array[lift(2)]),
				False
			), array[lift(2)]
		)
	) as prune_a, b, mult
--select *
from borisExample
where range_set_equal(
	range_set_multiply(a, array[lift(2)]),
	range_set_add(a, array[lift(2)])
) is not false;



select range_set_divide(
			prune_eq(
				range_set_multiply(a, array[lift(2)]), 
				range_set_add(a, array[lift(2)]),
				False
			), array[lift(2)]
		) as adiv,
		range_set_subtract(
			prune_eq(
				range_set_multiply(a, array[lift(2)]), 
				range_set_add(a, array[lift(2)]),
				False
			), array[lift(2)]
		) as asub,
		prune_eq(
				range_set_multiply(a, array[lift(2)]), 
				range_set_add(a, array[lift(2)]),
				False
			) as aeqa,
		prune_and(
			range_set_divide(
				prune_eq(
					range_set_multiply(a, array[lift(2)]), 
					range_set_add(a, array[lift(2)]),
					False
				), array[lift(2)]
			),
			range_set_subtract(
				prune_eq(
					range_set_multiply(a, array[lift(2)]), 
					range_set_add(a, array[lift(2)]),
					False
				), array[lift(2)]
			)
		) as intersec
from borisexample
where range_set_equal(
	range_set_multiply(a, array[lift(2)]),
	range_set_add(a, array[lift(2)])
) is not false;
