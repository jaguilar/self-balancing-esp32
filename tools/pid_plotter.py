# /usr/bin/env python3

import matplotlib.pyplot as plt
import sys

# Function to read CSV from a file.


def read_csv(file_name):
    with open(file_name, 'r') as f:
        lines = f.readlines()
        data = []
        for line in lines:
            data.append(line.strip().split(','))
    return data

# Function to recast CSV data into appropriate numeric types.


def recast_csv(data):
    time = []
    setpoint = []
    measurement = []
    error = []
    output = []
    p = []
    i = []
    d = []
    for row in data:
        time.append(int(row[0]))
        setpoint.append(float(row[1]))
        measurement.append(float(row[2]))
        error.append(float(row[2]) - float(row[1]))
        output.append(float(row[3]))
        p.append(float(row[4]))
        i.append(float(row[5]))
        d.append(float(row[6]))
    return time, setpoint, measurement, error, output, p, i, d

# Function to display a plot of the data using matplotlib.
# time is the X axis. There are two Y axes, one for setpoint, measurement, and error.
# The other for P, I, D, and output.


def show_graph(data):
    time, setpoint, measurement, error, output, p, i, d = data
    # Clear any existing graph.
    plt.clf()
    fig, ax1 = plt.subplots()
    ax2 = ax1.twinx()
    ax1.plot(time, setpoint, 'g-', label='Setpoint')
    ax1.plot(time, measurement, 'b-', label='Measurement')
    ax1.plot(time, error, 'r-', label='Error')
    ax2.plot(time, output, 'y-', label='Output')
    ax2.plot(time, p, 'c-', label='P')
    ax2.plot(time, i, 'm-', label='I')
    ax2.plot(time, d, 'k-', label='D')
    ax1.set_xlabel('Time (ms)')
    ax1.set_ylabel('Setpoint, Measurement, Error', color='b')
    ax2.set_ylabel('Output, P, I, D', color='r')
    ax1.legend(loc='upper left')
    ax2.legend(loc='lower left')
    plt.show()

# Main function to read the CSV file and display the graph.


def main():
    if len(sys.argv) < 2:
        print("Usage: pid_plotter.py <file.csv>")
        sys.exit(1)
    data = read_csv(sys.argv[1])
    data = recast_csv(data)
    show_graph(data)


if __name__ == '__main__':
    main()
