; A134271: a(n) = a(n-2) + 2*a(n-3), n > 3; with a(0)=0, a(1)=1, a(2)=2, a(3)=0.
; 0,1,2,0,4,4,4,12,12,20,36,44,76,116,164,268,396,596,932,1388,2124,3252,4900,7500,11404,17300,26404,40108,61004,92916,141220,214924,327052,497364,756900,1151468,1751628,2665268,4054564,6168524,9385100

mov $2,2
mov $4,2
sub $0,2
lpb $0
  sub $0,1
  mov $3,$1
  mov $1,$2
  mov $2,$4
  mov $4,$3
  add $1,$2
  mul $2,2
lpe
add $0,$4