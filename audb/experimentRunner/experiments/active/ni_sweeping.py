from collections import defaultdict
from cliUtility import *
from DataTypes import *
from dataclasses import replace
from main import ExperimentGroup, format_datasize, format_name, ExperimentSuite, make_log_sweep

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

def ni_sweep(ni_list: int, n: int, trigger_size, reduce_to_size):
    group = ExperimentGroup(f'ni{ni_list[-1]}_red{trigger_size}_{reduce_to_size}_sweep', 'num_intervals', None)
    
    for ni in ni_list:
        experiment = replace(
            template,
            dataset_size             = n,
            num_trials               = 3,
            uncertain_ratio          = 0.0,
            independent_variable     = 'num_intervals',
            interval_size_range      = (1, 100_000),
            start_interval_range     = (1, 1000),
            gap_size_range           = (2000, 2001),
            interval_width_range     = (1, 2),
            num_intervals            = ni,
            reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
        )

        experiment.name = format_name(experiment)
        group.experiments[experiment.name] = experiment

    return group

# ================ #
def plot_ni_gap_sweep(ni_list: int, n: int, suite_name: str = None):
    suite_name = suite_name if suite_name is not None else f'ni_sweeping{format_datasize(n)}'
    if suite_name not in experiments:
        experiments[suite_name] = ExperimentSuite(suite_name)
    
    # experiments[suite_name].add(ni_sweep(ni_list, n, 500, 250))
    # experiments[suite_name].add(ni_sweep(ni_list, n, 500, 100))
    # experiments[suite_name].add(ni_sweep(ni_list, n, 500, 10))
    # experiments[suite_name].add(ni_sweep(ni_list, n, 150, 100))
    # experiments[suite_name].add(ni_sweep(ni_list, n, 150, 10))
    # experiments[suite_name].add(ni_sweep(ni_list, n, 70, 50))
    experiments[suite_name].add(ni_sweep(ni_list, n, 15, 10))
    experiments[suite_name].add(ni_sweep(ni_list, n, 3, 1))


# n_list = make_log_sweep(1, 1000, 20)
ni_list = make_log_sweep(1, 15, 8)

plot_ni_gap_sweep(ni_list, 100)
plot_ni_gap_sweep(ni_list, 200)
# plot_ni_gap_sweep(ni_list, 400)



