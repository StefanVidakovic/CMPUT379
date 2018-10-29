In order to have better efficiency when processesing the large valgrind outputs,
valws uses a linked list to store the address referenced by a page. The linked
list allows for the index in our array of unique page numbers to be known, hence
reducing the need for a O(n) lookup.

The performance study was done using gnuplot. The plotting functions scripts are
included in the assignment folder. In order to produce plots using these scripts
first compile the sample program, then run valgrind, valws and the script with the
following syntax as an example:

valgrind --tool=lackey --trace-mem=yes ./mergesort 5000 |& ./valws379 -i 1024 10000000 | awk 'NR%1000==0' | gnuplot mem_plot.p

Here the array to be sorted is 5000 elements, the output of valgrind for mergesort
on this array is piped into valws379 where instructions are ignored, the page size is 1024 bytes,
the window size is 1 million, the output of valws379 is then piped into an brief awk script which
removes a fraction of the very large input to the gnuplot.
