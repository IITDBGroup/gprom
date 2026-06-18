drop extension i4r_audb_extension;
create extension i4r_audb_extension;

-- ===================================
-- Test1: Range Arithmetic
-- ===================================
DROP TABLE IF EXISTS t1_r_arithmetic;
CREATE TEMP TABLE t1_r_arithmetic (
    name text,
    code text,
    actual int4range,
    expected int4range
);
INSERT INTO t1_r_arithmetic VALUES
    -- basic arithmetic
    ('add_basic', 'range_add(''[1,6)'', ''[9,21)'')', range_add('[1,6)', '[9,21)'), '[10,26)'),
    ('subtract_basic', 'range_subtract(''[1,6)'', ''[9,21)'')', range_subtract('[1,6)', '[9,21)'), '[-19,-3)'),
    ('multiply_basic', 'range_multiply(''[1,6)'', ''[9,21)'')', range_multiply('[1,6)', '[9,21)'), '[9,101)'),
    ('divide_basic', 'range_divide(''[3,10)'', ''[3,4)'')', range_divide('[3,10)', '[3,4)'), '[1,4)'),
    
    -- empty cases- add
    ('add_empty_param1', 'range_add(''empty''::int4range, ''[1,5)'')', range_add('empty'::int4range, '[1,5)'), '[1,5)'),
    ('add_empty_param2', 'range_add(''[1,5)'', ''empty''::int4range)', range_add('[1,5)', 'empty'::int4range), '[1,5)'),
    ('add_both_empty', 'range_add(''empty''::int4range, ''empty''::int4range)', range_add('empty'::int4range, 'empty'::int4range), 'empty'::int4range),
    
    -- empty cases- sub
    ('subtract_empty_param1', 'range_subtract(''empty''::int4range, ''[1,5)'')', range_subtract('empty'::int4range, '[1,5)'), 'empty'::int4range),
    ('subtract_empty_param2', 'range_subtract(''[1,5)'', ''empty''::int4range)', range_subtract('[1,5)', 'empty'::int4range), '[1,5)'),
    ('subtract_both_empty', 'range_subtract(''empty''::int4range, ''empty''::int4range)', range_subtract('empty'::int4range, 'empty'::int4range), 'empty'::int4range),
    
    -- empty cases- mult
    ('multiply_empty_param1', 'range_multiply(''empty''::int4range, ''[1,5)'')', range_multiply('empty'::int4range, '[1,5)'), 'empty'::int4range),
    ('multiply_empty_param2', 'range_multiply(''[1,5)'', ''empty''::int4range)', range_multiply('[1,5)', 'empty'::int4range), 'empty'::int4range),
    ('multiply_both_empty', 'range_multiply(''empty''::int4range, ''empty''::int4range)', range_multiply('empty'::int4range, 'empty'::int4range), 'empty'::int4range),
    
    -- empty cases- div
    ('divide_empty_param1', 'range_divide(''empty''::int4range, ''[1,5)'')', range_divide('empty'::int4range, '[1,5)'), 'empty'::int4range),
    ('divide_empty_param2', 'range_divide(''[1,5)'', ''empty''::int4range)', range_divide('[1,5)', 'empty'::int4range), 'empty'::int4range),
    ('divide_both_empty', 'range_divide(''empty''::int4range, ''empty''::int4range)', range_divide('empty'::int4range, 'empty'::int4range), 'empty'::int4range);


-- ===================================
-- Test2: Set Arithmetic
-- ===================================
DROP TABLE IF EXISTS t2_s_arithmetic;
CREATE TEMP TABLE t2_s_arithmetic (
    name text,
    code text,
    actual int4range[],
    expected int4range[]
);

