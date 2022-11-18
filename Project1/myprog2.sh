cd $1
for file in *
do
	if [ $file == *.c ]
	then
		continue
	elif [ $file == *.h ]
	then
		continue
	elif [ "$file" == "Makefile" ]
	then
		continue
	elif [ "$file" == "makefile" ]
	then 
		continue
	else
		rm $file
	fi
ls
done