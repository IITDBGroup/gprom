# Three-Dataset Design Summary: sales_rset / flights_rset / hospital_outlier_rset
## 1. Summary of the Three Databases

| Dataset | Core scenario | Source of uncertainty | Scale |
|--------|----------|----------------|------|
| **flights_rset** | Primary-key repair (PK repair) | Multiple candidate rows per flight; multi-segment time ranges | 128 rows |
| **hospital_outlier_rset** | Outliers | Singleton intervals vs. wide outlier/missing intervals | 972 rows |
| **sales_rset** | Multi-interval envelope aggregation | Multiple sales/cost candidates per row | 1000 rows |


---

## 2. flights_rset — Primary-Key Repair

### 2.1 Design Highlights

- **Source**: Flight schedule data; the same `flight_num` may correspond to multiple candidate rows (distinguished by `u_r`), representing primary-key conflicts.
- **Uncertain columns**: `sched_dep`, `act_dep`, `sched_arr`, `act_arr` (`int4range[]`).
- **Certain columns**: `flight_num`, `airline`, `origin`, `dest`, `u_r`.
- **Key pattern**: `sched_dep` is a singleton interval; `act_dep` / `act_arr` are merged multi-segment ranges, including `[0,1441)` for full-day uncertainty.
- **Data file**: `gprom/flights_rset.csv`

### 2.2 Queries and Results

#### SUM — Departure delay SUM (R≈0.43)

```sql
-- NP
USET (SELECT SUM(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep);

-- P
USET WITH PRUNING (SELECT SUM(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep);
```

| Mode | Result | ic\_sum | R |
|------|------|---------|---|
| NP | `{"[77661,110325)"}` | 32664 | — |
| P | `{"[64361,78397)"}` | 14036 | **0.43** |

#### SUM — Triple-conjunct strong pruning (R≈0.23)

```sql
USET WITH PRUNING (
  SELECT SUM(act_dep) FROM flights_rset IS UADB
  WHERE sched_dep < act_dep AND act_dep > 900 AND sched_arr < act_arr
);
```

| Mode | Result | ic\_sum | R |
|------|------|---------|---|
| NP | `{"[37314,64529)"}` | 27215 | — |
| P | `{"[29930,36160)"}` | 6230 | **0.23** |

#### MIN /MAX

```sql
USET WITH PRUNING (
  SELECT MIN(act_dep), MAX(act_dep) FROM flights_rset IS UADB WHERE sched_dep < act_dep
);
```

| Column | Result (P) |
|----|-----------|
| min(act_dep) | `{"[361,415)"}` |
| max(act_dep) | `{"[1266,1441)"}` |

#### GRP — GROUP BY airline

```sql
USET WITH PRUNING (
  SELECT airline, SUM(act_dep) FROM flights_rset IS UADB
  WHERE sched_dep < act_dep GROUP BY airline
);
```

| airline | sum(act_dep) |
|---------|--------------|
| UA | `{"[27834,32845)"}` |
| AA | `{"[36527,45553)"}` |

#### CMP1 — Four columns + dual time comparison (projection, excerpt)

```sql
USET WITH PRUNING (
  SELECT sched_dep, act_dep, sched_arr, act_arr FROM flights_rset IS UADB
  WHERE sched_dep < act_dep AND sched_arr < act_arr
);
```

| sched_dep | act_dep | sched_arr | act_arr |
|-----------|---------|-----------|---------|
| `{"[1015,1016)"}` | `{"[1016,1049)",...}` | `{"[1205,1206)"}` | NULL |
| `{"[419,420)"}` | `{"[420,459)",...}` | `{"[634,635)"}` | NULL |
| `{"[1200,1201)"}` | NULL | `{"[1310,1311)"}` | `{"[1311,1321)",...}` |

#### CMP — Point interval + delay

```sql
USET WITH PRUNING (
  SELECT sched_dep, act_dep FROM flights_rset IS UADB
  WHERE sched_dep = 1185 AND sched_dep < act_dep
);
```

| sched_dep | act_dep |
|-----------|---------|
| `{"[1185,1186)"}` | `{"[1186,1214)",...,"[1318,1319)"}` |

####Cross-column OR + SUM(act_dep) (R≈0.43)

```sql
USET WITH PRUNING (
  SELECT SUM(act_dep) FROM flights_rset IS UADB
  WHERE sched_dep < act_dep OR sched_arr < act_arr
);
```

| Result (P) | Note |
|-----------|------|
| `{"[64361,78397)"}` | Only the `sched_dep<act_dep` branch prunes `act_dep` |

####  Same-column dual disjunct `prune_set_or` (R≈0.22)

```sql
USET WITH PRUNING (
  SELECT SUM(act_dep) FROM flights_rset IS UADB
  WHERE sched_dep < act_dep OR act_dep > 1000
);
```

| Result (P) | R |
|-----------|---|
| `{"[16610,23805)"}` | **0.22** |

#### E-F-OR4 — OR + AND nesting

