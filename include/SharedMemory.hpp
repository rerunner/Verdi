#ifndef SHAREDMEMORY_HPP
#define SHAREDMEMORY_HPP

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/flat_map.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <numeric>
#include <vector>

namespace bip = boost::interprocess;
namespace bu = boost::uuids;
using uint128_t = boost::multiprecision::uint128_t;

/*
// Process owner init
auto repo1<MarkLayout>("name", 55);
auto repo2<ScanData>("name", 1000);
create("segment name", repo1, repo2);

// Client init
auto repo2<ScanData>("name", 1000);
auto repo2<ScanData>(1000);
open("segment name", repo2);
*/

namespace Verdi
{
namespace SharedMemory
{
static bu::uuid gen_uuid()
{
   bu::random_generator id_gen;
   return id_gen();
}

class ManagedSharedSegment
{
   typedef std::shared_ptr<bip::managed_shared_memory> managed_segment_ptr;

public:
   // Static members
   // =======================
   template<class... Repositories>
   static void
         create(const std::string &sharedMemoryName, Repositories &... repos)
   {
      // Compute repo size
      const std::size_t repo_size_mul = 1;
      const auto repo_sizes = {get_max_repo_size_b(repos)...};
      std::size_t total_repo_size_b =
            std::accumulate(repo_sizes.begin(), repo_sizes.end(), 0)
            * repo_size_mul;

      // Create Segment
      auto managed_segment = std::make_shared<ManagedSharedSegment>(
            sharedMemoryName,
            total_repo_size_b);

      // Connect repos
      using expand_type = int[];
      expand_type{(connect_repos(repos, managed_segment), 0)...};
   }

   template<class... Repositories>
   static void
         open(const std::string &sharedMemoryName, Repositories &... repos)
   {
      // Create Segment
      auto managed_segment =
            std::make_shared<ManagedSharedSegment>(sharedMemoryName);

      // Connect repos
      using expand_type = int[];
      expand_type{(connect_repos(repos, managed_segment), 0)...};
   }

   template<class RepoType>
   static std::size_t get_max_repo_size_b(const RepoType &repo)
   {
      return repo.max_allocated_size();
   }

   template<class RepoType>
   static void connect_repos(
         RepoType &repo,
         const std::shared_ptr<ManagedSharedSegment> &segment)
   {
      repo.connect(segment);
   }

   // Class members
   // ====================================================
   explicit ManagedSharedSegment(
         const std::string &sharedMemoryName,
         const std::size_t segment_size) :
      segment_name(sharedMemoryName),
      segment(new bip::managed_shared_memory(
            bip::open_or_create,
            segment_name.c_str(),
            segment_size)),
      delete_on_destruct(true)
   {
   }

   explicit ManagedSharedSegment(const std::string &sharedMemoryName) :
      segment_name(sharedMemoryName),
      segment(new bip::managed_shared_memory(
            bip::open_only,
            segment_name.c_str())),
      delete_on_destruct(false)
   {
   }

   ManagedSharedSegment(const ManagedSharedSegment &other) :
      segment_name(other.segment_name),
      segment(other.segment),
      delete_on_destruct(other.delete_on_destruct)
   {
   }

   ManagedSharedSegment(ManagedSharedSegment &&other) :
      segment_name(other.segment_name),
      segment(std::move(other.segment)),
      delete_on_destruct(other.delete_on_destruct)
   {
   }

   ~ManagedSharedSegment()
   {
      //! \note: Delete segment manager shared pointer first
      //!        before attempting to delete shared memory segment
      segment.reset();
      if (delete_on_destruct)
      {
         deleteSegment();
      }
   };

   std::string name() const noexcept { return segment_name; }

   managed_segment_ptr get() const noexcept { return segment; }

private:
   // Class members
   // ====================================================
   void deleteSegment()
   {
      (void)bip::shared_memory_object::remove(segment_name.c_str());
   }