INSERT INTO t2_s_arithmetic VALUES
    -- basic arithmetic
    ('add_basic', 'set_add(array[int4range(1,3), int4range(2,4)], array[int4range(1,3), int4range(2,4)])', set_add(array[int4range(1,3), int4range(2,4)], array[int4range(1,3), int4range(2,4)]), array[int4range(2,5), int4range(3,6), int4range(3,6), int4range(4,7)]),
    ('subtract_basic', 'set_subtract(array[int4range(10,21), int4range(25,51)], array[int4range(5,11), int4range(10,16)])', set_subtract(array[int4range(10,21), int4range(25,51)], array[int4range(5,11), int4range(10,16)]), array[int4range(0,15), int4range(-5,10), int4range(15,45), int4range(10,40)]),
    ('multiply_basic', 'set_multiply(array[int4range(1,3), int4range(2,4)], array[int4range(1,3), int4range(2,4)])', set_multiply(array[int4range(1,3), int4range(2,4)], array[int4range(1,3), int4range(2,4)]), array[int4range(1,5), int4range(2,7), int4range(2,7), int4range(4,10)]),
    ('divide_basic', 'set_divide(array[int4range(10,13), int4range(20,41)], array[int4range(2,3), int4range(4,5)])', set_divide(array[int4range(10,13), int4range(20,41)], array[int4range(2,3), int4range(4,5)]), array[int4range(5,7), int4range(2,4), int4range(10,21), int4range(5,11)]),
    
    -- empty cases- add
    ('set_add_empty_param1', 'set_add(array[]::int4range[], array[int4range(1,5)])', set_add(array[]::int4range[], array[int4range(1,5)]), array[]::int4range[]),
    ('set_add_empty_param2', 'set_add(array[int4range(1,5)], array[]::int4range[])', set_add(array[int4range(1,5)], array[]::int4range[]), array[]::int4range[]),
    ('set_add_both_empty', 'set_add(array[]::int4range[], array[]::int4range[])', set_add(array[]::int4range[], array[]::int4range[]), array[]::int4range[]),
    
    -- empty cases- sub
    ('set_subtract_empty_param1', 'set_subtract(array[]::int4range[], array[int4range(1,5)])', set_subtract(array[]::int4range[], array[int4range(1,5)]), array[]::int4range[]),
    ('set_subtract_empty_param2', 'set_subtract(array[int4range(1,5)], array[]::int4range[])', set_subtract(array[int4range(1,5)], array[]::int4range[]), array[]::int4range[]),
    ('set_subtract_both_empty', 'set_subtract(array[]::int4range[], array[]::int4range[])', set_subtract(array[]::int4range[], array[]::int4range[]), array[]::int4range[]),
    
    -- empty cases- mult
    ('set_multiply_empty_param1', 'set_multiply(array[]::int4range[], array[int4range(1,5)])', set_multiply(array[]::int4range[], array[int4range(1,5)]), array[]::int4range[]),
    ('set_multiply_empty_param2', 'set_multiply(array[int4range(1,5)], array[]::int4range[])', set_multiply(array[int4range(1,5)], array[]::int4range[]), array[]::int4range[]),
    ('set_multiply_both_empty', 'set_multiply(array[]::int4range[], array[]::int4range[])', set_multiply(array[]::int4range[], array[]::int4range[]), array[]::int4range[]),
    
    -- empty cases- div
    ('set_divide_empty_param1', 'set_divide(array[]::int4range[], array[int4range(1,5)])', set_divide(array[]::int4range[], array[int4range(1,5)]), array[]::int4range[]),
    ('set_divide_empty_param2', 'set_divide(array[int4range(1,5)], array[]::int4range[])', set_divide(array[int4range(1,5)], array[]::int4range[]), array[]::int4range[]),
    ('set_divide_both_empty', 'set_divide(array[]::int4range[], array[]::int4range[])', set_divide(array[]::int4range[], array[]::int4range[]), array[]::int4range[]);

-- ===================================
-- Test3: Range Logical Operators
-- ===================================

