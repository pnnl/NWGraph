#!/bin/bash

#very simple for now, look in common locations
find_hyde() {
    BUILD_DIR=$1
    if [ -f $BUILD_DIR/hyde ]
    then
        echo $BUILD_DIR/hyde
        return 0
    fi
    if [ -f $BUILD_DIR/Debug/hyde ]
    then
        echo $BUILD_DIR/Debug/hyde
        return 0
    fi
    if [ -f $BUILD_DIR/Release/hyde ]
    then
        echo $BUILD_DIR/Release/hyde
        return 0
    fi
    return 1
}   

EXE_DIR=$(dirname "$0")

pushd ${EXE_DIR} > /dev/null

# HYDE_ROOT=/Users/lums658/Contrib/hyde

HYDE_ROOT=/home/nwmath/hyde


HYDE_PATH=`find_hyde "${HYDE_ROOT}/build"`
echo HYDE_PATH is ${HYDE_PATH}

# NWGRAPH_ROOT=/Users/lums658/Working/NWmath/NWgr
NWGRAPH_ROOT=/home/nwmath/NWmath/NWgr

NWGRAPH_SRC=${NWGRAPH_ROOT}/include/nwgraph

HYDE_DST_ROOT=${NWGRAPH_ROOT}/docs/sphinx/libref

NWGRAPH_INCLUDES="-I${NWGRAPH_ROOT} -I${NWGRAPH_ROOT}/include -I${NWGRAPH_SRC} -I${NWGRAPH_SRC}/algorithms -I${NWGRAPH_SRC}/util -I${NWGRAPH_SRC}/containers -I${NWGRAPH_SRC}/adaptors -I${NWGRAPH_SRC}/io"

CUR_LIBS="${NWGRAPH_SRC} ${NWGRAPH_SRC}/algorithms ${NWGRAPH_SRC}/adaptors ${NWGRAPH_SRC}/containers ${NWGRAPH_SRC}/io ${NWGRAPH_SRC}/util"
# CUR_LIBS="${NWGRAPH_SRC}"

for CUR_LIB in ${CUR_LIBS}; do
    HYDE_SRC_ROOT=${NWGRAPH_SRC}
    echo "Processing library $CUR_LIB"

    for CUR_FILE in ${CUR_LIB}/*.hpp ; do
	
	echo "Processing file $CUR_FILE"
	
	CUR_COMMAND="${HYDE_PATH} -hyde-very-verbose -hyde-src-root=${HYDE_SRC_ROOT} -hyde-yaml-dir=${HYDE_DST_ROOT} -hyde-update --auto-toolchain-includes "$CUR_FILE" -- -std=c++20 ${NWGRAPH_INCLUDES}"

	echo $CUR_COMMAND
	eval $CUR_COMMAND
	
    done
done

popd > /dev/null
