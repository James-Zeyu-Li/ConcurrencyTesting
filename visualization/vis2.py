import os
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Suppress warnings for clarity
import warnings
warnings.simplefilter(action='ignore', category=FutureWarning)

# Set Seaborn theme
sns.set(style="whitegrid")

# Define paths
base_dir = os.path.abspath(os.path.join(
    os.path.dirname(__file__), '..', 'Build'))
visualization_dir = os.path.abspath(os.path.join(
    os.path.dirname(__file__), '..', 'Visualization'))

# Ensure visualization directory exists
os.makedirs(visualization_dir, exist_ok=True)

# Helper function to save plots


def save_plot(fig, csv_filename, plot_number):
    plot_name = f"{os.path.splitext(csv_filename)[0]}_{plot_number}.png"
    save_path = os.path.join(visualization_dir, plot_name)
    fig.savefig(save_path, bbox_inches='tight')
    print(f"Saved plot: {save_path}")

# Visualization for IO data


def generate_plots_for_io(df, csv_filename):
    plot_number = 1
    df['TotalThreads'] = df['ConsumerCount'] + df['ReaderCount']

    # 1. TotalThreads vs TotalTime
    fig1 = plt.figure(figsize=(14, 8))
    sns.scatterplot(data=df, x='TotalThreads', y='TotalTime(us)', hue='LockType',
                    style='LockType', size='OperationCount', sizes=(50, 300), alpha=0.7)
    for i, row in df.iterrows():
        plt.text(row['TotalThreads'], row['TotalTime(us)'],
                 f"C:{row['ConsumerCount']}\nR:{row.get('ReaderCount', '-')}", fontsize=8, ha='right', alpha=0.7)
    plt.title('Impact of Total Threads on TotalTime(us)', fontsize=16)
    plt.xlabel('Total Threads (Consumer + Reader)', fontsize=14)
    plt.ylabel('Total Time (us)', fontsize=14)
    plt.legend(title='LockType / OperationCount',
               bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    save_plot(fig1, csv_filename, plot_number)
    plt.close(fig1)
    plot_number += 1

    # 2. Contention types comparison
    if "MutexContention" in df.columns or "ReadContention" in df.columns or "WriteContention" in df.columns:
        melted_df = df.melt(id_vars=['LockType'], value_vars=[
                            'MutexContention', 'ReadContention', 'WriteContention'], var_name='ContentionType', value_name='ContentionCount')
        melted_df['ContentionType'] = melted_df['ContentionType'].replace(
            {'MutexContention': 'Mutex', 'ReadContention': 'Read', 'WriteContention': 'Write'})
        fig2 = plt.figure(figsize=(12, 6))
        sns.barplot(data=melted_df, x='LockType', y='ContentionCount',
                    hue='ContentionType', palette='Set3')
        plt.title('LockType vs Contention Count', fontsize=16)
        plt.xlabel('Lock Type', fontsize=14)
        plt.ylabel('Contention Count', fontsize=14)
        plt.legend(title='Contention Type')
        plt.tight_layout()
        save_plot(fig2, csv_filename, plot_number)
        plt.close(fig2)
        plot_number += 1

    # 6. OperationCount vs TotalReadTime & TotalWriteTime
    fig6 = plt.figure(figsize=(12, 6))
    sns.lineplot(data=df, x='OperationCount', y='TotalWriteTime(us)', hue='LockType',
                 style='LockType', ci=None, linestyle='--', label='Total Write Time')
    sns.lineplot(data=df, x='OperationCount', y='TotalReadTime(us)', hue='LockType',
                 style='LockType', ci=None, linestyle='-', label='Total Read Time')
    plt.title('OperationCount vs TotalReadTime & TotalWriteTime', fontsize=16)
    plt.xlabel('Operation Count (log scale)', fontsize=14)
    plt.ylabel('Total Time (us)', fontsize=14)
    plt.xscale('log')
    plt.legend(title='Metric / Lock Type',
               bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    save_plot(fig6, csv_filename, plot_number)
    plt.close(fig6)
    plot_number += 1

    # 7. Contention vs TotalThreads
    fig7 = plt.figure(figsize=(14, 8))
    melted_df = df.melt(id_vars=['TotalThreads', 'LockType'], value_vars=[
                        'MutexContention', 'ReadContention', 'WriteContention'], var_name='ContentionType', value_name='ContentionCount')
    melted_df['ContentionType'] = melted_df['ContentionType'].replace(
        {'MutexContention': 'Mutex', 'ReadContention': 'Read', 'WriteContention': 'Write'})
    sns.lineplot(data=melted_df, x='TotalThreads', y='ContentionCount',
                 hue='ContentionType', style='LockType', markers=True, dashes=False)
    plt.title('Contention vs TotalThreads', fontsize=16)
    plt.xlabel('Total Threads (Consumer + Reader)', fontsize=14)
    plt.ylabel('Contention Count', fontsize=14)
    plt.legend(title='Contention Type / Lock Type',
               bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    save_plot(fig7, csv_filename, plot_number)
    plt.close(fig7)
    plot_number += 1

    # 8. ConsumerCount vs TotalWriteTime
    fig8 = plt.figure(figsize=(12, 6))
    sns.boxplot(data=df, x='ConsumerCount', y='TotalWriteTime(us)',
                hue='LockType', palette='Set2')
    plt.title('ConsumerCount vs TotalWriteTime', fontsize=16)
    plt.xlabel('Consumer Count', fontsize=14)
    plt.ylabel('Total Write Time (us)', fontsize=14)
    plt.legend(title='Lock Type')
    plt.tight_layout()
    save_plot(fig8, csv_filename, plot_number)
    plt.close(fig8)
    plot_number += 1

    # 9. ReaderCount vs TotalReadTime
    fig9 = plt.figure(figsize=(12, 6))
    sns.boxplot(data=df, x='ReaderCount', y='TotalReadTime(us)',
                hue='LockType', palette='Set3')
    plt.title('ReaderCount vs TotalReadTime', fontsize=16)
    plt.xlabel('Reader Count', fontsize=14)
    plt.ylabel('Total Read Time (us)', fontsize=14)
    plt.legend(title='Lock Type')
    plt.tight_layout()
    save_plot(fig9, csv_filename, plot_number)
    plt.close(fig9)
    plot_number += 1


# Process IO data
csv_filename = "ResultIO.csv"
csv_path = os.path.join(base_dir, csv_filename)
if os.path.exists(csv_path):
    df = pd.read_csv(csv_path)
    print(f"Processing IO data: {csv_filename}")
    generate_plots_for_io(df, csv_filename)
else:
    print(f"CSV file not found: {csv_path}")
