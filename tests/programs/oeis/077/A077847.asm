; A077847: Expansion of (1-x)^(-1)/(1-2*x-2*x^2+2*x^3).
; 1,3,9,23,59,147,367,911,2263,5615,13935,34575,85791,212863,528159,1310463,3251519,8067647,20017407,49667071,123233663,305766655,758666495,1882398975,4670597631,11588660223,28753717759,71343560703,177017236479,439214158847

mov $3,1
add $0,1
lpb $0
  sub $0,1
  add $2,$3
  mov $3,$1
  add $4,$2
  mov $1,$2
  mul $1,2
  mov $2,$4
  add $2,1
lpe
mov $0,$4