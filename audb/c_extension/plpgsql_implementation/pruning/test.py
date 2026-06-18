import pandas as pd

df = pd.DataFrame({"A": [(1, 3), (2, 2)],
                   "B": [(4, 4), (5, 10)]
                })

dfneg = pd.DataFrame({"A": [[(-10, -7), (-3, 0), (1, 3)], (2, 2)],
                   "B": [(4, 4), (5, 10)]
                })

# assume left side is a int4range or (), and right side is a constant
def prune_less_than(left_side, right_side):
    low, high = left_side
    if low > right_side:
        return None
    return (low, min(high, right_side))


# assume left side is a int4range or (), and right side is a constant
def prune_greater_than(left_side, right_side):
    low, high = left_side
    if high < right_side:
        return None
    return (low, max(high, right_side))




if __name__ == '__main__':
    print("Original Data: \n", dfneg)
    # df["A-le-pruned"] = df["A"].apply(lambda x: prune_greater_than(x, 2))
    # print(df["A-le-pruned"])

    # for row in df['A']:
    #     trow = prune_greater_than(row, 2)

    

    

# SELECT a + 3 FROM r WHERE a = b;

# -> (a + 3)
# SELECT range_set_add(a,'{[3,3]}'::int4range[])) FROM r
# SELECT range_set_add(a,'{[3,3]}'::int4range[])) FROM r WHERE range_set_equal(a,b) is not false;
