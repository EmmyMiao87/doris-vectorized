// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "vec/columns/column_string.h"
#include "vec/columns/column_vector.h"
#include "vec/data_types/data_type_number.h"
#include "vec/data_types/data_type_string.h"
#include "vec/functions/function.h"
#include "vec/functions/function_helpers.h"
//#include<vec/columns/column_fixed_string.h>
//#include<vec/columns/column_array.h>

namespace doris::vectorized {

namespace ErrorCodes {
extern const int ILLEGAL_COLUMN;
extern const int ILLEGAL_TYPE_OF_ARGUMENT;
} // namespace ErrorCodes

template <typename Impl, typename Name, typename ResultType>
class FunctionStringOrArrayToT : public IFunction {
public:
    static constexpr auto name = Name::name;
    //    static FunctionPtr create(const Context &)
    static FunctionPtr create() { return std::make_shared<FunctionStringOrArrayToT>(); }

    String getName() const override { return name; }

    size_t getNumberOfArguments() const override { return 1; }

    DataTypePtr getReturnTypeImpl(const DataTypes& arguments) const override {
        if (!isStringOrFixedString(arguments[0]) && !isArray(arguments[0]))
            throw Exception("Illegal type " + arguments[0]->getName() +
                                    " of argument of function " + getName(),
                            ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT);

        return std::make_shared<DataTypeNumber<ResultType>>();
    }

    bool useDefaultImplementationForConstants() const override { return true; }

    Status executeImpl(Block& block, const ColumnNumbers& arguments, size_t result,
                       size_t /*input_rows_count*/) override {
        const ColumnPtr column = block.getByPosition(arguments[0]).column;
        if (const ColumnString* col = checkAndGetColumn<ColumnString>(column.get())) {
            auto col_res = ColumnVector<ResultType>::create();

            typename ColumnVector<ResultType>::Container& vec_res = col_res->getData();
            vec_res.resize(col->size());
            Impl::vector(col->getChars(), col->getOffsets(), vec_res);

            block.getByPosition(result).column = std::move(col_res);
        }
        //        else if (const ColumnFixedString * col_fixed = checkAndGetColumn<ColumnFixedString>(column.get()))
        //        {
        //            if (Impl::is_fixed_to_constant)
        //            {
        //                ResultType res = 0;
        //                Impl::vector_fixed_to_constant(col_fixed->getChars(), col_fixed->getN(), res);
        //
        //                block.getByPosition(result).column = block.getByPosition(result).type->createColumnConst(col_fixed->size(), toField(res));
        //            }
        //            else
        //            {
        //                auto col_res = ColumnVector<ResultType>::create();
        //
        //                typename ColumnVector<ResultType>::Container & vec_res = col_res->getData();
        //                vec_res.resize(col_fixed->size());
        //                Impl::vector_fixed_to_vector(col_fixed->getChars(), col_fixed->getN(), vec_res);
        //
        //                block.getByPosition(result).column = std::move(col_res);
        //            }
        //        }
        //        else if (const ColumnArray * col_arr = checkAndGetColumn<ColumnArray>(column.get()))
        //        {
        //            auto col_res = ColumnVector<ResultType>::create();
        //
        //            typename ColumnVector<ResultType>::Container & vec_res = col_res->getData();
        //            vec_res.resize(col_arr->size());
        //            Impl::array(col_arr->getOffsets(), vec_res);
        //
        //            block.getByPosition(result).column = std::move(col_res);
        //        }
        else
            throw Exception("Illegal column " +
                                    block.getByPosition(arguments[0]).column->getName() +
                                    " of argument of function " + getName(),
                            ErrorCodes::ILLEGAL_COLUMN);
        return Status::OK();
    }
};

} // namespace doris::vectorized