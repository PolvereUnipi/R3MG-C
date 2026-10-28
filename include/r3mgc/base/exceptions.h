// -----------------------------------------------------------------------------
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception OR LGPL-2.1-or-later
// Copyright (C) 2026 by the R3MG-C authors
//
// This file is part of the R3MG-C library.
//
// Detailed license information governing the source code
// can be found in LICENSE.md at the top level directory.
//
// -----------------------------------------------------------------------------

#ifndef r3mgc_base_exceptions_h
#define r3mgc_base_exceptions_h

#include <r3mgc/base/config.h>

#include <exception>
#include <ostream>
#include <sstream>
#include <string>

/**
 * @file
 *
 * A small exception mechanism modelled on the one of deal.II.
 *
 * Two kinds of checks are provided:
 * - R3MGC_Assert(cond, exc): an internal consistency check, only active in
 *   debug mode (i.e., when NDEBUG is not defined). It is meant to catch
 *   programming errors, both in the library and in user code calling it.
 * - R3MGC_AssertThrow(cond, exc): a check that is always active, used for
 *   errors that can happen at run time even in a correct program (e.g., an
 *   agglomeration hierarchy that is not admissible for the given
 *   parameters).
 *
 * Contrary to deal.II, a failed R3MGC_Assert does not abort the program: both
 * macros throw the exception. This makes the library friendlier to callers
 * from other languages, and lets the test suite check that the right error
 * is raised.
 *
 * The macros carry the R3MGC_ prefix so that they never clash with the ones
 * of deal.II (or of any other library) in a program that includes both.
 *
 * New exception classes are declared with the R3MGC_DeclException* macros:
 * @code
 * R3MGC_DeclException2(ExcTooFewPoints,
 *                      unsigned int,
 *                      unsigned int,
 *                      << "Found " << arg1 << " points, but at least "
 *                      << arg2 << " are needed.");
 * @endcode
 */

R3MGC_NAMESPACE_OPEN

/**
 * Base class of all the exceptions thrown by the library.
 *
 * Besides the message specific to every exception (printed by
 * print_info()), it stores where the exception was raised: file, line,
 * function, the violated condition, and the name of the exception. All of it
 * is combined in the string returned by what().
 */
class ExceptionBase : public std::exception
{
public:
  /**
   * Default constructor. The location fields are filled by set_fields(),
   * which is called by the R3MGC_Assert and R3MGC_AssertThrow macros.
   */
  ExceptionBase() = default;

  /**
   * Virtual destructor, to allow for derived classes.
   */
  virtual ~ExceptionBase() noexcept override = default;

  /**
   * Store the location where the exception was raised, and build the
   * message returned by what().
   */
  void
  set_fields(const char *file,
             const int   line,
             const char *function,
             const char *condition,
             const char *exception_name);

  /**
   * Return the complete description of the exception: location, violated
   * condition, name, and additional information.
   */
  virtual const char *
  what() const noexcept override;

  /**
   * Return the name of the exception, e.g., "ExcMessage(\"...\")", as it was
   * written in the call to the macro that raised it.
   */
  const char *
  get_exc_name() const;

  /**
   * Return only the additional information specific to this exception, i.e.
   * the text written by print_info(). Unlike what(), this string does not
   * depend on file names and line numbers, which makes it useful in tests.
   */
  std::string
  get_additional_info() const;

  /**
   * Write the information specific to this exception to @p out. Derived
   * classes override this function; the default prints nothing.
   */
  virtual void
  print_info(std::ostream &out) const;

protected:
  /**
   * File where the exception was raised.
   */
  const char *file = "";

  /**
   * Line where the exception was raised.
   */
  int line = 0;

  /**
   * Name of the function where the exception was raised.
   */
  const char *function = "";

  /**
   * The violated condition, as a string.
   */
  const char *condition = "";

  /**
   * The name of the exception, as a string.
   */
  const char *exception_name = "";

  /**
   * The full message returned by what(). It is built by set_fields(), since
   * what() cannot allocate memory.
   */
  std::string what_str;
};



/**
 * Declare an exception class without parameters, which prints the fixed
 * message @p text.
 */
#define R3MGC_DeclExceptionMsg(Exception, text)   \
  class Exception : public ::r3mgc::ExceptionBase \
  {                                               \
  public:                                         \
    virtual void                                  \
    print_info(std::ostream &out) const override  \
    {                                             \
      out << "    " text << std::endl;            \
    }                                             \
  }

/**
 * Declare an exception class with one parameter. The message @p outsequence
 * is a sequence of stream insertions starting with <<, and can refer to the
 * parameter as arg1.
 */
#define R3MGC_DeclException1(Exception, type1, outsequence) \
  class Exception : public ::r3mgc::ExceptionBase           \
  {                                                         \
  public:                                                   \
    Exception(type1 const &a1)                              \
      : arg1(a1)                                            \
    {}                                                      \
    virtual void                                            \
    print_info(std::ostream &out) const override            \
    {                                                       \
      out << "    " outsequence << std::endl;               \
    }                                                       \
                                                            \
  private:                                                  \
    type1 const arg1;                                       \
  }

/**
 * Declare an exception class with two parameters, named arg1 and arg2 in
 * @p outsequence.
 */
#define R3MGC_DeclException2(Exception, type1, type2, outsequence) \
  class Exception : public ::r3mgc::ExceptionBase                  \
  {                                                                \
  public:                                                          \
    Exception(type1 const &a1, type2 const &a2)                    \
      : arg1(a1)                                                   \
      , arg2(a2)                                                   \
    {}                                                             \
    virtual void                                                   \
    print_info(std::ostream &out) const override                   \
    {                                                              \
      out << "    " outsequence << std::endl;                      \
    }                                                              \
                                                                   \
  private:                                                         \
    type1 const arg1;                                              \
    type2 const arg2;                                              \
  }