DROP TABLE IF EXISTS t3_r_logical;
CREATE TEMP TABLE t3_r_logical (
    name text,
    actual bool,
    expected bool,
    code text
);
INSERT INTO t3_r_logical (name, code, actual, expected) VALUES
    -- true results
    ('lt_true', 'range_lt(int4range(1,3), int4range(20,40))', range_lt(int4range(1,3), int4range(20,40)), true),
    ('lte_true', 'range_lte(int4range(1,21), int4range(20,40))', range_lte(int4range(1,21), int4range(20,40)), true),
    ('gt_true', 'range_gt(int4range(20,40), int4range(1,3))', range_gt(int4range(20,40), int4range(1,3)), true),
    ('gte_true', 'range_gte(int4range(2,40), int4range(1,3))', range_gte(int4range(2,40), int4range(1,3)), true),
    
    -- false results
    ('lt_false', 'range_lt(int4range(20,40), int4range(1,3))', range_lt(int4range(20,40), int4range(1,3)), false),
    ('lte_false', 'range_lte(int4range(21,40), int4range(1,21))', range_lte(int4range(21,40), int4range(1,21)), false),
    ('gt_false', 'range_gt(int4range(1,3), int4range(20,40))', range_gt(int4range(1,3), int4range(20,40)), false),
    ('gte_false', 'range_gte(int4range(1,3), int4range(3,40))', range_gte(int4range(1,3), int4range(3,40)), false),
    
    -- null results
    ('lt_overlap', 'range_lt(int4range(20,40), int4range(25,35))', range_lt(int4range(20,40), int4range(25,35)), null),
    ('lte_overlap', 'range_lte(int4range(21,40), int4range(1,26))', range_lte(int4range(21,40), int4range(1,26)), null),
    ('gt_overlap', 'range_gt(int4range(1,33), int4range(20,40))', range_gt(int4range(1,33), int4range(20,40)), null),
    ('gte_overlap', 'range_gte(int4range(1,33), int4range(3,40))', range_gte(int4range(1,33), int4range(3,40)), null),
    ('lt_null', 'range_lt(NULL, int4range(3,40))', range_lt(NULL, int4range(3,40)), null),
    ('lte_null', 'range_lte(NULL, int4range(3,40))', range_lte(NULL, int4range(3,40)), null),
    ('gt_null', 'range_gt(int4range(1,33), NULL)', range_gt(int4range(1,33), NULL), null),
    ('gte_null', 'range_gte(int4range(1,33), NULL)', range_gte(int4range(1,33), NULL), null),
    ('null_null', 'range_gte(NULL, NULL)', range_gte(NULL, NULL), null),
    
    -- empty cases
    ('lt_empty_true', 'range_lt(''empty'', int4range(25,35))', range_lt('empty', int4range(25,35)), true),
    ('lte_empty_true', 'range_lte(''empty'', int4range(1,26))', range_lte('empty', int4range(1,26)), true),
    ('gt_empty_true', 'range_gt(int4range(1,33), ''empty'')', range_gt(int4range(1,33), 'empty'), true),
    ('gte_empty_true', 'range_gte(int4range(1,33), ''empty'')', range_gte(int4range(1,33), 'empty'), true),
    ('gt_empty_false', 'range_gt(''empty'', int4range(25,35))', range_gt('empty', int4range(25,35)), false),
    ('gte_empty_false', 'range_gte(''empty'', int4range(1,26))', range_gte('empty', int4range(1,26)), false),
    ('lt_empty_false', 'range_lt(int4range(1,33), ''empty'')', range_lt(int4range(1,33), 'empty'), false),
    ('lte_empty_false', 'range_lte(int4range(1,33), ''empty'')', range_lte(int4range(1,33), 'empty'), false);

DROP TABLE IF EXISTS t4_s_logical;
CREATE TEMP TABLE t4_s_logical (
    name text,
    actual bool,
    expected bool,
    code text
);

