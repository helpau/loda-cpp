; A022322: a(n) = a(n-1) + a(n-2) + 1, with a(0) = 1 and a(1) = 8.
; 1,8,10,19,30,50,81,132,214,347,562,910,1473,2384,3858,6243,10102,16346,26449,42796,69246,112043,181290,293334,474625,767960,1242586,2010547,3253134,5263682,8516817,13780500,22297318,36077819,58375138,94452958,152828097,247281056,400109154,647390211,1047499366,1694889578,2742388945,4437278524,7179667470,11616945995,18796613466,30413559462,49210172929,79623732392,128833905322,208457637715,337291543038,545749180754,883040723793,1428789904548,2311830628342,3740620532891,6052451161234,9793071694126

mov $3,6
add $0,2
lpb $0
  sub $0,1
  mov $1,$3
  add $2,1
  mov $3,$2
  add $2,$1
lpe
mov $0,$1