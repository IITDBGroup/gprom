"""
    run experiments on for different data distirbutions. Normal, Zipf, etc...
"""

from dataclasses import replace

from cliUtility import *
from DataTypes import *
from main import ExperimentGroup, format_datasize, format_name, ExperimentSuite, make_log_sweep
import numpy as np


'''
experiments is a dict of {str: ExperimentGroup}. ALlows for many unrelated experiments to run from 1 file
Naming convention is "GroupName/ID": {ExperimentGroup of related experiments}
persists in namespace of caller program
'''

experiments = dict()

# dummy used to access members
template = ExperimentSettings(
    data_type=DataType.SET, 
    dataset_size=10_000, 
    uncertain_ratio=0.0, 
    mult_size_range=(1,5),
    interval_size_range=(1, 1000), 
    num_intervals=4, 
    mode="all",
    num_trials=3, 
    gap_size_range=(0,100), 
    name= "temp",
    reduce_triggerSz_sizeLim=(10, 5),
)

def dist_gap_sweep(dist: DistributionConfig, gap_sizes: list, max_ni: int = 10, n_list: list = None, trigger_size: int = 10, reduce_to_size: int = 5, gap_width: int = 1):
    if n_list is None:
        n_list = []

    group = ExperimentGroup(f'dist_gap_sweep{dist.distribution.name}_{trigger_size}_{reduce_to_size}_sweep', 'distribution_config', None)
    
    for n in n_list:
        for g in gap_sizes:
            # for ni in range(1, max_ni+1):
                ni = 5
            
                # gap_sequence = [g] * (ni-2) + [g**3 + np.random.randint(1, 300_000)]

                experiment = replace(
                    template,
                    dataset_size             = n,
                    num_trials               = 3,
                    uncertain_ratio          = 0.0, 
                    independent_variable     = 'gap_size_range',
                    interval_size_range      = (1, 100_000),
                    start_interval_range     = (1, 10_000),
                    gap_size_range           = (g, g + g),  # fix the gap size
                    # gap_size_sequence        = gap_sequence,
                    interval_width_range     = (5, 6),
                    num_intervals            = ni,
                    reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
                    distribution_config = dist,
                )
                experiment.name = f"{format_name(experiment)}_g{g}_ni{ni}_n{n}_r{trigger_size}_{reduce_to_size}"
                group.experiments[experiment.name] = experiment
        
    return group

def plot_dist_gap_sweep(max_ni: int, n_list: list, gap_sizes:list, dist:DistributionConfig, suite_name: str = None):
    suite_name = suite_name if suite_name is not None else f'ni_gap_sweeping{format_datasize(n_list[-1])}'
    if suite_name not in experiments:
        experiments[suite_name] = ExperimentSuite(suite_name)
    
    # red_params = [(15, 10), (10, 5), (4, 2), (9, 3), (5, 2), (1, 1), (3,1)
    
    experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 500, 250, 1000))
    # experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 500, 100, 1000))
    experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 500, 10, 1000))
    experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 150, 100, 1000))
    experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 150, 10, 1000))
    # experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 70, 50, 1000))
    experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 70, 10, 1000))
    # experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 15, 10, 1000))
    experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 10, 5, 1000))
    # experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 4, 2, 1000))
    # experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 9, 3, 1000))
    # experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 5, 2, 1000))
    experiments[suite_name].add(dist_gap_sweep(dist, gap_sizes, max_ni, n_list, 3, 1, 1000))

n_list = make_log_sweep(1, 2000, 20)
# gap_sizes = [2, 5, 10, 25, 35, 50]
gap_sizes = [5, 50, 200, 1000, 5000]

zipf = DistributionConfig(
    distribution=DistributionType.ZIPFIAN,
    pos_zipf_a=1.8,
    width_zipf_a=1.5,
)
plot_dist_gap_sweep(5, n_list, gap_sizes, zipf, "dist_gap_sweep_zipf1.8w1.5")

uniform = DistributionConfig(
    distribution=DistributionType.UNIFORM,
)
plot_dist_gap_sweep(5, n_list, gap_sizes, uniform, "dist_gap_sweep_uniform")

normal = DistributionConfig(
    distribution=DistributionType.NORMAL,
    pos_mean = None,
    pos_std = None,
    width_mean = 100,
    width_std = 25,
)
plot_dist_gap_sweep(5, n_list, gap_sizes, normal, "dist_gap_sweep_normal")