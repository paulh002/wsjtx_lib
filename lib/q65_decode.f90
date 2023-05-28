module q65_decode

  integer nsnr0,nfreq0
  real xdt0
  character msg0*37,cq0*3

  type :: q65_decoder
     procedure(q65_decode_callback), pointer :: callback
   contains
     procedure :: decode
  end type q65_decoder

  abstract interface
     subroutine q65_decode_callback (this,nutc,snr1,nsnr,dt,freq,    &
          decoded,idec,nused,ntrperiod)
       import q65_decoder
       implicit none
       class(q65_decoder), intent(inout) :: this
       integer, intent(in) :: nutc
       real, intent(in) :: snr1
       integer, intent(in) :: nsnr
       real, intent(in) :: dt
       real, intent(in) :: freq
       character(len=37), intent(in) :: decoded
       integer, intent(in) :: idec
       integer, intent(in) :: nused
       integer, intent(in) :: ntrperiod
     end subroutine q65_decode_callback
  end interface

contains

  subroutine decode(this,callback,iwave,nqd0,nutc,ntrperiod,nsubmode,nfqso,  &
       ntol,ndepth,nfa0,nfb0,lclearave,single_decode,lagain,max_drift0,      &
       lnewdat0,emedelay,mycall,hiscall,hisgrid,nQSOprogress,ncontest,       &
       lapcqonly,navg0,nqf)

! Top-level routine that organizes the decoding of Q65 signals
! Input:  iwave            Raw data, i*2
!         nutc             UTC for time-tagging the decode
!         ntrperiod        T/R sequence length (s)
!         nsubmode         Tone-spacing indicator, 0-4 for A-E
!         nfqso            Target signal frequency (Hz)
!         ntol             Search range around nfqso (Hz)
!         ndepth           Optional decoding level
!         lclearave        Flag to clear the message-averaging arrays
!         emedelay         Sync search extended to cover EME delays
!         nQSOprogress     Auto-sequencing state for the present QSO
!         ncontest         Supported contest type
!         lapcqonly        Flag to use AP only for CQ calls
! Output: sent to the callback routine for display to user

    use timer_module, only: timer
    use packjt77
    use, intrinsic :: iso_c_binding
    use q65                               !Shared variables
    use prog_args
    use types
 
    parameter (NMAX=300*12000)  !Max TRperiod is 300 s
    parameter (MAX_CALLERS=40)  !For multiple q3 decodes in NA VHf Contest mode

    class(q65_decoder), intent(inout) :: this

    procedure(q65_decode_callback) :: callback
    character(len=12) :: mycall, hiscall  !Used for AP decoding
    character(len=6) :: hisgrid
    character*37 decoded                  !Decoded message
    character*37 decodes(100)
    character*77 c77
    character*78 c78
    character*6 cutc
    character c6*6,c4*4,cmode*4
    character*80 fmt
    integer*2 iwave(NMAX)                 !Raw data
    real, allocatable :: dd(:)            !Raw data
    real xdtdecodes(100)
    real f0decodes(100)
    integer dat4(13)                      !Decoded message as 12 6-bit integers
    integer dgen(13)
    integer nqf(20)
    integer stageno                       !Added by W3SZ
    integer time
    logical lclearave,lnewdat0,lapcqonly,unpk77_success
    logical single_decode,lagain
    complex, allocatable :: c00(:)        !Analytic signal, 6000 Sa/s
    complex, allocatable :: c0(:)         !Analytic signal, 6000 Sa/s
    type(q3list) callers(MAX_CALLERS)

! Start by setting some parameters and allocating storage for large arrays
    call sec0(0,tdecode)
    stageno=0
    ndecodes=0
    decodes=' '
    f0decodes=0.
    xdtdecodes=0.
    nfa=nfa0
    nfb=nfb0
    nqd=nqd0
    lnewdat=lnewdat0
    max_drift=max_drift0
    idec=-1
    idf=0
    idt=0
    nrc=-2
    mode_q65=2**nsubmode
    npts=ntrperiod*12000
    nfft1=ntrperiod*12000
    nfft2=ntrperiod*6000
    npasses=1
    nhist2=0
    if(lagain) ndepth=ior(ndepth,3)       !Use 'Deep' for manual Q65 decodes
    dxcall13=hiscall  ! initialize for use in packjt77
    mycall13=mycall
    if(ncontest.eq.1) then
