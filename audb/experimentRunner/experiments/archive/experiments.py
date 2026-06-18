from cliUtility import *
from DataTypes import *
from numerize import numerize
from dataclasses import replace
from main import format_datasize, format_name, ExperimentGroup


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
        reduce_triggerSz_sizeLim=(10, 5),)


########################
# gap size
########################
# gap_size_range_t2 = ExperimentGroup('gap_size_range_experiments', 'gap_size_range', None)
# for i in range(0, 500, 50):
#     experiment = replace(
#         template,
#         independent_variable='gap_size_range',
#         num_trials=5,
#         dataset_size=10000,
        
#         # Key fixes:
#         interval_size_range=(0, 5000),          # Larger bounds so intervals fit
#         start_interval_range=(0, 500),          # Let starts vary per row
#         gap_size_range=(i, i + 100),            # Variable gaps: [0,100), [100,200), etc.
#         interval_width_range=(50, 150),         # Some width variation
#         num_intervals=5,                        # Consistent number
#     )
#     experiment.name = format_name(experiment)
#     gap_size_range_t2.experiments[experiment.name] = experiment

# experiments['gap_size_range2'] = gap_size_range_t2


########################
# dataset size
########################
# dataset_size_t1 = ExperimentGroup('gap_size_range_experiments', 'dataset_size', None)
# for i in range(0, 1_000_000, 100_000):
#     experiment = replace(
#         template,
#         independent_variable='dataset_size',
#         num_trials=5,
#         dataset_size=i,
        
#         interval_size_range=(0, 5000),
#         start_interval_range=(0, 500),
#         gap_size_range=(0, 100),     
#         interval_width_range=(50, 150),
#         num_intervals=5,
#     )
#     experiment.name = format_name(experiment)
#     dataset_size_t1.experiments[experiment.name] = experiment

# experiments['dataset_size_t1'] = dataset_size_t1


########################
# number of intervals
########################
# num_intervals_t1 = ExperimentGroup('gap_size_range_experiments', 'num_intervals', None)
# for i in range(1, 10):
#     experiment = replace(
#         template,
#         independent_variable='num_intervals',
#         num_trials=5,
#         dataset_size=100_000,
#         num_intervals = i,

#         interval_size_range=(0, 5000),
#         start_interval_range=(0, 500),
#         gap_size_range=(0, 100),
#         interval_width_range=(50, 150),
#     )
#     experiment.name = format_name(experiment)
#     num_intervals_t1.experiments[experiment.name] = experiment

# experiments['num_intervals_t1'] = num_intervals_t1


# #######################
# ## uncertain ratio as N increases
# #######################
# for size in range(100_000, 500_000, 100_000):
#     group_name = f'uncert_ratio_n{format_datasize(size)}'
#     group = ExperimentGroup(group_name, 'uncertain_ratio', None)

#     # effect of scaling uncertainty
#     for unc in [x / 100 for x in range(0, 100, 5)]:
#         experiment = replace(
#             template,
#             uncertain_ratio = unc, 
#             dataset_size = size,
#             independent_variable = 'uncertain_ratio',
#             num_trials = 5,

#             interval_size_range=(0, 5000),
#             gap_size_range=(0,100),
#             start_interval_range=(0,500),
#             interval_width_range=(50,150),
#             num_intervals=5
#         )
    
#         experiment.name = format_name(experiment)
#         group.experiments[experiment.name] = experiment

#     experiments[group_name] = group




######################
# reduce_triggerSz_sizeLim tuning
######################
# reduction_tuning = ExperimentGroup('reduction_param_tuning2', 'reduce_triggerSz_sizeLim', None)
# for trigger_size in range(1, 50, 1):
#     for reduce_to_size in range(1, 50, 1):
#         if trigger_size - reduce_to_size  > 10:
#             experiment = replace(
#                 template,
#                 dataset_size = 1000,
#                 num_trials = 5,
#                 uncertain_ratio = 0.0, 
#                 independent_variable = 'reduce_triggerSz_sizeLim',
#                 reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
#                 interval_size_range=(0, 5000),
#                 gap_size_range=(0,100),
#                 start_interval_range=(0,500),
#                 interval_width_range=(50,150),
#                 num_intervals=4
#             )
        
#             experiment.name = format_name(experiment)
#             reduction_tuning.experiments[experiment.name] = experiment

# experiments['reduction_param_tuning2'] = reduction_tuning



# ######################
# # WIDE reduce_triggerSz_sizeLim tuning
# ######################
# reduction_tuning = ExperimentGroup('reduction_param_tuningWIDE', 'reduce_triggerSz_sizeLim', None)
# # for trigger_size in range(1, 50, 1):
# #     for reduce_to_size in range(1, 50, 1):
# for trigger_size in range(1, 20, 1):
#     for reduce_to_size in range(1, 20, 1):
#         if trigger_size - reduce_to_size  >= 10:
#         # if trigger_size > reduce_to_size:
#             experiment = replace(
#                 template,
#                 dataset_size = 1000,
#                 num_trials = 1,
#                 uncertain_ratio = 0.0, 
#                 mult_size_range=(1,2),
#                 independent_variable = 'reduce_triggerSz_sizeLim',
#                 reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
#                 interval_size_range=(0, 500_000),
#                 gap_size_range=(25000,100_000),
#                 start_interval_range=(1, 10000),
#                 interval_width_range=(1, 100),
#                 num_intervals=5
#             )
#             experiment.name = format_name(experiment)
#             reduction_tuning.experiments[experiment.name] = experiment

