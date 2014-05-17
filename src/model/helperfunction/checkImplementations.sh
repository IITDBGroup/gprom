#!/bin/bash
# Script that checks whether we have helper function implementations for each node type
# It retrieves the node types from nodetype.h so if a node type is not mentioned there it will
# not be found. However, this is a serious error anyways

searchForFunctions()
{
    cFile=$1
    functionPrefix=$2
    nodeTypes="${3}"
    echo "****************************************"
    echo "searching for functions in ${cFile} using prefix ${functionPrefix}"
    echo "****************************************"
    for t in ${nodeTypes};
    do
	curFunction="${functionPrefix}${t}"
	check=`cat ${cFile} | grep "^${curFunction}"`
	if [ "${check}X" == "X" ]; then
	    echo "did not find ${curFunction}"
	fi
    done
}

NodeTypesT=`grep ../../../include/model/node/nodetype.h -e 'T_[A-Z][a-z][a-zA-Z]\+' -o | tr '\n' ' '`
NodeTypes=`echo "${NodeTypesT}" | sed -e 's/T_//g' -e 's/Invalid//g' -e 's/FromItem//g' -e 's/QueryOperator//g'`
echo "found node types:"; echo "${NodeTypes}"

searchForFunctions "copy.c" "copy" "${NodeTypes}"
searchForFunctions "equal.c" "equal" "${NodeTypes}"
searchForFunctions "to_string.c" "out" "${NodeTypes}"
searchForFunctions "deepFree.c" "free" "${NodeTypes}"
searchForFunctions "hash.c" "hash" "${NodeTypes}"
searchForFunctions "to_dot.c" "dot" "${NodeTypes}"
