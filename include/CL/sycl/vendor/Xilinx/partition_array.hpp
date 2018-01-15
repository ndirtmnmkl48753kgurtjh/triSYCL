#ifndef TRISYCL_SYCL_VENDOR_XILINX_PARTITION_ARRAY_HPP
#define TRISYCL_SYCL_VENDOR_XILINX_PARTITION_ARRAY_HPP

/** \file This is a class for arrays that can be partitioned.

    Since the limitation in Silence xocc, the implementation now cam only
    support 1-dim arrays partition.

    This file is distributed under the University of Illinois Open Source
    License. See LICENSE.TXT for details.
*/

#include <array>
#include <cstddef>

namespace cl {
namespace sycl {
namespace xilinx {

/** \addtogroup helpers Some helpers for the implementation
    @{
*/


/** For specifying the array partition types.

    To be used when define or declare a vendor supported partition array in
    kernel.
*/
namespace partition {
  /** Three partition types: cyclic, block, complete.

      none represents standard array.
  */
  enum class type {
    cyclic,
    block,
    complete,
    none
  };


  /** Represent a cyclic partition.

      The single array would be partitioned into several small physical
      memories in this case. These small physical memories can be
      accessed simultaneously which drive the performance. Each
      element in the array would be partitioned to each memory in order
      and cyclically.

      That is if we have a 4-element array which contains 4 integers
      0, 1, 2, and 3. If we set factor to 2, and partition
      dimension to 1 for this cyclic partition array. Then, the
      contents of this array will be distributed to 2 physical
      memories: one contains 1, 3 and the other contains 2,4.

      \param PhyMemNum is the number of physical memories that user wants to
      have.

      \param PDim is the dimension that user wants to apply cyclic partition on.
      If PDim is 0, all dimensions will be partitioned with cyclic order.
  */
  template <std::size_t PhyMemNum = 1, std::size_t PDim = 1>
  struct cyclic {
    static constexpr auto phsycal_mem_num = PhyMemNum;
    static constexpr auto partition_dim = PDim;
    static constexpr auto partition_type = type::cyclic;
  };


  /** Represent a block partition.

      The single array would be partitioned into several small
      physical memories and can be accessed simultaneously, too.
      However, the first physical memory will be filled up first, then
      the next.

      That is if we have a 4-element array which contains 4 integers
      0, 1, 2, and 3. If we set factor to 2, and partition
      dimension to 1 for this cyclic partition array. Then, the
      contents of this array will be distributed to 2 physical
      memories: one contains 1, 2 and the other contains 3,4.

      \param ElmInEachPhyMem is the number of elements in each physical memory
      that user wants to have.

      \param PDim is the dimension that user wants to apply block partition on.
      If PDim is 0, all dimensions will be partitioned with block order.
  */
  template <std::size_t ElmInEachPhyMem = 1, std::size_t PDim = 1>
  struct block {
    static constexpr auto ele_in_each_phsycal_mem = ElmInEachPhyMem;
    static constexpr auto partition_dim = PDim;
    static constexpr auto partition_type = type::block;
  };


  /** Represent a complete partition.

      The single array would be partitioned into individual elements.
      That is if we have a 4-element array with one dimension, the
      array is completely partitioned into distributed RAM or 4
      independent registers.

      \param PDim is the dimension that user wants to apply complete partition
      on. If PDim is 0, all dimensions will be completely partitioned.
  */
  template <std::size_t PDim = 1>
  struct complete {
    static constexpr auto partition_dim = PDim;
    static constexpr auto partition_type = type::complete;
  };


  /** Represent a none partition.

      The single array would be the same as std::array.
  */
  struct none {
    static constexpr auto partition_type = type::none;
  };
}


/** Define an array class with partition feature.

    Since on FPGA, users can customize the memory architecture in the system
    and within the CU. Array partition help us to partition single array to
    multiple memories that can be accessed simultaneously getting higher memory
    bandwidth.

    \param ValueType is the type of element.

    \param Size is the size of the array.

    \param PartitionType is the array partition type: cyclic, block, and
    complete. The default type is none.

    \todo Deal with multi-dimension array.
*/
template <typename ValueType,
          std::size_t Size,
          typename PartitionType = partition::none>
struct partition_array {

  ValueType elems[Size];
  static constexpr auto array_size = Size;
  static constexpr auto partition_type = PartitionType::partition_type;
  using element_type = ValueType;


  /// Provide iterator
  auto begin() { return elems; }
  const auto begin() const { return elems; }
  auto end() { return elems+Size; }
  const auto end() const { return elems+Size; }


  /// Evaluate size
  constexpr auto size() const noexcept {
    return Size;
  }


  /// Construct an array
  partition_array() {
    // Add the intrinsic according expressing to the target compiler the
    // partitioning to use
    if constexpr (partition_type == partition::type::cyclic)
      _ssdm_SpecArrayPartition(&(*this)[0], PartitionType::partition_dim,
                               "CYCLIC", PartitionType::phsycal_mem_num, "");
    if constexpr (partition_type == partition::type::block)
      _ssdm_SpecArrayPartition(&(*this)[0], PartitionType::partition_dim,
                               "BLOCK", PartitionType::ele_in_each_phsycal_mem,
                               "");
    if constexpr (partition_type == partition::type::complete)
      _ssdm_SpecArrayPartition(&(*this)[0], PartitionType::partition_dim,
                               "COMPLETE", 0, "");
  }


  /// A constructor from another array of the same size
  template <typename SourceBasicType>
  partition_array(const partition_array<PartitionType, Size,
                  SourceBasicType> &src)
    : partition_array{ } {
    std::copy_n(&src[0], Size, &(*this)[0]);
  }


  /// Construct an array from a std::array
  template <typename SourceBasicType>
  partition_array(const std::array<SourceBasicType, Size> &src)
    : partition_array{ } {
    std::copy_n(&src[0], Size, &(*this)[0]);
  }


  /// Construct an array from initializer_list
  template <typename SourceBasicType>
  partition_array(std::initializer_list<SourceBasicType> l)
    : partition_array{ } {
    std::size_t i = 0;
    for (auto itr = l.begin(); itr != l.end(); itr++)
      (*this)[i++] = *itr;
  }


  /// Provide a subscript operator
  constexpr ValueType& operator[](std::size_t i) {
    return elems[i];
  }


  constexpr const ValueType& operator[](std::size_t i) const {
    return elems[i];
  }


  /// Return the partition type of the array
  constexpr auto get_partition_type() const {
    return partition_type;
  }
};


/// @} End the helpers Doxygen group

}
}
}

/*
    # Some Emacs stuff:
    ### Local Variables:
    ### ispell-local-dictionary: "american"
    ### eval: (flyspell-prog-mode)
    ### End:
*/

#endif// TRISYCL_SYCL_VENDOR_XILINX_PARTITION_ARRAY_HPP
