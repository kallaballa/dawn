// Copyright 2022 The Dawn Authors
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

#include "dawn/common/Assert.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/CacheKey.h"

#include "vulkan-tools/src/icd/generated/vk_typemap_helper.h"

#include <map>

namespace dawn::native::vulkan {

    namespace detail {

        template <typename... VK_STRUCT_TYPES>
        void ValidatePnextImpl(const VkBaseOutStructure* root) {
            const VkBaseOutStructure* next =
                reinterpret_cast<const VkBaseOutStructure*>(root->pNext);
            while (next != nullptr) {
                // Assert that the type of each pNext struct is exactly one of the specified
                // templates.
                ASSERT(((LvlTypeMap<VK_STRUCT_TYPES>::kSType == next->sType ? 1 : 0) + ... + 0) ==
                       1);
                next = reinterpret_cast<const VkBaseOutStructure*>(next->pNext);
            }
        }

        template <typename VK_STRUCT_TYPE>
        void SerializePnextImpl(CacheKey* key, const VkBaseOutStructure* root) {
            const VkBaseOutStructure* next =
                reinterpret_cast<const VkBaseOutStructure*>(root->pNext);
            const VK_STRUCT_TYPE* found = nullptr;
            while (next != nullptr) {
                if (LvlTypeMap<VK_STRUCT_TYPE>::kSType == next->sType) {
                    if (found == nullptr) {
                        found = reinterpret_cast<const VK_STRUCT_TYPE*>(next);
                    } else {
                        // Fail an assert here since that means that the chain had more than one of
                        // the same typed chained object.
                        ASSERT(false);
                    }
                }
                next = reinterpret_cast<const VkBaseOutStructure*>(next->pNext);
            }
            if (found != nullptr) {
                key->Record(found);
            }
        }

        template <typename VK_STRUCT_TYPE,
                  typename... VK_STRUCT_TYPES,
                  typename = std::enable_if_t<(sizeof...(VK_STRUCT_TYPES) > 0)>>
        void SerializePnextImpl(CacheKey* key, const VkBaseOutStructure* root) {
            SerializePnextImpl<VK_STRUCT_TYPE>(key, root);
            SerializePnextImpl<VK_STRUCT_TYPES...>(key, root);
        }

    }  // namespace detail

    template <typename... VK_STRUCT_TYPES>
    void SerializePnext(CacheKey* key, const VkBaseOutStructure* root) {
        detail::ValidatePnextImpl<VK_STRUCT_TYPES...>(root);
        detail::SerializePnextImpl<VK_STRUCT_TYPES...>(key, root);
    }

    // Empty template specialization so that we can put this in to ensure failures occur if new
    // extensions are added without updating serialization.
    template <>
    void SerializePnext(CacheKey* key, const VkBaseOutStructure* root) {
        detail::ValidatePnextImpl<>(root);
    }

}  // namespace dawn::native::vulkan
