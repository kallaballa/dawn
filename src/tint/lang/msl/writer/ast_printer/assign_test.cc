// Copyright 2020 The Tint Authors.
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

#include "src/tint/lang/msl/writer/ast_printer/test_helper.h"

namespace tint::msl::writer {
namespace {

using MslASTPrinterTest = TestHelper;

TEST_F(MslASTPrinterTest, Emit_Assign) {
    auto* lhs = Var("lhs", ty.i32());
    auto* rhs = Var("rhs", ty.i32());
    auto* assign = Assign(lhs, rhs);
    WrapInFunction(lhs, rhs, assign);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(assign)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), "  lhs = rhs;\n");
}

}  // namespace
}  // namespace tint::msl::writer
