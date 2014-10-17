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

#ifdef SOCKETLITE_OS_WINDOWS
template <typename TKey, typename THash = stdext::hash_compare<TKey, std::less<TKey> >, typename TKeyCompare = std::less<TKey>, typename TMutex = SL_Sync_NullMutex>
#else
template <typename TKey, typename THash = SL_DEFAULT_HASHFUNC<TKey>, class TKeyCompare = std::less<TKey>, typename TMutex = SL_Sync_NullMutex>
#endif
class SL_Utility_HashTreeSet
{
public:
    typedef std::set<TKey, TKeyCompare> DATA_SET;
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
        for (; i<SL_NUM_2_POW; ++i)
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
        for (i=0; i<bucket_size; ++i)
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
        bucket_mask_ = 0;
        elements_num_ = 0;
        return 0;
    }

    inline bool find(const key_type &key)
    {
        BUCKET *bucket = find_position(key);

        bucket->mutex.lock();
        typename DATA_SET::iterator iter = bucket->data_set.find(key);
        if (iter != bucket->data_set.end())
        {
            bucket->mutex.unlock();
            return true;
        }
        bucket->mutex.unlock();

        return false;
    }

    inline bool insert(const key_type &key)
    {
        BUCKET *bucket = find_position(key);

        bool ret = false;
        bucket->mutex.lock();
        size_type before_size = bucket->data_set.size();
        bucket->data_set.insert(key);
        if (bucket->data_set.size() > before_size)
        {
            ++elements_num_;
            ret = true;
        }
        bucket->mutex.unlock();

        return ret;
    }

    inline bool erase(const key_type &key)
    {
        BUCKET *bucket = find_position(key);

        bool ret = false;
        bucket->mutex.lock();
        size_type remove_num = bucket->data_set.erase(key);
        if (remove_num > 0)
        {
            --elements_num_;
            ret = true;
        }
        bucket->mutex.unlock();

        return ret;
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
        return (0 == size());
    }

    inline BUCKET* bucket(size_type index)
    {
        assert(index < bucket_size());

        return buckets_[index];
    }

private:
    inline BUCKET* find_position(const key_type &key)
    {
        assert(buckets_.size() > 0);

        size_t bucket_index = hasher_(key) & bucket_mask_;
        return buckets_[bucket_index];
    }

private:
    hash_fn                 hasher_;
    std::vector<BUCKET * >  buckets_;       //桶数组
    size_type               bucket_mask_;   //桶掩码(bucket_mask_ = bucket_size - 1)
    size_type               elements_num_;  //元素个数
    mutex_type              mutex_;

    SL_ObjectPool_SimpleEx<BUCKET, SL_Sync_NullMutex> bucket_pool_;
};

#endif

