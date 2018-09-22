#
# This program takes simulation data as input, and converts it into a format that
# is suitable for Excel Charts.
#
import sys

# Prints the data for a single chart (wait times or turnaround times).
def printData(title, data):
	print(title)
	columns = set()
	for d in data:
		for c in data[d]:
			columns.add(c)
	print("d-values", end="")
	for c in sorted(columns):
		print(", q=" + str(c), end="")
	print()
	for d in sorted(data):
		print(d, end="")
		for c in sorted(columns):
			if c in data[d]:
				print("," + str(data[d][c]), end="")
		print()


# The main program. Takes the input from stdin line-by-line, as output from
# rrsim compiled with -DSTATS flag.
#
# Saves the two sets of data groups by d- and q-values.
waits = {}
turnarounds = {}
for line in sys.stdin:
	if len(line) > 0:
		parts = line.split()
		(q, d, w, ta) = (int(parts[0]), int(parts[1]), float(parts[2]), float(parts[3]))
		if d not in waits: waits[d] = {}
		waits[d][q] = w
		if d not in turnarounds: turnarounds[d] = {}
		turnarounds[d][q] = ta
	
printData("Average Wait Times", waits)
print()
printData("Average Turnaround Times", turnarounds)