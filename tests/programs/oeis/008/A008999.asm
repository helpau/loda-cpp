; A008999: a(n) = 2*a(n-1) + a(n-4).
; 1,2,4,8,17,36,76,160,337,710,1496,3152,6641,13992,29480,62112,130865,275722,580924,1223960,2578785,5433292,11447508,24118976,50816737,107066766,225581040,475281056,1001378849,2109824464,4445229968,9365740992,19732860833,41575546130,87596322228,184558385448,388849631729,819274809588,1726145941404,3636850268256,7662550168241,16144375146070,34014896233544,71666642735344,150995835638929,318136046423928,670286989081400,1412240620898144,2975477077435217,6269090201294362,13208467391670124

mov $5,1
lpb $0
  sub $0,1
  mov $4,$3
  mov $3,$2
  mov $2,$1
  mov $1,$5
  mul $5,2
  add $5,$4
lpe
mov $0,$5