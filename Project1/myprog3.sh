count=$(grep -c $2 $1) # counting occurences
sed -i 's/\<'$2'\>/'$3'/g' $1 #find similar words
if [ "$count" -eq "0" ]; then # if count = 0
  echo "The searched word was not found"
else
  echo "All $count occurences of '$2' in '$1' file has changed with '$3'"
fi