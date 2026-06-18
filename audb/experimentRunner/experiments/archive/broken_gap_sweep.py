from dataclasses import replace

from cliUtility import *
from DataTypes import *
from main import ExperimentGroup, format_datasize, format_name, ExperimentSuite

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

def build_gap_experiment(n, g, ni, trigger_size, reduce_to_size, start_range = 5, gap_width = 1, interval_width:tuple = (5,6)) -> ExperimentSettings:
    experiment = replace(
                template,
                dataset_size             = n,
                num_trials               = 5,
                uncertain_ratio          = 0.0,
                independent_variable     = 'gap_size_range',
                interval_size_range      = (1, 100_000),
                start_interval_range     = (1, start_range),
                gap_size_range           = (g, g + gap_width),  # fix the gap size
                interval_width_range     = interval_width,
                num_intervals            = ni,
                reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
            )
            # experiment.name = f"{format_name(experiment)}_g{g}"  # make sure gap_size is in name to avoid overwrite
    experiment.name = f"{format_name(experiment)}_g{g}_ni{ni}_n{n}_r{trigger_size}_{reduce_to_size}"

    return experiment

def ni_gap_sweep_dynamic(iv, gap_sizes: list, ni_list: list, n_list: list, red_list: list, suite_name: str = None):
    groups = []
    
    for trigger_size, reduce_to_size in red_list:
        # make unique name within Suite    
        group = ExperimentGroup(f'ni_gap_red{trigger_size}_{reduce_to_size}_sweep', iv, None)

        # param sweep
        for n in n_list:
            for g in gap_sizes:
                for ni in ni_list:
                    experiment = build_gap_experiment(n, g, ni, trigger_size, reduce_to_size)

                    group.experiments[experiment.name] = experiment
        
        groups.append(group)
    return groups

def plot_ni_gap_sweep_dynamic(gap_sizes: list, ni_list: list, n_list: list, red_list: list, iv = 'gap_size_range', suite_name: str = None):
    suite_name = suite_name if suite_name is not None else f'ni_gap_sweeping_n{format_datasize(n_list[-1])}'
    if suite_name not in experiments:
        experiments[suite_name] = ExperimentSuite(suite_name)
    
    groups = ni_gap_sweep_dynamic(iv, gap_sizes, ni_list, n_list, red_list)
    
    for group in groups:
        experiments[suite_name].add(group)

## ============================== ##
gap_sizes = [10, 50, 100, 250, 500, 1000]
red_list = [(15, 10), (10, 5), (4, 2), (9, 3), (5, 2), (1, 1), (3,1)]
ni_list = [i for i in range(11)]
n_list = []
for i in range(1, 51):
    if i <= 40:
        n_list.append(i * 5)
    else:
        n_list.append(n_list[-1] + 200)

plot_ni_gap_sweep_dynamic(gap_sizes, ni_list, n_list, red_list)








# def plot_ni_gap_sweep(max_ni: int, n_list: list, suite_name: str = None):
#     suite_name = suite_name if suite_name is not None else f'ni_gap_sweeping{format_datasize(n_list[-1])}'
#     if suite_name not in experiments:
#         experiments[suite_name] = ExperimentSuite(suite_name)
    
#     gap_sizes = [10, 50, 100, 250, 500, 1000]
#     red_list = [(15, 10), (10, 5), (4, 2), (9, 3), (5, 2), (1, 1), (3,1)]
#     ni_list = [i for i in range(1,11)]
#     experiments[suite_name].add(ni_gap_sweep(gap_sizes, n_list,  ni_list, red_list))
#     # experiments[suite_name].add(ni_gap_sweep(gap_sizes, n_list,  ni_list, red_list))
#     # experiments[suite_name].add(ni_gap_sweep(gap_sizes, n_list,  ni_list, red_list))
#     # experiments[suite_name].add(ni_gap_sweep(gap_sizes, n_list,  ni_list, red_list))
#     # experiments[suite_name].add(ni_gap_sweep(gap_sizes, n_list,  ni_list, red_list))
#     # experiments[suite_name].add(ni_gap_sweep(gap_sizes, n_list,  ni_list, red_list))
#     # experiments[suite_name].add(ni_gap_sweep(gap_sizes, max_ni, n, 1, 1))


# def ni_gap_sweep(gap_sizes: list, n_list: list, max_ni: list, red_list: list):

#     for trigger_size, reduce_to_size in red_list:
#         group = ExperimentGroup(f'ni_gap_red{trigger_size}_{reduce_to_size}_sweep', 'num_intervals', None)

#         for n in n_list:
#             for g in gap_sizes:
#                 # for ni in range(1, max_ni+1):
#                     ni = 5
#                     experiment = replace(
#                         template,
#                         dataset_size             = n,
#                         num_trials               = 1,
#                         uncertain_ratio          = 0.0,
#                         independent_variable     = 'gap_size_range',
#                         interval_size_range      = (1, 100_000),
#                         start_interval_range     = (1, 5),
#                         gap_size_range           = (g, g+1),  # fix the gap size
#                         # gap_size                 = g,
#                         interval_width_range     = (5, 6),
#                         num_intervals            = ni,
#                         reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
#                     )
#                     # experiment.name = f"{format_name(experiment)}_g{g}"  # make sure gap_size is in name to avoid overwrite
#                     experiment.name = f"{format_name(experiment)}_g{g}_ni{ni}_n{n}_r{trigger_size}_{reduce_to_size}"
#                     group.experiments[experiment.name] = experiment
        
#     return group

# n_list = []
# for i in range(1, 50):
#     if i <= 40:
#         n_list.append(i)
#     else:
#         n_list.append(n_list[-1] + 200)

# plot_ni_gap_sweep(2, n_list)