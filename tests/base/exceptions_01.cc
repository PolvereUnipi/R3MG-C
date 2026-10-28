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

// Test the exception mechanism: R3MGC_AssertThrow throws in every build type,
// the standard exceptions print the expected messages, user-declared
// exceptions work, and what() contains the location and the violated
// condition.

#include <r3mgc/base/exceptions.h>

#include <string>

#include "../tests.h"

using namespace r3mgc;

R3MGC_DeclException2(ExcTooFewPoints,
                     unsigned int,
                     unsigned int,
                     << "Found " << arg1 << " points, but at least " << arg2
                     << " are needed.");



void
throw_message()
{
  R3MGC_AssertThrow(1 + 1 == 3, ExcMessage("Arithmetic is broken."));
}



int
main()
{
  initlog();

  check_throw(throw_message());
  check_throw(R3MGC_AssertThrow(false, ExcDimensionMismatch(3, 4)));
  check_throw(R3MGC_AssertThrow(false, ExcIndexRange(5, 0, 5)));
  check_throw(R3MGC_AssertThrow(false, ExcNotImplemented()));
  check_throw(R3MGC_AssertThrow(false, ExcInternalError()));
  check_throw(R3MGC_AssertThrow(false, ExcTooFewPoints(2, 4)));
  check_throw(R3MGC_AssertThrow(true, ExcMessage("Never thrown.")));

  // The exceptions can also be caught as std::exception, and what() contains
  // the violated condition and the name of the function.
  try
    {
      throw_message();
    }
  catch (const std::exception &exc)
    {
      const std::string what = exc.what();
      r3mglog << "what() contains the condition: "
              << (what.find("1 + 1 == 3") != std::string::npos) << std::endl;
      r3mglog << "what() contains the function: "
              << (what.find("throw_message") != std::string::npos) << std::endl;
      r3mglog << "what() contains the message: "
              << (what.find("Arithmetic is broken.") != std::string::npos)
              << std::endl;
    }
}
