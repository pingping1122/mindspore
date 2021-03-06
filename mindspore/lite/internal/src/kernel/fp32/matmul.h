/**
 * Copyright 2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MINDSPORE_LITE_INTERNAL_SRC_KERNEL_FP32_MATMUL_H_
#define MINDSPORE_LITE_INTERNAL_SRC_KERNEL_FP32_MATMUL_H_

#include "internal/include/model.h"
#include "src/runtime/allocator.h"

int DoMatMulInferShape(const TensorPtrVector &in_tensors, const TensorPtrVector &out_tensors, OpParameter *param);
int DoMatMul(const TensorPtrVector &in_tensors, const TensorPtrVector &out_tensors, Node *node,
             mindspore::lite::Allocator *allocator);

#endif  // MINDSPORE_LITE_INTERNAL_SRC_KERNEL_FP32_MATMUL_H_
