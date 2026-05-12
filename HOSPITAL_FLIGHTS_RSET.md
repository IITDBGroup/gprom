# Hospital Outlier Rset & Flights Rset — Purpose and GProM Usage

## 在gprom使用语法

### Hospital Outlier Rset
```bash 
USET WITH PRUNING (
    SELECT Score, ZipCode
    FROM hospital_outlier_rset IS UADB
    WHERE Score = 90 AND Score < ZipCode
);
```

### Flights Rset
```bash 
USET WITH PRUNING (
    SELECT sched_dep, act_dep
    FROM flights_rset IS UADB
    WHERE sched_dep = 1185 AND sched_dep < act_dep
);
```



