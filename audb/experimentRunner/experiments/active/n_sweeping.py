from collections import defaultdict
from cliUtility import *
from DataTypes import *
from dataclasses import replace
from main import ExperimentGroup, format_name, ExperimentSuite

'''
experiments is a dict of {str: ExperimentGroup}. ALlows for many unrelated experiments to run from 1 file
Naming convention is "GroupName/ID": {ExperimentGroup of related experiments}
persists in namespace of caller program
'''

# zipf = DistributionConfig(DistributionType=DistributionType.ZIPFIAN, width_zipf_a=1, pos_zipf_a=1.5)
# zipf = DistributionConfig()
# zipf.distribution = DistributionType.ZIPFIAN
# zipf.pos_zipf_a = 1.8
# zipf.width_zipf_a = 1.5

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
    # distribution_config= zipf,
)

def static_n_sweep(max_n: int = 100_000, step: int = 10_000, trigger_size: int = 10, reduce_to_size: int = 5):
    group = ExperimentGroup(f'n{max_n}_red{trigger_size}_{reduce_to_size}_sweep', 'dataset_size', None)

    for n in range(step, max_n + step, step):
        experiment = replace(
            template,
            dataset_size             = n,
            num_trials               = 3,
            uncertain_ratio          = 0.0,
            independent_variable     = 'dataset_size',
            interval_size_range      = (1, 100_000),
            start_interval_range     = (1, 500),
            gap_size_range           = (2000, 2001),
            interval_width_range     = (1, 100),
            num_intervals            = 5,
            reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
            
        )

        experiment.name = format_name(experiment)
        group.experiments[experiment.name] = experiment

    return group

def plot_all_n_sweep(n:int, step:int, suite_name:str = None):
    suite_name = suite_name if suite_name is not None else 'n_sweeping'
    if suite_name not in experiments:
        experiments[suite_name] = ExperimentSuite(suite_name)
    
    suite = experiments[suite_name]
    
    
    suite.add(static_n_sweep(n, step, 15, 10))
    # suite.add(static_n_sweep(n, step, 500, 10))
    # suite.add(static_n_sweep(n, step, 150, 10))
    # suite.add(static_n_sweep(n, step, 50, 10))
    # suite.add(static_n_sweep(n, step, 10, 5))
    # suite.add(static_n_sweep(n, step, 4, 2))
    # suite.add(static_n_sweep(n, step, 9, 3))
    # suite.add(static_n_sweep(n, step, 5, 2))
    # suite.add(static_n_sweep(n, step, 3, 1))
    # suite.add(static_n_sweep(n, step, 1, 1))

# plot_all_n_sweep(100_000, 10000, 'n_sweeping100k')
plot_all_n_sweep(400, 100, 'n_sweeping4k')