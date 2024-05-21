.code 

callgate proc
      call far ptr[rcx]
callgate endp


callgateRet proc
      retf
callgateRet endp

end