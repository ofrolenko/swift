#!/bin/bash

#for branch in "$(git branch -a)"; 
#do
#    echo $branch
#    #git log --oneline $branch ^remotes/origin/master;
#done

FILE=test.txt
text='test text for file'

create_file_in_repo () {
  if [ ! -f "$FILE" ]; then
    echo ${1} > $FILE
    git add $FILE
    echo "file was added to the repo"
  fi
}

change_file () {
  text=${1}
  count=${note}
  echo "${text} ${note}" >> $FILE
  echo "file is changed"
}

do_commit () {
  count=${1}
  note=${2}
  branch=${3}
  change_file $text $count
  git commit -am "TST-1 created test commit ${note}"
  echo "committed for branch ${branch}"
}

do_push() {
  branch=${1}
  git push origin $branch
}

go_to_branch () {
  branch=${1}
  git checkout $branch
  echo "moved to branch ${branch}"
}

do_branch_acions () {
  branch=${1}
  counter=${2}
  go_to_branch $branch $counter
  create_file_in_repo $text
  for i in {1..3}
  do
    do_commit $counter $i $branch
  done
  do_push $branch
}

counter=0
calc_counter () {
  ((counter=counter+1))
}



for branch in $(git for-each-ref --format='%(refname:short)' refs/heads)
do
  branch_name=$branch
  calc_counter
  do_branch_acions $branch_name $counter
done


#for branch in $(git for-each-ref --format='%(refname:short)' refs/remotes)
#do
#  branch_name = $branch | cut -d '/' -f 2
#  calc_counter
#  do_branch_acions $branch_name $counter
#done


