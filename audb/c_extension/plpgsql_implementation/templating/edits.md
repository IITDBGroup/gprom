might need to make changes to certain files to account for different range types

examples:
(range_)set_divide.sql
using floor and ceil for range types vs least and greatest for non commutative operators

SELECT int4range(LEAST(p1, p2, p3, p4), GREATEST(p1, p2, p3, p4), '[]')
