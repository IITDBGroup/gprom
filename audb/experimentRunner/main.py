# standard
import pathlib
import sys
import random
import time
import os
import json
import hashlib
from dataclasses import dataclass

# 3rd party
from numerize import numerize
import pandas as pd
import psycopg2, psycopg2.extras
import numpy as np

# local
from cliUtility import *
from DataTypes import *
from StatisticsPlotter import *

@dataclass
class ExperimentSuite:
    """ contains many related groups of experiments (ExperimentGroups) """
    name: str
    groups: dict[str, ExperimentGroup] = None

    def __post_init__(self):
        if self.groups is None:
            self.groups = {}

    def add(self, group: ExperimentGroup):
        if group.name in self.groups:
            raise ValueError(f"Duplicate group name: {group.name}")
        self.groups[group.name] = group

@dataclass
class ExperimentGroup:
    """ container class that stores groups of single IV experiments (dict of ExperimentSettings + metadata) """
    name: str
    independent_variable: str
    experiments: dict = None
    
    def __post_init__(self):
        if self.experiments is None:
            self.experiments = {}

@dataclass
class ExperimentSettings:
    """
        Class contains the modifiable settings of a test
        if num_intervals is used, num_intervals_range shouldn't be used (left default=NULL)
        if gap_size is used, gap_size_range shouldn't be used (left default=NULL)
          ...
    """

    name: str                                   # required 
    data_type: DataType                         # always Set or Range
    distribution_config: DistributionConfig = field(default_factory=DistributionConfig)     # distribution type
    curr_trial: int = 0                         # keep track locally 
    experiment_id: str = None                   # unique string name that identifies specific experiment
    num_trials: int = 1                         # always fixed
    dataset_size: int = 100                     # always fixed
    uncertain_ratio: float = 0.00               # uncertainty ratio is split 50% in data, 50% in multiplicity columns. Uncert in data == NULL, uncert in mult = [0,N]
    interval_size_range: tuple = (1, 1000)      # the size of each interval
    mult_size_range: tuple = (1,5)              # required 
    independent_variable: str = None            # flag for what var we test. Used internally
    start_interval_range: tuple = (interval_size_range[0], interval_size_range[1])   
    reduce_triggerSz_sizeLim: tuple = (10,5)    # (trigger size, size to reduce to)
    domain_max: int = None
    
    # use these value if not None, otherwise use tuple if not None, both none = error
    interval_width: int = None
    interval_width_range: tuple = None
    num_intervals: int = None       
    num_intervals_range: tuple = None
    gap_size: int = None
    gap_size_range: tuple = None    
    gap_size_sequence: list = None  # sequence of gaps. allows for creating outlier ex. [5, 5, 5, 10000]
    gap_seq_formula_str: str = None # exact formula of gap_size_sequence
    
    prune: bool = False
    prune_alpha : int = 0
    
    mode: str = None                # NOT USED YET what modes of test suite to execute
    save_ddl:bool = False           # store ddl code to make tables 
    save_csv: bool = True           # store csv with statistics and results of test

    # shortened abbreviation of atributes
    iv_map = {
        "dataset_size": "n",
        "uncertain_ratio": "unc",
        "interval_size_range": "isr",
        "mult_size_range": "msr",
        "num_intervals": "ni_nir",
        "num_intervals_range": "ni_nir",
        "gap_size": "gs_gsr",
        "gap_size_range": "gs_gsr",
        "reduce_triggerSz_sizeLim": "red_sz"
    }

    def to_dict(self) -> dict:
        ''' convenience function converting class to dict''' 

        dt = 'range' if self.data_type == DataType.RANGE else 'set'
        return {
            # have to ignore these i forget why? i think reprod errors bc they depend on key which is run dependent
            # 'name': self.name,
            # 'curr_trial': self.curr_trial,
            # 'experiment_id': self.experiment_id,
            'data_type': dt,
            'num_trials': self.num_trials,
            'dataset_size': self.dataset_size,
            'uncertain_ratio': self.uncertain_ratio,
            "interval_size_range": self.interval_size_range,
            'mult_size_range': self.mult_size_range,
            'independent_variable': self.independent_variable,
            'start_interval_range': self.start_interval_range,
            'reduce_triggerSz_sizeLim': self.reduce_triggerSz_sizeLim,
            'interval_width': self.interval_width,
            'interval_width_range': self.interval_width_range,
            'num_intervals': self.num_intervals,
            'gap_size': self.gap_size,
            'num_intervals_range': self.num_intervals_range,
            'gap_size_range': self.gap_size_range,
            'prune' : self.prune,
        }
    
