.code 
  

extern test1:proc  ;��������

add1 proc
    add rcx, rdx
    mov rax,rcx
    ret
add1 endp



test2 proc
mov rcx,0ch   ;������0x  Ҫ��h��׺��ʾ16����,ǰ���0Ҫ����
mov rdx,5
sub rsp,48h
call test1   ;����c�ĺ���
add rsp,48h
ret
test2 endp



end
