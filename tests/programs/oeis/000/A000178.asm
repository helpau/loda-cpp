; A000178: Superfactorials: product of first n factorials.
; Submitted by Jamie Morken
; 1,1,2,12,288,34560,24883200,125411328000,5056584744960000,1834933472251084800000,6658606584104736522240000000,265790267296391946810949632000000000,127313963299399416749559771247411200000000000,792786697595796795607377086400871488552960000000000000,69113789582492712943486800506462734562847413501952000000000000000,90378331112371142262979521568630736335023247731599748366336000000000000000000,1890966832292234727042877370627225068196418587883634153182519380410368000000000000000000000

mov $1,1
mov $2,1
mov $3,1
lpb $0
  sub $0,1
  add $2,1
  mul $3,$1
  mul $1,$2
lpe
mov $0,$3