! NA VHF, WW-Digi, or ARRL Digi Contest
       open(24,file=trim(data_dir)//'/tsil.3q',status='unknown',     &
            form='unformatted')
       read(24,end=2) nhist2
       if(nhist2.ge.1 .and. nhist2.le.40) then
          read(24,end=2) callers(1:nhist2)
          now=time()
          do i=1,nhist2
             hours=(now - callers(i)%nsec)/3600.0
             if(hours.gt.24.0) then
                callers(i:nhist2-1)=callers(i+1:nhist2)
                nhist2=nhist2-1
             endif
          enddo
       else
          nhist2=0
       endif
2      close(24)
    endif

! Determine the T/R sequence: iseq=0 (even), or iseq=1 (odd)
    n=nutc
    if(ntrperiod.ge.60 .and. nutc.le.2359) n=100*n
    write(cutc,'(i6.6)') n
    read(cutc,'(3i2)') ih,im,is
    nsec=3600*ih + 60*im + is
    iseq=mod(nsec/ntrperiod,2)

    if(lclearave) call q65_clravg
    allocate(dd(npts))
    allocate (c00(0:nfft1-1))
    allocate (c0(0:nfft1-1))

    if(lagain) then
       call q65_hist(nfqso,dxcall=hiscall,dxgrid=hisgrid)
    endif

    nsps=1800
    if(ntrperiod.eq.30) then
       nsps=3600
    else if(ntrperiod.eq.60) then
       nsps=7200
    else if(ntrperiod.eq.120) then
       nsps=16000
    else if(ntrperiod.eq.300) then
       nsps=41472
    endif

    baud=12000.0/nsps
    this%callback => callback
    nFadingModel=1

!    ibwa=max(1,int(1.8*log(baud*mode_q65)) + 5)
!### This needs work!
    ibwa=1                          !Q65-60A
    if(mode_q65.eq.2) ibwa=3        !Q65-60B
    if(mode_q65.eq.4) ibwa=8        !Q65-60C
    if(mode_q65.eq.2) ibwa=9        !Q65-60D
    if(mode_q65.eq.2) ibwa=10       !Q65-60E
!###

!    ibwb=min(15,ibwa+4)
    ibwb=min(15,ibwa+6)
    maxiters=40
    if(iand(ndepth,3).eq.2) maxiters=60
    if(iand(ndepth,3).eq.3) then
       ibwa=max(1,ibwa-2)
       ibwb=ibwb+2
       maxiters=100
    endif

! Generate codewords for full-AP list decoding
    if(ichar(hiscall(1:1)).eq.0) hiscall=' '
    if(ichar(hisgrid(1:1)).eq.0) hisgrid=' '
    ncw=0
    if(nqd.eq.1 .or. lagain .or. ncontest.eq.1) then
       if(ncontest.eq.1) then
          call q65_set_list2(mycall,hiscall,hisgrid,callers,nhist2,   &
               codewords,ncw)
       else
          call q65_set_list(mycall,hiscall,hisgrid,codewords,ncw)
       endif
    endif
    dgen=0
    call q65_enc(dgen,codewords)         !Initialize the Q65 codec
    nused=1
    iavg=0

! W3SZ patch: Initialize AP params here, rather than afer the call to ana64().
    call ft8apset(mycall,hiscall,ncontest,apsym0,aph10) ! Generate ap symbols
    where(apsym0.eq.-1) apsym0=0
    npasses=2
    if(nQSOprogress.eq.5) npasses=3

    call timer('q65_dec0',0)
! Call top-level routine in q65 module: establish sync and try for a
! q3 or q0 decode.
    call q65_dec0(iavg,iwave,ntrperiod,nfqso,ntol,lclearave,  &
         emedelay,xdt,f0,snr1,width,dat4,snr2,idec,stageno)
    call timer('q65_dec0',1)

    if(idec.ge.0) then
       dtdec=xdt                    !We have a q3 or q0 decode at nfqso
       f0dec=f0
       go to 100
    endif

    if(ncontest.eq.1 .and. lagain .and. iand(ndepth,16).eq.16) go to 50
    if(ncontest.eq.1 .and. lagain .and. iand(ndepth,16).eq.0) go to 100

