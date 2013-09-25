/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
   University of Utah.

   
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

#include <Modules/Legacy/Fields/SetFieldNodes.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;

//    SCIRunAlgo::SetMeshNodesAlgo algo_;

SetFieldNodes::SetFieldNodes()
  : Module(ModuleLookupInfo("SetFieldNodes", "ChangeMesh", "SCIRun"), false)
{
}

void SetFieldNodes::execute()
{
  FieldHandle field_input_handle,field_output_handle;
  if(!(get_input_handle("Field",field_input_handle,true))) return;

  MatrixHandle matrix_input_handle;
  get_input_handle("Matrix Nodes",matrix_input_handle,true);

  if (inputs_changed_ ||
      !oport_cached("Field"))
  {
    update_state(Executing);

    if(!(algo_.run(field_input_handle,matrix_input_handle,field_output_handle))) return;
    send_output_handle("Field", field_output_handle);
  }
}
