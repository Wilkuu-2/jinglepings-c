


trap "kill 0" EXIT

N=$1 
shift 

for i in {1..$N}; do $@ & 
done  

wait