/**
 * Declare an exception class with three parameters, named arg1, arg2 and
 * arg3 in @p outsequence.
 */
#define R3MGC_DeclException3(Exception, type1, type2, type3, outsequence) \
  class Exception : public ::r3mgc::ExceptionBase                         \
  {                                                                       \
  public:                                                                 \
    Exception(type1 const &a1, type2 const &a2, type3 const &a3)          \
      : arg1(a1)                                                          \
      , arg2(a2)                                                          \
      , arg3(a3)                                                          \
    {}                                                                    \
    virtual void                                                          \
    print_info(std::ostream &out) const override                          \
    {                                                                     \
      out << "    " outsequence << std::endl;                             \
    }                                                                     \
                                                                          \
  private:                                                                \
    type1 const arg1;                                                     \
    type2 const arg2;                                                     \
    type3 const arg3;                                                     \
  }



/**
 * The general purpose exceptions used throughout the library.
 */
namespace StandardExceptions
{
  /**
   * An exception carrying an arbitrary message. This is the most frequently
   * used exception of the library.
   */
  R3MGC_DeclException1(ExcMessage, std::string, << arg1);

  /**
   * Two quantities that should be equal (e.g., the sizes of two vectors) are
   * not.
   */
  R3MGC_DeclException2(ExcDimensionMismatch,
                       std::size_t,
                       std::size_t,
                       << "Dimension " << arg1 << " not equal to " << arg2
                       << ".");

  /**
   * An index is outside the half-open range [arg2, arg3).
   */
  R3MGC_DeclException3(ExcIndexRange,
                       std::size_t,
                       std::size_t,
                       std::size_t,
                       << "Index " << arg1 << " is not in the half-open range ["
                       << arg2 << "," << arg3 << ").");

  /**
   * A piece of functionality has not been implemented (yet).
   */
  R3MGC_DeclExceptionMsg(ExcNotImplemented,
                         "You are trying to use functionality that is not "
                         "implemented yet.");

  /**
   * A condition that should never fail, unless there is a bug in the
   * library.
   */
  R3MGC_DeclExceptionMsg(ExcInternalError,
                         "This exception indicates an internal error of the "
                         "library. Please report it to the developers.");
} // namespace StandardExceptions

using namespace StandardExceptions;



namespace internal
{
  /**
   * Fill the location fields of @p e and throw it. This function is called
   * by the R3MGC_Assert and R3MGC_AssertThrow macros, and should not be
   * called directly.
   */
  template <class ExceptionType>
  [[noreturn]] void
  throw_exception(const char   *file,
                  const int     line,
                  const char   *function,
                  const char   *condition,
                  const char   *exception_name,
                  ExceptionType e)
  {
    e.set_fields(file, line, function, condition, exception_name);
    throw e;
  }
} // namespace internal



/* ------------------------- Inline functions ------------------------------ */

inline void
ExceptionBase::set_fields(const char *f,
                          const int   l,
                          const char *func,
                          const char *cond,
                          const char *exc_name)
{
  file           = f;
  line           = l;
  function       = func;
  condition      = cond;
  exception_name = exc_name;

  std::ostringstream out;
  out << "\n--------------------------------------------------------\n"
      << "An error occurred in line <" << line << "> of file <" << file
      << "> in function\n"
      << "    " << function << "\n"
      << "The violated condition was: \n"
      << "    " << condition << "\n"
      << "The name and call sequence of the exception was:\n"
      << "    " << exception_name << "\n"
      << "Additional information: \n";
  print_info(out);
  out << "--------------------------------------------------------\n";

  what_str = out.str();
}



inline const char *
ExceptionBase::what() const noexcept
{
  return what_str.c_str();
}



inline const char *
ExceptionBase::get_exc_name() const
{
  return exception_name;
}



inline std::string
ExceptionBase::get_additional_info() const
{
  std::ostringstream out;
  print_info(out);
  return out.str();
}



inline void
ExceptionBase::print_info(std::ostream &) const
{}

R3MGC_NAMESPACE_CLOSE



/**
 * Check that @p cond holds, and throw @p exc otherwise. This check is always
 * active, also in release mode.
 */
#define R3MGC_AssertThrow(cond, exc)                       \
  do                                                       \
    {                                                      \
      if (!(cond))                                         \
        ::r3mgc::internal::throw_exception(                \
          __FILE__, __LINE__, __func__, #cond, #exc, exc); \
    }                                                      \
  while (false)

/**
 * Check that @p cond holds, and throw @p exc otherwise. This check is only
 * active in debug mode: in release mode the condition is not even
 * evaluated.
 */
#ifdef R3MGC_WITH_DEBUG
#  define R3MGC_Assert(cond, exc) R3MGC_AssertThrow(cond, exc)
#else
#  define R3MGC_Assert(cond, exc) \
    do                            \
      {                           \
      }                           \
    while (false)
#endif

/**
 * Check, in debug mode only, that the two sizes @p dim1 and @p dim2 are
 * equal.
 */
#define R3MGC_AssertDimension(dim1, dim2)        \
  R3MGC_Assert(static_cast<std::size_t>(dim1) == \
                 static_cast<std::size_t>(dim2), \
               ::r3mgc::ExcDimensionMismatch((dim1), (dim2)))

/**
 * Check, in debug mode only, that @p index lies in the half-open range
 * [0, @p range).
 */
#define R3MGC_AssertIndexRange(index, range)      \
  R3MGC_Assert(static_cast<std::size_t>(index) <  \
                 static_cast<std::size_t>(range), \
               ::r3mgc::ExcIndexRange((index), 0, (range)))

#endif
