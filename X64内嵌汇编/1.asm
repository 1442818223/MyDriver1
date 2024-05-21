.code 
  

extern test1:proc  ;必须声明

add1 proc
    add rcx, rdx
    mov rax,rcx
    ret
add1 endp



test2 proc
mov rcx,0ch   ;不能用0x  要用h后缀表示16进制,前面的0要补上
mov rdx,5
sub rsp,48h
call test1   ;调用c的函数
add rsp,48h
ret
test2 endp



end
