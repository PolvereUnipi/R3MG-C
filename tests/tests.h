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

#ifndef r3mgc_tests_h
#define r3mgc_tests_h

/**
 * @file
 *
 * Common infrastructure for the tests of the library, modelled on deal.II's
 * tests/tests.h.
 *
 * Every test is a program that writes its results to the global log stream
 * `r3mglog`. After initlog() has been called, the log is written to the file
 * `output` in the current directory, which the test driver
 * (cmake/run_test.cmake) compares with the expected output stored next to
 * the source of the test. Every line of the log starts with the current
 * prefixes, e.g.
 * @code
 * r3mglog.push("2d");
 * r3mglog << "n_levels = " << 3 << std::endl;  // writes "R3MG:2d::n_levels =
 * 3" r3mglog.pop();
 * @endcode
 *
 * Floating point numbers are written with a fixed number of significant
 * digits, and numbers smaller than a threshold in absolute value are written
 * as 0, so that round-off differences between compilers and platforms do not
 * change the output.
 */

#include <r3mgc/base/bounding_box.h>
#include <r3mgc/base/exceptions.h>
#include <r3mgc/base/point.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace r3mgc_tests
{
  /**
   * A minimal version of deal.II's LogStream: a stream that prepends a stack
   * of prefixes to every line, and filters floating point numbers.
   */
  class LogStream
  {
  public:
    /**
     * Constructor. The stream is not attached to any file, and the only
     * prefix is "R3MG".
     */
    LogStream()
      : prefixes{"R3MG"}
    {
      buffer.precision(6);
    }

    /**
     * Write the log to @p out from now on.
     */
    void
    attach(std::ostream &out)
    {
      file = &out;
    }

    /**
     * Add @p prefix to the stack of prefixes of every new line.
     */
    void
    push(const std::string &prefix)
    {
      prefixes.push_back(prefix);
    }

    /**
     * Remove the last prefix added with push().
     */
    void
    pop()
    {
      if (prefixes.size() > 1)
        prefixes.pop_back();
    }

    /**
     * Set the number of significant digits of floating point numbers.
     */
    void
    precision(const int n_digits)
    {
      buffer.precision(n_digits);
    }

    /**
     * Set the threshold below which floating point numbers are written as
     * zero.
     */
    void
    threshold_double(const double t)
    {
      double_threshold = t;
    }

    /**
     * Write any object for which operator<< is defined.
     */
    template <typename T>
    LogStream &
    operator<<(const T &t)
    {
      buffer << t;
      return *this;
    }

    /**
     * Write a double, filtered.
     */
    LogStream &
    operator<<(const double x)
    {
      buffer << filter(x);
      return *this;
    }

    /**
     * Write a float, filtered.
     */
    LogStream &
    operator<<(const float x)
    {
      buffer << filter(x);
      return *this;
    }

    /**
     * Write the coordinates of a point, each one filtered.
     */
    template <int dim, typename Number>
    LogStream &
    operator<<(const r3mgc::Point<dim, Number> &p)
    {
      for (unsigned int d = 0; d < dim; ++d)
        {
          if (d > 0)
            buffer << ' ';
          buffer << filter(p[d]);
        }
      return *this;
    }

    /**
     * Write the corners of a bounding box, each coordinate filtered.
     */
    template <int dim, typename Number>
    LogStream &
    operator<<(const r3mgc::BoundingBox<dim, Number> &box)
    {
      return *this << box.get_boundary_points().first << " : "
                   << box.get_boundary_points().second;
    }

    /**
     * Apply a stream manipulator. std::endl terminates the current line, and
     * writes it to the attached file with its prefixes.
     */
    LogStream &
    operator<<(std::ostream &(*manipulator)(std::ostream &))
    {
      if (manipulator == static_cast<std::ostream &(*)(std::ostream &)>(
                           &std::endl<char, std::char_traits<char>>))
        {
          if (file != nullptr)
            {
              std::string line_prefix;
              for (unsigned int i = 0; i < prefixes.size(); ++i)
                line_prefix += (i == 0 ? "" : ":") + prefixes[i];
              *file << line_prefix << "::" << buffer.str() << std::endl;
            }
          buffer.str("");
        }
      else
        manipulator(buffer);
      return *this;
    }

  private:
    /**
     * Return @p x, or zero if |x| is below the threshold (this also turns
     * -0 into 0).
     */
    template <typename Number>
    Number
    filter(const Number x) const
    {
      return (std::abs(x) < double_threshold) ? Number(0.) : x;
    }

    /**
     * The file the log is written to.
     */
    std::ostream *file = nullptr;

    /**
     * The stack of prefixes.
     */
    std::vector<std::string> prefixes;

    /**
     * The current line.
     */
    std::ostringstream buffer;

    /**
     * Numbers whose absolute value is smaller than this are written as zero.
     */
    double double_threshold = 1e-12;
  };



  /**
   * Return the class name of the exception whose call sequence is
   * @p exc_name, e.g., "ExcMessage" for "ExcMessage(\"some text\")".
   */
  inline std::string
  exception_class_name(const std::string &exc_name)
  {
    return exc_name.substr(0, exc_name.find('('));
  }



  /**
   * Remove leading and trailing white space from @p s.
   */
  inline std::string
  trim(const std::string &s)
  {
    const auto first = s.find_first_not_of(" \t\n");
    if (first == std::string::npos)
      return "";
    const auto last = s.find_last_not_of(" \t\n");
    return s.substr(first, last - first + 1);
  }
} // namespace r3mgc_tests



/**
 * The global log stream of the tests.
 */
inline r3mgc_tests::LogStream r3mglog;



/**
 * Attach r3mglog to the file `output` in the current directory. This must be
 * the first call of every test.
 */
inline void
initlog()
{
  static std::ofstream logfile("output");
  r3mglog.attach(logfile);
}



/**
 * Execute @p statement, and write to r3mglog the class name and the message
 * of the r3mgc exception it throws, or a line saying that no exception was
 * thrown. Neither the file and line where the exception was raised nor the
 * text of the call that raised it are logged, so that the output does not
 * depend on how the sources are written.
 *
 * Exceptions raised by R3MGC_Assert are only thrown in debug mode: tests
 * using this macro on such checks need a separate expected output for each
 * build type (`<name>.debug.output` and `<name>.release.output`).
 */
#define check_throw(statement)                                              \
  do                                                                        \
    {                                                                       \
      try                                                                   \
        {                                                                   \
          statement;                                                        \
          r3mglog << "No exception thrown by: " << #statement << std::endl; \
        }                                                                   \
      catch (const r3mgc::ExceptionBase &exc)                               \
        {                                                                   \
          r3mglog << "Exception "                                           \
                  << r3mgc_tests::exception_class_name(exc.get_exc_name())  \
                  << std::endl;                                             \
          r3mglog << "  " << r3mgc_tests::trim(exc.get_additional_info())   \
                  << std::endl;                                             \
        }                                                                   \
    }                                                                       \
  while (false)

#endif
