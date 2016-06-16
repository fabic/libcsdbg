#!/bin/bash

# Show script usage message and exit
usage()
{
	echo="echo -e"
	name=`basename $0`

	$echo "Project libcsdbg installer"
	$echo "Usage: $name [-c] [-u] [-m] [-s] [-d] [-h]\r\n"

	$echo "'$name' will compile and install its target by default."
	$echo "The following options change the default behaviour:\r\n"

	$echo "-c  Clear the source tree (make clean)"
	$echo "-u  Uninstall the package (make uninstall)"
	$echo "-m  Compile but don't install"
	$echo "-s  Don't parallelize compilation on multicore systems"
	$echo "-d  Create/update the documentation (make doc)"
	$echo "-h  Show this message"
	exit 1
}


# Flag controlled by the -c option
do_clear=0

# Flag controlled by the -u option
do_uninstall=0

# Flag controlled by the -m option
do_install=1

# Flag controlled by the -s option
do_parallel=1

# Flag controlled by the -d option
do_docs=0


# Parse command line arguments
for opt in "$@" ;
do
	if [ "$opt" == "-c" ];
	then
		let do_clear=1

	elif [ "$opt" == "-u" ];
	then
		let do_uninstall=1

	elif [ "$opt" == "-m" ];
	then
		let do_install=0

	elif [ "$opt" == "-s" ];
	then
		let do_parallel=0

	elif [ "$opt" == "-d" ];
	then
		let do_docs=1

	else
		usage
	fi
done


# Set the number of make threads by probing the number of processors/cores
if [ $do_parallel -eq 1 ];
then
	let jobs=`cat /proc/cpuinfo | grep processor | wc -l`
else
	let jobs=1
fi


# Clear the source tree (doesn't remove the compiled documentation)
if [ $do_clear -eq 1 ];
then
	make -j $jobs clean
	retval=$?

	if [ $retval -ne 0 ];
	then
		exit $retval
	fi
fi


# Uninstall the package
if [ $do_uninstall -eq 1 ];
then
	make -j $jobs uninstall
	retval=$?

	if [ $retval -ne 0 ];
	then
		exit $retval
	fi
fi


# Update the documentation
if [ $do_docs -eq 1 ];
then
	make -j $jobs doc
	retval=$?

	if [ $retval -ne 0 ];
	then
		exit $retval
	fi
fi


# Compile the library
make -j $jobs
retval=$?


# Install the library
if [ $retval -eq 0 -a $do_install -eq 1 ];
then
	make -j $jobs install
	retval=$?
fi
exit $retval

