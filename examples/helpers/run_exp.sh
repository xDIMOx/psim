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
	PRODUCER_SYNTHLOAD="10 ((i&1)==0?9+rem(randu(),7):9-rem(randu(),7)) \
	    ((i&1)==0?i+((randu()&1023)>>2):i-((randu()&1023)>>2))          \
	    1+(randu()&1023)"
	CONSUMER_SYNTHLOAD="13                                              \
	    ((item&1)==0?9+rem(randu(),7):9-rem(randu(),7))                 \
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

exp_producerconsumerv2() {
	EXE="producer-consumerv2"
	PRODUCER_SYNTHLOAD="10 ((val&1)==0?9+rem(randu(),7):9-rem(randu(),7)) \
	    ((val&1)==0?val+((randu()&1023)>>2):val-((randu()&1023)>>2))      \
	    1+(randu()&1023)"
	CONSUMER_SYNTHLOAD="13                                              \
	    ((item&1)==0?9+rem(randu(),7):9-rem(randu(),7))                 \
	    ((item&1)==0?item+((randu()&1023)>>2):item-((randu()&1023)>>2)) \
	    1+(randu()&1023)"
	NPROC="4 8 16 32 64"

	for ps in ${PRODUCER_SYNTHLOAD}; do
		for cs in ${CONSUMER_SYNTHLOAD}; do
			# run experiment
			for np in ${NPROC}; do
				NEXE=${EXE}_${np}_${ps}_${cs}
				make \
				    PRODCONV2_CFLAGS="-DNCONSUMERS=$((np - 2))\
				    -DMAXELEM=16 -DMAXVAL=1024                \
				    -DPRODUCER_WAIT=\"${ps}\"                 \
				    -DCONSUMER_WAIT=\"${cs}\"" \
				    clean producer-consumerv2 >/dev/null
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

exp_diningphilosophers() {
	EXE="dining-philosophers"
	IDEAS="10 ((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5)))"
	THINK="((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5))) \
	    ((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25)))"
	EAT="((randu()&3)==1?(10+rem(randu(),5)):(10-rem(randu(),5))) \
	    ((randu()&3)==1?(50+rem(randu(),25)):(50-rem(randu(),25)))"
	PHILOS="5 8 10 16 20 32"

	for ideas in ${IDEAS}; do
		for th in ${THINK}; do
			for eat in ${EAT}; do
				# run experiment
				for philos in ${PHILOS}; do
					NEXE=${EXE}_${philos}_${ideas}
					NEXE=${NEXE}_${th}_${eat}
					make DINPHIL_CFLAGS="    \
					    -DIDEAS=\"${ideas}\" \
					    -DPHILOS=${philos}   \
					    -DTHINK=\"${th}\"    \
					    -DEAT=\"${eat}\"" \
					    clean \
					    dining-philosophers >/dev/null
					mv ${EXE} ${NEXE}
					${PSIM} -c ${philos} ${NEXE} &
				done
				wait
				# extract data from performance counters
				rm -f tmp*.csv
				PERFCT="perfct_${EXE}_*_${ideas}_${th}_${eat}"
				for perfct in ${PERFCT}; do
					${CY} ${perfct} >>tmp0.csv
					${BU} ${perfct} >>tmp1.csv
					${MF} ${perfct} >>tmp2.csv
					${LOCKCY} ${perfct} >>tmp3.csv
					${LOCKF} ${perfct} >>tmp4.csv
				done
				sort -n -t',' -k1,1 tmp0.csv \
				    >cycles_${ideas}_${th}_${eat}.csv
				sort -n -t',' -k1,1 tmp1.csv \
				    >busutil_${ideas}_${th}_${eat}.csv
				sort -n -t',' -k1,1 tmp2.csv \
				    >memfail_${ideas}_${th}_${eat}.csv
				sort -n -t',' -k1,1 tmp3.csv \
				    >lockcy_${ideas}_${th}_${eat}.csv
				sort -n -t',' -k1,1 tmp4.csv \
				    >lockfail_${ideas}_${th}_${eat}.csv
			done
		done
	done

}

case $1 in
1|producer-consumer)
	exp_producerconsumer ;;
2|producer-consumerv2)
	exp_producerconsumerv2 ;;
3|dining-philosophers)
	exp_diningphilosophers ;;
clean)
	rm -f perfct* producer-consumer_[0-9]* dining-philosopher_[0-9]* \
	    *.csv ;;
*)
	echo 'Invalid experiment' ;;
esac
