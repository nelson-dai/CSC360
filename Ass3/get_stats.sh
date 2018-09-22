rm -f stats.txt
for q in 50 100 250 500
	do
		for d in 0 5 10 15 20 25
			do
				./simgen 1000 3141 | ./rrsim --quantum $q --dispatch $d >> stats.txt
		done
done
