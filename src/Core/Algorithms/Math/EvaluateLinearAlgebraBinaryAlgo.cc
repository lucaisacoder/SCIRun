/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Math/EvaluateLinearAlgebraBinaryAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Parser/ArrayMathEngine.h> 

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun;

EvaluateLinearAlgebraBinaryAlgorithm::EvaluateLinearAlgebraBinaryAlgorithm()
{
  addParameter(Variables::Operator, 0);
	addParameter(Variables::FunctionString, "x+y"); 
}

EvaluateLinearAlgebraBinaryAlgorithm::Outputs EvaluateLinearAlgebraBinaryAlgorithm::run(const EvaluateLinearAlgebraBinaryAlgorithm::Inputs& inputs, const EvaluateLinearAlgebraBinaryAlgorithm::Parameters& params) const
{
  DenseMatrixHandle result;
  DenseMatrixConstHandle lhs = inputs.get<0>();
  DenseMatrixConstHandle rhs = inputs.get<1>();
  ENSURE_ALGORITHM_INPUT_NOT_NULL(lhs, "lhs");
  ENSURE_ALGORITHM_INPUT_NOT_NULL(rhs, "rhs");

  Operator oper = params.get<0>();
  /// @todo: absolutely need matrix move semantics here!!!!!!!
  switch (oper)
  {
  case ADD:
    result.reset(lhs->clone());
    *result += *rhs;
    break;
  case SUBTRACT:
    result.reset(lhs->clone());
    *result -= *rhs;
    break;
  case MULTIPLY:
    result.reset(lhs->clone());
    *result *= *rhs;
    break;
	case FUNCTION:
			{
				result.reset(lhs->clone());
				NewArrayMathEngine engine;
				//engine.setLogger(this); 
				MatrixHandle lhsInput, rhsInput; 
				lhsInput.reset(lhs->clone()); 
				rhsInput.reset(rhs->clone()); 
			
				if (!(engine.add_input_fullmatrix("x", lhsInput ) )) ;//return;
				if (!(engine.add_input_fullmatrix("y", rhsInput ) )) ;//return;
      
				//std::string func = function_.get();
				boost::optional<std::string> func = params.get<1>(); 
				std::string function_string = func.get(); 

				function_string = "RESULT="+function_string;
				engine.add_expressions(function_string);

				MatrixHandle omatrix = lhsInput;
				if(!(engine.add_output_fullmatrix("RESULT",omatrix))) ;//return;
      
				// Actual engine call, which does the dynamic compilation, the creation of the
				// code for all the objects, as well as inserting the function and looping 
				// over every data point
				if (!(engine.run())) ;//return;
				//send_output_handle("Output", omatrix);
			}
    break;   
  default:
    THROW_ALGORITHM_INPUT_ERROR("ERROR: unknown binary operation");
    break;
  }

  return result;
}

AlgorithmOutput EvaluateLinearAlgebraBinaryAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto LHS = input.get<DenseMatrix>(Variables::LHS);
  auto RHS = input.get<DenseMatrix>(Variables::RHS);
	auto func = boost::make_optional(get(Variables::FunctionString).toString()); 

  auto result = run(boost::make_tuple(LHS, RHS), boost::make_tuple(Operator(get(Variables::Operator).toInt()), func));

  AlgorithmOutput output;
  output[Variables::Result] = result;
  return output;
}

