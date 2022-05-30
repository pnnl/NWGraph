



for f in ${*}; do
    
    if ! fgrep '{index}' ${f} > /dev/null ; then

	t=$(fgrep 'title' ${f} | sed -e 's/title://')
        echo "" >> ${f}
	echo '```{index} ' ${t} >> ${f}
        echo '```' >> ${f}
	echo "" >> ${f}

    fi


done
