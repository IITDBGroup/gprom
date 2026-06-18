   # Overview

   ## Prerequisites
   1. Docker container with extension installed. (see root README)
   2. Dependencies
   
      ```
      python3 -m venv .venv
      source .venv/bin/activate 
      pip install --upgrade pip
      pip install -r configs/requirements.txt
      ```
   3. If modified Docker Compose, update configs/database.ini to match your setup.compose, change configs/database.ini. Otherwise ignore

   
   **Entrypoint:** `main.py`

   **OuputPath** `/data/results/<SEED>/<SuiteName>/`

   ### User workflow
   1. Create an experiment config file  
      - `.py` (recommended, most flexible)
      - `.yaml` (simpler for static configs)
   2. Run the script using CLI flags

   ### internal workflow:
   generate data -> insert into DB -> run queries -> collect data -> write to csv-> (eventually make optional) create plots (StatisticsPlotter.py)


   ### 3 main classes:
   - ExperimentSuite - container containing many releted groups of experiments
   - ExperimentGroup - contains set of experiments 
   - ExperimentSettings - individual representation of experiment. Where all the IV's are.

   Assuming the CLI -py mode is used and an experiment procedure/config format similar to any of the \*.py examples in experimentRunner/experiments/active, the main data structure is a nested dict: 'ExperimentSuiteName': ExperiementSuite.
   ```
   experiments (dict)
   |-- ExperimentSuite
         |-- groups (dict)
               |-- ExperimentGroup
                     |-- experiments (dict)
                           |-- ExperimentSettings
   ```


   # Quickstart
   ### Python file/module (recommended)
   
   - Cli Flag Usage

      `python3 main.py -h
   `

   
   - Create and Run gap_sweeping experiment suite. Extends to any suite

      `python main.py -py experiments/active/gap_sweeping.py 
   `

   - Create and Run your experiment suite and drop all tables ("t_\*") in pg before.

      `python main.py -py experiments/active/<your_file>.py  -cb       # cb takes optional str input
   `

   ### YAML file
   `python main.py --yaml experiments/active/yourExperiment.yaml
   `

   ### Clean leftover tables before run (recommended)
   `python main.py --code ... --cb "t_%"
   `

   ## Suggested Workflow
   * editor open at certain experiment config file `../experimentRunner/experiments/active/<your_file>`
   * 1 terminal connected to containerized database for db debugging and 1-off experiments
   * 1 terminal at `../experimentRunner` running `experimentRunner/experiments/active/<your_file>`
   * If self analysis: Jupyter Notebooks open in browser


   # Improvements
   - plotting 
   - experiment creation files need work 
   - Statistics side of things. Struggle on multidimensional analysis
