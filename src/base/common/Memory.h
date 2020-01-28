#ifndef NEP_BASE_COMMON_MEMORY_H
#define NEP_BASE_COMMON_MEMORY_H

#include <new>
#include <stdlib.h>

namespace neptune {
namespace base {

template <typename Type> inline Type *gNew()
{
  Type *Pointer = NULL;
#ifdef _NO_EXCEPTION
  Pointer = new Type;
#else
  try
  {
    Pointer = new Type;
  }
  catch (...)
  {
    Pointer = NULL;
  }
#endif // _NO_EXCEPTION
  return Pointer;
}

template <typename Type> inline Type *gNewA(unsigned uiItemNum)
{
  Type *Pointer = NULL;
#ifdef _NO_EXCEPTION
  Pointer = new Type[uiItemNum];
#else
  try
  {
      Pointer = new Type[uiItemNum];
  }
  catch (...)
  {
      Pointer = NULL;
  }
#endif // _NO_EXCEPTION
  return Pointer;
}

// if not use the default construct, please use this macro, but is can't reference
#ifdef _NO_EXCEPTION
#define ARG_NEW(Pointer, Class, ...) \
  do \
  { \
    Pointer = new Class(__VA_ARGS__); \
  } \
  while (0)
#else
#define ARG_NEW(Pointer, Class, ...) \
  do \
  { \
    try \
    { \
      Pointer = new Class(__VA_ARGS__); \
    } \
    catch (...) \
    { \
      Pointer = NULL; \
    } \
  } \
  while (0)
#endif

template <typename Type> inline Type *gConstruct(void *p)
{
  Type *Pointer = NULL;
#ifdef _NO_EXCEPTION
  Pointer = new (p) Type;
#else
  try
  {
    Pointer = new (p) Type;
  }
  catch (...)
  {
    Pointer = NULL;
  }
#endif // _NO_EXCEPTION
  return Pointer;
}

/*
 * if not use the default construct, please use this macro, but is can't reference
 */
#ifdef _NO_EXCEPTION

#define CONSTRUCT(Pointer, Class, Memory, ...) \
    do \
    { \
        Pointer = new (Memory) Class(__VA_ARGS__); \
    } \
    while (0)
#else
#define CONSTRUCT(Pointer, Class, Memory, ...) \
    do \
    { \
        try \
        { \
            Pointer = new (Memory) Class(__VA_ARGS__); \
        } \
        catch (...) \
        { \
            Pointer = NULL; \
        } \
    } \
    while (0)
#endif // _NO_EXCEPTION

#ifdef _NO_EXCEPTION

#define FRIEND_NEW(Pointer, Class) \
    do \
    { \
        Pointer = new Class; \
    } \
    while (0)
#else
#define FRIEND_NEW(Pointer, Class) \
    do \
    { \
        try \
        { \
            Pointer = new Class; \
        } \
        catch (...) \
        { \
            Pointer = NULL; \
        } \
    } \
    while (0)
#endif

#ifdef _NO_EXCEPTION

#define FRIEND_NEW_A(Pointer, Class, Num) \
    do \
    { \
        Pointer = new Class [Num]; \
    } \
    while (0)
#else
#define FRIEND_NEW_A(Pointer, Class, Num) \
    do \
    { \
        try \
        { \
            Pointer = new Class [Num]; \
        } \
        catch (...) \
        { \
            Pointer = NULL; \
        } \
    } \
    while (0)
#endif

template <typename Type> inline void gDelete(Type *&rp)
{
    if (rp != NULL)
    {
#ifdef _NO_EXCEPTION
        delete rp;
        rp = NULL;
#else
        try
        {
            delete rp;
            rp = NULL;
        }
        catch (...)
        {

        }
#endif // _NO_EXCEPTION
        rp = NULL;
    }
}

template <typename Type> inline void gDeleteA(Type *&rp)
{
    if (rp != NULL)
    {
#ifdef _NO_EXCEPTION
        delete [] rp;
        rp = NULL;
#else
        try
        {
            delete [] rp;
            rp = NULL;
        }
        catch (...)
        {

        }
#endif // _NO_EXCEPTION
        rp = NULL;
    }
}

template <typename Type> inline void gDestruct(Type *p)
{
#ifdef _NO_EXCEPTION
    p->~Type();
#else
    try
    {
        p->~Type();
    }
    catch (...)
    {

    }
#endif // _NO_EXCEPTION
}

template <typename Type> inline void gFree(Type *&rp)
{
    if (rp != NULL)
    {
        free(rp);
        rp = NULL;
    }
}

#ifdef _NO_EXCEPTION

#define FRIEND_DEL(Pointer) \
    do \
    { \
        if (Pointer != NULL) \
        { \
            delete Pointer; \
            Pointer = NULL; \
        } \
    } \
    while (0)
#else
#define FRIEND_DEL(Pointer) \
    do \
    { \
        if (Pointer != NULL) \
        { \
            try \
            { \
                delete Pointer; \
                Pointer=NULL;\
            } \
            catch (...) \
            { \
                 \
            } \
            Pointer = NULL; \
        } \
    } \
    while (0)
#endif

#ifdef _NO_EXCEPTION

#define FRIEND_DEL_A(Pointer) \
    do \
    { \
        if (Pointer != NULL) \
        { \
            delete [] Pointer; \
            Pointer = NULL; \
        } \
    } \
    while (0)
#else
#define FRIEND_DEL_A(Pointer) \
    do \
    { \
        if (Pointer != NULL) \
        { \
            try \
            { \
                delete [] Pointer; \
                Pointer = NULL;\
            } \
            catch (...) \
            { \
                 \
            } \
            Pointer = NULL; \
        } \
    } \
    while (0)
#endif

} //namespace base
} //namespace neptune

#endif //NEP_BASE_COMMON_MEMORY_H
