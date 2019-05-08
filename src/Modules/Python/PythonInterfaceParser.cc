/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2016 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#include <Modules/Python/PythonInterfaceParser.h>
#include <Modules/Python/InterfaceWithPython.h>
#include <Core/Logging/Log.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Modules::Python;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Python;

PythonInterfaceParser::PythonInterfaceParser(const std::string& moduleId,
  const ModuleStateHandle& state,
  const std::vector<std::string>& portIds)
  : moduleId_(moduleId), state_(state), portIds_(portIds)
{
}

std::string PythonInterfaceParser::convertOutputSyntax(const std::string& line) const
{
  auto outputVarsToCheck = InterfaceWithPython::outputNameParameters();

  for (const auto& var : outputVarsToCheck)
  {
    auto varName = state_->getValue(var).toString();

    auto regexString = "(\\h*)" + varName + " = (.+)";
    //std::cout << "REGEX STRING " << regexString << std::endl;
    boost::regex outputRegex(regexString);
    boost::smatch what;
    if (regex_match(line, what, outputRegex))
    {
      int rhsIndex = what.size() > 2 ? 2 : 1;
      auto whitespace = what.size() > 2 ? boost::lexical_cast<std::string>(what[1]) : "";
      auto rhs = boost::lexical_cast<std::string>(what[rhsIndex]);
      auto converted = whitespace + "scirun_set_module_transient_state(\"" +
        moduleId_ + "\",\"" + varName + "\"," + rhs + ")";
      //std::cout << "CONVERTED TO " << converted << std::endl;
      return converted;
    }
  }

  return line;
}

std::string PythonInterfaceParser::convertInputSyntax(const std::string& line) const
{
  for (const auto& portId : portIds_)
  {
    auto inputName = state_->getValue(Name(portId)).toString();
    //std::cout << "FOUND INPUT VARIABLE NAME: " << inputName << " for port " << portId << std::endl;
    //std::cout << "NEED TO REPLACE " << inputName << " with\n\t" << "scirun_get_module_input_value(\"" << moduleId_ << "\", \"" << portId << "\")" << std::endl;
    auto index = line.find(inputName);
    if (index != std::string::npos)
    {
      auto codeCopy = line;
      return codeCopy.replace(index, inputName.length(),
        "scirun_get_module_input_value(\"" + moduleId_ + "\", \"" + portId + "\")");
    }
  }
  return line;
}

std::string PythonInterfaceParser::convertStandardCodeBlock(const PythonCodeBlock& block) const
{
  if (block.isMatlab)
    throw std::invalid_argument("Cannot process matlab block");

  std::ostringstream convertedCode;
  std::vector<std::string> lines;
  boost::split(lines, block.code, boost::is_any_of("\n"));
  for (const auto& line : lines)
  {
    convertedCode << convertInputSyntax(convertOutputSyntax(line)) << "\n";
  }
  return convertedCode.str();
}

PythonCode PythonInterfaceParser::extractSpecialBlocks(const std::string& code) const
{
  PythonCode blocks;
  static boost::regex matlabBlock("(.*)\\/\\*matlab\\n(.*)\\nmatlab\\*\\/(.*)");

  boost::smatch what;
  if (regex_match(code, what, matlabBlock))
  {
    auto firstPart = std::string(what[1]);
    boost::trim(firstPart);
    auto matlabPart = std::string(what[2]);
    boost::trim(matlabPart);
    auto secondPart = std::string(what[3]);
    boost::trim(secondPart);

    parsePart(blocks, firstPart);

    if (!matlabPart.empty())
      blocks.push_back({matlabPart, true});

    parsePart(blocks, secondPart);
  }
  else
  {
    return {{code, false}};
  }
  return blocks;
}

void PythonInterfaceParser::parsePart(PythonCode& blocks, const std::string& part) const
{
  if (!part.empty())
  {
    if (part.find("/*matlab") != std::string::npos)
    {
      auto rec = extractSpecialBlocks(part);
      blocks.insert(blocks.begin(), rec.begin(), rec.end());
    }
    else
      blocks.push_back({part, false});
  }
}

PythonCodeBlock PythonInterfaceParser::concatenateNormalBlocks(const PythonCode& codeList) const
{
  std::ostringstream ostr;
  for (const auto& block : codeList)
  {
    if (!block.isMatlab)
    {
      ostr << block.code << '\n';
    }
  }
  return {ostr.str(), false };
}