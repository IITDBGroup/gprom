# external imports
import ast
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.ticker import ScalarFormatter
from matplotlib.backends.backend_pdf import PdfPages
import seaborn as sns
from pathlib import Path
from typing import List

# local imports
from DataTypes import *

class StatisticsPlotter:
    """ handles all statistical analysis and visualization of experiment results"""

    REDUCE_PARAM_NAME = 'reduce_triggerSz_sizeLim'

    def __init__(self, resultFilepath: str, seed: str):
        self.resultFilepath = resultFilepath
        self.master_seed = seed
        self.iv = None
        self.n_range_str = None
        self.param_str = None

    # ----------------------------------  
    # --- MAIN entrypoint ---
    # ----------------------------------
    def plot_experiment_suite(self, csv_results: list) -> None:
        ''' MAIN entrypoint '''

        df = self.load_all_csvs(csv_results)    
        self.set_n_range_str(df)
        self.build_param_str(df)
        iv = df['independent_variable'][0]
        self.iv = iv

        # plot stuff- uncomment for certain plots (some worse than others)
        self.plot_time_coverage_by_gap(df)
        # self.plot_reduction_heatmap(df)
        self.plot_3_row_red_vs_TimeNCover(df)
        self.plot_convergence_coverage_vs_n(df)
        self.plot_convergence_vs_gap(df)
        # self.plot_gap_vs_time_vs_result_size(df)
        # self.plot_gap_vs_time_vs_underreduction(df)
        # self.plot_efficiency_vs_ni(df)
        # self.plot_time_vs_coverage_all_ni(df)
        
        
        # self.plot_efficiency_vs_iv(df)            #uncomment this later
        self.plot_time_vs_coverage_all_iv(df)
    
        print("Results saved in: ", self.resultFilepath)

    # Not work
    def plot_experiment_group(self, group_csv_results: list, independent_variable: str) -> None:
        df = pd.read_csv(group_csv_results)
        
        self.plot_timeNaccuracy_vs_iv(df, independent_variable)
        self.run_reduce_plot_suite(df, independent_variable)
    
    def save_fig(self, fig, fname) -> str:
        outpath = f"{self.resultFilepath}/{fname}"
        fig.savefig(outpath, dpi=300, bbox_inches='tight')
        plt.close(fig)
        return outpath


    # ----------------------------------  
    # --- Plotting Code ---
    # ----------------------------------
    
    # def plot_pareto_front(self, df: pd.DataFrame)
    
    def plot_reduction_heatmap(self, df: pd.DataFrame) -> str:
        """generate heatmap for reduction parameter tuning"""
                
        # parse tuple column
        parsed = df[self.REDUCE_PARAM_NAME].apply(
            lambda x: eval(x) if isinstance(x, str) else x
        )
        df['trigger_sz'] = parsed.apply(lambda x: x[0])
        df['reduce_to_sz'] = parsed.apply(lambda x: x[1])
        
        # pivot table input for heatmap 
        sum_pivot = df.pivot_table(values='sum_time_mean', index='reduce_to_sz', columns='trigger_sz')   
        
        fig, (ax) = plt.subplots(1, 1, figsize=(12, 5))
        fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7, color='black')

        # SUM heatmap, focused on reduction params
        sns.heatmap(sum_pivot, annot=True, fmt='.1f', cmap='RdYlGn_r', ax=ax, cbar_kws={'label': 'Time (ms)'})
        ax.set_title('SUM Time Heatmap', fontsize=14, fontweight='bold')
        ax.set_xlabel('Trigger Size', fontsize=12)
        ax.set_ylabel('Reduce To Size', fontsize=12)
        
        plt.tight_layout()
        outfile = f'red_vs_time_heatmap_{self.master_seed}'
        outpath = f"{self.resultFilepath}/{outfile}"
        plt.savefig(outpath, dpi=300, bbox_inches='tight')
        plt.close()
        return outpath
    
    def plot_time_coverage_by_gap(self, df: pd.DataFrame) -> str:
        ''' plot time and coverage vs gap '''

        indep_variable = self.iv

        # Sort dataframe by dataset size
        df_sorted = df.sort_values(indep_variable)

        # Group by reduction parameters
        dfg = df_sorted.groupby(self.REDUCE_PARAM_NAME)

        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 10), sharex=True)       
        fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7, color='black')
        for p, group in dfg:
            # Make sure each group is sorted by IV
            group = group.sort_values('gap_size_range_tuple')
            x = group[indep_variable]
            y_time = group['sum_time_mean']
            y_time_err = group['sum_time_std']
            y_coverage = group['result_coverage_mean']

            # Plot time and coverage
            ax1.errorbar(x, y_time, yerr=y_time_err, fmt='o-', capsize=5, label=str(p))
            ax2.errorbar(x, y_coverage, fmt='o-', capsize=5, label=str(p))

        # TIME axis labels
        ax1.set_ylabel('Time (ms)')
        ax1.set_title(f'Time vs {indep_variable} by Reduction Parameters')
        ax1.grid(True, alpha=0.3)
        ax1.tick_params(axis='x', rotation=45)
        ax1.legend(title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)

        # COVERAGE axis labels
        ax2.set_ylabel('Coverage')
        ax2.set_xlabel(indep_variable)
        ax2.set_title(f'Coverage vs {indep_variable} by Reduction Parameters')
        ax2.grid(True, alpha=0.3)
        ax2.tick_params(axis='x', rotation=45)
        ax2.legend(title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)

        plt.tight_layout()
        outfile = f'gap_vs_time_coverage_{self.master_seed}'
        outpath = f"{self.resultFilepath}/{outfile}"
        plt.savefig(outpath, dpi=300, bbox_inches='tight')
        plt.close()
        return outpath

    def plot_3_row_red_vs_TimeNCover(self, df: pd.DataFrame) -> str:
        ''' calculates time/cover/distance vs redution params for each ni'''
        
        iv = self.iv
        if iv != 'num_intervals':
            return

        reduce_params = sorted(df['reduce_triggerSz_sizeLim_tuple'].unique())
        x_labels = [str(r) for r in reduce_params]
        n = len(df['num_intervals'].unique())

        # 0: Time vs red params for each ni
        # 1: Cover vs red params for each ni
        # 2: Distance = sqrt(time^2 + cover^2)
        # fig, axes=  plt.subplots(3, n, figsize=(6*n,12))
        fig, axes=  plt.subplots(2, n, figsize=(6*n,12))
        fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7, color='black')

        for i, ni in enumerate(sorted(df['num_intervals'].unique())):
            # plot for each ni
            df_ni = df[df['num_intervals'] == ni].copy()

            # PLot 1: time
            ax = axes[0, i]
            y = df_ni['time_norm']
            ax.plot(x_labels, y, marker='o')
            ax.set_title(f'ni={ni}')
            ax.set_xlabel('red')
            ax.tick_params(axis='x', rotation=45)
            ax.grid(True)

            # Plot 2: coverage
            ax = axes[1, i]
            y = df_ni['coverage_norm']
            ax.plot(x_labels, y, marker='o')
            ax.set_title(f'ni={ni}')
            ax.set_xlabel('red')
            ax.tick_params(axis='x', rotation=45)
            ax.grid(True)

            # # Plot 3: distance of both time and coverage
            # # normalize time and coverage to equal weight (x-min)/(max-min)
            # df_ni['time_norm'] = self.safe_normalize(df_ni['sum_time_mean'])
            # df_ni['cov_norm'] = self.safe_normalize(df_ni['result_coverage_mean'])
            # scores = (df_ni['time_norm']**2 + df_ni['cov_norm']**2)**0.5
            
            # ax = axes[2, i]
            # ax.plot(x_labels, scores, marker='o')
            # ax.set_title(f'ni={ni}')
            # ax.set_xlabel('red')
            # ax.tick_params(axis='x', rotation=45)
            # ax.grid(True)
            
        axes[0][0].set_ylabel('Time (ms)')
        axes[1][0].set_ylabel('Coverage (smaller=better)')
        # axes[2][0].set_ylabel('Distance (smaller=bettwe)')
        # handles, labels = axes[0][0][-1].get_legend_handles_labels()
        # fig.legend(handles, labels, title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)
        # fig.legend(title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)

        plt.tight_layout()
        outfile = f'distance_vs_NI_and_red_{self.master_seed}'
        outpath = f"{self.resultFilepath}/{outfile}"
        plt.savefig(outpath, dpi=300, bbox_inches='tight')
        plt.close()
        return outpath
    
    def plot_convergence_vs_n(self, df: pd.DataFrame) -> str:
        redParams = sorted(df['reduce_triggerSz_sizeLim_tuple'].unique())
    
        fig, (ax) = plt.subplots(1, 1, figsize=(12, 5))
        fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7, color='black')
        
        # for i, (triggerSz, szLimit) in enumerate(redParams):
        for i, redParamTuple in enumerate(redParams):
            subdf = df[df['reduce_triggerSz_sizeLim_tuple'] == redParamTuple]
            ax.plot(subdf['dataset_size'], subdf['minEffectiveIntervalCountMean'], marker='o', label=f'{redParamTuple}')
        
        ax.set_xlabel('Dataset Size')
        ax.set_ylabel('Interval Count')
        ax.set_title('Convergence for different triggers')
        ax.legend(title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)
        
        ax.grid(True)
        plt.tight_layout()
        # plt.show()
        outfile = f'convergence_vs_n_{self.master_seed}'
        outpath = f"{self.resultFilepath}/{outfile}"
        plt.savefig(outpath, dpi=300, bbox_inches='tight')
        plt.close(fig)
        return outpath
    
    def plot_convergence_coverage_vs_n(self, df: pd.DataFrame) -> str:
        if self.iv != 'dataset_size':
            return
        redParams = sorted(df['reduce_triggerSz_sizeLim_tuple'].unique())
        fig, axes = plt.subplots(2, 1, figsize=(12, 8))
        fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7, color='black')

        for redParamTuple in redParams:
            subdf = df[df['reduce_triggerSz_sizeLim_tuple'] == redParamTuple]
            axes[0].plot(subdf['dataset_size'], subdf['minEffectiveIntervalCountMean'], marker='o', label=str(redParamTuple))
            axes[1].plot(subdf['dataset_size'], subdf['result_coverage_mean'], marker='o', label=str(redParamTuple))

        axes[0].set_ylabel('Interval Count')
        axes[1].set_ylabel('Coverage')
        axes[1].set_xlabel('Dataset Size')
        axes[0].set_title('Convergence Interval Count')
        axes[1].set_title('Convergence Coverage')
        axes[1].legend(title='(trigger, size_limit)', loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)

        for ax in axes:
            ax.grid(True)

        return self.save_fig(fig, f'convergence_coverage_vs_n_{self.master_seed}')

    def plot_convergence_vs_gap(self, df: pd.DataFrame, filename="convergence_vs_gap.pdf") -> str:
        '''find when certain gap size ceonverge at what N '''

        filename = f"{self.resultFilepath}/{filename}"
        iv = self.iv
        if iv != 'gap_size' and iv != 'gap_size_range':
            return

        gaps = sorted(df['gap_size_range_tuple'].unique())
        reduce_params = sorted(
            df[['resizeTrigger', 'sizeLimit']]
            .drop_duplicates()
            .itertuples(index=False, name=None)
        )

        with PdfPages(filename) as pdf:
            fig, axes = plt.subplots(1, len(gaps), figsize=(6 * len(gaps), 5))
            fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7, color='black')

            if len(gaps) == 1:
                axes = [axes]
                
            for i, gap in enumerate(gaps):
                ax = axes[i]
                gap_df = df[df['gap_size_range_tuple'] == gap]

                for trigger, size_limit in reduce_params:
                    subdf = gap_df[
                        (gap_df['resizeTrigger'] == trigger) &
                        (gap_df['sizeLimit'] == size_limit)
                    ].sort_values('dataset_size')

                    if subdf.empty:
                        continue

                    ax.plot(
                        subdf['dataset_size'],
                        subdf['minEffectiveIntervalCountMean'],
                        marker='o',
                        label=f'({trigger}, {size_limit})'
                    )

                ax.set_xlabel("Dataset Size (n)")
                ax.set_ylabel("Min Effective Interval Count")
                ax.set_title(f"Gap={gap}")
                ax.grid(True)
            
            axes[-1].legend(title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)
            # plt.show()
            # outfile = f'convergence_vs_gap_{self.master_seed}'
            # outpath = f"{self.resultFilepath}/{outfile}"
            # plt.savefig(outpath, dpi=300, bbox_inches='tight')
            plt.tight_layout()
            pdf.savefig(bbox_inches='tight')
            plt.close()

            fig, axes = plt.subplots(1, len(gaps), figsize=(6 * len(gaps), 5))
            fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7, color='black')

            if len(gaps) == 1:
                axes = [axes]
                
            for i, gap in enumerate(gaps):
                ax = axes[i]
                gap_df = df[df['gap_size_range_tuple'] == gap]

                for trigger, size_limit in reduce_params:
                    subdf = gap_df[
                        (gap_df['resizeTrigger'] == trigger) &
                        (gap_df['sizeLimit'] == size_limit)
                    ].sort_values('dataset_size')

                    if subdf.empty:
                        continue

                    ax.plot(
                        subdf['dataset_size'],
                        subdf['coverage_norm'],
                        marker='o',
                        label=f'({trigger}, {size_limit})'
                    )

                ax.set_xlabel("Dataset Size (n)")
                ax.set_ylabel("coverage_normalized")
                ax.set_title(f"Gap={gap}")
                ax.grid(True)
            
            axes[-1].legend(title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)
            # plt.show()
            # outfile = f'convergence_vs_gap_{self.master_seed}'
            # outpath = f"{self.resultFilepath}/{outfile}"
            # plt.savefig(outpath, dpi=300, bbox_inches='tight')
            plt.tight_layout()
            pdf.savefig(bbox_inches='tight')
            plt.close()


            fig, axes = plt.subplots(1, len(gaps), figsize=(6 * len(gaps), 5))
            fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7, color='black')

            if len(gaps) == 1:
                axes = [axes]
                
            for i, gap in enumerate(gaps):
                ax = axes[i]
                gap_df = df[df['gap_size_range_tuple'] == gap]

                for trigger, size_limit in reduce_params:
                    subdf = gap_df[
                        (gap_df['resizeTrigger'] == trigger) &
                        (gap_df['sizeLimit'] == size_limit)
                    ].sort_values('dataset_size')

                    if subdf.empty:
                        continue

                    ax.plot(
                        subdf['dataset_size'],
                        subdf['result_coverage_mean'],
                        marker='o',
                        label=f'({trigger}, {size_limit})'
                    )

                ax.set_xlabel("Dataset Size (n)")
                ax.set_ylabel("coverage_mean")
                ax.set_title(f"Gap={gap}")
                ax.grid(True)
            
            axes[-1].legend(title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)
            # plt.show()
            # outfile = f'convergence_vs_gap_{self.master_seed}'
            # outpath = f"{self.resultFilepath}/{outfile}"
            # plt.savefig(outpath, dpi=300, bbox_inches='tight')
            plt.tight_layout()
            pdf.savefig(bbox_inches='tight')
            plt.close()
            # return outpath

    def plot_gap_vs_time_vs_result_size(self, df: pd.DataFrame) -> str:
        '''1 x n plot of time vs result_size per gap size, one point per reduction config'''
        
        gaps = sorted(df['gap_size_range_tuple'].unique())
        n_gaps = len(gaps)
    
        fig, axes = plt.subplots(1, n_gaps, figsize=(6 * n_gaps, 6), sharey=True)
        fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7)
    
        if n_gaps == 1:
            axes = [axes]

        # group by reduciton params
        for i, gap in enumerate(gaps):
            ax = axes[i]
            gap_df = df[df['gap_size_range_tuple'] == gap]
    
            grouped = gap_df.groupby(self.REDUCE_PARAM_NAME).agg(
                time=('sum_time_mean', 'mean'),
                result_size=('result_size_mean', 'mean'),
            ).reset_index()

            # scatter time vs result size with label on red params, and small annot on red param
            for _, row in grouped.iterrows():
                ax.scatter(row['time'], row['result_size'], zorder=3, label=str(row[self.REDUCE_PARAM_NAME]))
                ax.annotate(str(row[self.REDUCE_PARAM_NAME]), (row['time'], row['result_size']), textcoords="offset points", xytext=(5, 5), fontsize=7)
    
            ax.set_title(f'gap={gap}')
            ax.set_xlabel('Avg Time (ms)')
            ax.grid(True, alpha=0.3)

        axes[0].set_ylabel('Avg Result Size (# intervals)')
        axes[-1].legend(title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)
        fig.suptitle('Time vs Result Size by Gap Size\n(bottom-left = ideal)', y=1.02)

        plt.tight_layout()
        outfile = f'gap_vs_time_vs_result_size_{self.master_seed}'
        outpath = f"{self.resultFilepath}/{outfile}"
        plt.savefig(outpath, dpi=300, bbox_inches='tight')
        plt.close()
        return outpath

    def plot_gap_vs_time_vs_underreduction(self, df: pd.DataFrame) -> str:
        '''1 x n plot of time vs result_size per gap size, one point per reduction config'''
        
        gaps = sorted(df['gap_size_range_tuple'].unique())
        n_gaps = len(gaps)
    
        fig, axes = plt.subplots(1, n_gaps, figsize=(6 * n_gaps, 6), sharey=True)
        fig.text(0.5, -0.02, self.param_str, ha='center', fontsize=7)
    
        if n_gaps == 1:
            axes = [axes]

        # group by reduciton params
        for i, gap in enumerate(gaps):
            ax = axes[i]
            gap_df = df[df['gap_size_range_tuple'] == gap]


            grouped = gap_df.groupby(self.REDUCE_PARAM_NAME).agg(
                time=('sum_time_mean', 'mean'),
                result_size=('result_size_mean', 'mean'),
                size_limit=('sizeLimit', 'first'),
            ).reset_index()
            
            grouped['under_reduction'] = (grouped['size_limit'] - grouped['result_size']).clip(lower=0)
            # grouped['score'] = grouped['under_reduction'] / (grouped['time'] + 1e-9)
            grouped['under_reduction_score'] = grouped['under_reduction'] / grouped['size_limit']

            # scatter time vs result size with label on red params, and small annot on red param
            for _, row in grouped.iterrows():
                ax.scatter(row['time'], row['under_reduction_score'], zorder=3, label=str(row[self.REDUCE_PARAM_NAME]))
                ax.annotate(str(row[self.REDUCE_PARAM_NAME]), (row['time'], row['result_size']), textcoords="offset points", xytext=(5, 5), fontsize=7)
    
            ax.set_title(f'gap={gap}')
            ax.set_xlabel('Avg Time (ms)')
            # ax.invert_yaxis()
            ax.grid(True, alpha=0.3)
        axes[0].invert_yaxis()
        # axes[0].set_ylabel('Avg Result Size (# intervals)')
        axes[0].set_ylabel('Under Reduction (relative to size limit)')
        axes[-1].legend(title='(trigger, size_limit)',loc='center left', bbox_to_anchor=(1.0, 0.5), fontsize=8)
        fig.suptitle('Time vs Result Size by Gap Size\n(bottom-left = ideal)', y=1.02)

        plt.tight_layout()
        # plt.show()
        outfile = f'underreduction_{self.master_seed}'
        outpath = f"{self.resultFilepath}/{outfile}"
        plt.savefig(outpath, dpi=300, bbox_inches='tight')
        plt.close()
        return outpath

    def plot_efficiency_heatmap(self, df:pd.DataFrame):
        '''
            heatmap reduce_triggers(x) vs NI(y) 
            efficiency = combined['efficiency'] = combined['coverage_norm'] / combined['time_norm']
        '''

        pivot_eff = df.pivot_table(
            index='num_intervals',
            columns='reduce_triggerSz_sizeLim',
            values='efficiency',
            aggfunc='mean'
        )
        plt.figure(figsize=(8,5))
        sns.heatmap(pivot_eff, annot=True, fmt=".2f", cmap="coolwarm")
        plt.title("Coverage per Unit Time by Num Intervals and Reduction Config")
        plt.show()

        # pivot_eff = df.pivot_table(
        #     index='num_intervals',
        #     columns='reduce_triggerSz_sizeLim',
        #     values='efficiency',
        #     aggfunc='mean'
        # )
        # plt.figure(figsize=(8,5))
        # sns.heatmap(pivot_eff, annot=True, fmt=".2f", cmap="coolwarm")
        # plt.title("Coverage per Unit Time by Num Intervals and Reduction Config")
        # plt.show()

    def plot_efficiency_vs_ni(self, df:pd.DataFrame, filename="efficiency_vs_ni.pdf"):
        filename = f"{self.resultFilepath}/{filename}"
        with PdfPages(filename) as pdf:
            # heatmap reduce_triggers(x) vs NI(y) 
            pivot_eff = df.pivot_table(
                index='num_intervals',
                columns='reduce_triggerSz_sizeLim',
                values='efficiency',
                aggfunc='mean'
            )
            plt.figure(figsize=(6,3))
            sns.heatmap(pivot_eff, annot=True, fmt=".2f", cmap="coolwarm", annot_kws={"size": 6})
            plt.title("Efficiency (coverage_norm/time_norm)")
            plt.xlabel("red")
            plt.ylabel("ni")
            pdf.savefig(bbox_inches='tight')
            plt.close()
            
            ni_values = sorted(df['num_intervals'].unique())
            red_configs = sorted(df['reduce_triggerSz_sizeLim'].unique())
            fig, axes = plt.subplots(1, len(ni_values), figsize=(4*len(ni_values), 3))
            axes = axes.flatten() if len(ni_values) > 1 else [axes]
            fig.text(0.5, 0.05, self.param_str, ha='center', va='center', fontsize=7, color='black', wrap=True)

            handles, labels = None, None
            for i, ni in enumerate(ni_values):
                ax = axes[i]
                ni_df = df[df['num_intervals'] == ni]
                for r in red_configs:
                    sub = ni_df[ni_df['reduce_triggerSz_sizeLim'] == r]
                    ax.scatter(sub['time_norm'], sub['coverage_norm'], label=str(r), s=50)

                if handles is None:
                    handles, labels = ax.get_legend_handles_labels()

                # no scintific notation
                ax.xaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
                ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
                
                ax.set_title(f'ni={ni}')
                ax.set_xlabel('relative time')
                ax.set_ylabel('relative coverage')
                ax.grid(True, alpha=0.3)
                # ax.legend(fontsize=8)
            # fig.legend(handles,labels,title='(trigger, size_limit)',loc='center left',bbox_to_anchor=(1.02, 0.5),fontsize=8)
            plt.tight_layout(rect=[0, 0.08, 0.85, 1])
            pdf.savefig(bbox_inches='tight')
            plt.close()

            fig = plt.figure(figsize=(6,2))
            fig.legend(handles, labels, title='(trigger, size_limit)', loc='center', fontsize=8)
            pdf.savefig(bbox_inches='tight')
            plt.close()

    def plot_time_vs_coverage_all_ni(self, df, filename="time_vs_coverage_all_ni.pdf"):
        ''' plots 1 viz grouping by ni and red param. does not plot for each ni'''

        filename = f"{self.resultFilepath}/{filename}"
        agg_df = df.groupby(['num_intervals', 'reduce_triggerSz_sizeLim']).agg({
            'time_norm': 'mean',
            'coverage_norm': 'mean'
        }).reset_index()

        with PdfPages(filename) as pdf:
            fig, ax = plt.subplots(figsize=(8, 5))

            sns.scatterplot(
                data=agg_df,
                x='time_norm',
                y='coverage_norm',
                hue='reduce_triggerSz_sizeLim',
                style='num_intervals',
                s=60,
                ax=ax
            )

            ax.set_xlabel("Relative Time")
            ax.set_ylabel("Relative Coverage")
            ax.set_title("Time vs Coverage (all num_intervals)")
            ax.grid(alpha=0.3)
            ax.xaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
            ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
            ax.legend(title='ni / red', bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=7)

            plt.tight_layout()
            pdf.savefig(bbox_inches='tight')
            plt.close()

            fig, ax = plt.subplots(figsize=(8, 5))
            sns.scatterplot(
                data=agg_df,
                x='time_norm',
                y='coverage_norm',
                hue='reduce_triggerSz_sizeLim',
                # style='num_intervals',
                s=60,
                ax=ax
            )

            ax.set_xlabel("Relative Time")
            ax.set_ylabel("Relative Coverage")
            ax.set_title("Time vs Coverage (all num_intervals)")
            ax.grid(alpha=0.3)
            ax.xaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
            ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
            ax.legend(title='ni / red', bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=7)
            
            fig.text(0.5, 0.05, self.param_str, ha='center', va='center', fontsize=7, color='black', wrap=True)

            plt.tight_layout()
            pdf.savefig(bbox_inches='tight')
            plt.close()
            
    def plot_efficiency_vs_iv(self, df: pd.DataFrame, filename="efficiency_vs_iv.pdf"):
        iv = self.iv 
        filename = f"{self.resultFilepath}/{filename}"
        
        # if iv == "gap_size" or iv == "gap_size_range":
        if iv == "gap_size_range":
            iv = 'gap_size_range_tuple'

        with PdfPages(filename) as pdf:
            # heatmap: reduce_triggers(x) vs IV(y)
            pivot_eff = df.pivot_table(
                index=iv,
                columns='reduce_triggerSz_sizeLim',
                values='efficiency',
                aggfunc='mean'
            )
            plt.figure(figsize=(6,3))
            sns.heatmap(pivot_eff, annot=True, fmt=".2f", cmap="coolwarm", annot_kws={"size": 6})
            plt.title("Efficiency (coverage_norm/time_norm)")
            plt.xlabel("reduce_triggerSz_sizeLim")
            plt.ylabel(iv)
            pdf.savefig(bbox_inches='tight')
            plt.close()
            
            iv_values = sorted(df[iv].unique())
            red_configs = sorted(df['reduce_triggerSz_sizeLim'].unique())
            fig, axes = plt.subplots(1, len(iv_values), figsize=(4*len(iv_values), 3))
            axes = axes.flatten() if len(iv_values) > 1 else [axes]
            fig.text(0.5, 0.05, self.param_str, ha='center', va='center', fontsize=7, color='black', wrap=True)

            handles, labels = None, None
            for i, val in enumerate(iv_values):
                ax = axes[i]
                sub_df = df[df[iv] == val]
                for r in red_configs:
                    r_df = sub_df[sub_df['reduce_triggerSz_sizeLim'] == r]
                    ax.scatter(r_df['time_norm'], r_df['coverage_norm'], label=str(r), s=50)

                if handles is None:
                    handles, labels = ax.get_legend_handles_labels()

                ax.set_title(f'{iv}={val}')
                ax.set_xlabel('relative time')
                ax.set_ylabel('relative coverage')
                ax.xaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
                ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
                ax.grid(True, alpha=0.3)

            plt.tight_layout(rect=[0, 0.08, 0.85, 1])
            pdf.savefig(bbox_inches='tight')
            plt.close()

            fig = plt.figure(figsize=(6,2))
            fig.legend(handles, labels, title='(trigger, size_limit)', loc='center', fontsize=8)
            pdf.savefig(bbox_inches='tight')
            plt.close()

    def plot_time_vs_coverage_all_iv(self, df, filename="time_vs_coverage_all_rv.pdf"):
        ''' plots 1 viz grouping by ni and red param. does not plot for each ni'''

        iv = self.iv
        # if iv == "gap_size" or iv == "gap_size_range":
        if iv == "gap_size_range":
            iv = 'gap_size_range_tuple'

        filename = f"{self.resultFilepath}/{filename}"
        agg_df = df.groupby([iv, 'reduce_triggerSz_sizeLim']).agg({
            'time_norm': 'mean',
            'coverage_norm': 'mean'
        }).reset_index()

        with PdfPages(filename) as pdf:
            fig, ax = plt.subplots(figsize=(8, 5))

            sns.scatterplot(
                data=agg_df,
                x='time_norm',
                y='coverage_norm',
                hue='reduce_triggerSz_sizeLim',
                style=iv,
                s=60,
                ax=ax
            )

            ax.set_xlabel("Relative Time")
            ax.set_ylabel("Relative Coverage")
            ax.set_title("Time vs Coverage (all num_intervals)")
            ax.grid(alpha=0.3)
            ax.xaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
            ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
            ax.legend(title='iv / red', bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=7)

            plt.tight_layout()
            pdf.savefig(bbox_inches='tight')
            plt.close()

            fig, ax = plt.subplots(figsize=(8, 5))
            sns.scatterplot(
                data=agg_df,
                x='time_norm',
                y='coverage_norm',
                hue='reduce_triggerSz_sizeLim',
                # style='num_intervals',
                s=60,
                ax=ax
            )

            ax.set_xlabel("Relative Time")
            ax.set_ylabel("Relative Coverage")
            ax.set_title("Time vs Coverage (all num_intervals)")
            ax.grid(alpha=0.3)
            ax.xaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
            ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=False, useOffset=False))
            ax.legend(title='iv / red', bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=7)
            
            fig.text(0.5, 0.05, self.param_str, ha='center', va='center', fontsize=7, color='black', wrap=True)

            plt.tight_layout()
            pdf.savefig(bbox_inches='tight')
            plt.close()

    # ----------------------------------  
    # --- helpers ---
    # ----------------------------------
    def get_dataset_size_bounds(self, df: pd.DataFrame) -> str:
        sizes = sorted(df['dataset_size'].unique())
        min_n = sizes[0]
        max_n = sizes[-1]
        if len(sizes) > 1:
            step_n = sizes[1] - sizes[0]
        else:
            step_n = 0
        
        # return f"{min_n}..{max_n} step {step_n}"
        return f"{min_n}..{max_n}"

    def set_n_range_str (self, df):
            self.n_range_str = self.get_dataset_size_bounds(df)

    def load_all_csvs(self, csv_paths: List[str]) -> pd.DataFrame:
        """Load and combine multiple experiment CSVs. Do simple data processing for later convenience"""
        dfs = []
        for path in csv_paths:
            df = pd.read_csv(path, index_col=0)
            df['source_file'] = Path(path).parent.name  # track which experiment it came from
            dfs.append(df)
        
        combined = pd.concat(dfs, ignore_index=True)
        
        # add in diff representations of red params for convenience
        combined[['resizeTrigger', 'sizeLimit']] = combined['reduce_triggerSz_sizeLim'].str.strip('()').str.split(',', expand=True).astype(int)
        combined['gap_size_range_tuple'] = combined['gap_size_range'].apply(ast.literal_eval)
        combined['reduce_triggerSz_sizeLim_tuple'] = combined['reduce_triggerSz_sizeLim'].apply(ast.literal_eval)
        combined['time_norm'] = combined['sumMetrics_time_mean'] / combined['sumMetrics_time_mean'].max()
        combined['coverage_norm'] = combined['result_coverage_mean'] / combined['result_coverage_mean'].max()
        combined['efficiency'] = combined['coverage_norm'] / combined['time_norm']
        return combined

    def build_dist_str(self, df) -> str:
        dist = df['distribution'].iloc[0]
        if isinstance(dist, str):
            dist = DistributionType[dist.split('.')[-1]]  # "DistributionType.ZIPFIAN" -> DistributionType.ZIPFIAN


        if dist == DistributionType.NORMAL:
            return (f'normal(pos_mean={df['pos_mean'].iloc[0]}, pos_std={df['pos_std'].iloc[0]}, '
                    f'width_mean={df['width_mean'].iloc[0]}, width_std={df['width_std'].iloc[0]})')
        elif dist == DistributionType.ZIPFIAN:
            return (f'zipfian(pos_a={df['pos_zipf_a'].iloc[0]}, width_a={df['width_zipf_a'].iloc[0]})')
        elif dist == DistributionType.UNIFORM:
            return (f'uniform')
        elif dist == DistributionType.CLUSTERED:
            return (f'clustered(pos_clusters={df['pos_n_clusters'].iloc[0]}, pos_spread={df['pos_cluster_spread'].iloc[0]}, '
                    f'width_clusters={df['width_n_clusters'].iloc[0]}, width_spread={df['width_cluster_spread'].iloc[0]})')
        else:
            return 'distribution=unknown'
    
    def build_param_str(self, df) -> str:
        self.param_str = (
            f' | iv={sorted(df["independent_variable"].unique())} | '
            f'n={self.n_range_str} | '
            f'start={sorted(df["start_interval_range"].unique())} | '
            f'dist= {self.build_dist_str(df)}| '
            f'gaps={sorted(df["gap_size_range"].unique())} | '
            f'widths={sorted(df["interval_width_range"].unique())} | '
            f'uncert={sorted(df["uncertain_ratio"].unique())} | '
            f'dataPath={self.resultFilepath} | '
            f'seed={self.master_seed} | '
        )