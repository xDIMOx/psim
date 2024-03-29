#!/bin/sh
# Check LICENSE file for copyright and license details.
#
# Run experiments and generates files with the data for later analysis

: ${REPOBASEDIR:="${HOME}/src/psim"} # root of the repository
HELPERS="${REPOBASEDIR}/examples/helpers/" # helpers directory
SRC="${REPOBASEDIR}/examples/src/" # simulated program's source directory

PSIM="${REPOBASEDIR}/psim" # simulator executable

# helper scripts
CY="${HELPERS}/cy.awk"
BU="${HELPERS}/busutil.awk"
NU="${HELPERS}/netutil.awk"
MF="${HELPERS}/memfail.awk"
LOCKCY="${HELPERS}/lockcy.awk"
CW="${HELPERS}/cw.awk"
LOCKF="${HELPERS}/lockfail.awk"
OUTPROD="${HELPERS}/outprod.awk"
INBUF="${HELPERS}/inbuf.awk"
ALTBUF="${HELPERS}/altbuf.awk"
OUTBUF="${HELPERS}/outbuf.awk"
OUTCON="${HELPERS}/outcon.awk"
INCON="${HELPERS}/incon.awk"
OUTPROD2="${HELPERS}/outprod2.awk"
OUTCON2="${HELPERS}/outcon2.awk"
INCON2="${HELPERS}/incon2.awk"

pflag=0 # plot flag

set -e

exp_producerconsumer() {
	EXE="producer-consumer"
	PRODUCER_SYNTHLOAD="10 ((i&1)==0?9+(rand()&7):9-(rand()&7)) \
	    ((i&1)==0?i+((rand()&1023)>>2):i-((rand()&1023)>>2))          \
	    1+(rand()&1023)"
	CONSUMER_SYNTHLOAD="13                                              \
	    ((item&1)==0?9+(rand()&7):9-(rand()&7))                 \
	    ((item&1)==0?item+((rand()&1023)>>2):item-((rand()&1023)>>2)) \
	    1+(rand()&1023)"
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

	chartdir=chart_producer-consumer

	mkdir ${chartdir}

	mv *.png ${chartdir}
}

exp_producerconsumerv2() {
	EXE="producer-consumerv2"
	PRODUCER_SYNTHLOAD="10 ((val&1)==0?9+(rand()&7):9-(rand()&7)) \
	    ((val&1)==0?val+((rand()&1023)>>2):val-((rand()&1023)>>2))      \
	    1+(rand()&1023)"
	CONSUMER_SYNTHLOAD="13                                              \
	    ((item&1)==0?9+(rand()&7):9-(rand()&7))                 \
	    ((item&1)==0?item+((rand()&1023)>>2):item-((rand()&1023)>>2)) \
	    1+(rand()&1023)"
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

	chartdir=chart_producer-consumerv2

	mkdir ${chartdir}

	mv *.png ${chartdir}
}

