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
#include <iostream>
#include <memory>
#include "utils/log_adapter.h"
#include "common/common_test.h"
#include "src/common/file_utils.h"
#include "src/common/file_utils_ext.h"
#include "src/runtime/kernel/arm/fp32_grad/bn_grad.h"
#include "nnacl/fp32_grad/batch_norm.h"
#include "nnacl/fp32/batchnorm.h"
#include "src/kernel_registry.h"

namespace mindspore {

class TestBNGradFp32 : public mindspore::CommonTest {
 public:
  TestBNGradFp32() {}
  lite::Tensor *CreateInTensor(std::string file_name, std::vector<int> dim);
};

lite::Tensor *TestBNGradFp32::CreateInTensor(std::string file_name, std::vector<int> dim) {
  size_t input_size = 0;
  auto input_data = reinterpret_cast<float *>(mindspore::lite::ReadFile(file_name.c_str(), &input_size));
  auto tensor = new lite::Tensor(TypeId::kNumberTypeFloat32, dim);
  tensor->SetData(input_data);
  EXPECT_EQ(input_size, tensor->Size());
  return tensor;
}

TEST_F(TestBNGradFp32, BNGradFp32) {
  // prepare stage
  auto bn_param = static_cast<BNGradParameter *>(malloc(sizeof(BNGradParameter)));
  bn_param->epsilon_ = 0.00001;
  bn_param->momentum_ = 0.1;
  const int batch = 2;
  const int channels = 3;
  const int height = 4;
  const int width = 5;

  auto dy_tensor = CreateInTensor("./test_data/bngrad/dy_2_4_5_3.bin", {batch, height, width, channels});
  auto x_tensor = CreateInTensor("./test_data/bngrad/input_x_2_4_5_3.bin", {batch, height, width, channels});
  auto scale_tensor = CreateInTensor("./test_data/bngrad/scale_3.bin", {1, 1, 1, channels});
  auto mean_tensor = CreateInTensor("./test_data/bngrad/save_mean_3.bin", {1, 1, 1, channels});
  auto var_tensor = CreateInTensor("././test_data/bngrad/save_var_3.bin", {1, 1, 1, channels});
  // prepare output tensors
  lite::Tensor dx_tensor(TypeId::kNumberTypeFloat32, {batch, height, width, channels});
  dx_tensor.MallocData();
  lite::Tensor dscale_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  dscale_tensor.MallocData();
  lite::Tensor dbias_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  dbias_tensor.MallocData();

  std::vector<lite::Tensor *> inputs = {dy_tensor, x_tensor, scale_tensor, mean_tensor, var_tensor};
  std::vector<lite::Tensor *> outputs = {&dx_tensor, &dscale_tensor, &dbias_tensor};

  kernel::KernelKey desc = {kernel::kCPU, TypeId::kNumberTypeFloat32, schema::PrimitiveType_BNGrad};

  auto creator = lite::KernelRegistry::GetInstance()->GetCreator(desc);
  auto kernel_obj = creator(inputs, outputs, reinterpret_cast<OpParameter *>(bn_param), NULL, desc, nullptr);

  for (int i = 0; i < 3; i++) {
    kernel_obj->Run();
  }

  int loop_count = 100;
  auto time_start = mindspore::lite::GetTimeUs();
  for (int i = 0; i < loop_count; i++) {
    kernel_obj->Run();
  }
  auto time_end = mindspore::lite::GetTimeUs();
  auto cost = time_end - time_start;
  auto time_avg = cost / loop_count;
  std::cout << "single thread running time : " << time_avg << "us\n";
  std::cout << "==========dx==========\n";
  auto dx = reinterpret_cast<float *>(outputs[0]->MutableData());
  for (int i = 0; i < 7; i++) std::cout << dx[i] << " ";
  std::cout << "\n";
  auto res = mindspore::lite::CompareRelativeOutput(dx, "./test_data/bngrad/output_dx_2_4_5_3.bin");
  std::cout << "\n=======dscale=======\n";
  auto dscale = reinterpret_cast<float *>(outputs[1]->MutableData());
  for (int i = 0; i < channels; i++) std::cout << dscale[i] << " ";
  std::cout << "\n";
  res = mindspore::lite::CompareRelativeOutput(dscale, "./test_data/bngrad/output_dscale_3.bin");
  EXPECT_EQ(res, 0);
  std::cout << "==========dbias==========\n";
  auto dbias = reinterpret_cast<float *>(outputs[2]->MutableData());
  for (int i = 0; i < 3; i++) std::cout << dbias[i] << " ";
  std::cout << "\n";
  res = mindspore::lite::CompareRelativeOutput(dbias, "./test_data/bngrad/output_dbias_3.bin");
  EXPECT_EQ(res, 0);
  for (auto v : inputs) {
    delete[] reinterpret_cast<float *>(v->MutableData());
    v->SetData(nullptr);
    delete v;
  }
  delete kernel_obj;
  MS_LOG(INFO) << "BNGradFp32 passed";
}

TEST_F(TestBNGradFp32, BNTtrainFp32) {
  auto bn_param = static_cast<BatchNormParameter *>(malloc(sizeof(BatchNormParameter)));
  bn_param->epsilon_ = 0.00001;
  const int batch = 2;
  const int channels = 3;
  const int height = 4;
  const int width = 5;
  bn_param->channel_ = channels;
  auto x_tensor = CreateInTensor("./test_data/bngrad/input_x_2_4_5_3.bin", {batch, height, width, channels});

  lite::Tensor scale_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  scale_tensor.MallocData();
  auto scale = reinterpret_cast<float *>(scale_tensor.MutableData());
  std::fill(scale, scale + channels, 1.0f);

  lite::Tensor bias_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  bias_tensor.MallocData();
  auto bias = reinterpret_cast<float *>(bias_tensor.MutableData());
  std::fill(bias, bias + channels, 1.0f);

  lite::Tensor mean_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  mean_tensor.MallocData();
  auto mean = reinterpret_cast<float *>(mean_tensor.MutableData());
  std::fill(mean, mean + channels, 0.0f);

  lite::Tensor var_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  var_tensor.MallocData();
  auto var = reinterpret_cast<float *>(var_tensor.MutableData());
  std::fill(var, var + channels, 1.0f);

  std::vector<lite::Tensor *> inputs = {x_tensor, &scale_tensor, &bias_tensor, &mean_tensor, &var_tensor};

  lite::Tensor out_tensor(TypeId::kNumberTypeFloat32, {batch, height, width, channels});
  ASSERT_EQ(out_tensor.MallocData(), 0);

  lite::Tensor run_mean_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  ASSERT_EQ(run_mean_tensor.MallocData(), 0);

  lite::Tensor run_var_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  ASSERT_EQ(run_var_tensor.MallocData(), 0);

  lite::Tensor save_mean_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  ASSERT_EQ(save_mean_tensor.MallocData(), 0);

  lite::Tensor save_var_tensor(TypeId::kNumberTypeFloat32, {1, 1, 1, channels});
  ASSERT_EQ(save_var_tensor.MallocData(), 0);

  std::vector<lite::Tensor *> outputs = {&out_tensor, &run_mean_tensor, &run_var_tensor, &save_mean_tensor,
                                         &save_var_tensor};

  kernel::KernelKey desc = {kernel::kCPU, TypeId::kNumberTypeFloat32, schema::PrimitiveType_FusedBatchNorm};

  mindspore::lite::InnerContext context;
  context.device_type_ = lite::DT_CPU;
  context.thread_num_ = 1;
  ASSERT_EQ(lite::RET_OK, context.Init());

  auto creator = lite::KernelRegistry::GetInstance()->GetCreator(desc);
  auto kernel_obj = creator(inputs, outputs, reinterpret_cast<OpParameter *>(bn_param), &context, desc, nullptr);

  kernel_obj->train();
  kernel_obj->Run();

  float *run_mean = reinterpret_cast<float *>(run_mean_tensor.MutableData());
  float *run_var = reinterpret_cast<float *>(run_var_tensor.MutableData());
  std::cout << "================run_mean==============================\n";
  for (int i = 0; i < channels; i++) std::cout << run_mean[i] << " ";
  std::cout << "\n";
  std::cout << "================run_var==============================\n";
  for (int i = 0; i < channels; i++) std::cout << run_var[i] << " ";
  std::cout << "\n";
  delete[] reinterpret_cast<float *>(x_tensor->MutableData());
  auto res = mindspore::lite::CompareRelativeOutput(run_mean, "./test_data/bngrad/running_mean_3.bin");
  EXPECT_EQ(res, 0);
  res = mindspore::lite::CompareRelativeOutput(run_var, "./test_data/bngrad/running_var_3.bin");
  EXPECT_EQ(res, 0);

  x_tensor->SetData(nullptr);
  delete x_tensor;
  delete kernel_obj;
}
}  // namespace mindspore