! Prepare for a single-period decode with iaptype = 0, 1, 2, or 4
    jpk0=(xdt+1.0)*6000                      !Index of nominal start of signal
    if(ntrperiod.le.30) jpk0=(xdt+0.5)*6000  !For shortest sequences
    if(jpk0.lt.0) jpk0=0
    call ana64(iwave,npts,c00)          !Convert to complex c00() at 6000 Sa/s
    if(lapcqonly) npasses=1
    iaptype=0
    do ipass=0,npasses                  !Loop over AP passes
       apmask=0                         !Try first with no AP information
       apsymbols=0
       if(ipass.ge.1) then
          ! Subsequent passes use AP information appropiate for nQSOprogress
          call q65_ap(nQSOprogress,ipass,ncontest,lapcqonly,iaptype,   &
               apsym0,apmask1,apsymbols1)
          write(c78,1050) apmask1
1050      format(78i1)
          read(c78,1060) apmask
1060      format(13b6.6)
          write(c78,1050) apsymbols1
          read(c78,1060) apsymbols
       endif

       call timer('q65loop1',0)
       call q65_loops(c00,npts/2,nsps/2,nsubmode,ndepth,jpk0,   &
            xdt,f0,iaptype,xdt1,f1,snr2,dat4,idec)
       call timer('q65loop1',1)
       if(idec.ge.0) then
          dtdec=xdt1
          f0dec=f1
          go to 100       !Successful decode, we're done
       endif
    enddo  ! ipass

    if(iand(ndepth,16).eq.0 .or. navg(iseq).lt.2) go to 100

! There was no single-transmission decode. Try for an average 'q3n' decode.
50  iavg=1
    call timer('list_avg',0)
! Call top-level routine in q65 module: establish sync and try for a q3
! decode, this time using the cumulative 's1a' symbol spectra.
    call q65_dec0(iavg,iwave,ntrperiod,nfqso,ntol,lclearave,  &
         emedelay,xdt,f0,snr1,width,dat4,snr2,idec,stageno)
    call timer('list_avg',1)

    if(idec.ge.0) then
       dtdec=xdt               !We have a list-decode result from averaged data
       f0dec=f0
       nused=navg(iseq)
       go to 100
    endif

! There was no 'q3n' decode.  Try for a 'q[0124]n' decode.
! Call top-level routine in q65 module: establish sync and try for a q[012]n
! decode, this time using the cumulative 's1a' symbol spectra.

    call timer('q65_avg ',0)
    iavg=2
    call q65_dec0(iavg,iwave,ntrperiod,nfqso,ntol,lclearave,  &
         emedelay,xdt,f0,snr1,width,dat4,snr2,idec,stageno)
    call timer('q65_avg ',1)
    if(idec.ge.0) then
       dtdec=xdt                          !We have a q[012]n result
       f0dec=f0
       nused=navg(iseq)
    endif

100 if(idec.lt.0 .and. max_drift.eq.50) then
       stageno = 5
       call timer('q65_dec0',0)
       ! Call top-level routine in q65 module: establish sync and try for a
       ! q3 or q0 decode.
       call q65_dec0(iavg,iwave,ntrperiod,nfqso,ntol,lclearave,  &
            emedelay,xdt,f0,snr1,width,dat4,snr2,idec,stageno)
       call timer('q65_dec0',1)
       if(idec.ge.0) then
          dtdec=xdt             !We have a q[012]n result
          f0dec=f0
       endif
    endif                       ! if(idec.lt.0)

    decoded='                                     '
    if(idec.ge.0) then
! idec Meaning
! ------------------------------------------------------
! -1:  No decode
!  0:  Decode without AP information
!  1:  Decode with AP for "CQ        ?   ?"
!  2:  Decode with AP for "MyCall    ?   ?"
!  3:  Decode with AP for "MyCall DxCall ?"

! Unpack decoded message for display to user
       write(c77,1000) dat4(1:12),dat4(13)/2
