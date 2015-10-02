/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_TEEM_CONVERTERS_CONVERTNRRDTOMATRIX_H
#define MODULES_TEEM_CONVERTERS_CONVERTNRRDTOMATRIX_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Teem/Converters/share.h>

namespace SCIRun {

  namespace Core {
    namespace Algorithms {
      namespace Teem {
        ALGORITHM_PARAMETER_DECL(MatrixType);
        ALGORITHM_PARAMETER_DECL(SparseColumns);
      }
    }
  }

namespace Modules {
namespace Teem {

  class SCISHARE ConvertNrrdToMatrix : public Dataflow::Networks::Module,
    public Has3InputPorts<NrrdPortTag, NrrdPortTag, NrrdPortTag>,
    public Has1OutputPort<MatrixPortTag>
  {
  public:
    ConvertNrrdToMatrix();
    virtual void execute() override;
    virtual void setStateDefaults() override;

    INPUT_PORT(0, Data, NrrdDataType);
    INPUT_PORT(1, Rows, NrrdDataType);
    INPUT_PORT(2, Columns, NrrdDataType);
    OUTPUT_PORT(0, OutputMatrix, Matrix);

    static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
  private:
    Core::Datatypes::MatrixHandle create_matrix_from_nrrds(boost::optional<NrrdDataHandle> dataH,
      boost::optional<NrrdDataHandle> rowsH,
      boost::optional<NrrdDataHandle> colsH, int cols);
    template<class PTYPE>
    Core::Datatypes::MatrixHandle create_column_matrix(NrrdDataHandle dataH);
    template<class PTYPE>
    Core::Datatypes::MatrixHandle create_dense_matrix(NrrdDataHandle dataH);
    template<class PTYPE>
    Core::Datatypes::MatrixHandle create_sparse_matrix(NrrdDataHandle dataH, NrrdDataHandle rowsH, NrrdDataHandle colsH, int cols);
  };

}}}

#endif