outputName = skyers
warnings = -Wall
threads = -pthread
debug = -D DEBUG

compile:
	mpic++ ${warnings} ${threads} ${wildcard *.cpp} -o ${outputName}
debug:
	mpic++ ${warnings} ${threads} ${wildcard *.cpp} -o ${outputName} ${debug}
run:
	mpirun -n ${size} ./skyers ${filename}
clean:
	rm ${outputName}