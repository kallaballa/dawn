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

#include "src/program.h"

#include <sstream>
#include <utility>

#include "gmock/gmock.h"
#include "src/ast/function.h"
#include "src/ast/test_helper.h"
#include "src/ast/variable.h"
#include "src/type/alias_type.h"
#include "src/type/f32_type.h"
#include "src/type/struct_type.h"

namespace tint {
namespace {

using ProgramTest = ast::TestHelper;

TEST_F(ProgramTest, Creation) {
  EXPECT_EQ(mod->AST().Functions().size(), 0u);
}

TEST_F(ProgramTest, ToStrEmitsPreambleAndPostamble) {
  const auto str = mod->to_str();
  auto* const expected = "Module{\n}\n";
  EXPECT_EQ(str, expected);
}

TEST_F(ProgramTest, IsValid_Empty) {
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_GlobalVariable) {
  auto* var = Var("var", ast::StorageClass::kInput, ty.f32);
  mod->AST().AddGlobalVariable(var);
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_Null_GlobalVariable) {
  mod->AST().AddGlobalVariable(nullptr);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_Invalid_GlobalVariable) {
  auto* var = Var("var", ast::StorageClass::kInput, nullptr);
  mod->AST().AddGlobalVariable(var);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_Alias) {
  auto* alias = ty.alias("alias", ty.f32);
  mod->AST().AddConstructedType(alias);
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_Null_Alias) {
  mod->AST().AddConstructedType(nullptr);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_Struct) {
  auto* st = ty.struct_("name", {});
  auto* alias = ty.alias("name", st);
  mod->AST().AddConstructedType(alias);
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_Struct_EmptyName) {
  auto* st = ty.struct_("", {});
  auto* alias = ty.alias("name", st);
  mod->AST().AddConstructedType(alias);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_Function) {
  auto* func = Func("main", ast::VariableList(), ty.f32, ast::StatementList{},
                    ast::FunctionDecorationList{});

  mod->AST().Functions().Add(func);
  EXPECT_TRUE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_Null_Function) {
  mod->AST().Functions().Add(nullptr);
  EXPECT_FALSE(mod->IsValid());
}

TEST_F(ProgramTest, IsValid_Invalid_Function) {
  auto* func = Func("main", ast::VariableList{}, nullptr, ast::StatementList{},
                    ast::FunctionDecorationList{});

  mod->AST().Functions().Add(func);
  EXPECT_FALSE(mod->IsValid());
}

}  // namespace
}  // namespace tint