exp_diningphilosophers() {
	EXE="dining-philosophers"
	IDEAS="10 ((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))"
	THINK="((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))) \
	    ((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))"
	EAT="((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))) \
	    ((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))"

	for ideas in ${IDEAS}; do
		for th in ${THINK}; do
			for eat in ${EAT}; do
				# run experiment
				NEXE=${EXE}_${ideas}
				NEXE=${NEXE}_${th}_${eat}
				make DINPHIL_CFLAGS="    \
				    -DIDEAS=\"${ideas}\" \
				    -DPHILOS=5           \
				    -DTHINK=\"${th}\"    \
				    -DEAT=\"${eat}\""    \
				    clean                \
				    ${EXE} >/dev/null
				mv ${EXE} ${NEXE}
				${PSIM} -c 5 ${NEXE} >/tmp/${NEXE}.out
				# extract data from performance counters
				rm -f tmp*.csv
				PCT="perfct_${EXE}_${ideas}_${th}_${eat}.csv"
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

	for gp in ${HELPERS}/*_dining-philosophers.gp; do
		gnuplot ${gp} || return 1
	done

	chartdir=chart_dining-philosophers

	mkdir ${chartdir}

	mv *.png ${chartdir}
}

exp_dproducerconsumer() {
	EXE="d_producer-consumer"
	PRODUCER_SYNTHLOAD="10 ((i&1)==0?9+(rand()&7):9-(rand()&7)) \
	    ((i&1)==0?i+((rand()&1023)>>2):i-((rand()&1023)>>2))          \
	    1+(rand()&1023)"
	CONSUMER_SYNTHLOAD="13                                              \
	    ((item&1)==0?9+(rand()&7):9-(rand()&7))                 \
	    ((item&1)==0?item+((rand()&1023)>>2):item-((rand()&1023)>>2)) \
	    1+(rand()&1023)"
	TOPO="1x3 2x2 2x4 4x4 4x8 8x8"

	for ps in ${PRODUCER_SYNTHLOAD}; do
		for cs in ${CONSUMER_SYNTHLOAD}; do
			# run experiment
			for topo in ${TOPO}; do
				NEXE=${EXE}_${topo}_${ps}_${cs}
				x=${topo%x*}
				y=${topo#*x}
				nc=$(( (x * y) - 2 ))
				make DPRODCON_CFLAGS="-DNCONSUMERS=${nc} \
				    -DMAXELEM=16 -DMAXVAL=1024          \
				    -DPRODUCER_WAIT=\"${ps}\"           \
				    -DCONSUMER_WAIT=\"${cs}\""          \
				    clean d_producer-consumer >/dev/null
				mv ${EXE} ${NEXE}
				${PSIM} -n ${topo} ${NEXE} &
			done
			wait
			# extract data from performance counters
			rm -f tmp*.csv
			for perfct in perfct_${EXE}_*_${ps}_${cs}.csv; do
				${CY} ${perfct} >>tmp0.csv
				${NU} ${perfct} >>tmp1.csv
				${CW} ${perfct} >>tmp2.csv
				${OUTPROD} ${perfct} >>tmp3.csv
				${INBUF} ${perfct} >>tmp4.csv
				${ALTBUF} ${perfct} >>tmp5.csv
				${OUTBUF} ${perfct} >>tmp6.csv
				${OUTCON} ${perfct} >>tmp7.csv
				${INCON} ${perfct} >>tmp8.csv
			done
			sort -n -t',' -k1,1 tmp0.csv >cycles_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp1.csv >netutil_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp2.csv >cw_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp3.csv >outprod_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp4.csv >inbuf_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp5.csv >altbuf_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp6.csv >outbuf_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp7.csv >outcon_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp8.csv >incon_${ps}_${cs}.csv
		done
	done

	[ ${pflag} -eq 0 ] && return 0

	for gp in ${HELPERS}/*_dproducer-consumer.gp; do
		gnuplot ${gp} || return 1
	done

	chartdir=chart_dproducer-consumer

	mkdir ${chartdir}

	mv *.png ${chartdir}
}

exp_dproducerconsumerv2() {
	EXE="d_producer-consumerv2"
	PRODUCER_SYNTHLOAD="10 ((i&1)==0?9+(rand()&7):9-(rand()&7)) \
	    ((i&1)==0?i+((rand()&1023)>>2):i-((rand()&1023)>>2))    \
	    1+(rand()&1023)"
	CONSUMER_SYNTHLOAD="13                                            \
	    ((item&1)==0?9+(rand()&7):9-(rand()&7))                       \
	    ((item&1)==0?item+((rand()&1023)>>2):item-((rand()&1023)>>2)) \
	    1+(rand()&1023)"
	TOPO="2x2 2x4 4x4 4x8 8x8"

	for ps in ${PRODUCER_SYNTHLOAD}; do
		for cs in ${CONSUMER_SYNTHLOAD}; do
			# run experiment
			for topo in ${TOPO}; do
				NEXE=${EXE}_${topo}_${ps}_${cs}
				x=${topo%x*}
				y=${topo#*x}
				nc=$(( (x * y) - 3 ))
				make DPRODCONV2_CFLAGS="-DNCONSUMERS=${nc}  \
				    -DMAXELEM=16 -DMAXVAL=1024              \
				    -DPRODUCER_WAIT=\"${ps}\"               \
				    -DCONSUMER_WAIT=\"${cs}\" -DPRODUCER0=0 \
				    -DPRODUCER1=$((1 + y)) -DBUFFER=1"      \
				    clean d_producer-consumerv2 >/dev/null
				mv ${EXE} ${NEXE}
				${PSIM} -n ${topo} ${NEXE} &
			done
			wait
			# extract data from performance counters
			rm -f tmp*.csv
			for perfct in perfct_${EXE}_*_${ps}_${cs}.csv; do
				${CY} ${perfct} >>tmp0.csv
				${NU} ${perfct} >>tmp1.csv
				${CW} ${perfct} >>tmp2.csv
				${OUTPROD2} ${perfct} >>tmp3.csv
				${ALTBUF} ${perfct} >>tmp5.csv
				${OUTBUF} ${perfct} >>tmp6.csv
				${OUTCON2} ${perfct} >>tmp7.csv
				${INCON2} ${perfct} >>tmp8.csv
			done
			sort -n -t',' -k1,1 tmp0.csv >cycles_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp1.csv >netutil_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp2.csv >cw_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp3.csv >outprod_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp5.csv >altbuf_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp6.csv >outbuf_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp7.csv >outcon_${ps}_${cs}.csv
			sort -n -t',' -k1,1 tmp8.csv >incon_${ps}_${cs}.csv
		done
	done

	[ ${pflag} -eq 0 ] && return 0

	for gp in ${HELPERS}/*_dproducer-consumerv2.gp; do
		gnuplot ${gp} || return 1
	done

	chartdir=chart_dproducer-consumerv2

	mkdir ${chartdir}

	mv *.png ${chartdir}
}

exp_ddiningphilosophers() {
	EXE="d_dining-philosophers"
	IDEAS="10 ((rem(rand(),13))>6?(10+(rand()&7)):(10-(rand()&7)))"
	THINK="((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))) \
	    ((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))"
	EAT="((rem(rand(),13))>6?(9+(rand()&7)):(9-(rand()&7))) \
	    ((rem(rand(),13))>6?(50+rem(rand(),25)):(50-rem(rand(),25)))"

	for ideas in ${IDEAS}; do
		for th in ${THINK}; do
			for eat in ${EAT}; do
				# run experiment
				NEXE=${EXE}_${ideas}_${th}_${eat}
				make DINPHIL_CFLAGS="    \
				    -DIDEAS=\"${ideas}\" \
				    -DTHINK=\"${th}\"    \
				    -DEAT=\"${eat}\""    \
				    clean ${EXE} >/dev/null
				mv ${EXE} ${NEXE}
				${PSIM} -n 6x2 ${NEXE} >/tmp/${NEXE}.out 2>/tmp/${NEXE}.debug
				# extract data from performance counters
				perfct="perfct_${NEXE}.csv"
				${CY} -vEXCLUDE='^8' ${perfct} \
				    >cycles_${ideas}_${th}_${eat}.csv
				${NU} -vEXCLUDE='^8' ${perfct} \
				    >netutil_${ideas}_${th}_${eat}.csv
				${CW} -vEXCLUDE='^8' ${perfct} \
				    >cw_${ideas}_${th}_${eat}.csv
			done
		done
	done

	[ ${pflag} -eq 0 ] && return 0

	for gp in ${HELPERS}/*_ddining-philosophers.gp; do
		gnuplot ${gp} || return 1
	done

	chartdir=chart_ddining-philosophers

	mkdir -p ${chartdir}

	mv *.png ${chartdir}
}


while getopts "hp" opt; do
	case ${opt} in
	'p')
		export HELPERS
		pflag=1 ;;
	'h'|'?')
		printf "usage: %s -h\n" $0 >&2
		printf "       %s [-p] TARGET\n" $0 >&2
		printf "\n" >&2
		printf "The -p option requires gnuplot installed. It\n" >&2
		printf "generates charts for the experiments. The charts\n" >&2
		printf "will located in %s/chart_TARGET\n" ${SRC} >&2
		printf "\n" >&2
		printf "TARGET can be one of:\n" >&2
		printf "	1|producer-consumer\n" >&2
		printf "	2|producer-consumerv2\n" >&2
		printf "	3|dining-philosophers\n" >&2
		printf "	4|d_producer-consumer\n" >&2
		printf "	5|d_producer-consumerv2\n" >&2
		printf "	6|d_dining-philosophers\n" >&2
		printf "	clean\n" >&2
		printf "\n" >&2
		printf "To this script to work, REPOBASEDIR have to be\n" >&2
		printf "set correctly. It's current value is:\n" >&2
		printf "	REPOBASEDIR=%s\n" ${REPOBASEDIR} >&2
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
4|d_producer-consumer)
	exp_dproducerconsumer ;;
5|d_producer-consumerv2)
	exp_dproducerconsumerv2 ;;
6|d_dining-philosophers)
	exp_ddiningphilosophers ;;
clean)
	echo 'cleaning'
	rm -f perfct* {d_,}producer-consumer{v2,}_[0-9]* \
	    {d_,}dining-philosophers_* *.csv *.dat *.png ;;
*)
	echo 'Invalid experiment' ;;
esac
