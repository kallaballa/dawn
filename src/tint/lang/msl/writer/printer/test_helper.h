// Copyright 2023 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SRC_TINT_LANG_MSL_WRITER_PRINTER_TEST_HELPER_H_
#define SRC_TINT_LANG_MSL_WRITER_PRINTER_TEST_HELPER_H_

#include <iostream>
#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/msl/writer/printer/printer.h"

namespace tint::msl::writer {

/// Base helper class for testing the MSL generator implementation.
template <typename BASE>
class MslPrinterTestHelperBase : public BASE {
  public:
    MslPrinterTestHelperBase() : generator_(&mod) {}

    /// The test module.
    ir::Module mod;
    /// The test builder.
    ir::Builder b{mod};
    /// The type manager.
    type::Manager& ty{mod.Types()};

  protected:
    /// The MSL generator.
    Printer generator_;

    /// Validation errors
    std::string err_;

    /// @returns the error string from the validation
    std::string Error() const { return err_; }

    /// @returns true if the IR module is valid
    bool IRIsValid() {
        auto res = ir::Validate(mod);
        if (!res) {
            err_ = res.Failure().str();
            return false;
        }
        return true;
    }
};

using MslPrinterTest = MslPrinterTestHelperBase<testing::Test>;

template <typename T>
using MslPrinterTestWithParam = MslPrinterTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_PRINTER_TEST_HELPER_H_