```sql
USET WITH PRUNING (
  SELECT SUM(act_dep) FROM flights_rset IS UADB
  WHERE sched_dep < act_dep OR (sched_arr < act_arr AND act_arr > 1200)
);
```

| Result (P) |
|-----------|
| `{"[64361,78397)"}` |

#### OR-GRP — GROUP BY airline + OR

```sql
USET WITH PRUNING (
  SELECT airline, SUM(act_dep) FROM flights_rset IS UADB
  WHERE sched_dep < act_dep OR act_dep > 1000 GROUP BY airline
);
```

| airline | sum(act_dep) |
|---------|--------------|
| UA | `{"[3405,5045)"}` |
| AA | `{"[13205,18761)"}` |



---

## 3. hospital_outlier_rset — Outliers

### 3.1 Design Highlights

- **Outlier labeling**:
  - `is_outlier=FALSE` → `Score=[v,v+1)` (certain)
  - `is_outlier=TRUE/NULL` → `Score=[0,101)` (uncertain)
- **Uncertain columns**: `Score`, `ZipCode`; **Certain columns**: `ProviderNumber`, `HospitalName`, `is_outlier`, `u_r`.
- **Row mix**: ~720 normal + 68 outliers + 184 missing.
- **Data file**: `gprom/hospital_outlier_rset.csv`

### 3.2 Queries and Results

#### SUM — Score window SUM (recommended, R≈0.08)

```sql
-- NP
USET (SELECT SUM(Score) FROM hospital_outlier_rset IS UADB WHERE Score > 90 AND Score < 100);

-- P
USET WITH PRUNING (SELECT SUM(Score) FROM hospital_outlier_rset IS UADB WHERE Score > 90 AND Score < 100);
```

| Mode | Result | ic\_sum | R |
|------|------|---------|---|
| NP | `{"[54087,79288)"}` | 25201 | — |
| P | `{"[50969,52986)"}` | 2017 | **0.08** |

#### SUM — Cross-column comparison (R≈0.09)

```sql
USET WITH PRUNING (
  SELECT SUM(Score) FROM hospital_outlier_rset IS UADB
  WHERE Score > 90 AND Score < ZipCode
);
```

| Mode | Result | ic\_sum | R |
|------|------|---------|---|
| NP | `{"[54087,79288)"}` | 25201 | — |
| P | `{"[74769,77038)"}` | 2269 | **0.09** |

#### SUM — Point interval (negative case)

```sql
USET WITH PRUNING (
  SELECT SUM(Score) FROM hospital_outlier_rset IS UADB WHERE Score = 90
);
```

| Result (P) | R |
|-----------|---|
| `{"[24930,24931)"}` | 1.0 |

#### MIN / MAX

```sql
USET WITH PRUNING (
  SELECT MIN(Score), MAX(Score) FROM hospital_outlier_rset IS UADB
  WHERE Score > 90 AND Score < 100
);
```

| Column | Result (P) |
|----|-----------|
| min(score) | `{"[91,92)"}` |
| max(score) | `{"[99,100)"}` |

#### CMP — Projection Score + ZipCode (first 5 rows)

```sql
USET WITH PRUNING (
  SELECT Score, ZipCode FROM hospital_outlier_rset IS UADB
  WHERE Score > 90 AND Score < ZipCode
);
```

| score | zipcode |
|-------|---------|
| `{"[91,101)"}` | `{"[35233,35234)"}` | (×5 rows)

#### CMP — Equality + cross-column (first 5 rows)

```sql
USET WITH PRUNING (
  SELECT Score, ZipCode FROM hospital_outlier_rset IS UADB
  WHERE Score = 90 AND Score < ZipCode
);
```

| score | zipcode |
|-------|---------|
| `{"[90,91)"}` | `{"[35233,35234)"}` | (×5 rows)

#### OR + AND nesting (R≈0.09)

```sql
USET WITH PRUNING (
  SELECT SUM(Score) FROM hospital_outlier_rset IS UADB
  WHERE Score > 90 AND Score < 100 OR Score > 95
);
```

| Result (P) | R |
|-----------|---|
| `{"[39319,41588)"}` | **0.09** |

####  Point interval OR window (R≈0.09)

```sql
USET WITH PRUNING (
  SELECT SUM(Score) FROM hospital_outlier_rset IS UADB
  WHERE Score = 90 OR (Score > 90 AND Score < 100)
);
```

| Result (P) |
|-----------|
| `{"[22680,22681)","[22681,24949)"}` |




---

## 4. sales_rset — Multi-Interval Envelope

### 4.1 Design Highlights

- **Source**: Synthetic store sales data (`sales_rset.csv`), 1000 rows.
- **Uncertain columns**: `daily_sales`, `daily_cost` (multi-segment `int4range[]`).
- **Certain columns**: `store_id`, `region`, `u_r`.
- **Data file**: `gprom/sales_rset.csv`
### 4.2 Queries and Results

#### SUM — Single predicate (negative case, R=1.0)