1000   format(12b6.6,b5.5)
       call unpack77(c77,1,decoded,unpk77_success) !Unpack to get decoded
       idupe=0
       do i=1,ndecodes
          if(decodes(i).eq.decoded) idupe=1
       enddo
       if(idupe.eq.0) then
          ndecodes=min(ndecodes+1,100)
          decodes(ndecodes)=decoded
          f0decodes(ndecodes)=f0dec
          xdtdecodes(ndecodes)=dtdec
          call q65_snr(dat4,dtdec,f0dec,mode_q65,snr2)
          nsnr=nint(snr2)
          call this%callback(nutc,snr1,nsnr,dtdec,f0dec,decoded,    &
               idec,nused,ntrperiod)
          if(ncontest.eq.1) then
             call q65_hist2(nint(f0dec),decoded,callers,nhist2)
          else
             call q65_hist(nint(f0dec),msg0=decoded)
          endif
          if(iand(ndepth,128).ne.0 .and. .not.lagain .and.      &
               int(abs(f0dec-nfqso)).le.ntol ) call q65_clravg    !AutoClrAvg
          call sec0(1,tdecode)
          open(22,file=trim(data_dir)//'/q65_decodes.txt',status='unknown',  &
               position='append',iostat=ios)
          if(ios.eq.0) then
! Save decoding parameters to q65_decoded.dat, for later analysis.
             write(cmode,'(i3)') ntrperiod
             cmode(4:4)=char(ichar('A')+nsubmode)
             c6=hiscall(1:6)
             if(c6.eq.'      ') c6='<b>   '
             c4=hisgrid(1:4)
             if(c4.eq.'    ') c4='<b> '
             fmt='(i6.4,1x,a4,i5,4i2,8i3,i4,f6.2,f7.1,f6.1,f7.1,f6.2,'//   &
                  '1x,a6,1x,a6,1x,a4,1x,a)'
             if(ntrperiod.le.30) fmt(5:5)='6'
             if(idec.eq.3) nrc=0
             write(22,fmt) nutc,cmode,nfqso,nQSOprogress,idec,idfbest,idtbest, &
                  ibwa,ibwb,ibw,ndistbest,nused,icand,ncand,nrc,ndepth,xdt,    &
                  f0,snr2,plog,tdecode,mycall(1:6),c6,c4,trim(decoded)
             close(22)
          endif
       endif
    endif
    navg0=1000*navg(0) + navg(1)
    if(single_decode .or. lagain) go to 900

    do icand=1,ncand
! Prepare for single-period candidate decodes with iaptype = 0, 1, 2, or 4
       snr1=candidates(icand,1)
       xdt= candidates(icand,2)
       f0 = candidates(icand,3)
       do i=1,ndecodes
          fdiff=f0-f0decodes(i)
          if(fdiff.gt.-baud*mode_q65 .and. fdiff.lt.65*baud*mode_q65) go to 800
       enddo

!###  TEST REGION
       if(ncontest.eq.-1) then
          call timer('q65_dec0',0)
! Call top-level routine in q65 module: establish sync and try for a
! q3 or q0 decode.
          call q65_dec0(iavg,iwave,ntrperiod,nint(f0),ntol,lclearave,  &
               emedelay,xdt,f0,snr1,width,dat4,snr2,idec,stageno)
          call timer('q65_dec0',1)
          if(idec.ge.0) then
             dtdec=xdt                    !We have a q3 or q0 decode at f0
             f0dec=f0
             go to 200
          endif
       endif
!###
       jpk0=(xdt+1.0)*6000                   !Index of nominal start of signal
       if(ntrperiod.le.30) jpk0=(xdt+0.5)*6000  !For shortest sequences
       if(jpk0.lt.0) jpk0=0
       call ana64(iwave,npts,c00)       !Convert to complex c00() at 6000 Sa/s
       call ft8apset(mycall,hiscall,ncontest,apsym0,aph10) ! Generate ap symbols
       where(apsym0.eq.-1) apsym0=0

       npasses=2
       if(nQSOprogress.eq.5) npasses=3
       if(lapcqonly) npasses=1
       iaptype=0
       do ipass=0,npasses                  !Loop over AP passes