class ExperimentRunner:
    """
        ExperimentRunner runs entire parts, or specific parts of a test (gen_data, insert_db).
        assuming proper data type and format, user can: run_experiment()
    """

    NORMALIZE = True
    
    DATA_TYPE_CONFIG = {
        DataType.RANGE: {
            "combine_sum": "combine_range_mult_sum",
            "combine_min": "combine_range_mult_min",
            "combine_max": "combine_range_mult_max",
        },
        DataType.SET: {
            "combine_sum": "combine_set_mult_sum",
            "combine_min": "combine_set_mult_min",
            "combine_max": "combine_set_mult_max",
        },
    }

    def __init__(self, db_config, seed):
        self.db_config = db_config          # config file to connect to postgres
        self.results = []                   # agrgegate results
        self.master_seed = seed             # for entire script reproducibility
        self.trial_seed = None              # dependent on master_seed and trialNum
        self.resultFilepath: str = None     # outputs
        self.name = None                    # internal id
        self.groupName = None               # bucket experiments together
        self.csv_paths = []                 # store all df results of every exp for mass analysis

    def full_run(self, experiment: ExperimentSettings) -> list:
        """ generate data, insert to DB, run experiments and save results to rv """
        # TODO

    def run_experiment(self, experiment: ExperimentSettings) -> list:
        ''' creates or reuses generated data. runs experiment for each trial '''

        experiment_results = []

        # Set seed once for the whole experiment
        self.trial_seed = (self.master_seed) % (2**32)
        np.random.seed(self.trial_seed)

        # Set experiment_id BEFORE generating/inserting data
        experiment.curr_trial = 1
        experiment.experiment_id = self.__generate_name(experiment)

        # Generate data once and insert under the fixed experiment_id
        db_data_format, file_data_format = self.generate_data(experiment)
        if experiment.save_ddl:
            self.__save_ddl_file(experiment, file_data_format)
        self.__insert_data_db(experiment, db_data_format)

        # experiment_id stays the same every trial — same table, same data
        for trial in range(experiment.num_trials):
            experiment.curr_trial = trial + 1
            print(f'DEBUG trial:{experiment.curr_trial}')

            # new connection per trial to reduce potential bias
            conn = self.__connect_db()
            try:
                trial_results = self.run_queries(experiment, conn)
            finally:
                conn.close()

            # save trial results to experiment results
            experiment_results.append(trial_results)

        aggregated_results = self.__calc_aggregate_results(experiment, experiment_results)
        self.results.append(aggregated_results)
        return experiment_results
            
    def generate_data(self, experiment :ExperimentSettings):
        '''
            Generates pseudorandom data based on user specified experiment settings. 
            * NOTE Specfic data serialization for different formats (i.e file and db ddl differs)
        '''
        db_formatted_rows = []
        file_formatted_rows = []
        
        for i in range(experiment.dataset_size):
            if experiment.data_type == DataType.RANGE:
                obj1 = self.__generate_range2(experiment)
                obj2 = self.__generate_range2(experiment)
                val = str(obj1) if not obj1.isNone else None
                val2 = str(obj2) if not obj2.isNone else None
            elif experiment.data_type == DataType.SET:
                obj1 = self.__generate_set2(experiment)
                obj2 = self.__generate_set2(experiment)
                val = str(obj1) if (obj1.rset and not getattr(obj1, 'isNone', False)) else None
                val2 = str(obj2) if (obj2.rset and not getattr(obj2, 'isNone', False)) else None

            mult_obj = self.__generate_mult(experiment)
            mult = str(mult_obj)
            # row tuple looks like:     | val | mult |, val = set or individual range
            db_formatted_rows.append((val, val2, mult))
            
            # save in ddl preffered format if requested
            if experiment.save_ddl:
                val = obj1.str_ddl()
                val2 = obj1.str_ddl()
                mult = mult_obj.str_ddl()
                file_formatted_rows.append((val, val2, mult))
                
        return db_formatted_rows, file_formatted_rows

    def run_queries(self, experiment: ExperimentSettings, conn = None):
        '''Run aggregation tests and collect metrics.
           NOTE- does not account for cold/warm cache, or scan cost/planning vs agrgegation/execution cost
        '''
        
        results = {
            'row_count' : 0,
            'min_time' : None,
            'max_time' : None,
            'sum_time' : None,
            'sumMetrics_time': None,
            'prune_min_time' : None,
            'prune_max_time' : None,
            'prune_sum_time' : None,
            'prune_sumMetrics_time': None,
            
            'sumMetrics_result' : None,
            'reduce_calls' : None,
            'max_interval_count': None,
            'total_interval_count': None,
            'combine_calls': None,
            'result_size': None,
            'result_coverage': None,
            'min_interval_count': None,
            'total_min_coverage': None,
            
            'prune_sumMetrics_result' : None,
            'prune_reduce_calls' : None,
            'prune_max_interval_count': None,
            'prune_total_interval_count': None,
            'prune_combine_calls': None,
            'prune_result_size': None,
            'prune_result_coverage': None,
            'prune_min_interval_count': None,
            'prune_total_min_coverage': None,
        }
        table = experiment.experiment_id
        config = self.DATA_TYPE_CONFIG[experiment.data_type]

        agg_jobs = [
            ('min_time', 'MIN', config['combine_min'], []),
            ('max_time', 'MAX', config['combine_max'], []),
            ('sum_time', 'SUM', config['combine_sum'], list(experiment.reduce_triggerSz_sizeLim)),
            ('sumMetrics_time', 'SUM_METRICS', config['combine_sum'], [experiment.reduce_triggerSz_sizeLim[0], experiment.reduce_triggerSz_sizeLim[1], not self.NORMALIZE])
        ]
        random.shuffle(agg_jobs)            # shuffle bc of bias for warm cache later on. ideally average out a bit 

        try:
            # with self.__connect_db() as conn:
            with conn.cursor() as cur:
                # print(f" DEBUG SQL- running queries for : {table}") 
                
                # count 
                cur.execute(f"SELECT COUNT(*) FROM {table};")
                results['row_count'] = cur.fetchone()[0]
                
                # run shuffled aggregates
                for key, agg, func, params in agg_jobs:
                    if key != 'sumMetrics_time':
                        if experiment.prune:
                            results[key] = self.__run_aggregate_cond(cur, table, agg, func, "set_lt(val, val2) is not false", *params)
                            results[f'prune_{key}'] = self.__run__pruning(cur, table, agg, func, experiment.prune_alpha, *params)
                        else:
                            results[key] = self.__run_aggregate(cur, table, agg, func, *params)
                
                # get additional results for sumMetrics. Run experiment and time profile once each
                metrics = self.__get_sum_metrics(cur, table, config['combine_sum'], experiment.reduce_triggerSz_sizeLim[0], experiment.reduce_triggerSz_sizeLim[1], not self.NORMALIZE)
                if metrics: 
                    results['sumMetrics_result'] = metrics['result']
                    results['reduce_calls'] = metrics['reduce_calls']
                    results['max_interval_count'] = metrics['max_interval_count']
                    results['total_interval_count'] = metrics['total_interval_count']
                    results['combine_calls'] = metrics['combine_calls']
                    results['result_size'] = metrics['result_size']
                    results['min_interval_count'] = metrics['min_interval_count']
                    results['total_min_coverage'] = metrics['total_min_coverage']
                    results['result_coverage'] = self.__calculate_coverage(metrics['result'])
                if experiment.prune:
                    p_metrics = self.__get_prune_sum_metrics(cur, table, config['combine_sum'], experiment.reduce_triggerSz_sizeLim[0], experiment.reduce_triggerSz_sizeLim[1], experiment.prune_alpha, not self.NORMALIZE)
                    if p_metrics:
                        results['prune_sumMetrics_result'] = p_metrics['result']
                        results['prune_reduce_calls'] = p_metrics['reduce_calls']
                        results['prune_max_interval_count'] = p_metrics['max_interval_count']
                        results['prune_total_interval_count'] = p_metrics['total_interval_count']
                        results['prune_combine_calls'] = p_metrics['combine_calls']
                        results['prune_result_size'] = p_metrics['result_size']
                        results['prune_min_interval_count'] = p_metrics['min_interval_count']
                        results['prune_total_min_coverage'] = p_metrics['total_min_coverage']
                        # results['prune_result_coverage'] = self.__calculate_coverage(p_metrics['result'])
                        
        except Exception as e:
            print(f"Error running queries for {experiment.experiment_id}: {e}")
            raise
        
        return results
    
    def clean_tables(self, find_trigger="t_%", batch_size=200):
        ''' batch drop all tables with wildcard match {find_trigger}'''
        
        print(f"\nCleaning/ Dropping all Tables starting with '{find_trigger}'")
        with self.__connect_db() as conn:
            with conn.cursor() as cur:
                try:
                    cur.execute(f"""
                        SELECT tablename 
                        FROM pg_tables 
                        WHERE schemaname = 'public' 
                            AND tablename LIKE '{find_trigger}'
                        ORDER BY tablename;
                    """)
                    tables = [row[0] for row in cur.fetchall()]

                    if not tables:
                        print(f"  No tables found matching: {find_trigger}\n")
                        return

                    dropped = 0
                    for i in range(0, len(tables), batch_size):
                        batch = tables[i: i+batch_size]
                        
                        # with self.__connect_db() as tempConn:
                        for table in batch:
                            try:
                                cur.execute(f'DROP TABLE IF EXISTS "{table}" CASCADE;')
                                dropped+=1
                            except Exception as e:
                                print(f"    Failed to drop '{table}': {e}")
                                raise
                        conn.commit()
                except Exception as e:
                    print(f"    Error cleaning tables: {e}")
        print(f"\nDropped {dropped} tables\n")

    # def set_file_path(self, suite_name: str, group_name: str, experiment_name:str) -> None:
    #     """creates experiment folder path based on group and experiment name.
    #     if experiment_name is None, creates a folder for the entire group.

    #     Format:
    #     - with experiment: ./data/results/<group>/<experiment_name>_sd<seed>
    #     - group-only:    ./data/results/<group>/sd<seed>"""
        
    #     folder_name = f"{experiment_name}" if experiment_name else ""

    #     if group_name and suite_name:
    #         self.resultFilepath = os.path.join("data", "results", str(self.master_seed), suite_name, group_name, folder_name)
    #     else:
    #         self.resultFilepath = os.path.join("data", "results", str(self.master_seed), folder_name)

    #     os.makedirs(self.resultFilepath, exist_ok=True)
    
    def set_file_path(self, suite_name: str, group_name: str, experiment_name:str) -> None:
        """
            creates experiment folder path based on group and experiment name.
            if experiment_name is None, creates a folder for the entire group.

            Format:
            - with experiment: ./data/results/<group>/<experiment_name>_sd<seed>
            - group-only:    ./data/results/<group>/sd<seed>
        """
        
        folder_name = f"{experiment_name}" if experiment_name else ""
        if group_name and suite_name:
            self.resultFilepath = os.path.join("data", "results", str(self.master_seed)+f"_{suite_name}", group_name, folder_name)
        else:
            self.resultFilepath = os.path.join("data", "results", str(self.master_seed), folder_name)

        os.makedirs(self.resultFilepath, exist_ok=True)

    # ----------------------------------  
    # --- Internal Helpers (Private) ---
    # ----------------------------------    
    def __sample_int(self, low: int, high: int, experiment:ExperimentSettings, isWidth: bool=False) -> int:
        '''sample a single int in [low, high) using specified distribution'''
        if low >= high:
            return low
        
        dist = experiment.distribution_config
        
        if dist.distribution == DistributionType.UNIFORM:
            return int(np.random.randint(low, high))
        
        elif dist.distribution == DistributionType.NORMAL:
            # defaultize the lack of input... maybe just raise here
            if isWidth:
                mean = dist.width_mean if dist.width_mean is not None else (low + high) / 2
                std  = dist.width_std if dist.width_std is not None else (high - low) / 6
            else:
                mean = dist.pos_mean if dist.pos_mean is not None else (low + high) / 2
                std  = dist.pos_std if dist.pos_std is not None else (high - low) / 6
            return int(np.clip(np.random.normal(mean, std), low, high))
        
        elif dist.distribution == DistributionType.ZIPFIAN:
            if isWidth:
                val = int(np.random.zipf(dist.width_zipf_a)) -1
            else:
                val = int(np.random.zipf(dist.pos_zipf_a)) -1

            return int(np.clip(val + low, low, high - 1))
        
        elif dist.distribution == DistributionType.CLUSTERED:
            raise NotImplementedError("Stuggled on clustered, but this would be good i think!")
            return int(np.random.randint(low, high))
        
        raise ValueError(f"Unknown distribution: {dist.distribution}")
        
    def __generate_range2(self, experiment:ExperimentSettings) -> RangeType:
        # uncertain ratio. distributed half as value NULLS, and other half as mult = [0,X]
        if np.random.random() < experiment.uncertain_ratio * 0.5:  
            return RangeType(0, 0, True)
        
        low, high = experiment.interval_size_range
        lb = self.__sample_int(low, high, experiment, False)
        width = self.__sample_int(low, high, experiment, True)
        return RangeType(lb, lb + width)
    
    def __generate_set2(self, experiment:ExperimentSettings) -> RangeSetType:
        # if experiment.num_intervals is not None then use, otherwise if experiment.num_intervals_range then use. otherwise raise error
        if experiment.num_intervals is not None:
            num_intervals = experiment.num_intervals
        elif experiment.num_intervals_range is not None:
            num_intervals = np.random.randint(*experiment.num_intervals_range)
        else:
            raise ValueError("Either num_intervals or num_intervals_range must be specified")
        
        # entire set is unknown
        if np.random.random() < experiment.uncertain_ratio * 0.5:  
            return RangeSetType([], cu=False)
        
        rset = []

        # set the first starting point
        if experiment.start_interval_range is not None:
            start = self.__sample_int(*experiment.start_interval_range, experiment, False)
            # start = np.random.randint(*experiment.start_interval_range)
        else:
            start = experiment.interval_size_range[0]

        # for each interval
        for i in range(num_intervals):    
            if np.random.random() < experiment.uncertain_ratio * 0.5:  
                continue
            
            # get the interval width
            if experiment.interval_width is not None:
                interval_width = experiment.interval_width
            elif experiment.interval_width_range is not None:
                # interval_width = np.random.randint(*experiment.interval_width_range)
                interval_width = self.__sample_int(*experiment.interval_width_range, experiment, True)
            else:
                raise ValueError("Either interval_width or interval_width_range must be specified")
            interval_end = start + max(1, interval_width)
            
            # should never trigger, incase does
            if interval_end <= start:
                print(f"BAD RANGE: start={start}, interval_end={interval_end}, width={interval_width}, i={i}")
                rset.append(RangeType([], interval_end, False))  
            else:     
                rset.append(RangeType(start, interval_end, False))

            # find next gap if not last
            if i < num_intervals - 1:
                if experiment.gap_size_sequence is not None:
                    idx = min(i, len(experiment.gap_size_sequence) - 1)
                    val = experiment.gap_size_sequence[idx]
                    # tuple = sample randomly in range, int = fixed gap
                    gap = int(np.random.randint(val[0], val[1] + 1)) if isinstance(val, tuple) else int(val)
                elif experiment.gap_size is not None:
                    gap = experiment.gap_size
                elif experiment.gap_size_range is not None:
                    gap = self.__sample_int(*experiment.gap_size_range, experiment)
                else:
                    gap = 0
                
                start = interval_end + gap

            # next next start exceeds bounds, we can't add more intervals
            if experiment.domain_max is not None and start >= experiment.domain_max:
                break
        
        return RangeSetType(rset, cu=False)

    def __generate_mult(self, experiment:ExperimentSettings) -> RangeType:
        # uncertain ratio. maybe should account for half nulls, half mult 0
        if np.random.random() < experiment.uncertain_ratio * 0.5:  
            return RangeType(0, 0, True)
        
        lb = np.random.randint(*experiment.mult_size_range)
        ub = np.random.randint(lb+1, experiment.mult_size_range[1]+1)
        return RangeType(lb, ub)
    
    def __insert_data_db(self, experiment: ExperimentSettings, data):
        '''Insert data into database specified in config file'''
        with self.__connect_db() as conn:
            with conn.cursor() as cur:
                table_name = experiment.experiment_id
                cur.execute(f"DROP TABLE IF EXISTS {table_name};")

                if experiment.data_type == DataType.RANGE:
                    cur.execute(f"CREATE TABLE {table_name} (id INT GENERATED ALWAYS AS IDENTITY, val int4range, val2 int4range, mult int4range);")                
                    template = "(%s::int4range, %s::int4range, %s::int4range)"
                elif experiment.data_type == DataType.SET:
                    cur.execute(f"CREATE TABLE {table_name} (id INT GENERATED ALWAYS AS IDENTITY, val int4range[], val2 int4range[], mult int4range);")
                    template = "(%s::int4range[], %s::int4range[], %s::int4range)"
                
                sql = f"INSERT INTO {table_name} (val, val2, mult) VALUES %s"
                psycopg2.extras.execute_values(cur, sql, data, template)
                conn.commit()
    
    ####### PRUNE #######
    def build_transform(self, table, transform_func=None, prune_alpha=None):
        """
        Returns SQL subquery for FROM clause
        """

        if transform_func is None:
            return table

        if prune_alpha is None:
            return f"""
                SELECT
                    {transform_func}(val, val2, false) as val,
                    mult
                FROM {table}
            """

        # if alpha-based transform exists
        return f"""
            SELECT
                {transform_func}(val, set_divide(val2, array[lift_scalar({prune_alpha})]), false) as val,
                mult
            FROM {table}
        """


    def __run_aggregate(self, cur, table, agg_name, combine_func, *agg_params):
        '''General aggregate runner with no WHERE clause'''

        params_sql = ",".join(str(param) for param in agg_params)
        sql = f"""EXPLAIN (analyze, format json)
            SELECT {agg_name} ({combine_func}(val, mult) {',' if params_sql else ''}{params_sql})
            FROM {table};"""
        
        # print(sql)
        cur.execute(sql)
        results = cur.fetchone()[0]
        plan_root = results[0]
        plan = plan_root["Plan"]
        agg_time = plan["Actual Total Time"]
        
        return agg_time
    
    def __run_aggregate_cond(self, cur, table, agg_name, combine_func, cond, *agg_params):
        '''General aggregate runner with WHERE clause'''

        params_sql = ",".join(str(param) for param in agg_params)
        sql = f"""
            EXPLAIN (analyze, format json)
            SELECT {agg_name} ({combine_func}(val, mult) {',' if params_sql else ''}{params_sql})
            FROM {table}
            WHERE {cond};"""
        
        # print(sql)
        cur.execute(sql)
        results = cur.fetchone()[0]
        plan_root = results[0]
        plan = plan_root["Plan"]
        agg_time = plan["Actual Total Time"]
        
        return agg_time

    # add in prune_alpha for
    def __run__pruning(self, cur, table, agg_name, combine_func, prune_alpha, *agg_params):
        '''
            runs basic
            select  sum(val)
            from    R
            where   val < val2
        '''
        
        params_sql = ",".join(str(param) for param in agg_params)
        sql = f"""EXPLAIN (analyze, format json)
            SELECT {agg_name} ({combine_func}(val, mult) {',' if params_sql else ''}{params_sql})
            FROM (          
                SELECT 
                    prune_set_lt(val, val2, false) as val,
                    mult
                
                -- prune_set_lt(val, set_divide(val2, array[lift_scalar({prune_alpha})]), false) as val,
                -- int4range(lower(mult) * case when set_lt(a,b) is NULL then 0 else 1 end, upper(mult)) as mult
                FROM {table}
            ) sub;"""
        cur.execute(sql)

        # print(sql)
        results = cur.fetchone()[0]
        plan_root = results[0]
        plan = plan_root["Plan"]
        agg_time = plan["Actual Total Time"]
    
        return agg_time
    
    def __get_sum_metrics(self, cur, table, combine_func, trigger_sz, size_lim, normalize: bool):
        '''get sum_metrics metrics from composite type result using field accessors'''
        
        # sql = f"""
        #     SELECT 
        #         (result).result,
        #         (result).resizeTrigger,
        #         (result).sizeLimit,
        #         (result).reduceCalls,
        #         (result).maxIntervalCount,
        #         (result).totalIntervalCount,
        #         (result).combineCalls,
        #         (result).minEffectiveIntervalCount,
        #         (result).convergedToTotSize
        #     FROM (
        #         (SELECT sum_metrics({combine_func}(val, mult), {trigger_sz}, {size_lim}, {normalize}) as result
        #         FROM {table})
        #     ) subq;"""

        # prune 
        sql = f"""
            SELECT 
                (result).result,
                (result).resizeTrigger,
                (result).sizeLimit,
                (result).reduceCalls,
                (result).maxIntervalCount,
                (result).totalIntervalCount,
                (result).combineCalls,
                (result).minEffectiveIntervalCount,
                (result).convergedToTotSize
            FROM (
                SELECT 
                    sum_metrics({combine_func}(val, mult), {trigger_sz}, {size_lim}, {normalize}) as result
                FROM {table}
                WHERE set_lt(val, val2) is not false
            ) subq;"""
        
        cur.execute(sql)
        # print(sql)
        result = cur.fetchone()     
        if result is None:
            return None
        
        result_array = result[0] 
        resize_trigger = result[1]
        size_limit = result[2]
        reduce_calls = result[3]
        max_interval_count = result[4]
        total_interval_count = result[5]
        combine_calls = result[6]
        min_int_count = result[7]
        tot_min_size = result[8]

        metrics = {
            'result': result_array,             # list of NumericRange objects
            'resize_trigger': resize_trigger,
            'size_limit': size_limit,
            'reduce_calls': reduce_calls,
            'max_interval_count': max_interval_count,
            'total_interval_count': total_interval_count,
            'combine_calls': combine_calls,
            'min_interval_count': min_int_count,
            'total_min_coverage': tot_min_size,
            'result_size': len(result_array) if result_array else 0,
        }
    
        return metrics
    

    def __get_prune_sum_metrics(self, cur, table, combine_func, trigger_sz, size_lim, prune_alpha, normalize: bool):
        '''get sum_metrics metrics from composite type result using field accessors'''
        
        sql = f"""
            SELECT 
                (result).result,
                (result).resizeTrigger,
                (result).sizeLimit,
                (result).reduceCalls,
                (result).maxIntervalCount,
                (result).totalIntervalCount,
                (result).combineCalls,
                (result).minEffectiveIntervalCount,
                (result).convergedToTotSize
            FROM (
                SELECT 
                    sum_metrics({combine_func}(val, mult), {trigger_sz}, {size_lim}, {normalize}) as result
                FROM (
                    SELECT 
                        prune_set_lt(val, val2, false) as val,
                        mult
                        -- prune_set_lt(val, set_divide(val2, array[lift_scalar({prune_alpha})]), false) as val,
                        -- prune_set_lt(val, val2, false) as val,
                    FROM {table}
                ) sub1
            ) subq;"""
        
        cur.execute(sql)
        # print(sql)
        result = cur.fetchone()     
        if result is None:
            return None
        
        result_array = result[0] 
        resize_trigger = result[1]
        size_limit = result[2]
        reduce_calls = result[3]
        max_interval_count = result[4]
        total_interval_count = result[5]
        combine_calls = result[6]
        min_int_count = result[7]
        tot_min_size = result[8]

        metrics = {
            'result': result_array,             # list of NumericRange objects
            'resize_trigger': resize_trigger,
            'size_limit': size_limit,
            'reduce_calls': reduce_calls,
            'max_interval_count': max_interval_count,
            'total_interval_count': total_interval_count,
            'combine_calls': combine_calls,
            'min_interval_count': min_int_count,
            'total_min_coverage': tot_min_size,
            'result_size': len(result_array) if result_array else 0,
        }
    
        return metrics
           
    def __calculate_coverage(self, interval_set):
        '''adds all values contained within every interval in set'''
        
        cover = 0
        # print("IS.   ", interval_set)
        for interval in interval_set:
            if interval is None:
                continue
            cover += interval.upper - interval.lower
        return cover

    def __calc_aggregate_results(self, experiment: ExperimentSettings, trial_results: dict) -> dict:
        ''' combines all result data, and returns dict of all experiment metadata leter used to convert to CSV'''

        dist = experiment.distribution_config
        distribution_fields = {}

        if dist.distribution == DistributionType.NORMAL:
            distribution_fields = {
                'pos_mean': dist.pos_mean,
                'pos_std': dist.pos_std,
                'width_mean': dist.width_mean,
                'width_std': dist.width_std,
            }
        elif dist.distribution == DistributionType.ZIPFIAN:
            distribution_fields = {
                'pos_zipf_a': dist.pos_zipf_a,
                'width_zipf_a': dist.width_zipf_a,
            }
        elif dist.distribution == DistributionType.CLUSTERED:
            distribution_fields = {
                'pos_n_clusters': dist.pos_n_clusters,
                'pos_cluster_spread': dist.pos_cluster_spread,
                'width_n_clusters': dist.width_n_clusters,
                'width_cluster_spread': dist.width_cluster_spread,
            }

        def extract(key):
            return [r[key] for r in trial_results if r.get(key) is not None]

        row_count = extract('row_count')
        min_times = extract('min_time')
        max_times = extract('max_time')
        sum_times = extract('sum_time')
        sumMetrics_time = extract('sumMetrics_time')

        p_min_times = extract('prune_min_time')
        p_max_times = extract('prune_max_time')
        p_sum_times = extract('prune_sum_time')
        p_sumMetrics_time = extract('prune_sumMetrics_time')

        sum_results = trial_results[0].get('sumMetrics_result') if trial_results else None   # actual result
        reduce_calls = extract('reduce_calls')
        max_intervals = extract('max_interval_count')
        total_intervals = extract('total_interval_count')
        combine_calls = extract('combine_calls')
        result_sizes = extract('result_size')
        result_coverages = extract('result_coverage')
        minEffectiveIntervalCount = extract('min_interval_count')
        convergedToTotSize = extract('total_min_coverage')

        p_sum_results = trial_results[0].get('prune_sumMetrics_result') if trial_results else None   # actual result
        p_reduce_calls = extract('prune_reduce_calls')
        p_max_intervals = extract('prune_max_interval_count')
        p_total_intervals = extract('prune_total_interval_count')
        p_combine_calls = extract('prune_combine_calls')
        p_result_sizes = extract('prune_result_size')
        p_result_coverages = extract('prune_result_coverage')
        p_minEffectiveIntervalCount = extract('prune_min_interval_count')
        p_convergedToTotSize = extract('prune_total_min_coverage')

        aggregated = {
            # experiment metadata
            'uid' : self.__generate_name(experiment, True),
            'master_seed': self.master_seed,
            'data_type' : 'range' if experiment.data_type == DataType.RANGE else 'set',
            'num_trials': experiment.num_trials,
            'dataset_size' : experiment.dataset_size,
            'uncertain_ratio': experiment.uncertain_ratio,
            'interval_size_range':experiment.interval_size_range,
            'mult_size_range': experiment.mult_size_range,
            'num_intervals': experiment.num_intervals,
            'start_interval_range': experiment.start_interval_range,
            'gap_size': experiment.gap_size,
            'interval_width': experiment.interval_width,
            'num_intervals_range': experiment.num_intervals_range,
            'gap_size_range': experiment.gap_size_range,
            'interval_width_range': experiment.interval_width_range,
            'reduce_triggerSz_sizeLim': experiment.reduce_triggerSz_sizeLim,
            'independent_variable': experiment.independent_variable,
            'distribution': dist.distribution,
            'prune' : experiment.prune,
            'prune_alpha' : experiment.prune_alpha,
            **distribution_fields,
            
            'row_count': np.mean(row_count) if row_count else None,
            # MIN stats
            'min_time_mean': np.mean(min_times) if min_times else None,
            'min_time_std': np.std(min_times) if min_times else None,    
            # MAX stats
            'max_time_mean': np.mean(max_times) if max_times else None,
            'max_time_std': np.std(max_times) if max_times else None,
            # SUM stats 
            'sum_time_mean': np.mean(sum_times) if sum_times else None,
            'sum_time_std': np.std(sum_times) if sum_times else None,
            'sumMetrics_time_mean': np.mean(sumMetrics_time) if sumMetrics_time else None,
            'sumMetrics_time_std': np.std(sumMetrics_time) if sumMetrics_time else None,

            # Prune MIN stats
            'p_min_time_mean': np.mean(p_min_times) if p_min_times else None,
            'p_min_time_std': np.std(p_min_times) if p_min_times else None,    
            # MAX stats
            'p_max_time_mean': np.mean(p_max_times) if p_max_times else None,
            'p_max_time_std': np.std(p_max_times) if p_max_times else None,
            # Prune SUM stats 
            'p_sum_time_mean': np.mean(p_sum_times) if p_sum_times else None,
            'p_sum_time_std': np.std(p_sum_times) if p_sum_times else None,
            'p_sumMetrics_time_mean': np.mean(p_sumMetrics_time) if p_sumMetrics_time else None,
            'p_sumMetrics_time_std': np.std(p_sumMetrics_time) if p_sumMetrics_time else None,
            
            # reduction stats
            'sum_results': sum_results if sum_results else None,
            'reduce_calls_mean': np.mean(reduce_calls) if reduce_calls else None,
            'max_interval_count_mean': np.mean(max_intervals) if max_intervals else None,
            'total_interval_count_mean': np.mean(total_intervals) if total_intervals else None,
            'combine_calls_mean': np.mean(combine_calls) if combine_calls else None,
            'result_size_mean': np.mean(result_sizes) if result_sizes else None,
            'minEffectiveIntervalCountMean': np.mean(minEffectiveIntervalCount) if minEffectiveIntervalCount else None,
            'convergedToTotSize': np.mean(convergedToTotSize) if convergedToTotSize else None,
            'result_coverage_mean': np.mean(result_coverages) if result_coverages else None,

            'p_sum_results': p_sum_results if p_sum_results else None,
            'p_reduce_calls_mean': np.mean(p_reduce_calls) if p_reduce_calls else None,
            'p_max_interval_count_mean': np.mean(p_max_intervals) if p_max_intervals else None,
            'p_total_interval_count_mean': np.mean(p_total_intervals) if p_total_intervals else None,
            'p_combine_calls_mean': np.mean(p_combine_calls) if p_combine_calls else None,
            'p_result_size_mean': np.mean(p_result_sizes) if p_result_sizes else None,
            'p_minEffectiveIntervalCountMean': np.mean(p_minEffectiveIntervalCount) if p_minEffectiveIntervalCount else None,
            'p_convergedToTotSize': np.mean(p_convergedToTotSize) if p_convergedToTotSize else None,
            'p_result_coverage_mean': np.mean(p_result_coverages) if p_result_coverages else None,
        }
        
        return aggregated

    def __connect_db(self):
        """ return psycopg connection object """
        return psycopg2.connect(**self.db_config)
    
    def __generate_name(self, experiment: ExperimentSettings, trialName: bool = False) -> str:
        """
            generates postgres safe name (< 63 chars). old name was being cut.
                format:     t_{dtype}_{iv_abbrev}_{10 char hash of experimentDict}

            if trialName param is set, then trial number will appended to result
        """
        dtype = 'r' if experiment.data_type == DataType.RANGE else 's'
        iv_abbrev = experiment.iv_map.get(experiment.independent_variable if experiment.independent_variable else 'iv')
        param_str = json.dumps(experiment.to_dict(), sort_keys=True, default=str)
        
        hashed = hashlib.sha1(param_str.encode()).hexdigest()[:10]
    
        if trialName:
            return f"t_{dtype}_iv_{iv_abbrev}_{hashed}_t{experiment.curr_trial}"
        
        return f"t_{dtype}_iv_{iv_abbrev}_{hashed}"
        
    def __save_ddl_file(self, experiment: ExperimentSettings, data):
        ''' write data to DDL file for later loading 
            #NOTE broken. Need way to store final group and apppend all DDL to proper directory
        '''
        raise NotImplementedError("Broken. Will fix if ever actually used. NOTE- Need way to store final group and append all DDL to proper directory. Currently it stores in group dirctory, but not the specific Experiment within this group.")
        experiment_folder_path = f'data/results/{self.groupName}/ddl'
        timestamp = time.strftime("d%d_m%m_y%Y")
        out_file = f'{timestamp}_{experiment.name}_sd{self.master_seed}'
        ddl_path = f'{experiment_folder_path}/{out_file}.sql'    

        os.makedirs(experiment_folder_path, exist_ok=True)           

        table_name = experiment.experiment_id
            
        with open(ddl_path, 'w') as file:
            if experiment.data_type == DataType.RANGE:
                file.write(f"CREATE TABLE {table_name} (id INT GENERATED ALWAYS AS IDENTITY, val int4range, mult int4range);\n\n")
            elif experiment.data_type == DataType.SET:
                file.write(f"CREATE TABLE {table_name} (id INT GENERATED ALWAYS AS IDENTITY, val int4range[], mult int4range);\n\n")
            
            batch_size = 250
            for i in range(0, len(data), batch_size):
                batch = data[i: i + batch_size]
                file.write(f"INSERT INTO {table_name} (val, mult) VALUES \n")

                values = []
                for val, mult in batch:                    
                    values.append(f"    ({val}, {mult})")

                file.write(',\n'.join(values))
                file.write(';\n\n')

        print(f"  DDL saved: {ddl_path}")
    
