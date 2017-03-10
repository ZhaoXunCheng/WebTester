#include<signal.h>

#ifndef Signal_h_
#define Signal_h_

typedef void SigFunc(int);

SigFunc* Signal(int signo, SigFunc* func){
    struct sigaction act;
    struct sigaction oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(signo == SIGALRM){
#ifdef  SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif // SA_INTERRUPT
    }
    else{
#ifdef  SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif // SA_RESTART
    }
    if(sigaction(signo, &act, &oact) < 0){
        return SIG_ERR;
    }
    return oact.sa_handler;
}


#endif
