# cm

```assembly
; Ackermann function
;
; input:
;   $0 = i
;   $1 = n
;
; output:
;   $2 = A(i,n)
;
; temporary variables:
;   $3 = i+1
;   $4 = j
;   $5 = transferring
;   $6..$(7+i) = diff (array)
;   $(8+i)..$(9+2i) = next (array)

; set arguments
mov $0,3      ; i = 3
mov $1,3      ; n = 3

; initialize i+1 variable
mov $3,$0     ; $3 = i
add $3,1      ; $3 = i+1

; initialize diff and next arrays
mov $4,$3     ; j = i+1
lpb $4,1      ; for j = i+1..1 do
  add $4,5    ;
  mov $$4,1   ;   diff[j] := 1
  add $4,$0   ;
  add $4,1    ;
  mov $$4,0   ;   next[j] := 0
  sub $4,$0   ;
  sub $4,7    ;
lpe           ; end for
add $6,$1     ; diff[0] := n+1

; main descent loop 
; lpb 5,

; lpe
```
