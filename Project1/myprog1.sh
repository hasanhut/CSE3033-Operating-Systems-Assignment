file="./$1" #reading file
arr=()
function repeat() { num="${2:-100}"; printf -- "$1%.0s" $(seq 1 $num); } # integer times *
while IFS= read -r line # reading txt file
do
	arr+=($line) # adding to array each line
done < "$file"
length=${#arr[@]}
for (( i = 0; i < ${#arr[@]}; i++ )); do
	repeat "*" ${arr[i]} # printing stars
	echo 
done