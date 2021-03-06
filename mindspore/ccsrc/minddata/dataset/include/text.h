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

#ifndef MINDSPORE_CCSRC_MINDDATA_DATASET_API_TEXT_H_
#define MINDSPORE_CCSRC_MINDDATA_DATASET_API_TEXT_H_

#include <vector>
#include <memory>
#include <string>

#include "minddata/dataset/core/constants.h"
#include "minddata/dataset/include/transforms.h"
#include "minddata/dataset/text/vocab.h"
#include "mindspore/ccsrc/minddata/dataset/core/data_type.h"

namespace mindspore {
namespace dataset {
namespace api {

// Transform operations for text
namespace text {

// Text Op classes (in alphabetical order)
class LookupOperation;

/// \brief Lookup operator that looks up a word to an id.
/// \param[in] vocab a Vocab object.
/// \param[in] unknown_token word to use for lookup if the word being looked up is out of Vocabulary (oov).
///   If unknown_token is oov, runtime error will be thrown.
/// \param[in] DataType type of the tensor after lookup, typically int32.
/// \return Shared pointer to the current TensorOperation.
std::shared_ptr<LookupOperation> Lookup(const std::shared_ptr<Vocab> &vocab, const std::string &unknown_token,
                                        const mindspore::dataset::DataType &data_type = DataType("int32"));

/* ####################################### Derived TensorOperation classes ################################# */

class LookupOperation : public TensorOperation {
 public:
  explicit LookupOperation(const std::shared_ptr<Vocab> &vocab, const std::string &unknown_token,
                           const DataType &data_type);

  ~LookupOperation() = default;

  std::shared_ptr<TensorOp> Build() override;

  bool ValidateParams() override;

 private:
  std::shared_ptr<Vocab> vocab_;
  std::string unknown_token_;
  int32_t default_id_;
  DataType data_type_;
};
}  // namespace text
}  // namespace api
}  // namespace dataset
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_MINDDATA_DATASET_API_TEXT_H_
