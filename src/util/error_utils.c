#include "util/error_utils.h"

#include <sys/errno.h>

const char *translate_errno(int error_number)
{
    switch (error_number) {

#define CASE_STATEMENT(e) \
    case e: return #e;

#define CASE_STATEMENT_COMMON(e, s) \
    case e: return s;

#ifdef EPERM
    CASE_STATEMENT(EPERM)
#endif

#ifdef ENOENT
    CASE_STATEMENT(ENOENT)
#endif

#ifdef ESRCH
    CASE_STATEMENT(ESRCH)
#endif

#ifdef EINTR
    CASE_STATEMENT(EINTR)
#endif

#ifdef EIO
    CASE_STATEMENT(EIO)
#endif

#ifdef ENXIO
    CASE_STATEMENT(ENXIO)
#endif

#ifdef E2BIG
    CASE_STATEMENT(E2BIG)
#endif

#ifdef ENOEXEC
    CASE_STATEMENT(ENOEXEC)
#endif

#ifdef EBADF
    CASE_STATEMENT(EBADF)
#endif

#ifdef ECHILD
    CASE_STATEMENT(ECHILD)
#endif

#ifdef EDEADLK
    CASE_STATEMENT(EDEADLK)
#endif

#ifdef ENOMEM
    CASE_STATEMENT(ENOMEM)
#endif

#ifdef EACCES
    CASE_STATEMENT(EACCES)
#endif

#ifdef EFAULT
    CASE_STATEMENT(EFAULT)
#endif

#ifdef ENOTBLK
    CASE_STATEMENT(ENOTBLK)
#endif

#ifdef EBUSY
    CASE_STATEMENT(EBUSY)
#endif

#ifdef EEXIST
    CASE_STATEMENT(EEXIST)
#endif

#ifdef EXDEV
    CASE_STATEMENT(EXDEV)
#endif

#ifdef ENODEV
    CASE_STATEMENT(ENODEV)
#endif

#ifdef ENOTDIR
    CASE_STATEMENT(ENOTDIR)
#endif

#ifdef EISDIR
    CASE_STATEMENT(EISDIR)
#endif

#ifdef EINVAL
    CASE_STATEMENT(EINVAL)
#endif

#ifdef ENFILE
    CASE_STATEMENT(ENFILE)
#endif

#ifdef EMFILE
    CASE_STATEMENT(EMFILE)
#endif

#ifdef ENOTTY
    CASE_STATEMENT(ENOTTY)
#endif

#ifdef ETXTBSY
    CASE_STATEMENT(ETXTBSY)
#endif

#ifdef EFBIG
    CASE_STATEMENT(EFBIG)
#endif

#ifdef ENOSPC
    CASE_STATEMENT(ENOSPC)
#endif

#ifdef ESPIPE
    CASE_STATEMENT(ESPIPE)
#endif

#ifdef EROFS
    CASE_STATEMENT(EROFS)
#endif

#ifdef EMLINK
    CASE_STATEMENT(EMLINK)
#endif

#ifdef EPIPE
    CASE_STATEMENT(EPIPE)
#endif

#ifdef EDOM
    CASE_STATEMENT(EDOM)
#endif

#ifdef ERANGE
    CASE_STATEMENT(ERANGE)
#endif

#ifdef EAGAIN
    CASE_STATEMENT_COMMON(EAGAIN, "EAGAIN|EWOULDBLOCK")
#endif

#ifdef EINPROGRESS
    CASE_STATEMENT(EINPROGRESS)
#endif

#ifdef EALREADY
    CASE_STATEMENT(EALREADY)
#endif

#ifdef ENOTSOCK
    CASE_STATEMENT(ENOTSOCK)
#endif

#ifdef EDESTADDRREQ
    CASE_STATEMENT(EDESTADDRREQ)
#endif

#ifdef EMSGSIZE
    CASE_STATEMENT(EMSGSIZE)
#endif

#ifdef EPROTOTYPE
    CASE_STATEMENT(EPROTOTYPE)
#endif

#ifdef ENOPROTOOPT
    CASE_STATEMENT(ENOPROTOOPT)
#endif

#ifdef EPROTONOSUPPORT
    CASE_STATEMENT(EPROTONOSUPPORT)
#endif

#ifdef ESOCKTNOSUPPORT
    CASE_STATEMENT(ESOCKTNOSUPPORT)
#endif

#ifdef ENOTSUP
    CASE_STATEMENT(ENOTSUP)
#endif

#ifdef EPFNOSUPPORT
    CASE_STATEMENT(EPFNOSUPPORT)
#endif

#ifdef EAFNOSUPPORT
    CASE_STATEMENT(EAFNOSUPPORT)
#endif

#ifdef EADDRINUSE
    CASE_STATEMENT(EADDRINUSE)
#endif

#ifdef EADDRNOTAVAIL
    CASE_STATEMENT(EADDRNOTAVAIL)
#endif

#ifdef ENETDOWN
    CASE_STATEMENT(ENETDOWN)
#endif

#ifdef ENETUNREACH
    CASE_STATEMENT(ENETUNREACH)
#endif

#ifdef ENETRESET
    CASE_STATEMENT(ENETRESET)
#endif

#ifdef ECONNABORTED
    CASE_STATEMENT(ECONNABORTED)
#endif

#ifdef ECONNRESET
    CASE_STATEMENT(ECONNRESET)
#endif

#ifdef ENOBUFS
    CASE_STATEMENT(ENOBUFS)
#endif

#ifdef EISCONN
    CASE_STATEMENT(EISCONN)
#endif

#ifdef ENOTCONN
    CASE_STATEMENT(ENOTCONN)
#endif

#ifdef ESHUTDOWN
    CASE_STATEMENT(ESHUTDOWN)
#endif

#ifdef ETOOMANYREFS
    CASE_STATEMENT(ETOOMANYREFS)
#endif

#ifdef ETIMEDOUT
    CASE_STATEMENT(ETIMEDOUT)
#endif

#ifdef ECONNREFUSED
    CASE_STATEMENT(ECONNREFUSED)
#endif

#ifdef ELOOP
    CASE_STATEMENT(ELOOP)
#endif

#ifdef ENAMETOOLONG
    CASE_STATEMENT(ENAMETOOLONG)
#endif

#ifdef EHOSTDOWN
    CASE_STATEMENT(EHOSTDOWN)
#endif

#ifdef EHOSTUNREACH
    CASE_STATEMENT(EHOSTUNREACH)
#endif

#ifdef ENOTEMPTY
    CASE_STATEMENT(ENOTEMPTY)
#endif

#ifdef EPROCLIM
    CASE_STATEMENT(EPROCLIM)
#endif

#ifdef EUSERS
    CASE_STATEMENT(EUSERS)
#endif

#ifdef EDQUOT
    CASE_STATEMENT(EDQUOT)
#endif

#ifdef ESTALE
    CASE_STATEMENT(ESTALE)
#endif

#ifdef EREMOTE
    CASE_STATEMENT(EREMOTE)
#endif

#ifdef EBADRPC
    CASE_STATEMENT(EBADRPC)
#endif

#ifdef ERPCMISMATCH
    CASE_STATEMENT(ERPCMISMATCH)
#endif

#ifdef EPROGUNAVAIL
    CASE_STATEMENT(EPROGUNAVAIL)
#endif

#ifdef EPROGMISMATCH
    CASE_STATEMENT(EPROGMISMATCH)
#endif

#ifdef EPROCUNAVAIL
    CASE_STATEMENT(EPROCUNAVAIL)
#endif

#ifdef ENOLCK
    CASE_STATEMENT(ENOLCK)
#endif

#ifdef ENOSYS
    CASE_STATEMENT(ENOSYS)
#endif

#ifdef EFTYPE
    CASE_STATEMENT(EFTYPE)
#endif

#ifdef EAUTH
    CASE_STATEMENT(EAUTH)
#endif

#ifdef ENEEDAUTH
    CASE_STATEMENT(ENEEDAUTH)
#endif

#ifdef EPWROFF
    CASE_STATEMENT(EPWROFF)
#endif

#ifdef EDEVERR
    CASE_STATEMENT(EDEVERR)
#endif

#ifdef EOVERFLOW
    CASE_STATEMENT(EOVERFLOW)
#endif

#ifdef EBADEXEC
    CASE_STATEMENT(EBADEXEC)
#endif

#ifdef EBADARCH
    CASE_STATEMENT(EBADARCH)
#endif

#ifdef ESHLIBVERS
    CASE_STATEMENT(ESHLIBVERS)
#endif

#ifdef EBADMACHO
    CASE_STATEMENT(EBADMACHO)
#endif

#ifdef ECANCELED
    CASE_STATEMENT(ECANCELED)
#endif

#ifdef EIDRM
    CASE_STATEMENT(EIDRM)
#endif

#ifdef ENOMSG
    CASE_STATEMENT(ENOMSG)
#endif

#ifdef EILSEQ
    CASE_STATEMENT(EILSEQ)
#endif

#ifdef ENOATTR
    CASE_STATEMENT(ENOATTR)
#endif

#ifdef EBADMSG
    CASE_STATEMENT(EBADMSG)
#endif

#ifdef EMULTIHOP
    CASE_STATEMENT(EMULTIHOP)
#endif

#ifdef ENODATA
    CASE_STATEMENT(ENODATA)
#endif

#ifdef ENOLINK
    CASE_STATEMENT(ENOLINK)
#endif

#ifdef ENOSR
    CASE_STATEMENT(ENOSR)
#endif

#ifdef ENOSTR
    CASE_STATEMENT(ENOSTR)
#endif

#ifdef EPROTO
    CASE_STATEMENT(EPROTO)
#endif

#ifdef ETIME
    CASE_STATEMENT(ETIME)
#endif

#ifdef ENOPOLICY
    CASE_STATEMENT(ENOPOLICY)
#endif

#ifdef ENOTRECOVERABLE
    CASE_STATEMENT(ENOTRECOVERABLE)
#endif

#ifdef EOWNERDEAD
    CASE_STATEMENT(EOWNERDEAD)
#endif

#ifdef EQFULL
    CASE_STATEMENT(EQFULL)
#endif

#ifdef ELAST
    CASE_STATEMENT(ELAST)
#endif

    default:
        break;
    }
    return "<unknown>";
}