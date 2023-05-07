subroutine unpack77(c77,nrx,msg,unpk77_success)
!
! nrx=1 when unpacking a received message
! nrx=0 when unpacking a to-be-transmitted message
! the value of nrx is used to decide when mycall13 or dxcall13 should
! be used in place of a callsign from the hashtable
!

  character*77 c77
  character*37 msg

800 return
end subroutine unpack77