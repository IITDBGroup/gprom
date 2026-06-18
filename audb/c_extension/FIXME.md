
audb_test=# select c_range_set_add(array[int4range(1,3)], array[int4range(1,3), null]);
 c_range_set_add
-----------------
 {NULL,NULL}
(1 row)

Should not be this




NULL vs Empty arithmetic:

audb_test=# select c_range_set_add(array[]::int4range[], array[int4range(1,5)]);
 c_range_set_add
-----------------
 {}
(1 row)

audb_test=# select c_range_set_add(null, array[int4range(1,5)]);
 c_range_set_add
-----------------
 {"[1,5)"}
(1 row)







---------Broken sum-----------

select set_add(array[int4range(1,3), int4range(6,10), int4range(20,30)], array[int4range(2,3), int4range(9,14)]);

 {"[3,5)","[10,16)","[8,12)","[15,23)","[22,32)","[29,43)"}


                       set_reduce_size
------------------------------------------------------------
 {"[3,5)","[10,16)","[8,12)","[15,23)","[22,32)","[29,43)"}
(1 row)

audb_test=# select set_reduce_size(set_add(array[int4range(1,3), int4range(6,10), int4range(20,30)], array[int4range(2,3), int4range(9,14)]), 5);
                 set_reduce_size
--------------------------------------------------
 {"[3,5)","[8,16)","[15,23)","[22,32)","[29,43)"}
(1 row)

audb_test=# select set_reduce_size(set_add(array[int4range(1,3), int4range(6,10), int4range(20,30)], array[int4range(2,3), int4range(9,14)]), 4);
            set_reduce_size
----------------------------------------
 {"[3,5)","[8,23)","[22,32)","[29,43)"}
(1 row)

audb_test=# select set_reduce_size(set_add(array[int4range(1,3), int4range(6,10), int4range(20,30)], array[int4range(2,3), int4range(9,14)]), 3);
       set_reduce_size
------------------------------
 {"[3,5)","[8,32)","[29,43)"}
(1 row)

audb_test=# select set_reduce_size(set_add(array[int4range(1,3), int4range(6,10), int4range(20,30)], array[int4range(2,3), int4range(9,14)]), 2);
  set_reduce_size
--------------------
 {"[3,5)","[8,43)"}
(1 row)

audb_test=# select set_reduce_size(set_add(array[int4range(1,3), int4range(6,10), int4range(20,30)], array[int4range(2,3), int4range(9,14)]), 1);
 set_reduce_size
-----------------
 {"[3,43)"}
(1 row)