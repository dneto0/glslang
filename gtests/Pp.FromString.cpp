// Copyright 2026 Google LLC
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of Google Inc. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <gmock/gmock.h>
#include <string>
#include <ostream>

#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"

namespace glslangtest {
namespace {

using ::testing::Eq;

struct Result {
  Result() = default;
  explicit Result(const std::string& out_) : output(out_) {}
  std::string output;
  std::string error;
};

struct PreprocessingCase {
  std::string input;
  Result expected;
};
// Pretty-prints a case to the given stream.
// Indent, surround by delimiters, and make tabs visible.
void PrintTo(const PreprocessingCase& c, std::ostream* os) {
  auto bracketit = [os](std::string s) {
    bool fresh_line = true;
    for (auto c: s) {
      if (fresh_line) {
        *os << "  |";
        fresh_line = false;
      }
      switch (c) {
      case '\n':
        *os << "|\n";
        fresh_line = true;
        break;
      case '\t': // make tabs visible
        *os << "\\t";
        break;
      default:
        *os << c;
      }
    }
    if (fresh_line) {
      *os << "  |";
    }
    *os << "|";
  };
  *os << "{\ninput:\n";
  bracketit(c.input);
  *os << "\nexpected output:\n";
  bracketit(c.expected.output);
  *os << "\n}";
}

class PreprocessingTest : public ::testing::TestWithParam<PreprocessingCase> {
  const int kDefaultVersion = 450;
  const bool kDontForceDefaultVersionAndProfile = false;
  const bool kNotForwardCompatible = false;
  const EProfile kProfile = ECoreProfile;
  std::string preamble = "#version 450\n";
 public:
  const std::string& Preamble() const { return preamble; }
  Result Preprocess(std::string input) const {
    glslang::TShader shader(EShLangCompute);
    std::array<const char*,2> strings{preamble.c_str(), input.data()};
    shader.setStrings(strings.data(), strings.size());

    TBuiltInResource resources = *GetDefaultResources();
    glslang::TShader::ForbidIncluder includer;

    Result result;
    bool ok = shader.preprocess(&resources, kDefaultVersion, kProfile, kDontForceDefaultVersionAndProfile,
                                kNotForwardCompatible,
                                EShMsgOnlyPreprocessor, &result.output, includer);
    return result;
  }
};

TEST_P(PreprocessingTest, Samples)
{
    auto got = Preprocess(GetParam().input);
    // Assume that the preamble is passed through.
    EXPECT_THAT(got.output, Eq(Preamble() + GetParam().expected.output));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    Simple, PreprocessingTest,
    ::testing::ValuesIn(std::vector<PreprocessingCase>({
      {"#define A B", Result()},
      {"#define A B\nA", Result("B\n")},
      {"#define A B\nc", Result("B\n")},
      {"#define A B C\nc", Result("a\tB\n")},
    }))
);
// clang-format on

}  // anonymous namespace
}  // namespace glslangtest