INSERT INTO t4_s_logical (name, code, actual, expected) VALUES
    -- true results
    ('lt_true', 'set_lt(array[int4range(1,3), int4range(5,7)], array[int4range(10,30), int4range(50,70)])', set_lt(array[int4range(1,3), int4range(5,7)], array[int4range(10,30), int4range(50,70)]), true),
    ('lte_true', 'set_lte(array[int4range(1,3), int4range(5,11)], array[int4range(10,30), int4range(50,70)])', set_lte(array[int4range(1,3), int4range(5,11)], array[int4range(10,30), int4range(50,70)]), true),
    ('gt_true', 'set_gt(array[int4range(10,30), int4range(50,70)], array[int4range(1,3), int4range(5,7)])', set_gt(array[int4range(10,30), int4range(50,70)], array[int4range(1,3), int4range(5,7)]), true),
    ('gte_true', 'set_gte(array[int4range(10,30), int4range(50,70)], array[int4range(1,3), int4range(5,11)])', set_gte(array[int4range(10,30), int4range(50,70)], array[int4range(1,3), int4range(5,11)]), true),
    
    -- false results
    ('lt_false', 'set_lt(array[int4range(10,30), int4range(50,70)], array[int4range(1,3), int4range(5,7)])', set_lt(array[int4range(10,30), int4range(50,70)], array[int4range(1,3), int4range(5,7)]), false),
    ('lte_false', 'set_lte(array[int4range(21,40)], array[int4range(1,21)])', set_lte(array[int4range(21,40)], array[int4range(1,21)]), false),
    ('gt_false', 'set_gt(array[int4range(1,3), int4range(5,7)], array[int4range(10,30), int4range(50,70)])', set_gt(array[int4range(1,3), int4range(5,7)], array[int4range(10,30), int4range(50,70)]), false),
    ('gte_false', 'set_gte(array[int4range(1,3)], array[int4range(3,40)])', set_gte(array[int4range(1,3)], array[int4range(3,40)]), false),
    
    -- null results
    ('lt_overlap', 'set_lt(array[int4range(20,40)], array[int4range(25,35)])', set_lt(array[int4range(20,40)], array[int4range(25,35)]), null),
    ('lte_overlap', 'set_lte(array[int4range(21,40)], array[int4range(1,26)])', set_lte(array[int4range(21,40)], array[int4range(1,26)]), null),
    ('gt_overlap', 'set_gt(array[int4range(1,33)], array[int4range(20,40)])', set_gt(array[int4range(1,33)], array[int4range(20,40)]), null),
    ('gte_overlap', 'set_gte(array[int4range(1,33)], array[int4range(3,40)])', set_gte(array[int4range(1,33)], array[int4range(3,40)]), null),
    ('lt_null', 'set_lt(NULL, array[int4range(3,40)])', set_lt(NULL, array[int4range(3,40)]), null),
    ('lte_null', 'set_lte(NULL, array[int4range(3,40)])', set_lte(NULL, array[int4range(3,40)]), null),
    ('gt_null', 'set_gt(array[int4range(1,33)], NULL)', set_gt(array[int4range(1,33)], NULL), null),
    ('gte_null', 'set_gte(array[int4range(1,33)], NULL)', set_gte(array[int4range(1,33)], NULL), null),
    
    -- empty cases
    ('lt_empty_true', 'set_lt(array[]::int4range[], array[int4range(25,35)])', set_lt(array[]::int4range[], array[int4range(25,35)]), true),
    ('lte_empty_true', 'set_lte(array[]::int4range[], array[int4range(1,26)])', set_lte(array[]::int4range[], array[int4range(1,26)]), true),
    ('gt_empty_true', 'set_gt(array[int4range(1,33)], array[]::int4range[])', set_gt(array[int4range(1,33)], array[]::int4range[]), true),
    ('gte_empty_true', 'set_gte(array[int4range(1,33)], array[]::int4range[])', set_gte(array[int4range(1,33)], array[]::int4range[]), true),
    ('gt_empty_false', 'set_gt(array[]::int4range[], array[int4range(25,35)])', set_gt(array[]::int4range[], array[int4range(25,35)]), false),
    ('gte_empty_false', 'set_gte(array[]::int4range[], array[int4range(1,26)])', set_gte(array[]::int4range[], array[int4range(1,26)]), false),
    ('lt_empty_false', 'set_lt(array[int4range(1,33)], array[]::int4range[])', set_lt(array[int4range(1,33)], array[]::int4range[]), false),
    ('lte_empty_false', 'set_lte(array[int4range(1,33)], array[]::int4range[])', set_lte(array[int4range(1,33)], array[]::int4range[]), false);


-- ===================================
-- Test5: Range Aggregates (MIN/MAX/SUM)
-- ===================================
DROP TABLE IF EXISTS t5_r_aggregates;
CREATE TEMP TABLE t5_r_aggregates (
    name text,
    code text,
    actual int4range,
    expected int4range
);

-- Test data for range aggregates
DROP TABLE IF EXISTS t5_r_agg_data;
CREATE TEMP TABLE t5_r_agg_data (
    id int GENERATED ALWAYS AS IDENTITY,
    val int4range,
    mult int4range
);

INSERT INTO t5_r_agg_data (val, mult) VALUES
    (int4range(10,20), int4range(1,2)),  -- certain
    (int4range(5,15), int4range(1,2)),   -- certain (MIN)
    (int4range(30,40), int4range(1,2)),  -- certain (MAX)
    (int4range(1,10), int4range(0,2)),   -- uncertain (ignored)
    (int4range(2,4), int4range(1,2)),    -- certain (for SUM)
    (int4range(3,5), int4range(1,2)),    -- certain (for SUM)
    ('empty'::int4range, int4range(1,2)), -- empty
    (int4range(20,30), 'empty'::int4range), -- empty mult
    ('empty'::int4range, 'empty'::int4range), -- empty, empty
    (NULL, int4range(1,2)),              -- null
    (int4range(25,35), NULL),            -- null mult
    (NULL, NULL),              -- null, null
    (int4range(10,20), int4range(1,5));   -- multi result

