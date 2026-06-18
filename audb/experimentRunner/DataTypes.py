"""
    Creates the basic DataTypes with which we locally represent Native PGTypes: ArrayType and RangeType.
    Contains methods for operating arithmetic, logical, and general helper methods
"""

from __future__ import annotations
from enum import Enum
from dataclasses import dataclass, field
import itertools
import numpy as np

'''
    local represention of postres RangeType. helper methods include arithmetic,
    logical operators, and convenience methods
'''
class RangeType:
    def __init__(self, lb=0, ub=0, isNone=False):
        assert(lb<=ub)
        self.lb = lb
        self.ub = ub
        self.isNone = isNone
        
    def __add__(self, o):
        if isinstance(o, self.__class__):
            return RangeType(self.lb+o.lb, self.ub+o.ub)
        elif isinstance(o, int):
            return RangeType(self.lb+o, self.ub+o)
        else:
            raise TypeError("unsupported operand type(s) for +: '{}' and '{}'").format(self.__class__, type(other))
        
    
    def __and__(self, o):
        if not (isinstance(self.lb, bool) and isinstance(self.ub, bool) and
                isinstance(o.lb, bool) and isinstance(o.ub, bool)):
            raise ValueError("Both operands must be RangeType objects with boolean bounds.")
        
        # Calculate the new bounds
        new_lb = self.lb and o.lb
        new_ub = self.ub and o.ub
        
        return RangeType(new_lb, new_ub)
    
    def __or__(self, o):
        if not (isinstance(self.lb, bool) and isinstance(self.ub, bool) and
                isinstance(o.lb, bool) and isinstance(o.ub, bool)):
            raise ValueError("Both operands must be RangeType objects with boolean bounds.")
        
        # Calculate the new bounds
        new_lb = self.lb or o.lb
        new_ub = self.ub or o.ub
        
        return RangeType(new_lb, new_ub)
    
    def __mul__(self, o):
        if isinstance(o, self.__class__):
            lb = min(self.lb*o.lb, self.lb*o.ub, self.ub*o.lb, self.ub*o.ub)
            ub = max(self.lb*o.lb, self.lb*o.ub, self.ub*o.lb, self.ub*o.ub)
            return RangeType(lb, ub)
        elif isinstance(o, int):
            lb = min(self.lb*o, self.ub*o)
            ub = max(self.lb*o, self.ub*o)
            return RangeType(lb, ub)
        else:
            raise TypeError("unsupported operand type(s) for *: '{}' and '{}'").format(self.__class__, type(other))
    
    def __hash__(self):
        # Combine the lower and upper bounds into a hashable representation
        return hash(self.lb, self.ub)
    
    def __eq__(self, o):
        return RangeType(self.lb==self.ub and self.lb==o.lb and o.lb==o.ub, self.i(o) is not None)

    def __gt__(self, o):
        return RangeType(self.lb > o.ub, self.ub > o.lb)

    def __ge__(self, o):
        return RangeType(self.lb >= o.ub, self.ub >= o.lb)
    
    def __lt__(self, o):
        return RangeType(self.lb < o.ub, self.ub < o.lb)

    def __le__(self, o):
        return RangeType(self.lb <= o.ub, self.ub <= o.lb)

    def u(self, o):
        return RangeType(min(self.lb,o.lb), max(self.ub,o.ub))
    
    def i(self, o):
        lb = max(self.lb,o.lb)
        ub = min(self.ub,o.ub)
        if lb <= ub:
            return RangeType(max(self.lb,o.lb), min(self.ub,o.ub))
        return None
    
    def __eq__(self, other):
        if self.ub >= other.lb and other.ub >= self.lb:
            return True
        return False
    
    def __repr__(self):
        return f"[{self.lb}, {self.ub}]"
    
    def __str__(self):
        return f"[{self.lb}, {self.ub}]"

    # easier to work in postgres    
    def str_ddl(self):
        if self.isNone:
            return "NULL"
        return f"int4range({self.lb}, {self.ub})"
    
    # to be used only for local development and testing. 
    def generate_values(self, experiment:ExperimentSettings) -> RangeType:
        # uncertain ratio. maybe should account for half nulls, half mult 0
        if np.random.random() < experiment.uncertain_ratio * 0.5:  
            return RangeType(0,0,True)
        
        lb = np.random.randint(*experiment.interval_size_range)
        ub = np.random.randint(lb+1, experiment.interval_size_range[1]+1)
        return RangeType(lb, ub)