# experiments['reduction_param_tuning2'] = reduction_tuning

# # GOOD
# ######################
# # WIDE reduce_triggerSz_sizeLim tuning
# ######################
# reduction_tuning = ExperimentGroup('reduction_param_tuningWIDE', 'reduce_triggerSz_sizeLim', None)
# # for trigger_size in range(1, 50, 1):
# #     for reduce_to_size in range(1, 50, 1):
# for trigger_size in range(1, 20, 1):
#     for reduce_to_size in range(1, 20, 1):
#         if trigger_size - reduce_to_size  >= 10:
#         # if trigger_size > reduce_to_size:
#             experiment = replace(
#                 template,
#                 dataset_size = 500,
#                 num_trials = 1,
#                 uncertain_ratio = 0.0, 
#                 mult_size_range=(1,2),
#                 independent_variable = 'reduce_triggerSz_sizeLim',
#                 reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
#                 interval_size_range=(0, 500_000),
#                 gap_size_range=(25000,100_000),
#                 start_interval_range=(1, 10000),
#                 interval_width_range=(1, 100),
#                 num_intervals=5
#             )
#             experiment.name = format_name(experiment)
#             reduction_tuning.experiments[experiment.name] = experiment

# experiments['reduction_param_tuning2'] = reduction_tuning


# # GOOD
# # WIDE reduce_triggerSz_sizeLim tuning
# ######################
# reduction_tuning = ExperimentGroup('reduction_param_tuningQUAD', 'reduce_triggerSz_sizeLim', None)
# # for trigger_size in range(1, 50, 1):
# #     for reduce_to_size in range(1, 50, 1):
# for ni in range(1,5, 1):
#     for trigger_size in range(1, 5, 1):
#         for reduce_to_size in range(1, 5, 1):
#             # if trigger_size - reduce_to_size  > 15:
#             if trigger_size > reduce_to_size:
#                 experiment = replace(
#                     template,
#                     dataset_size = 100,
#                     num_trials = 5,
#                     uncertain_ratio = 0.0, 
#                     independent_variable = 'reduce_triggerSz_sizeLim',
#                     reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
                    
#                     interval_size_range  = (1, 2_000_000_000),
#                     start_interval_range = (1, 1_000),
#                     gap_size_range       = (500_000, 2_000_000),
#                     interval_width_range = (50, 200),
#                     num_intervals        = 5,  
#                 )

#                 experiment.name = format_name(experiment)
#                 reduction_tuning.experiments[experiment.name] = experiment

#     experiments['reduction_param_tuningWIDE'] = reduction_tuning



# step 1, find, sweep thru and find good numberof intervals to work with
# ni_sweep = ExperimentGroup('ni_config', 'number_intervals', None)

# for ni in range(1, 10):
#     experiment = replace(
#         template,
#         dataset_size             = 100,
#         num_trials               = 3,
#         uncertain_ratio          = 0.0,
#         independent_variable     = 'num_intervals',
#         reduce_triggerSz_sizeLim = (5, 1), 
#         interval_size_range      = (1, 2_000_000_000),
#         start_interval_range     = (1, 1_000),
#         gap_size_range           = (500_000, 2_000_000),
#         interval_width_range     = (50, 200),
#         num_intervals            = ni,
#     )
#     experiment.name = f"ni{ni}_redSz5_1"
#     ni_sweep.experiments[experiment.name] = experiment

# experiments['ni_viability'] = ni_sweep

# ni_sweep = ExperimentGroup('ni_viability', 'num_intervals', None)

# n_sweep = ExperimentGroup('ni_config', 'reduce_triggerSz_sizeLim', None)
# for n in range(100_000, 1_100_000, 100_000):
#     # for trigger_size in range(1, 10):
#     #     for reduce_to_size in range(1, 10):
#             # if trigger_size > reduce_to_size:
#                 # continue
#             experiment = replace(
#                 template,
#                 dataset_size             = n,
#                 num_trials               = 3,
#                 uncertain_ratio          = 0.0,
#                 independent_variable     = 'reduce_triggerSz_sizeLim',
#                 interval_size_range      = (1, 2_000_000_000),
#                 start_interval_range     = (1, 1_000),
#                 gap_size_range           = (500_000, 2_000_000),
#                 interval_width_range     = (50, 200),
#                 num_intervals            = 2,
#                 reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
#             )
#             experiment.name = f"ni{ni}_redSz{trigger_size}_{reduce_to_size}"
#             ni_sweep.experiments[experiment.name] = experiment

# experiments['ni_config'] = ni_sweep


# n_sweep = ExperimentGroup('n_sweep', 'dataset_size', None)
# for n in range(1000, 11000, 1000):
#     for trigger_size in range(1, 10):
#         for reduce_to_size in range(1, 10):
#             if trigger_size > reduce_to_size:
#                 continue
            
#             experiment = replace(
#                 template,
#                 dataset_size             = n,
#                 num_trials               = 1,
#                 uncertain_ratio          = 0.0,
#                 independent_variable     = 'dataset_size',
#                 interval_size_range      = (1, 1_000),
#                 start_interval_range     = (1, 2),
#                 gap_size_range           = (1000, 3000),
#                 interval_width_range     = (2, 50),
#                 num_intervals            = 2,
#                 reduce_triggerSz_sizeLim = (trigger_size, reduce_to_size),
#             )
#             experiment.name = f"n{n}_redSz{trigger_size}_{reduce_to_size}"
#             n_sweep.experiments[experiment.name] = experiment

# experiments['ni_config'] = n_sweep