INSERT INTO t5_r_aggregates VALUES
    -- MIN tests
    ('min_basic', 'min(combine_range_mult_min(val, mult))', (SELECT min(combine_range_mult_min(val, mult)) FROM t5_r_agg_data), int4range(5,15)),
    ('min_ignores_uncertain', 'min(combine_range_mult_min(val, mult)) WHERE id IN (3,4)', (SELECT min(combine_range_mult_min(val, mult)) FROM t5_r_agg_data WHERE id IN (3,4)), int4range(30,40)),
    ('min_empty_table', 'min(combine_range_mult_min(val, mult)) WHERE 1=0', (SELECT min(combine_range_mult_min(val, mult)) FROM t5_r_agg_data WHERE 1=0), NULL),
    -- MAX tests
    ('max_basic', 'max(combine_range_mult_max(val, mult))', (SELECT max(combine_range_mult_max(val, mult)) FROM t5_r_agg_data), int4range(30,40)),
    ('max_ignores_uncertain', 'max(combine_range_mult_max(val, mult)) WHERE id IN (1,4)', (SELECT max(combine_range_mult_max(val, mult)) FROM t5_r_agg_data WHERE id IN (1,4)), int4range(10,20)),
    ('max_empty_table', 'max(combine_range_mult_max(val, mult)) WHERE 1=0', (SELECT max(combine_range_mult_max(val, mult)) FROM t5_r_agg_data WHERE 1=0), NULL);
    -- SUM tests
    -- ('sum_basic', 'sum(combine_range_mult_sum(val, mult), 500, 100) WHERE id IN (5,6)', (SELECT sum(combine_range_mult_sum(val, mult), 500, 100) FROM t5_r_agg_data WHERE id IN (5,6)), int4range(5,9)),
    -- ('sum_empty_table', 'sum(combine_range_mult_sum(val, mult), 500, 100) WHERE 1=0', (SELECT sum(combine_range_mult_sum(val, mult), 500, 100) FROM t5_r_agg_data WHERE 1=0), NULL);


-- ===================================
-- Test6: Set Aggregates (MIN/MAX)
-- ===================================
DROP TABLE IF EXISTS t6_s_aggregates;
CREATE TEMP TABLE t6_s_aggregates (
    name text,
    code text,
    actual int4range[],
    expected int4range[]
);

-- Test data for set aggregates
DROP TABLE IF EXISTS t6_s_agg_data;
CREATE TEMP TABLE t6_s_agg_data (
    id int GENERATED ALWAYS AS IDENTITY,
    val int4range[],
    mult int4range
);

INSERT INTO t6_s_agg_data (val, mult) VALUES
    (array[int4range(1,3), int4range(6,10), int4range(20,30)], int4range(1,4)),  -- certain
    (array[int4range(2,3), int4range(9,14)], int4range(1,4)),                    -- certain
    (array[int4range(50,60), int4range(70,80)], int4range(0,2)),                 -- uncertain (ignored)
    (array['empty'::int4range], int4range(1,2)),                                      -- empty array
    (array[int4range(15,25)], 'empty'::int4range),                               -- empty mult
    (array['empty'::int4range], 'empty'::int4range),                      -- empty empty
    (NULL, int4range(1,500)),                                                      -- null
    (array[int4range(40,50)], NULL),
    (NULL, NULL);


