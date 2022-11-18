num=$1
arr=() # array for the numbers
arr2=() # array for the combined numbers
tot=0
while [ $num -gt 0 ] # this while loop finds last digit of number
do
    mod=$((num % 10))    #It will split each digits
    arr+=($mod)
    num=$((num / 10))    #divide num by 10.
done
arrLength=`expr ${#arr[@]} - 1`
for (( i = 0; i < $arrLength; i++ )); do # this loop is combine 2 numbers and adding to arr2
    arr2+=(${arr[i]}${arr[i+1]})
done
for i in ${arr2[@]}; do # this loop is sum all elements of arr2
  let tot+=$i
done
echo $tot # sum of the numbers
