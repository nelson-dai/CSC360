# Task 2 - Generating Charts

## Gathering statistics from multiple runs of `rrsim`

I ran the `rrsim` program with a large number of simulated jobs as described in the writeup:

```
$ ./simgen 1000 3141 | ./rrsim --quantum 50 --dispatch 0
```

I adjusted the output of `rrsim` so that when I'm collecting stats instead of reviewing cpu ticks it only
outputs the final average wait and turnaround times.  This mode is set by compiling with the `-DSTATS` argument:

```
$ gcc -c -Wall -g -DSTATS linkedlist.c rrsim.c -o rrsim
```

Now, running `rrsim` will just output four final stats data items separated by a single space: <quantum> <dispatch> <ave_wait_time> <ave_turnaround_time>

```
$ make rrsim-stats
$ ./simgen 1000 3141 | ./rrsim --quantum 50 --dispatch 0

output:  50 0 250785.125000 251313.640625
```


Then ran the command for different quantum values and dispatch values mentioned in the PDF and collect the stats that are
generated into a text file. I used a small bash script to run the command multiple times:

```
get_stats.sh
------------
rm -f stats.txt
for q in 50 100 250 500
	do
		for d in 0 5 10 15 20 25
			do
				./simgen 1000 3141 | ./rrsim --quantum $q --dispatch $d >> stats.txt
		done
done
```

This runs the program for quantum values 50, 100, 250 and 500, each with dispatch values of 0, 5, 10, 15, 20 and 25, and collects them all in a file called `stats.txt`.

```
stats.txt
---------
50 0 250785.125000 251313.640625
50 5 282363.000000 282891.406250
50 10 321698.718750 322226.875000
50 15 360967.375000 361495.562500
50 20 400047.750000 400576.156250
50 25 438993.625000 439522.031250
100 0 241948.625000 242477.031250
100 5 257936.031250 258464.250000
100 10 277896.375000 278424.718750
100 15 297867.000000 298395.312500
100 20 317797.937500 318326.468750
100 25 337701.250000 338229.375000
250 0 227657.937500 228186.484375
250 5 234463.156250 234991.609375
250 10 242930.390625 243459.046875
250 15 251422.546875 251951.078125
250 20 259903.078125 260431.515625
250 25 268377.281250 268905.750000
500 0 205745.609375 206274.062500
500 5 209407.171875 209935.750000
500 10 214002.312500 214530.703125
500 15 218609.546875 219138.000000
500 20 223199.078125 223727.390625
500 25 227778.312500 228306.687500
```
This is all the data we need for the charts.



## Organizing the chart data

The `stats.txt` file has all the necessary data points, but not in a format that is easily imported into graphing programs. I chose to import the data into Excel like this:

```
Average Wait Times				
d-values	 q=50	 q=100	 q=250	 q=500
0	250785.125	241948.625	227657.9375	205745.6094
5	282363	257936.0313	234463.1563	209407.1719
10	321698.7188	277896.375	242930.3906	214002.3125
15	360967.375	297867	251422.5469	218609.5469
20	400047.75	317797.9375	259903.0781	223199.0781
25	438993.625	337701.25	268377.2813	227778.3125
				
Average Turnaround Times				
d-values	 q=50	 q=100	 q=250	 q=500
0	251313.6406	242477.0313	228186.4844	206274.0625
5	282891.4063	258464.25	234991.6094	209935.75
10	322226.875	278424.7188	243459.0469	214530.7031
15	361495.5625	298395.3125	251951.0781	219138
20	400576.1563	318326.4688	260431.5156	223727.3906
25	439522.0313	338229.375	268905.75	228306.6875
```

I wrote a Python program called `process_data.py` to read in the `stats.txt` and output the above data tables:

```
$ make rrsim-stats
$ get_stats.sh
$ process_data.py < stats.txt > chart_data.csv
```
This creates the chart_data.csv file that can be imported into an Excel Worksheet. I highlighted the columns from `q50` to `q500` and selected "Line Chart" from the menu. 
This made a chart but without the proper X-Axis data, so I clicked on the chart and select "Set Chart Data..." from the context menu. 
I insert the data range of the d-values column into the Text Box for the X-Axis labels. This creates the final chart.