INSERT INTO t6_s_aggregates VALUES
    -- MIN tests
    ('min_set_basic', 'min(combine_set_mult_min(val, mult)) WHERE id IN (1,2)', (SELECT min(combine_set_mult_min(val, mult)) FROM t6_s_agg_data WHERE id IN (1,2)), array[int4range(1,3), int4range(6,14)]),
    ('min_set_single_row', 'min(combine_set_mult_min(val, mult)) WHERE id = 1', (SELECT min(combine_set_mult_min(val, mult)) FROM t6_s_agg_data WHERE id = 1), array[int4range(1,3), int4range(6,10), int4range(20,30)]),
    ('min_set_ignores_uncertain', 'min(combine_set_mult_min(val, mult)) WHERE id IN (2,3)', (SELECT min(combine_set_mult_min(val, mult)) FROM t6_s_agg_data WHERE id IN (2,3)), array[int4range(2,3), int4range(9,14)]),
    ('min_set_empty_table', 'min(combine_set_mult_min(val, mult)) WHERE 1=0', (SELECT min(combine_set_mult_min(val, mult)) FROM t6_s_agg_data WHERE 1=0), NULL),
    -- MAX tests
    ('max_set_basic', 'max(combine_set_mult_max(val, mult)) WHERE id IN (1,2)', (SELECT max(combine_set_mult_max(val, mult)) FROM t6_s_agg_data WHERE id IN (1,2)), array[int4range(2,3), int4range(6,14), int4range(20,30)]),
    ('max_set_single_row', 'max(combine_set_mult_max(val, mult)) WHERE id = 2', (SELECT max(combine_set_mult_max(val, mult)) FROM t6_s_agg_data WHERE id = 2), array[int4range(2,3), int4range(9,14)]),
    ('max_set_ignores_uncertain', 'max(combine_set_mult_max(val, mult)) WHERE id IN (1,3)', (SELECT max(combine_set_mult_max(val, mult)) FROM t6_s_agg_data WHERE id IN (1,3)), array[int4range(1,3), int4range(6,10), int4range(20,30)]),
    ('max_set_empty_table', 'max(combine_set_mult_max(val, mult)) WHERE 1=0', (SELECT max(combine_set_mult_max(val, mult)) FROM t6_s_agg_data WHERE 1=0), NULL);


-- ===================================
-- Test7: Range Special/edge Cases
-- ===================================
DROP TABLE IF EXISTS t7_r_special;
CREATE TEMP TABLE t7_r_special (
    name text,
    code text,
    actual int4range,
    expected int4range
);

-- INSERT INTO t7_r_special VALUES
-- ;


-- Test data for set aggregates
DROP TABLE IF EXISTS tX_s_agg_data_no_overlap;
CREATE TEMP TABLE tX_s_agg_data_no_overlap (
    id int GENERATED ALWAYS AS IDENTITY,
    agg_val int,
    val int4range[],
    mult int4range
);

INSERT INTO tX_s_agg_data_no_overlap (agg_val, val, mult) VALUES
    (1, array[int4range(1,3), int4range(6,10), int4range(20,30)], int4range(1,4)),
    (1, array[int4range(100,1000), int4range(600,1500)], int4range(1,4)),
    (2, array[int4range(10000,12000), int4range(15000,16200)], int4range(1,2)),
    (3, array[int4range(10000,12000), int4range(15000,16200)], int4range(1,2)),
    (2, array[int4range(100000,200000), int4range(300000,400000)], int4range(1,2));

DROP TABLE IF EXISTS tX_superWide2;
CREATE TEMP TABLE tX_superWide2 (
    id int GENERATED ALWAYS AS IDENTITY,
    val int4range[],
    mult int4range
);
INSERT INTO tX_superWide2 (val, mult) VALUES
    (array[int4range(1,3), int4range(10000,100000)], int4range(1,4)),
    (array[int4range(1,3), int4range(10000,100000)], int4range(1,4)),
    (array[int4range(1,3), int4range(10000,100000)], int4range(1,2)),
    (array[int4range(1,3), int4range(10000,100000)], int4range(1,2));

DROP TABLE IF EXISTS tX_superWide3;
CREATE TEMP TABLE tX_superWide3 (
    id int GENERATED ALWAYS AS IDENTITY,
    val int4range[],
    mult int4range
);
INSERT INTO tX_superWide3 (val, mult) VALUES
    (array[int4range(1,3), int4range(10000,100000), int4range(1000000,100000000)], int4range(1,4)),
    (array[int4range(1,3), int4range(10000,100000), int4range(1000000,100000000)], int4range(1,4)),
    (array[int4range(1,3), int4range(10000,100000), int4range(1000000,100000000)], int4range(1,2)),
    (array[int4range(1,3), int4range(10000,100000), int4range(1000000,100000000)], int4range(1,2));

DROP TABLE IF EXISTS tX_superWideN;
CREATE TEMP TABLE tX_superWideN (
    id int GENERATED ALWAYS AS IDENTITY,
    val int4range[],
    mult int4range
);
INSERT INTO tX_superWideN (val, mult) VALUES
    (array[int4range(1,3), int4range(10,100), int4range(1000,10000), int4range(100000,1000000), int4range(10000000,100000000)], int4range(1,4)),
    (array[int4range(1,3), int4range(10,100), int4range(1000,10000), int4range(100000,1000000), int4range(10000000,100000000)], int4range(1,4)),
    (array[int4range(1,3), int4range(10,100), int4range(1000,10000), int4range(100000,1000000), int4range(10000000,100000000)], int4range(1,2)),
    (array[int4range(1,3), int4range(10,100), int4range(1000,10000), int4range(100000,1000000), int4range(10000000,100000000)], int4range(1,2));

