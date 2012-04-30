#ifndef AREXPORT_H
#define AREXPORT_H

#if defined(_WIN32) || defined(WIN32)

#ifndef SWIG
#ifndef ARIA_STATIC
#undef AREXPORT
#define AREXPORT _declspec(dllexport)
#else // ARIA_STATIC
#define AREXPORT
#endif // ARIA_STATIC
#endif // SWIG

#else // WIN32

#define AREXPORT

#endif // WIN32

#endif // AREXPORT_H