def generate_seed(in_seed=None):
    '''genrate the master seed of this programs run. (can be included in runner or settings class)'''
    if in_seed is not None:
        seed = in_seed
    else:
        seed = int(time.time() * 1000) % (2**32)
        
    random.seed(seed)
    np.random.seed(seed)
    
    return seed

def format_datasize(size):
    if size >= 1_000_000: 
        return str.replace(numerize.numerize(size, 2), '.', '_')
    return numerize.numerize(size, 0)

def format_name(experiment: ExperimentSettings):
    """ generate unique name for an experiment based on its parameters.
        auto-includes dataset size, reduction config, and optional seed."""

    # base type: s = set, r = range
    dtype = 's' if getattr(experiment, 'data_type', None) == DataType.SET else 'r'
    
    # dataset size
    sz = f"n{getattr(experiment, 'dataset_size', 0)}"
    
    # reduction trigger/limit
    red = ""
    red_cfg = getattr(experiment, 'reduce_triggerSz_sizeLim', None)
    if red_cfg:
        red = f"red{red_cfg[0]}_{red_cfg[1]}"
    
    # shortened independent variable
    iv = getattr(experiment, 'independent_variable', 'iv')
    iv_val = getattr(experiment, iv, None)
    if iv and iv_val is not None:
        iv = f"iv_{experiment.iv_map[iv]}{iv_val}"
    
        
    seed = f"s{getattr(experiment, 'seed', '')}" if getattr(experiment, 'seed', None) else ""
    pa = "pa_" + str(experiment.prune_alpha) if experiment.prune else ""

    
    name = f"{dtype}_{sz}_{red}_{iv}_{pa}{seed}"
    return name

