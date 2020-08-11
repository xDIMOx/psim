#!/bin/sh
# Check LICENSE file for copyright and license details.
#
# Run experiments and generates files with the data for later analysis

PSIM="../psim"
CY="./cy.awk"
BU="./busutil.awk"
MF="./memfail.awk"
LOCKCY="./lockcy.awk"
LOCKF="./lockfail.awk"

set -e

exp_producerconsumer() {
	EXE="producer-consumer"
	PRODUCER_SYNTHLOAD="10 ((i&1)==0?9+(randu()%6):9-(randu()%6)) \
	    ((i&1)==0?i+((randu()&1023)>>2):i-((randu()&1023)>>2))    \
	    1+(randu()&1023)"
	CONSUMER_SYNTHLOAD="13 ((item&1)==0?9+(randu()%6):9-(randu()%6))    \
	    ((item&1)==0?item+((randu()&1023)>>2):item-((randu()&1023)>>2)) \
	    1+(randu()&1023)"
	NPROC="2 4 8 16 32 64"

	for ps in ${PRODUCER_SYNTHLOAD}; do
		for cs in ${CONSUMER_SYNTHLOAD}; do
			# run experiment
			for np in ${NPROC}; do
				NEXE=${EXE}_${np}_${ps}_${cs}
				make PRODCON_CFLAGS="-DNCONSUMERS=$((np - 1)) \
				    -DMAXELEM=16 -DMAXVAL=1024                \
				    -DPRODUCER_WAIT=\"${ps}\"                 \
				    -DCONSUMER_WAIT=\"${cs}\"" \
				    clean producer-consumer >/dev/null
				mv ${EXE} ${NEXE}
				${PSIM} -c ${np} ${NEXE} &
			done
			wait
			# extract data from performance counters
			rm -f tmp*.csv
			for perfct in perfct_${EXE}_*_${ps}_${cs}; do
				${CY} ${perfct} >>tmp0.csv
				${BU} ${perfct} >>tmp1.csv
				${MF} ${perfct} >>tmp2.csv
				${LOCKCY} ${perfct} >>tmp3.csv
				${LOCKF} ${perfct} >>tmp4.csv
			done 
			sort -n -t',' -k1,1 tmp0.csv >cycles_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp1.csv >busutil_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp2.csv >memfail_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp3.csv >lockcy_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp4.csv >lockfail_${ps}_${cs}.csv
		done
	done

}

while [ $# -gt 0 ]; do
	case $1 in
	1|producer-consumer)
		exp_producerconsumer ;;
	clean)
		rm -f perfct* producer-consumer_[0-9]* *.csv ;;
	*)
		echo 'Invalid experiment' ;;
	esac

	shift
done
