"""
    Contains all code regarding the CLI utility of the test engine.
    Contains all code regarding db config file parsing.
"""

from __future__ import annotations
import time
import argparse
import yaml
from configparser import ConfigParser
import os

from DataTypes import DataType
from main import ExperimentSettings, ExperimentGroup

def find(name, path):
    for root, dirs, files in os.walk(path):
        if name in files:
            return os.path.join(root, name)

def load_config(filename='database.ini', section='postgresql'):
    filepath = find(filename, '.')
    
    parser = ConfigParser()
    parser.read(filepath)

    config = {} 
    if parser.has_section(section):
        params = parser.items(section)
        for param in params:
            config[param[0]] = param[1]
    else:
        raise Exception(f'Section {section} not found in the {filename} file in {filepath}')
    return config

def positive_int(value):
        x = int(value)
        if x < 0:
            raise argparse.ArgumentTypeError("Must be positive")
        return x

def parse_args():
    '''
        creates and parses all the cli flags avaliable
    '''
    parser = argparse.ArgumentParser(
        description="AUDB Extension Experiment Runner", )

    exp_group = parser.add_mutually_exclusive_group(required=True)
    exp_group.add_argument(
        '-xf', '--yaml-experiments-file',
        type=str,
        help="YAML file with defined experiments"
    )
    exp_group.add_argument(
        '--quick',
        action='store_true',
        help="Quick run experiment fully defined with CLI flags"
    )
    exp_group.add_argument(
        '-py', '--code',
        type=str,
        help=".py file with defined experiments"
    )

    quick_group = parser.add_argument_group("Quick experiment options (must use --quick)")
    # experiment settings
    quick_group.add_argument(
        '-dt', '--data-type',
        choices=[ 'r', 's', 'range', 'set'],
        default='range',
        help='Data Type: range or set (default=range == r)'
    )
    quick_group.add_argument(
        '-nt', '--num-trials',
        type=positive_int,
        default=4,
        help='Number of trials (default=4)'
    )
    quick_group.add_argument(
        '-sz', '--dataset-size',
        type=positive_int,
        default=100,
        help='Dataset size/ Number rows. (default=100)'
    )
    quick_group.add_argument(
        '-ur', '--uncertainty-ratio',
        type=float,
        default=0.30,
        help='Uncertainty Ratio 0.0 - 1.0 (default=0.3)'
    )
    quick_group.add_argument(
        '-ni', '--num-intervals',
        type=positive_int,
        required=False,
        help='Fixed number of intervals in each Set'
    )
    quick_group.add_argument(
        '-gs', '--gap-size',
        type=positive_int,
        required=False,
        help='Fixed gap size between intervals'
    )
    quick_group.add_argument(
        '-nir', '--num-intervals-range',
        required=False,
        type=positive_int,
        nargs=2,
        help='Bounds for possible number of intervals in each Set. Ex: -nir lb ub'
    )
    quick_group.add_argument(
        '-gsr', '--gap-size-range',
        required=False,
        type=positive_int,
        nargs=2,
        help='Bounds for possible gap size between intervals Set. Ex: -gsr lb ub'
    )
    quick_group.add_argument(
        '-msr', '--mult-size-range',
        required=False,
        default=(1,5),
        type=positive_int,
        nargs=2,
        help='Bounds for possible multiplicity range. Ex: -msr lb ub (Default=(1,5))'
    )
    quick_group.add_argument(
        '-isr', '--interval-size-range',
        required=False,
        default=(1,100),
        type=positive_int,
        nargs=2,
        help='Bounds for possible interval size. Ex: -isr a b (Default=(1,100))'
    )
    quick_group.add_argument(
        '-rts', '--reduce-trigger-size',
        required=False,
        default=(10,10),
        type=positive_int,
        nargs=2,
        help='Parameters for reduction helper used in sum etc'
    )
    
    # output options
    quick_group.add_argument(
        '-csv', '--save_csv',
        required=False,
        type=str,
        default='data',
        help='Directory for output files (default: data/)'
    )

    quick_group.add_argument(
        '-ddl', '--save_ddl',
        required=False,
        action='store_true',
        default=False,
        help='Directory for DDL code (default: data/)'
    )

    # database options
    quick_group.add_argument(
        '-dbc', '--dbconfig',
        type=str,
        default='database.ini',
        help='Database configuration file. (*.ini) (Default=database.ini)'
    )
    quick_group.add_argument(
        '-idb', '--insert-to-db',
        action='store_true',
        default=False,
        help='Insert data into Database. (Default=False)'
    )
    quick_group.add_argument(
        '-cb', '--clean-before',
        nargs='?',
        const='t_%',  
        default=None,
        help="Clean existing tables before running. Usage: --clean-before [PATTERN] (Default pattern: t_*)"
    )
    quick_group.add_argument(
        '-ca', '--clean-after',
        nargs='?',
        const='t_%',
        default=None,
        help="Clean existing tables after running. Usage: --clean-after [PATTERN] (Default pattern: t_*)"
    )
    
    # general settings
    quick_group.add_argument(
        '-m', '--mode',
        nargs='?',
        choices=[ 'generate-data', 'run-tests', 'all' ],
        default='all',
        help='Select what and what not to run. Usage: --mode [generate-data run-tests...] (Default=all)'
    )
    quick_group.add_argument(
        '-q', '--quiet',
        action='store_true',
        default='False',
        help='Quiet mode. Minimal Console output'
    )
    quick_group.add_argument(
        '-s', '--seed',
        type=int,
        default=None,
        help='Seed used to generate pseudo-randomness.'
    )


    return parser.parse_args()