def make_log_sweep(n_min, n_max, points):
    '''return nonlinear sample space. clustered in beginning/ log'''
    return sorted(set(int(x) for x in np.logspace(
        np.log10(n_min),
        np.log10(n_max),
        points
    )))

def run_all():
    '''
        main entrypoint to running experiments. 
        - parses args, starts runner engine, runs experiments, and plots results
    '''

    ### Parse args and config
    args = parse_args()
    master_seed = generate_seed(args.seed)
    print("Unique Master seed: ", master_seed)

    try:
        db_config = load_config(args.dbconfig)    
    except Exception as e:
        print(f"Error loading config: {e}")
        exit(1)

    ### Start engine
    runner = ExperimentRunner(db_config, master_seed)

    ### Clean existing tables
    if args.clean_before:
        runner.clean_tables(args.clean_before)

    ### Load Experiments into dict(Suite_name: ExperimentSuite)
    experiments = _load_experiments(args, runner, db_config)

    ### Run and save independent results for each ExperimentSuite
    for suite in experiments.values():
        suite_results = []

        for group in suite.groups.values():
            results = _run_experiment_group(runner, suite.name, group)
            print(f'    Group results saved in: {runner.resultFilepath}')  
            suite_results.append(results)
        
        print(f"\n    Suite results: {suite_results}")

        # plot aggregate results for suite
        # _plot_experiment_suite(runner, suite_results)

    ### Clean tables before exit
    if args.clean_after:
        runner.clean_tables(args.clean_after)

    print("\nUnique Master seed: ", master_seed)