DROP TABLE IF EXISTS tX_superWide;
CREATE TEMP TABLE tX_superWide (
    id int GENERATED ALWAYS AS IDENTITY,
    val int4range[],
    mult int4range
);
INSERT INTO tX_superWide (val, mult) VALUES
    (array[int4range(1,3), int4range(1000,10000), int4range(50000,100000), int4range(100000,1000000), int4range(10000000,100000000)], int4range(1,4)),
    (array[int4range(1,3), int4range(1000,10000), int4range(50000,100000), int4range(100000,1000000), int4range(10000000,100000000)], int4range(1,4)),
    (array[int4range(1,3), int4range(1000,10000), int4range(50000,100000), int4range(100000,1000000), int4range(10000000,100000000)], int4range(1,2)),
    (array[int4range(1,3), int4range(1000,10000), int4range(50000,100000), int4range(100000,1000000), int4range(10000000,100000000)], int4range(1,2));


DROP TABLE IF EXISTS tX_superWide123;
CREATE TEMP TABLE tX_superWide123 (
    id int GENERATED ALWAYS AS IDENTITY,
    val int4range[],
    mult int4range
);
INSERT INTO tX_superWide123 (val, mult) VALUES
    (array[int4range(1,3)], int4range(1,4)),
    (array[int4range(1,3), int4range(10000,100000)], int4range(1,2)),
    (array[int4range(1,3), int4range(10000,100000), int4range(1000000,10000000)], int4range(1,4)),
    (array[int4range(1,3), int4range(10000,100000), int4range(1000000,10000000), int4range(10000000,40000000)], int4range(1,2));

DROP TABLE IF EXISTS tX_superNarrow3;
CREATE TEMP TABLE tX_superNarrow3 (
    id int GENERATED ALWAYS AS IDENTITY,
    val int4range[],
    mult int4range
);
INSERT INTO tX_superNarrow3 (val, mult) VALUES
    (array[int4range(1,66), int4range(100,150), int4range(200,500)], int4range(1,4)),
    (array[int4range(1,66), int4range(100,150), int4range(200,500)], int4range(1,4)),
    (array[int4range(1,66), int4range(100,150), int4range(200,500)], int4range(1,4)),
    (array[int4range(1,66), int4range(100,150), int4range(200,500)], int4range(1,4));


DROP TABLE IF EXISTS tX_prune_basic;
CREATE TEMP TABLE tX_prune_basic (
    id int GENERATED ALWAYS AS IDENTITY,
    a int4range[],
    b int4range[],
    mult int4range
);
INSERT INTO tX_prune_basic (a, b, mult) VALUES
    (array[int4range(1,2), int4range(10,12)], array[int4range(8,14)], int4range(1,4)),
    (array[int4range(20,25), int4range(40,42)], array[int4range(18,100)], int4range(1,4)),
    (array[int4range(1,9), int4range(20,25)], array[int4range(5,15)], int4range(1,4));
    -- (array[int4range(100,200), int4range(10,12)], array[int4range(8,14)], int4range(1,4)),

DROP TABLE IF EXISTS tX_r_prune_basic;
CREATE TEMP TABLE tX_r_prune_basic (
    id int GENERATED ALWAYS AS IDENTITY,
    a int4range,
    b int4range,
    mult int4range
);
INSERT INTO tX_r_prune_basic (a, b, mult) VALUES
    (int4range(1,12), int4range(8,14), int4range(1,4)),
    (int4range(2,25), int4range(18,100), int4range(1,4)),
    (int4range(1,2), int4range(100,200), int4range(1,4)),
    (int4range(100,200), int4range(1,2), int4range(1,4));
    
-- show failures
\echo Test1: Range Arithmetic
SELECT 'Test1: Range Arithmetic' as Test;
SELECT *, 't1_r_arithmetic' as source
FROM t1_r_arithmetic
WHERE actual IS DISTINCT FROM expected;

\echo Test2: Set Arithmetic
SELECT 'Test2: Set Arithmetic' as Test;
SELECT *, 't2_s_arithmetic' as source
FROM t2_s_arithmetic
WHERE actual IS DISTINCT FROM expected;