```sql
-- NP / P yield the same result
USET WITH PRUNING (
  SELECT SUM(daily_sales) FROM sales_rset IS UADB WHERE daily_sales > daily_cost
);
```

| Mode | Result | ic\_sum | R |
|------|------|---------|---|
| NP / P | `{"[1037753,1283026)"}` | 245273 | **1.0** |

> Row-level pruning still works (E-S-ROW R≈0.98), but the global SUM envelope is unchanged.

#### SUM — Dual predicate (R≈0.77)

```sql
USET WITH PRUNING (
  SELECT SUM(daily_sales) FROM sales_rset IS UADB
  WHERE daily_sales > 1000 AND daily_sales > daily_cost
);
```

| Mode | Result | ic\_sum | R |
|------|------|---------|---|
| NP | `{"[861093,1067216)"}` | 206123 | — |
| P | `{"[888862,1047569)"}` | 158707 | **0.77** |


#### SUM — GROUP BY region

```sql
USET WITH PRUNING (
  SELECT region, SUM(daily_sales) FROM sales_rset IS UADB
  WHERE daily_sales > daily_cost GROUP BY region
);
```

| region | sum(daily_sales) |
|--------|------------------|
| South | `{"[173124,263499)"}` |
| North | `{"[203861,250046)"}` |
| West | `{"[208893,231642)"}` |
| East | `{"[197999,257515)"}` |
| Central | `{"[253876,280328)"}` |

#### SUM — Small subset store_id=1

```sql
USET WITH PRUNING (
  SELECT SUM(daily_sales) FROM sales_rset IS UADB
  WHERE store_id = 1 AND daily_sales > daily_cost
);
```

| Result (P) |
|-----------|
| `{"[12363,14887)"}` |

#### MIN / MAX

```sql
USET WITH PRUNING (
  SELECT MIN(daily_sales), MAX(daily_sales) FROM sales_rset IS UADB
  WHERE daily_sales > daily_cost
);
```

| Column | Result (P) |
|----|-----------|
| min(daily_sales) | `{"[62,154)"}` |
| max(daily_sales) | `{"[2525,3719)"}` |

#### ROW — Projection pruning comparison (first 4 rows)

```sql
-- NP
USET (SELECT daily_sales, daily_cost FROM sales_rset IS UADB WHERE daily_sales > daily_cost);

-- P
USET WITH PRUNING (SELECT daily_sales, daily_cost FROM sales_rset IS UADB WHERE daily_sales > daily_cost);
```

| Row | NP daily_sales | P daily_sales |
|----|----------------|---------------|
| 1 | `{"[1157,1220)","[1176,1243)"}` | `{"[1157,1243)"}` |
| 2 | `{"[1542,1563)","[1828,1875)"}` | unchanged |
| 3 | `{"[954,1009)","[1199,1257)"}` | unchanged |
| 4 | 3 segments | 3 segments (partial tightening) |

#### Same-column OR (R≈0.37)

```sql
USET WITH PRUNING (
  SELECT SUM(daily_sales) FROM sales_rset IS UADB
  WHERE daily_sales > daily_cost OR daily_sales > 2000
);
```

| Mode | Result | ic\_sum | R |
|------|------|---------|---|
| NP | `{"[1037753,1283026)"}` | 245273 | — |
| P | `{"[182703,272446)"}` | 89743 | **0.37** |

#### Cross-column OR (negative case, R=1.0)

```sql
USET WITH PRUNING (
  SELECT SUM(daily_sales) FROM sales_rset IS UADB
  WHERE daily_sales > daily_cost OR daily_cost < 300
);
```

| Result (P) | R |
|-----------|---|
| `{"[1037753,1283026)"}` | 1.0 |

---

## 5. Experiment Summary

### 5.1 Key Findings

Experiments across all three datasets cover **primary-key repair**, **outliers**, and **large-scale multi-interval** uncertainty sources, quantifying pruning effect uniformly via the integer-coverage ratio \(R\).

| ExpID | Dataset | Query gist | ic\_sum NP→P | **R** | Role |
|-------|--------|----------|-------------|-------|------|
| **SUM** | flights | `SUM(act_dep)`, `sched_dep < act_dep` | 32664→14036 | **0.43** | PK repair flagship |
| **SUM** | flights | triple conjunct + `act_dep>900` | 27215→6230 | **0.23** | complex AND |
| **SUM** | hospital | `90 < score < 100` | 25201→2017 | **0.08** | strongest outlier pruning |
| **SUM** | hospital | `score < ZipCode` | 25201→2269 | **0.09** | cross-column |
| **SUM** | sales | `>1000 AND > cost` | 206123→158707 | **0.77** | large-scale positive |
| **OR** | sales | `>cost OR >2000` | 245273→89743 | **0.37** | OR positive |
| **OR** | sales | OR+AND nesting | 154150→19415 | **0.13** | strong OR pruning |
| **OR** | flights | same-column OR | 32664→7195 | **0.22** | `prune_set_or` |
| **OR** | hospital | OR+AND nesting | 25201→2269 | **0.09** | outlier OR |