def _load_experiments(args, runner, db_config) -> dict:
    '''Load experiment configuration from various sources. (CLI, YAML, Python Script)'''

    if args.quick:
        return create_quick_experiment(args)
    elif args.yaml_experiments_file:
        return load_experiments_from_file(args.yaml_experiments_file)
    elif args.code:
        namespace = {'runner': runner, 'db_config': db_config}      # persist memory
        exec(open(args.code).read(), namespace)     # CS361 lol
        return namespace.get('experiments', {})
    else:
        sys.exit("No experiment source specified (use --help for examples)")

def _run_experiment_group(runner: ExperimentRunner, suite_name: str, group: ExperimentGroup):
    '''Run all experiments in a group and generate results.'''

    print(f"\nRunning experiment group: [{suite_name}]- {group.name}")
    
    # reset runner metadata to current experiment group. results are saved at group level
    runner.results = []
    runner.name = suite_name
    runner.groupName = group.name
    runner.set_file_path(suite_name, group.name, None)

    # for every experiment within group, run it
    for experiment in group.experiments.values():
        runner.run_experiment(experiment)       #main
    
    os.makedirs(runner.resultFilepath, exist_ok=True)
    group_csv_path = f"{runner.resultFilepath}results_sd{runner.master_seed}.csv"
    
    df = pd.DataFrame(runner.results)
    df.to_csv(group_csv_path, index=False)
    
    return group_csv_path

def _plot_experiment_suite(runner: ExperimentRunner, csv_paths: list) -> None:
    if csv_paths is None:
            raise ValueError('No list of csv results for suite')

    # create a suite-level folder (one level above groups)
    last_group_csv = Path(runner.resultFilepath)
    suite_folder = last_group_csv.parent  # parent of group folder → suite folder
    os.makedirs(suite_folder, exist_ok=True)
    runner.resultFilepath = suite_folder

    plotter = StatisticsPlotter(runner.resultFilepath, runner.master_seed)
    plotter.plot_experiment_suite(csv_paths)


if __name__ == '__main__':
    start = time.perf_counter()
    run_all()
    end = time.perf_counter()

    print(f"Tests took {end-start:.3f} s")

    # example cli runs (not recommended)
    # python3 main.py --quick -dt r -nt 5 -sz 2 -ur .0 -ca        
    # python3 main.py -xf tests_config.yaml -cb   