\echo Test3: Range Logical Operators
SELECT 'Test3: Range Logical Operators' as Test;
SELECT *, 't3_r_logical' as source
FROM t3_r_logical
WHERE actual IS DISTINCT FROM expected;

\echo Test4: Set Logical Operators
SELECT 'Test4: Set Logical Operators' as Test;
SELECT *, 't4_s_logical' as source
FROM t4_s_logical
WHERE actual IS DISTINCT FROM expected;

\echo Test5: Range Aggregates
SELECT 'Test5: Range Aggregates' as Test;
SELECT *, 't5_r_aggregates' as source
FROM t5_r_aggregates
WHERE actual IS DISTINCT FROM expected;

\echo Test6: Set Aggregates
SELECT 'Test6: Set Aggregates' as Test;
SELECT *, 't6_s_aggregates' as source
FROM t6_s_aggregates
WHERE actual IS DISTINCT FROM expected;

-- \echo Test7: Special Aggregates
-- SELECT 'Test7: Set Aggregates' as Test;
-- SELECT *, 't7_s_aggregates' as source
-- FROM t7_s_aggregates
-- WHERE actual IS DISTINCT FROM expected;


-- select 
--     set_normalize(array[int4range(1,4), int4range(5,7), int4range(6,12), int4range(20,32)]) as norm_a,
--     set_normalize(array[int4range(2,5), int4range(5,10), int4range(7,14)]) as norm_b,
--     prune_set_lt(
--         array[int4range(1,4), int4range(5,7), int4range(6,12), int4range(20,32)],
--         array[int4range(2,5), int4range(5,10), int4range(7,14)],
--         false
--     ) as prune_set_lt_f,
--     prune_set_lt(
--         array[int4range(1,4), int4range(5,7), int4range(6,12), int4range(20,32)],
--         array[int4range(2,5), int4range(5,10), int4range(7,14)],
--         true
--     ) as prune_set_lt_t,
--     prune_set_lte(
--         array[int4range(1,4), int4range(5,7), int4range(6,12), int4range(20,32)],
--         array[int4range(2,5), int4range(5,10), int4range(7,14)],
--         false
--     ) as prune_set_lte_f,
--     prune_set_lte(
--         array[int4range(1,4), int4range(5,7), int4range(6,12), int4range(20,32)],
--         array[int4range(2,5), int4range(5,10), int4range(7,14)],
--         true
--     ) as prune_set_lte_t,
--     prune_set_gt(
--         array[int4range(1,4), int4range(5,7), int4range(6,12), int4range(20,32)],
--         array[int4range(2,5), int4range(5,10), int4range(7,14)],
--         false
--     ) as prune_set_gt_f,
--     prune_set_gt(
--         array[int4range(1,4), int4range(5,7), int4range(6,12), int4range(20,32)],
--         array[int4range(2,5), int4range(5,10), int4range(7,14)],
--         true
--     ) as prune_set_gt_t,
--     prune_set_gte(
--         array[int4range(1,4), int4range(5,7), int4range(6,12), int4range(20,32)],
--         array[int4range(2,5), int4range(5,10), int4range(7,14)],
--         false
--     ) as prune_set_gte_f,
--     prune_set_gte(
--         array[int4range(1,4), int4range(5,7), int4range(6,12), int4range(20,32)],
--         array[int4range(2,5), int4range(5,10), int4range(7,14)],
--         true
--     ) as prune_set_gt_t;


-- SELECT sum(combine_set_mult_sum(val, mult), 500, 100)
-- FROM my_table
-- WHERE set_lt(val, array[int4range(10,20)]) = true;


--  select 
--     sum(combine_set_mult_sum(val, mult), 500, 100),
--     num_range(sum(combine_set_mult_sum(val, mult), 500, 100))
--  from t_s_iv_ni_nir_0073f377a2;

--  -- compare unpruned vs pruned
-- SELECT 
--     array_length(no_prune, 1)  as intervals_no_prune,
--     set_coverage(no_prune)     as coverage_no_prune,
--     array_length(pruned, 1)    as intervals_pruned,
--     set_coverage(pruned)       as coverage_pruned,
-- FROM (
--     SELECT 
--         sum(combine_set_mult_sum(val, mult), 500, 100) as no_prune,
--         sum(combine_set_mult_sum(
--             prune_set_lt(val, array[int4range(1000, 1200)], false), 
--         mult), 500, 100) as pruned
--     FROM t_s_iv_n_9ece6d82ac
-- ) sub;