#ifndef SOCKETLITE_UTILITY_HASH_TREE_SET_H
#define SOCKETLITE_UTILITY_HASH_TREE_SET_H

#include <assert.h>
#include <functional>
#include <vector>
#include <set>

#include "SL_Config.h"
#include "SL_Hash_Fun.h"
#include "SL_Sync_Mutex.h"
#include "SL_Sync_SpinMutex.h"
#include "SL_Sync_Atomic.h"

template <typename TKey, typename THash = SL_DEFAULT_HASHFUNC<TKey>, class TKeyCompare = std::less<TKey>, typename TMutex = SL_Sync_SpinMutex>
class SL_Utility_HashTreeSet
{
public:
    typedef std::set<TKey, TKeyCompare> DATA_SET;
    typedef typename DATA_SET::iterator DATA_SET_ITERATOR;
    typedef TMutex   mutex_type;
    typedef TKey     key_type;
    typedef THash    hash_fn;
    typedef size_t   size_type;

    struct BUCKET
    {
        DATA_SET     data_set;
        mutex_type   mutex;
    };

    SL_Utility_HashTreeSet()
        : buckets_(NULL)
        , bucket_mask_(0)
        , elements_num_(0)
    {
    }

    ~SL_Utility_HashTreeSet()
    {
        clear();
    }

    int init(size_type bucket_size, size_type chunk_size = 10)
    {   
        clear();

        size_type i = 0;
        for (; i < SL_NUM_2_POW; ++i)
        {
            if (bucket_size <= SL_2_POW_LIST[i])
            {
                bucket_size = SL_2_POW_LIST[i];
                break;
            }
        }
        if (i == SL_NUM_2_POW)
        {
            bucket_size = SL_2_POW_LIST[SL_NUM_2_POW - 1];
        }
        bucket_mask_ = bucket_size - 1;

        if (chunk_size > bucket_size)
        {
            chunk_size = bucket_size;
        }
        else if (chunk_size == 0)
        {
            chunk_size = 1;
        }

        BUCKET *bucket;
        bucket_pool_.init(bucket_size / chunk_size + 1, bucket_size / chunk_size + 1, chunk_size);
        buckets_.reserve(bucket_size);
        for (i = 0; i < bucket_size; ++i)
        {
            bucket = bucket_pool_.alloc_object();
            buckets_.push_back(bucket);
        }

        return 0;
    }

    int clear()
    {
        buckets_.clear();
        bucket_pool_.clear();
        elements_num_.store(0);
        bucket_mask_ = 0;
        return 0;
    }

    inline bool find(const key_type &key)
    {
        BUCKET *bucket = find_bucket(key);

        typename DATA_SET::iterator iter;
        bucket->mutex.lock();
        iter = bucket->data_set.find(key);
        bucket->mutex.unlock();
        if (iter != bucket->data_set.end())
        {
            return true;
        }
        return false;
    }

    inline bool find(const key_type &key, DATA_SET_ITERATOR &result)
    {
        BUCKET *bucket = find_bucket(key);

        bucket->mutex.lock();
        result = bucket->data_set.find(key);
        bucket->mutex.unlock();
        if (result != bucket->data_set.end())
        {
            return true;
        }

        return false;
    }

    inline bool insert(const key_type &key)
    {
        BUCKET *bucket = find_bucket(key);

        size_type before_size;
        size_type after_size;
        bucket->mutex.lock();
        before_size = bucket->data_set.size();
        bucket->data_set.insert(key);
        after_size  = bucket->data_set.size();
        bucket->mutex.unlock();
        if (after_size > before_size)
        {
            ++elements_num_;
            return true;
        }

        return false;
    }

    inline bool erase(const key_type &key)
    {
        BUCKET *bucket = find_bucket(key);

        size_type remove_num;
        bucket->mutex.lock();
        remove_num = bucket->data_set.erase(key);
        bucket->mutex.unlock();
        if (remove_num > 0)
        {
            --elements_num_;
            return true;
        }

        return false;
    }

    inline bool erase(DATA_SET_ITERATOR &position)
    {
        BUCKET *bucket = find_bucket(*position);

        size_type before_size;
        size_type after_size;
        bucket->mutex.lock();
        before_size = bucket->data_set.size();
        bucket->data_set.erase(position);
        after_size  = bucket->data_set.size();
        bucket->mutex.unlock();
        if (after_size < before_size)
        {
            --elements_num_;
            return true;
        }

        return false;
    }

    inline size_type bucket_size() const
    {
        return buckets_.size();
    }

    inline size_type size()
    {
        return elements_num_.load();
    }

    inline bool empty()
    {
        return (elements_num_.load() == 0);
    }

