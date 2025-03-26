

# ~/z3/build/z3 trace=true -T:1 simpl_bad.smt2; mv z3.log bad.log
# ~/z3/build/z3 trace=true simpl_good.smt2; mv z3.log good.log

~/z3/build/z3 trace=true -T:1 scrambled_bad.smt2; mv z3.log bad.log
~/z3/build/z3 trace=true scrambled_good.smt2; mv z3.log good.log

wc -l good.log 
wc -l bad.log 

head -n 30000 bad.log > trunc.bad.log
head -n 30000 good.log > trunc.good.log
code --diff trunc.bad.log trunc.good.log