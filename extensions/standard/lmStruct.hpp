#pragma once
#include "../../interpreter/lamina_api/value.hpp"

#include <memory>
#include <string>

inline size_t hash_string(const std::string& key) {
    constexpr size_t FNV_OFFSET = 14695981039346656037ULL;
    constexpr size_t FNV_PRIME = 1099511628211ULL;
    size_t hash = FNV_OFFSET;
    for (const char c : key) {
        hash ^= static_cast<size_t>(c); // 异或当前字符
        hash *= FNV_PRIME;              // 乘以质数
    }
    return hash;
}


class lmStruct;
struct StringBucket;
using Node = StringBucket;

struct StringBucket {
    std::string key;
    Value value;
    size_t hash;                        // 缓存的哈希值
    std::shared_ptr<StringBucket> next; // 链表指针

    StringBucket(std::string k, const Value& v)
        : key(std::move(k)),
          value(v),
          hash(hash_string(key)),
          next(nullptr) {}
};

class lmStruct {
private:
    std::vector<std::shared_ptr<Node>> buckets_;    // 桶数组（每个桶是链表头指针）
    size_t elem_count_ = 0;
    const float load_factor_ = 0.7f;

    [[nodiscard]] size_t getBucketIndex(const size_t hash) const {
        return hash & (this->buckets_.size() - 1);
    }

    // 扩容
    void resize() {
        const size_t old_size = buckets_.size();
        const size_t new_size = old_size * 2;

        std::vector<std::shared_ptr<Node>> new_buckets(new_size, nullptr);

        // 重新哈希所有旧元素到新桶
        for (size_t i = 0; i < old_size; ++i) {
            std::shared_ptr<Node> current = buckets_[i];
            while (current != nullptr) {
                const std::shared_ptr<Node> next = current->next;
                const size_t new_idx = getBucketIndex(current->hash);

                current->next = new_buckets[new_idx];
                new_buckets[new_idx] = current;

                current = next;
            }
        }

        buckets_.swap(new_buckets); // 替换为新桶数组
    }

public:
<<<<<<< HEAD:extensions/standard/lmStruct.hpp
    std::shared_ptr<lmStruct> parent_;
    explicit lmStruct();
    explicit lmStruct(const std::vector<std::pair<std::string, Value>>& vec);
    ~lmStruct();
    Value insert(const std::string& key, Value val);
=======
	lStruct();
    explicit lStruct(const std::vector<std::pair<std::string, Value>>& vec);
    ~lStruct();
    Value insert(const std::string& key, Value& val);
>>>>>>> a5d98f47f1d3eba474d6d9de372f35b311daa1f3:extensions/standard/lstruct.hpp
    [[nodiscard]] std::shared_ptr<Node> find(const std::string& key) const;
    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] std::vector<std::pair<std::string, Value>> to_vector() const;
    // 深拷贝结构体
    lmStruct(const lmStruct& other)
        : load_factor_(other.load_factor_)
    {
        std::vector<std::pair<std::string, Value>> all_kv = other.to_vector();
        size_t init_bucket_size = 16;
        while (init_bucket_size < (all_kv.size() / load_factor_)) {
            init_bucket_size *= 2;
        }
        buckets_.resize(init_bucket_size, nullptr);  // 初始化桶数组
        for (auto& [key, val] : all_kv) {
            this->insert(key, val);
        }
        this->elem_count_ = other.elem_count_;
    }
};
<<<<<<< HEAD:extensions/standard/lmStruct.hpp
=======

Value getattr_raw(const std::shared_ptr<lStruct>& lstruct_, const std::string& attr_name);
void setattr_raw(std::shared_ptr<lStruct> lstruct_, const std::string& attr_name, Value value);

Value new_lstruct(const std::vector<std::pair<std::string, Value>>& vec);
/* function: new_lstruct
 * args[0] =>  source vector: vec<pair<str, Value>
 */
Value getattr(const std::vector<Value>& args);
/* function: getattr
 * args[0] => struct: lstruct
 * args[1] => name: str
 */
Value setattr(const std::vector<Value>& args);
/* function: setattr
 * args[0] => struct: lstruct
 * args[1] => name: str
 * args[2] => val: Value
 */
Value update(const std::vector<Value>& args);
/* function: update
 * args[0] => source struct: lstruct
 * args[1] =>  other struct: lstruct
 */

namespace Lamina {
    LAMINA_FUNC_MULTI_ARGS("getattr", getattr, 2);
    LAMINA_FUNC_MULTI_ARGS("setattr", setattr, 3);
    LAMINA_FUNC_MULTI_ARGS("update", update, 2);
} // namespace Lamina
>>>>>>> a5d98f47f1d3eba474d6d9de372f35b311daa1f3:extensions/standard/lstruct.hpp