   const std::string segment_name;
   managed_segment_ptr segment;
   const bool delete_on_destruct;
};

std::ostream &operator<<(std::ostream &out, const ManagedSharedSegment &obj)
{
   const auto segment = obj.get();
   const auto AllocatedMem = segment->get_size();
   const auto FreeMem = segment->get_free_memory();
   const auto Used = AllocatedMem - FreeMem;

   return out << "name, " << obj.name() << ", repositories, "
              << segment->get_num_named_objects() << ", alloc, " << AllocatedMem
              << ", free, " << FreeMem << ", used, " << Used;
}

typedef std::shared_ptr<ManagedSharedSegment> managed_shared_memory_ptr;

// template<class iter_type, class interprocess_lock_type>
// class synchronization_iterator
// {
// public:
//    using value_type = iter_type::value_type;
//    using difference_type = std::ptrdiff_t;
//    using pointer = value_type *;
//    using reference = value_type &;
//    using iterator_category = iter_type::iterator_category;

//    explicit synchronization_iterator(
//          iter_type wrapped_iter,
//          interprocess_lock_type &&intrp_lock) :
//       wrapped(wrapped_iter), lock(intrp_lock)
//    {
//    }
//    value_type operator*() const { return *wrapped; }
//    bool operator==(const synchronization_iterator &other) const
//    {
//       return wrapped == other.wrapped;
//    }
//    bool operator!=(const synchronization_iterator &other) const
//    {
//       return !(*this == other);
//    }
//    value_type operator++(int) { return *wrapped++; }
//    synchronization_iterator &operator++()
//    {
//       ++wrapped;
//       return *this;
//    }

// private:
//    iter_type wrapped;
//    interprocess_lock_type lock;
// };

class ShmContainer
{
public:
   virtual ~ShmContainer(){};

   virtual void connect(const managed_shared_memory_ptr &mem_segment) = 0;
   virtual std::size_t max_allocated_size() const noexcept = 0;
};

template<
      std::size_t NR,
      typename T,
      typename Key = bu::uuid,
      typename Compare = std::less<Key>>
class UnorderedRepository : public ShmContainer
{
public:
   using value_type = std::pair<Key, T>;
   using allocator_type = bip::
         allocator<value_type, bip::managed_shared_memory::segment_manager>;
   using container_type = bip::vector<value_type, allocator_type>;
   using key_type = Key;
   using mapped_type = T;
   using size_type = typename container_type::size_type;
   // using iterator = synchronization_iterator<
   //       container_type::iterator,
   //       scoped_lock<bip::interprocess_shareable_mutex>>;
   // using const_iterator = synchronization_iterator<
   //       container_type::const_iterator,
   //       sharable_lock<bip::interprocess_sharable_mutex>>;

   UnorderedRepository() :
      repo_name(typeid(container_type).name()),
      repo_size(NR),
      container(nullptr)
   {
   }

   explicit UnorderedRepository(const std::string &name) :
      repo_name(name), repo_size(NR), container(nullptr)
   {
   }

   // Container operations: Shared memory
   // ============================================
   void destroyManagedObject()
   {
      assert(segment != nullptr);
      segment->get()->destroy<container_type>(repo_name.c_str());
   }

   virtual void connect(const managed_shared_memory_ptr &mem_segment) final
   {
      segment = mem_segment;
      auto segment_manager = segment->get()->get_segment_manager();
      auto allocator = allocator_type{segment_manager};
      //! \note Attempts to find existing type or creates new if not found.
      //!       This ensures it never create multiple repositories of the same type
      container = segment->get()->find_or_construct<container_type>(
            repo_name.c_str())(allocator);
      //! \note Noop in case container is at reserved capacity, Maybe not do this?
      container->reserve(repo_size);
   }

   virtual std::size_t max_allocated_size() const noexcept final
   {
      const std::size_t const_offset_b = 1000;
      return sizeof(value_type) * max_size() + const_offset_b;
   }

   const std::string &name() const noexcept { return repo_name; }

   // Container operations: Iterators
   // ============================================
   // iterator begin() const
   // {
   //    bip::interprocess_sharable_mutex mutex;
   //    return synchronization_iterator(
   //          container->begin(),
   //          scoped_lock<bip::interprocess_sharable_mutex>(mutex));
   // }

   // Container operations: Capacity
   // ============================================
   bool empty() const noexcept
   {
      assert(container != nullptr);
      return container->empty();
   }

   size_type size() const noexcept
   {
      assert(container != nullptr);
      return container->size();
   }

   size_type max_size() const noexcept { return repo_size; }

   size_type capacity() const noexcept
   {
      assert(container != nullptr);
      return container->capacity();
   }

   // Container operations: Access
   // ============================================
   // 1. Implement safe iterators with shared mutex
   // 2. Use index based find operations
   // 3. Use (flat)map and disallow iteration
   const mapped_type &at(const key_type &key) const
   {
      assert(container != nullptr);
      // TODO: Take readlock
      return find_if([key](const value_type &v) { return v.first == key; })
            ->second;
   }

