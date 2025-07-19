
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef YOREL_YOMM2_POLICY_VPTR_VECTOR_HPP
#define YOREL_YOMM2_POLICY_VPTR_VECTOR_HPP

#include <yorel/yomm2/policies/core.hpp>

namespace yorel {
namespace yomm2 {
namespace policy {

template<class Policy>
struct yOMM2_API_gcc vptr_vector : virtual external_vptr {
    static std::vector<const std::uintptr_t*> vptrs;

    template<typename ForwardIterator>
    static void publish_vptrs(ForwardIterator first, ForwardIterator last) {
        using namespace policy;

        std::size_t size;

        if constexpr (has_facet<Policy, type_hash>) {
            Policy::hash_initialize(first, last);
            size = Policy::hash_length;
        } else {
            size = 0;

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    size = (std::max)(size, *type_iter);
                }
            }

            ++size;
        }

        vptrs.resize(size);

        if constexpr (has_facet<Policy, indirect_vptr>) {
            Policy::indirect_vptrs.resize(size);
        }

        for (auto iter = first; iter != last; ++iter) {
            for (auto type_iter = iter->type_id_begin();
                 type_iter != iter->type_id_end(); ++type_iter) {
                auto index = *type_iter;

                if constexpr (has_facet<Policy, type_hash>) {
                    index = Policy::hash_type_id(index);
                }

                vptrs[index] = iter->vptr();

                if constexpr (has_facet<Policy, indirect_vptr>) {
                    Policy::indirect_vptrs[index] = iter->indirect_vptr();
                }
            }
        }
    }

    template<class Class>
    static const std::uintptr_t* dynamic_vptr(const Class& arg) {
        auto index = Policy::dynamic_type(arg);

        if constexpr (has_facet<Policy, type_hash>) {
            index = Policy::hash_type_id(index);
        }

        return vptrs[index];
    }

    template<class Class>
	static const std::uintptr_t* try_dynamic_vptr(const Class& arg)
	{
		auto index = Policy::dynamic_type(arg);

		if constexpr (has_facet<Policy, type_hash>)
		{
			return Policy::has_type_id(index) ? dynamic_vptr(arg) : nullptr;
		}

        return nullptr;		
	}

    template<class... Class>
	static bool try_dynamic_vptrs(const Class&... args)
	{
		bool result{true};
		((result &= (try_dynamic_vptr(args) != nullptr)), ...);
		return result;
	}
};

template<class Policy>
std::vector<const std::uintptr_t*> vptr_vector<Policy>::vptrs;

}
}
}

#endif
