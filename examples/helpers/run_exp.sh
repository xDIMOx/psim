#!/bin/sh
# Check LICENSE file for copyright and license details.
#
# Run experiments and generates files with the data for later analysis

REPOROOT="${HOME}/src/psim" # root of the repository
HELPERS="${REPOROOT}/examples/helpers/" # helpers directory
SRC="${REPOROOT}/examples/src/" # simulated program's source directory

PSIM="${REPOROOT}/psim" # simulator executable

# helper scripts
CY="${HELPERS}/cy.awk"
BU="${HELPERS}/busutil.awk"
MF="${HELPERS}/memfail.awk"
LOCKCY="${HELPERS}/lockcy.awk"
LOCKF="${HELPERS}/lockfail.awk"

pflag=0 # plot flag

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
			for perfct in perfct_${EXE}_*_${ps}_${cs}.csv; do
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

	[ ${pflag} -eq 0 ] && return 0

	for gp in ${HELPERS}/*_producer-consumer.gp; do
		gnuplot ${gp} || return 1
	done

	chartdir=chart_producer-consumer_$(date +'%FT%H:%M')

	mkdir ${chartdir}

	mv *.png ${chartdir}
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
			for perfct in perfct_${EXE}_*_${ps}_${cs}.csv; do
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

	[ ${pflag} -eq 0 ] && return 0

	for gp in ${HELPERS}/*_producer-consumerv2.gp; do
		gnuplot ${gp} || return 1
	done

	chartdir=chart_producer-consumerv2_$(date +'%FT%H:%M')

	mkdir ${chartdir}

	mv *.png ${chartdir}
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
				PCT="perfct_${EXE}_*_${ideas}_${th}_${eat}.csv"
				for perfct in ${PCT}; do
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

	[ ${pflag} -eq 0 ] && return 0

	for gp in ${HELPERS}/*dining-philosophers.gp; do
		gnuplot ${gp} || return 1
	done

	chartdir=chart_dining-philosophers_$(date +'%FT%H:%M')

	mkdir ${chartdir}

	mv *.png ${chartdir}
}

while getopts "hp" opt; do
	case ${opt} in
	'p')
		export HELPERS
		pflag=1 ;;
	'h'|'?')
		printf "usage: %s -h\n" $0 >&2
		printf "       %s [-p] PROGRAM\n" $0 >&2
		printf "\n" >&2
		printf "The -p option requires gnuplot installed. It\n" >&2
		printf "generates charts for the experiments." >&2
		exit 1 ;;
	esac
done

cd ${SRC}

shift $(( OPTIND - 1 ))

case $1 in
1|producer-consumer)
	exp_producerconsumer ;;
2|producer-consumerv2)
	exp_producerconsumerv2 ;;
3|dining-philosophers)
	exp_diningphilosophers ;;
clean)
	echo 'cleaning'
	rm -f perfct* producer-consumer{v2,}_[0-9]* \
	    dining-philosophers_[0-9]* *.csv *.dat *.png ;;
*)
	echo 'Invalid experiment' ;;
esac