   // Container operations: Modifiers
   // ============================================
   void clear() noexcept
   {
      assert(container != nullptr);
      // TODO: Take writelock
      container->clear();
   }

   key_type insert(const mapped_type &value)
   {
      assert(typeid(key_type) == typeid(bu::uuid));
      assert(container != nullptr);
      throw_on_max_size();
      const auto id = gen_uuid();
      // TODO: Take writelock
      (void)container->emplace_back(value_type{id, value});

      return id;
   }

   void insert(const value_type &elm)
   {
      assert(container != nullptr);
      throw_on_max_size();
      // TODO: Take writelock
      (void)container->emplace_back(elm);
   }

   // mapped_type& operator[](const key_type& k)
   // {
   //    assert(container != nullptr);
   //    if (!contains(k))
   //    {
   //       throw_on_max_size();
   //    }

   //    // TODO: Take writelock??? How?
   //    return (*container)[k];
   // }

   // size_type erase(const key_type& key)
   // {
   //    assert(container != nullptr);
   //    // TODO: Take writelock
   //    return container->erase(key);
   // }

   // Container operations: Lookup
   // ============================================
   // size_type count(const key_type& key) const
   // {
   //    assert(container != nullptr);
   //    // TODO: Take readlock
   //    return container->count(key);
   // }

   bool contains(const key_type &key) const
   {
      assert(container != nullptr);
      // TODO: Take readlock
      return find_if([key](const value_type &v) { return v.first == key; });
   }

   template<class UnaryPredicate>
   boost::optional<value_type> find_if(UnaryPredicate q) const
   {
      assert(container != nullptr);

      // TODO: Take readlock
      auto itr = std::find_if(container->cbegin(), container->cend(), q);

      if (itr != container->cend())
      {
         return *itr;
      }

      return boost::none;
   }

   template<class UnaryPredicate>
   boost::optional<value_type> find_if_not(UnaryPredicate q) const
   {
      assert(container != nullptr);

      // TODO: Take readlock
      auto itr = std::find_if_not(container->cbegin(), container->cend(), q);

      if (itr != container->cend())
      {
         return *itr;
      }

      return boost::none;
   }

   template<class UnaryPredicate>
   std::vector<value_type> find_all(UnaryPredicate q) const
   {
      assert(container != nullptr);
      std::vector<value_type> to_vec;

      // TODO: Take readlock
      std::copy_if(
            container->cbegin(),
            container->cend(),
            std::back_inserter(to_vec),
            q);

      return to_vec;
   }

private:
   void throw_on_max_size()
   {
      if (size() >= max_size())
      {
         throw "Bad alloc, reached shm_capacity";
      }
   }

   const std::string repo_name;
   const size_type repo_size;
   container_type *container;
   managed_shared_memory_ptr segment;
};

template<std::size_t NR, typename T, typename Key, typename Compare>
std::ostream &operator<<(
      std::ostream &out,
      const UnorderedRepository<NR, T, Key, Compare> &obj)
{
   return out << "name, " << obj.name() << ", max_size, " << obj.max_size()
              << ", size, " << obj.size();
}

//TODO: difference btw mapped_region and managed_shared_memory/allocators
template<
      std::size_t NR,
      typename T,
      typename Key = bu::uuid,
      typename Compare = std::less<Key>>
class Repository : public ShmContainer
{
public:
   using value_type = std::pair<Key, T>;
   using allocator_type = bip::
         allocator<value_type, bip::managed_shared_memory::segment_manager>;
   using map_type = bip::flat_map<Key, T, Compare, allocator_type>;
   using key_type = typename map_type::key_type;
   using mapped_type = typename map_type::mapped_type;
   using size_type = typename map_type::size_type;

   Repository() :
      repo_name(typeid(map_type).name()), repo_size(NR), container(nullptr)
   {
   }

   explicit Repository(const std::string &name) :
      repo_name(name), repo_size(NR), container(nullptr)
   {
   }

   // Container operations: Shared memory
   // ============================================
   void destroyManagedObject()
   {
      assert(segment != nullptr);
      segment->get()->destroy<map_type>(repo_name.c_str());
   }