'''
    local represention of postres ArrayType. helper methods include arithmetic,
    logical operators, and convenience methods
'''
class RangeSetType:
    def __init__(self, rset, vtype = RangeType, cu=True):
        assert(type(rset) is list)
        self.vtype = vtype
        self.rset = rset
        if cu:
            self.cleanup()
        
    def __repr__(self):
        return f"{self.rset}"
    
    def __str__(self):
        if not self.rset or len(self.rset) == 0:
            return "{}"
        
        items = []
        for r in self.rset:
            if r.isNone:
                items.append("NULL")
            else:
                # Postgres array format: {"[1,2]", "[3,4]"}
                items.append(f'"{str(r)}"')
                
        return "{" + ",".join(items) + "}"
    
    # easier to work in postgres    
    def str_ddl(self):
        items = [f"{i.str_ddl()}" for i in self.rset]
        return "array[" + ",".join(items) + "]"
    
    def __len__(self):
        return len(self.rset)
    
    def cleanup(self):
        if self.vtype == RangeType:
            if len(self)<=1:
                return
            l = sorted(self.rset, key=lambda r: r.lb)
            res = []
            curR = l[0]
            for rv in sorted(self.rset, key=lambda r: r.lb):
                if rv.lb <= curR.ub:
                    curR = curR.u(rv)
                else:
                    res.append(curR)
                    curR = rv
            res.append(curR)
            self.rset = res
        
    # +
    def __add__(self, o, cu=True):
        rst = []
        if isinstance(o, self.__class__):
            for p in itertools.product(self.rset,o.rset):
                rst.append(p[0]+p[1])
            rt = RangeSetType(rst)
            if cu:
                rt.cleanup()
            return rt
        elif isinstance(o, int):
            for p in self.rset:
                rst.append(p+o)
            rt = RangeSetType(rst)
            if cu:
                rt.cleanup()
            return rt
        else:
            raise TypeError("unsupported operand type(s) for +: '{}' and '{}'").format(self.__class__, type(other))
        
    # *
    def __mul__(self, o, cu=True):
        rst = []
        if isinstance(o, self.__class__):
            for p in itertools.product(self.rset,o.rset):
                rst.append(p[0]*p[1])
            rt = RangeSetType(rst)
            if cu:
                rt.cleanup()
            return rt
        elif isinstance(o, int):
            for p in self.rset:
                rst.append(p*o)
            rt = RangeSetType(rst)
            if cu:
                rt.cleanup()
            return rt
        else:
            raise TypeError("unsupported operand type(s) for *: '{}' and '{}'").format(self.__class__, type(other))

    def __eq__(self, o):
        lb = False
        if len(self)==1 and len(o)==1: 
            l = next(iter(self.rset))
            r = next(iter(o.rset))
            lb = l.lb==l.ub and l.lb==r.lb and r.lb==r.ub
        return RangeType(lb, bool(self.i(o)))

    def u(self, o, cu=True):
        rst = []
        for p in itertools.product(self.rset,o.rset):
            rst.append(p[0].u(p[1]))
        rt = RangeSetType(rst)
        if cu:
            rt.cleanup()
        return rt
    
    def i(self, o, cu=True):
        rst = []
        for p in itertools.product(self.rset,o.rset):
            ir = p[0].i(p[1])
            if ir is not None:
                rst.append(p[0].i(p[1]))
        rt = RangeSetType(rst)
        if cu:
            rt.cleanup()
        return rt
    
    def lb(self):
        return sorted(self.rset, key=lambda r: r.lb)[0].lb
    
    def ub(self):
        return sorted(self.rset, key=lambda r: r.ub, reverse=True)[0].ub
    
    def itv(self):
        return RangeType(self.lb(),self.ub())
    
    def __gt__(self, o):
        if isinstance(o, int):
            return self.itv() > RangeType(o, o)
        return self.itv() > o.itv()

    def __ge__(self, o):
        if isinstance(o, int):
            return self.itv() >= RangeType(o, o)
        return self.itv() >= o.itv()
    
    def __lt__(self, o):
        if isinstance(o, int):
            return self.itv() < RangeType(o, o)
        return self.itv() < o.itv()

    def __le__(self, o):
        if isinstance(o, int):
            return self.itv() <= RangeType(o, o)
        return self.itv() <= o.itv()
    
    def generate_values(self, experiment:ExperimentSettings) -> RangeSetType:
        num_ranges = np.random.randint(*experiment.num_intervals_range)
        
        rset = []
        for i in range(num_ranges):    
            # uncertain ratio. maybe should account for half nulls, half mult 0
            if np.random.random() < experiment.uncertain_ratio * 0.5:  
                rset.append(RangeType(0,0,True))
                continue
            
            lb = np.random.randint(*experiment.interval_size_range)
            ub = np.random.randint(lb+1, experiment.interval_size_range[1]+1)
            
            rset.append(RangeType(lb,ub,False))
        
        return RangeSetType(rset, cu=False)
    
    def set_union(self, other):
        combined = self.rset + other.rset

        return RangeSetType(combined, cu=True)

    def set_intersection(self, other):
        rst = []
        for p in itertools.product(self.rset, other.rset):
            ir = p[0].i(p[1])
            if ir is not None:
                rst.append(p[0].i(p[1]))
        rt = RangeSetType(rst)
        
        rt.cleanup()
        return rt

class DataType(Enum):
    RANGE = RangeType
    SET = RangeSetType

class DistributionType(Enum):
    UNIFORM     = 1
    NORMAL      = 2
    ZIPFIAN     = 3
    CLUSTERED   = 4
    
@dataclass
class DistributionConfig:
    distribution: DistributionType = DistributionType.UNIFORM

    # normal
    pos_mean: float = None
    pos_std: float = None
    width_mean: float = None
    width_std: float = None

    # zipfian
    pos_zipf_a: float = 1.5
    width_zipf_a: float = 1.5

    # clustered
    pos_n_clusters: int = 5
    pos_cluster_spread: float = 0.1
    width_n_clusters: int = 5
    width_cluster_spread: float = 0.1

    # def to_dict(self):
    #     return {f.name: getattr(self, f.name) for f in fields(self)}