!          write(*,3001) nutc,icand,ipass,f0,xdt,snr1
!3001      format('a',i5.4,2i3,3f7.1)
          apmask=0                         !Try first with no AP information
          apsymbols=0
          if(ipass.ge.1) then
          ! Subsequent passes use AP information appropiate for nQSOprogress
             call q65_ap(nQSOprogress,ipass,ncontest,lapcqonly,iaptype,   &
                  apsym0,apmask1,apsymbols1)
             write(c78,1050) apmask1
             read(c78,1060) apmask
             write(c78,1050) apsymbols1
             read(c78,1060) apsymbols
          endif

          call timer('q65loop2',0)
          call q65_loops(c00,npts/2,nsps/2,nsubmode,ndepth,jpk0,   &
               xdt,f0,iaptype,xdt1,f1,snr2,dat4,idec)
          call timer('q65loop2',1)
!          write(*,3001) '=e',nfqso,ntol,ndepth,xdt,f0,idec
          if(idec.ge.0) then
             dtdec=xdt1
             f0dec=f1
             go to 200       !Successful decode, we're done
          endif
       enddo  ! ipass

200    decoded='                                     '
       if(idec.ge.0) then
! Unpack decoded message for display to user
          write(c77,1000) dat4(1:12),dat4(13)/2
          call unpack77(c77,1,decoded,unpk77_success) !Unpack to get decoded
          idupe=0
          do i=1,ndecodes
             if(decodes(i).eq.decoded) idupe=1
          enddo
          if(idupe.eq.0) then
             ndecodes=min(ndecodes+1,100)
             decodes(ndecodes)=decoded
             f0decodes(ndecodes)=f0dec
             call q65_snr(dat4,dtdec,f0dec,mode_q65,snr2)
             nsnr=nint(snr2)
             call this%callback(nutc,snr1,nsnr,dtdec,f0dec,decoded,    &
                  idec,nused,ntrperiod)
             if(ncontest.eq.1) then
                call q65_hist2(nint(f0dec),decoded,callers,nhist2)
             else
                call q65_hist(nint(f0dec),msg0=decoded)
             endif
             if(iand(ndepth,128).ne.0 .and. .not.lagain .and.      &
                  int(abs(f0dec-nfqso)).le.ntol ) call q65_clravg    !AutoClrAvg
             call sec0(1,tdecode)
             ios=1
             open(22,file=trim(data_dir)//'/q65_decodes.txt',status='unknown',&
                  position='append',iostat=ios)
             if(ios.eq.0) then
! Save decoding parameters to q65_decoded.dat, for later analysis.
                write(cmode,'(i3)') ntrperiod
                cmode(4:4)=char(ichar('A')+nsubmode)
                c6=hiscall(1:6)
                if(c6.eq.'      ') c6='<b>   '
                c4=hisgrid(1:4)
                if(c4.eq.'    ') c4='<b> '
                fmt='(i6.4,1x,a4,i5,4i2,8i3,i4,f6.2,f7.1,f6.1,f7.1,f6.2,'//   &
                     '1x,a6,1x,a6,1x,a4,1x,a)'
                if(ntrperiod.le.30) fmt(5:5)='6'
                if(idec.eq.3) nrc=0
                write(22,fmt) nutc,cmode,nfqso,nQSOprogress,idec,idfbest,    &
                     idtbest,ibwa,ibwb,ibw,ndistbest,nused,icand,ncand,nrc,  &
                     ndepth,xdt,f0,snr2,plog,tdecode,mycall(1:6),c6,c4,      &
                     trim(decoded)
                close(22)
             endif
          endif
       endif
800    continue
    enddo  ! icand
    if(iavg.eq.0 .and.navg(iseq).ge.2 .and. iand(ndepth,16).ne.0) go to 50

900 if(ncontest.ne.1 .or. lagain) go to 999
    if(ntrperiod.ne.60 .or. nsubmode.ne.0) go to 999

! This is first time here, and we're running Q65-60A in NA VHF Contest mode.
! Return a list of potential sync frequencies at which to try q3 decoding.

    k=0
    nqf=0
    bw=baud*mode_q65*65
    do i=1,ncand
!       snr1=candidates(i,1)
!       xdt= candidates(i,2)
       f0 = candidates(i,3)
       do j=1,ndecodes          ! Already decoded one at or near this frequency?
          fj=f0decodes(j)
          if(f0.gt.fj-5.0 .and. f0.lt.fj+bw+5.0) go to 990
       enddo
       k=k+1
       nqf(k)=nint(f0)
990    continue
    enddo

999 return
  end subroutine decode

end module q65_decode