   virtual void connect(const managed_shared_memory_ptr &mem_segment) final
   {
      segment = mem_segment;
      auto segment_manager = segment->get()->get_segment_manager();
      auto allocator = allocator_type{segment_manager};
      //! \note Attempts to find existing type or creates new if not found.
      //!       This ensures it never create multiple repositories of the same type
      container = segment->get()->find_or_construct<map_type>(
            repo_name.c_str())(Compare{}, allocator);
      //! \note Noop in case container is at reserved capacity, Maybe not do this?
      container->reserve(repo_size);
   }

   virtual std::size_t max_allocated_size() const noexcept final
   {
      const std::size_t const_offset_b = 1000;
      return sizeof(value_type) * max_size() + const_offset_b;
   }

   const std::string &name() const noexcept { return repo_name; }

   // Container operations: Capacity
   // ============================================
   bool empty() const noexcept
   {
      assert(container != nullptr);
      return container->empty();
   }

   size_type size() const noexcept
   {
      assert(container != nullptr);
      return container->size();
   }

   size_type max_size() const noexcept { return repo_size; }

   size_type capacity() const noexcept
   {
      assert(container != nullptr);
      return container->capacity();
   }

   // Container operations: Access
   // ============================================
   // 1. Implement safe iterators with shared mutex
   // 2. Use index based find operations
   // 3. Use (flat)map and disallow iteration
   const mapped_type &at(const key_type &key) const
   {
      assert(container != nullptr);
      // TODO: Take readlock
      return container->at(key);
   }

   // Container operations: Modifiers
   // ============================================
   void clear() noexcept
   {
      assert(container != nullptr);
      // TODO: Take writelock
      container->clear();
   }

   key_type insert(const mapped_type &value)
   {
      assert(typeid(key_type) == typeid(bu::uuid));
      assert(container != nullptr);
      throw_on_max_size();
      const auto id = gen_uuid();
      // TODO: Take writelock
      (void)container->insert(value_type{id, value});

      return id;
   }

   void insert(const value_type &elm)
   {
      assert(container != nullptr);
      throw_on_max_size();
      // TODO: Take writelock
      (void)container->insert(elm);
   }

   mapped_type &operator[](const key_type &k)
   {
      assert(container != nullptr);
      if (!contains(k))
      {
         throw_on_max_size();
      }

      // TODO: Take writelock??? How?
      return (*container)[k];
   }

   size_type erase(const key_type &key)
   {
      assert(container != nullptr);
      // TODO: Take writelock
      return container->erase(key);
   }

   // Container operations: Lookup
   // ============================================
   size_type count(const key_type &key) const
   {
      assert(container != nullptr);
      // TODO: Take readlock
      return container->count(key);
   }

   bool contains(const key_type &key) const
   {
      assert(container != nullptr);
      // TODO: Take readlock
      return container->contains(key);
   }

   template<class UnaryPredicate>
   boost::optional<value_type> find_if(UnaryPredicate q) const
   {
      assert(container != nullptr);

      // TODO: Take readlock
      auto itr = std::find_if(container->cbegin(), container->cend(), q);

      if (itr != container->cend())
      {
         return *itr;
      }

      return boost::none;
   }

   template<class UnaryPredicate>
   boost::optional<value_type> find_if_not(UnaryPredicate q) const
   {
      assert(container != nullptr);

      // TODO: Take readlock
      auto itr = std::find_if_not(container->cbegin(), container->cend(), q);

      if (itr != container->cend())
      {
         return *itr;
      }

      return boost::none;
   }

   template<class UnaryPredicate>
   std::vector<value_type> find_all(UnaryPredicate q) const
   {
      assert(container != nullptr);
      std::vector<value_type> to_vec;

      // TODO: Take readlock
      std::copy_if(
            container->cbegin(),
            container->cend(),
            std::back_inserter(to_vec),
            q);

      return to_vec;
   }

private:
   void throw_on_max_size()
   {
      if (size() >= max_size())
      {
         throw "Bad alloc, reached shm_capacity";
      }
   }

   const std::string repo_name;
   const size_type repo_size;
   map_type *container;
   managed_shared_memory_ptr segment;
};

template<std::size_t NR, typename T, typename Key, typename Compare>
std::ostream &
      operator<<(std::ostream &out, const Repository<NR, T, Key, Compare> &obj)
{
   return out << "name, " << obj.name() << ", max_size, " << obj.max_size()
              << ", size, " << obj.size();
}

} // end of namespace SharedMemory
} // end of namespace Verdi

#endif // SHAREDMEMORY_HPP
