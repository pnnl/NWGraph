#!/bin/bash
# This script will download and extract following graphs from suitesparse collection.
GRAPHS=(
https://suitesparse-collection-website.herokuapp.com/MM/SNAP/as-Skitter.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/SNAP/cit-Patents.tar.gz
#cit-Patents_nodename.mtx
https://suitesparse-collection-website.herokuapp.com/MM/LAW/eu-2005.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/LAW/hollywood-2009.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/LAW/in-2004.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/LAW/indochina-2004.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/italy_osm.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/GenBank/kmer_U1a.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/GenBank/kmer_V2a.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/kron_g500-logn17.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/kron_g500-logn18.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/kron_g500-logn19.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/kron_g500-logn20.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/kron_g500-logn21.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/LAW/ljournal-2008.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/MAWI/mawi_201512020030.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/Mycielski/mycielskian17.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/rgg_n_2_20_s0.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/rgg_n_2_21_s0.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/DIMACS10/rgg_n_2_22_s0.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/Gleich/wb-edu.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/Gleich/wikipedia-20051105.tar.gz
https://suitesparse-collection-website.herokuapp.com/MM/Gleich/wikipedia-20070206.tar.gz
)
# download file from url. check it has been downloaded first. use wget/curl.
# download [url] [filename]
function download {
    url=$1
    filename=$2

    # check file exits or not
    if test -f "$filename"; then
        echo "$filename already downloaded."
        return
    fi

    # if not, download using wget or curl
    if [ -x "$(which wget)" ] ; then
        wget -q $url -O $filename
    elif [ -x "$(which curl)" ]; then
        curl -o $filename -sfL $url
    else
        echo "Could not find curl or wget, please install one." >&2
    fi
}

for g in ${GRAPHS[@]}
do
echo "Downloading ${g}"
# extract graph.tar.gz
file_name="${g##*/}"
# get graph 
file="${file_name%.tar.gz}"
# test whether graph.mtx exists
if test -f "${file}.mtx"; then
    echo "${file}.mtx already exists, downloading abort."
    continue
fi
# otherwise download it
download ${g} ${file_name}
# extract graph.mtx from graph.tar.gz
echo "Extracting ${file_name}"
tar -xzf ${file_name} --strip-components 1
# delete graph.tar.gz
rm -f ${file_name}
done