    inline BUCKET* bucket(size_type index)
    {
        assert(index < bucket_size());

        return buckets_[index];
    }

private:
    inline BUCKET* find_bucket(const key_type &key)
    {
        assert(buckets_.size() > 0);

        size_t bucket_index = hasher_(key) & bucket_mask_;
        return buckets_[bucket_index];
    }

private:
    hash_fn                 hasher_;
    std::vector<BUCKET * >  buckets_;       //桶数组
    size_type               bucket_mask_;   //桶掩码(bucket_mask_ = bucket_size - 1)
    SL_Sync_Atomic_Int32    elements_num_;  //元素个数

    SL_ObjectPool_SimpleEx<BUCKET, SL_Sync_NullMutex> bucket_pool_;
};

//针对单线程作类模板偏特化,以提高单线程下的性能
template <typename TKey, typename THash, class TKeyCompare>
class SL_Utility_HashTreeSet<TKey, THash, TKeyCompare, SL_Sync_NullMutex>
{
public:
    typedef std::set<TKey, TKeyCompare> DATA_SET;
    typedef typename DATA_SET::iterator DATA_SET_ITERATOR;
    typedef TKey     key_type;
    typedef THash    hash_fn;
    typedef size_t   size_type;
    typedef DATA_SET BUCKET;

    SL_Utility_HashTreeSet()
        : buckets_(NULL)
        , bucket_mask_(0)
        , elements_num_(0)
    {
    }

    ~SL_Utility_HashTreeSet()
    {
        clear();
    }

    int init(size_type bucket_size, size_type chunk_size = 10)
    {   
        clear();

        size_type i = 0;
        for (; i < SL_NUM_2_POW; ++i)
        {
            if (bucket_size <= SL_2_POW_LIST[i])
            {
                bucket_size = SL_2_POW_LIST[i];
                break;
            }
        }
        if (i == SL_NUM_2_POW)
        {
            bucket_size = SL_2_POW_LIST[SL_NUM_2_POW - 1];
        }
        bucket_mask_ = bucket_size - 1;

        if (chunk_size > bucket_size)
        {
            chunk_size = bucket_size;
        }
        else if (chunk_size == 0)
        {
            chunk_size = 1;
        }

        BUCKET *bucket;
        bucket_pool_.init(bucket_size / chunk_size + 1, bucket_size / chunk_size + 1, chunk_size);
        buckets_.reserve(bucket_size);
        for (i = 0; i < bucket_size; ++i)
        {
            bucket = bucket_pool_.alloc_object();
            buckets_.push_back(bucket);
        }

        return 0;
    }

    int clear()
    {
        buckets_.clear();
        bucket_pool_.clear();
        elements_num_ = 0;
        bucket_mask_ = 0;
        return 0;
    }

    inline bool find(const key_type &key)
    {
        BUCKET *bucket = find_bucket(key);

        typename DATA_SET::iterator iter = bucket->find(key);
        if (iter != bucket->end())
        {
            return true;
        }
        return false;
    }

    inline bool find(const key_type &key, DATA_SET_ITERATOR &result)
    {
        BUCKET *bucket = find_bucket(key);

        result = bucket->find(key);
        if (result != bucket->end())
        {
            return true;
        }

        return false;
    }

    inline bool insert(const key_type &key)
    {
        BUCKET *bucket = find_bucket(key);

        size_type before_size = bucket->size();
        bucket->insert(key);
        if (bucket->size() > before_size)
        {
            ++elements_num_;
            return true;
        }

        return false;
    }

    inline bool erase(const key_type &key)
    {
        BUCKET *bucket = find_bucket(key);

        size_type remove_num = bucket->erase(key);
        if (remove_num > 0)
        {
            --elements_num_;
            return true;
        }

        return false;
    }

    inline bool erase(DATA_SET_ITERATOR &position)
    {
        BUCKET *bucket = find_bucket(*position);

        size_type before_size = bucket->size();
        bucket->erase(position);
        if (bucket->size() < before_size)
        {
            --elements_num_;
            return true;
        }

        return false;
    }

    inline size_type bucket_size() const
    {
        return buckets_.size();
    }

    inline size_type size() const
    {
        return elements_num_;
    }

    inline bool empty() const
    {
        return (0 == elements_num_);
    }

    inline BUCKET* bucket(size_type index)
    {
        assert(index < bucket_size());

        return buckets_[index];
    }

private:
    inline BUCKET* find_bucket(const key_type &key)
    {
        assert(buckets_.size() > 0);

        size_t bucket_index = hasher_(key) & bucket_mask_;
        return buckets_[bucket_index];
    }

private:
    hash_fn                 hasher_;
    std::vector<BUCKET * >  buckets_;       //桶数组
    size_type               bucket_mask_;   //桶掩码(bucket_mask_ = bucket_size - 1)
    uint32                  elements_num_;  //元素个数

    SL_ObjectPool_SimpleEx<BUCKET, SL_Sync_NullMutex> bucket_pool_;
};

#endif