def create_quick_experiment(args: argparse.Namespace) -> dict:
    '''
        parses the arguments from CLI into a single item dict {exp_nameTS: ExperimentSettings}
    '''
    name = f"quick_{time.strftime('%Y%m%d_%H%M%S')}"
    experiment = ExperimentSettings(
        name=name,
        data_type= DataType.RANGE if args.data_type=='range' or 'r' else DataType.SET,
        num_trials=args.num_trials,
        dataset_size=args.dataset_size,
        uncertain_ratio=args.uncertainty_ratio,
        num_intervals=args.num_intervals,
        gap_size=args.gap_size,
        num_intervals_range=args.num_intervals_range,
        gap_size_range=args.gap_size_range,
        mult_size_range=args.mult_size_range,
        interval_size_range=args.interval_size_range,
        save_csv=args.save_csv,
        save_ddl= args.save_ddl,
        # insert_to_db=args.insert_to_db,
        mode=args.mode,
        reduce_triggerSz_sizeLim=args.reduce_triggerSz_sizeLim,
    )

    return {name: experiment}

def load_experiments_from_file(filename: str) -> dict:
    """
    load experiments from YAML file
    returns: dict of {group_name: ExperimentGroup}
    
    Expected YAML structure:

    experiment_groups:
      - group_name: dataset_size_study
        independent_variable: dataset_size
        experiments:
          - name: small
            data_type: SET
            dataset_size: 1000
            ...
          - name: medium
            data_type: SET
            dataset_size: 10000
            ...
    """
    filepath = find(filename, '.')
    with open(filepath, 'r') as file:
        config = yaml.safe_load(file)

    experiments = {}    

    for group_config in config.get('experiment_groups', []):

        group_name = group_config['group_name']
        independent_variable = group_config.get('independent_variable', 'dataset_size')

        ExpGroup = ExperimentGroup(group_name, independent_variable, None)

        for exp_config in group_config.get('experiments', []):
            name = exp_config.get('name', 'unnamed')
        
            if exp_config['data_type'] == 'RANGE':
                datatype = DataType.RANGE 
            elif exp_config['data_type'] == 'SET':
                datatype = DataType.SET 
            else:
                raise Exception(f'Invalid DataType in {filepath}. View DataTypes.py.')

            experiment = ExperimentSettings(
                name=name, 
                data_type=datatype,
                num_trials=exp_config.get('num_trials', 1),
                dataset_size=exp_config.get('dataset_size', 100),
                uncertain_ratio=exp_config.get('uncertain_ratio', 0.0),

                num_intervals=exp_config.get('num_intervals', None),
                gap_size=exp_config.get('gap_size', None),
                
                mult_size_range=tuple(exp_config.get('mult_size_range', None)),
                interval_size_range=tuple(exp_config.get('interval_size_range', (1,100))),
                num_intervals_range=tuple(exp_config['num_intervals_range']) if 'num_intervals_range' in exp_config else None,
                gap_size_range=tuple(exp_config['gap_size_range']) if 'gap_size_range' in exp_config else None,

                save_ddl=exp_config.get('save_ddl', False),
                save_csv=exp_config.get('save_csv', False),
                mode=exp_config.get('mode', 'all'),
                                reduce_triggerSz_sizeLim=tuple(exp_config.get('reduce_triggerSz_sizeLim', (10, 10))),
                independent_variable=independent_variable,
            )

            ExpGroup.experiments[experiment.name] = experiment
            
        experiments[group_name] = ExpGroup

    return experiments