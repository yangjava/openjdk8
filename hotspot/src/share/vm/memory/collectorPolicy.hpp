/*
 * Copyright (c) 2001, 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_MEMORY_COLLECTORPOLICY_HPP
#define SHARE_VM_MEMORY_COLLECTORPOLICY_HPP

#include "memory/allocation.hpp"
#include "memory/barrierSet.hpp"
#include "memory/generationSpec.hpp"
#include "memory/genRemSet.hpp"
#include "utilities/macros.hpp"

// This class (or more correctly, subtypes of this class)
// are used to define global garbage collector attributes.
// This includes initialization of generations and any other
// shared resources they may need.
//
// In general, all flag adjustment and validation should be
// done in initialize_flags(), which is called prior to
// initialize_size_info().
//
// This class is not fully developed yet. As more collector(s)
// are added, it is expected that we will come across further
// behavior that requires global attention. The correct place
// to deal with those issues is this class.

// Forward declarations.
class GenCollectorPolicy;
class TwoGenerationCollectorPolicy;
class AdaptiveSizePolicy;
#if INCLUDE_ALL_GCS
class ConcurrentMarkSweepPolicy;
class G1CollectorPolicy;
#endif // INCLUDE_ALL_GCS

class GCPolicyCounters;
class MarkSweepPolicy;

// CollectorPolicy的定义在hotspot/src/share/vm/memory/collectorPolicy.hpp中，该类及其子类用于定义垃圾回收器使用的全局属性，并初始化分代内存及其他共享资源。
class CollectorPolicy : public CHeapObj<mtGC> {
 protected:
 // _gc_policy_counters：跟踪分代内存的性能的计数器
  GCPolicyCounters* _gc_policy_counters;

  virtual void initialize_alignments() = 0;
  virtual void initialize_flags();
  virtual void initialize_size_info();

  DEBUG_ONLY(virtual void assert_flags();)
  DEBUG_ONLY(virtual void assert_size_info();)

  // _initial_heap_byte_size：初始堆内存 
  size_t _initial_heap_byte_size;
  // _max_heap_byte_size：最大堆内存
  size_t _max_heap_byte_size;
  // _min_heap_byte_size：最小堆内存
  size_t _min_heap_byte_size;

  // _space_alignment：space分配粒度
  size_t _space_alignment;
  // heap分配粒度，_heap_alignment必须大于_space_alignment，且是_space_alignment的整数倍
  size_t _heap_alignment;

  // Needed to keep information if MaxHeapSize was set on the command line
  // when the flag value is aligned etc by ergonomics
  // 是否通过命令行参数设置了最大堆内存
  bool _max_heap_size_cmdline;

  // The sizing of the heap are controlled by a sizing policy.
  // 用来自适应调整堆内存大小的策略实现
  AdaptiveSizePolicy* _size_policy;

  // Set to true when policy wants soft refs cleared.
  // Reset to false by gc after it clears all soft refs.
  // 是否需要清除所有的软引用，当软引用清除结束，垃圾回收器会将其置为false
  bool _should_clear_all_soft_refs;

  // Set to true by the GC if the just-completed gc cleared all
  // softrefs.  This is set to true whenever a gc clears all softrefs, and
  // set to false each time gc returns to the mutator.  For example, in the
  // ParallelScavengeHeap case the latter would be done toward the end of
  // mem_allocate() where it returns op.result()
  // 当GC刚清除完所有的软引用时会设置该属性为true，当返回mutator时被设置成false
  bool _all_soft_refs_clear;

  CollectorPolicy();

 public:
  // 构造方法调用后就会调用initialize_all方法
  virtual void initialize_all() {
    // initialize_alignments用来初始化分代内存及内存分配相关属性的，没有默认实现
    initialize_alignments();
    // initialize_flags()初始化了永久代的一些大小配置参数
    initialize_flags();
    // initialize_size_info()设置了Java堆大小的相关参数
    initialize_size_info();
  }

  // Return maximum heap alignment that may be imposed by the policy
  static size_t compute_heap_alignment();

  size_t space_alignment()        { return _space_alignment; }
  size_t heap_alignment()         { return _heap_alignment; }

  size_t initial_heap_byte_size() { return _initial_heap_byte_size; }
  size_t max_heap_byte_size()     { return _max_heap_byte_size; }
  size_t min_heap_byte_size()     { return _min_heap_byte_size; }

  // CollectorPolicy定义了一个枚举Name来描述子类的名称
  enum Name {
    CollectorPolicyKind,
    TwoGenerationCollectorPolicyKind,
    ConcurrentMarkSweepPolicyKind,
    ASConcurrentMarkSweepPolicyKind,
    G1CollectorPolicyKind
  };

  AdaptiveSizePolicy* size_policy() { return _size_policy; }
  bool should_clear_all_soft_refs() { return _should_clear_all_soft_refs; }
  void set_should_clear_all_soft_refs(bool v) { _should_clear_all_soft_refs = v; }
  // Returns the current value of _should_clear_all_soft_refs.
  // _should_clear_all_soft_refs is set to false as a side effect.
  bool use_should_clear_all_soft_refs(bool v);
  bool all_soft_refs_clear() { return _all_soft_refs_clear; }
  void set_all_soft_refs_clear(bool v) { _all_soft_refs_clear = v; }

  // Called by the GC after Soft Refs have been cleared to indicate
  // that the request in _should_clear_all_soft_refs has been fulfilled.
  void cleared_all_soft_refs();

  // Identification methods.
  virtual GenCollectorPolicy*           as_generation_policy()            { return NULL; }
  virtual TwoGenerationCollectorPolicy* as_two_generation_policy()        { return NULL; }
  virtual MarkSweepPolicy*              as_mark_sweep_policy()            { return NULL; }
#if INCLUDE_ALL_GCS
  virtual ConcurrentMarkSweepPolicy*    as_concurrent_mark_sweep_policy() { return NULL; }
  virtual G1CollectorPolicy*            as_g1_policy()                    { return NULL; }
#endif // INCLUDE_ALL_GCS
  // Note that these are not virtual.
  bool is_generation_policy()            { return as_generation_policy() != NULL; }
  bool is_two_generation_policy()        { return as_two_generation_policy() != NULL; }
  bool is_mark_sweep_policy()            { return as_mark_sweep_policy() != NULL; }
#if INCLUDE_ALL_GCS
  bool is_concurrent_mark_sweep_policy() { return as_concurrent_mark_sweep_policy() != NULL; }
  bool is_g1_policy()                    { return as_g1_policy() != NULL; }
#else  // INCLUDE_ALL_GCS
  bool is_concurrent_mark_sweep_policy() { return false; }
  bool is_g1_policy()                    { return false; }
#endif // INCLUDE_ALL_GCS


  virtual BarrierSet::Name barrier_set_name() = 0;

  // Create the remembered set (to cover the given reserved region,
  // allowing breaking up into at most "max_covered_regions").
  virtual GenRemSet* create_rem_set(MemRegion reserved,
                                    int max_covered_regions);

  // This method controls how a collector satisfies a request
  // for a block of memory.  "gc_time_limit_was_exceeded" will
  // be set to true if the adaptive size policy determine that
  // an excessive amount of time is being spent doing collections
  // and caused a NULL to be returned.  If a NULL is not returned,
  // "gc_time_limit_was_exceeded" has an undefined meaning.
  virtual HeapWord* mem_allocate_work(size_t size,
                                      bool is_tlab,
                                      bool* gc_overhead_limit_was_exceeded) = 0;

  // This method controls how a collector handles one or more
  // of its generations being fully allocated.
  virtual HeapWord *satisfy_failed_allocation(size_t size, bool is_tlab) = 0;
  // This method controls how a collector handles a metadata allocation
  // failure.
  virtual MetaWord* satisfy_failed_metadata_allocation(ClassLoaderData* loader_data,
                                                       size_t size,
                                                       Metaspace::MetadataType mdtype);

  // Performace Counter support
  GCPolicyCounters* counters()     { return _gc_policy_counters; }

  // Create the jstat counters for the GC policy.  By default, policy's
  // don't have associated counters, and we complain if this is invoked.
  virtual void initialize_gc_policy_counters() {
    ShouldNotReachHere();
  }

  virtual CollectorPolicy::Name kind() {
    return CollectorPolicy::CollectorPolicyKind;
  }

  // Returns true if a collector has eden space with soft end.
  virtual bool has_soft_ended_eden() {
    return false;
  }

  // Do any updates required to global flags that are due to heap initialization
  // changes
  // 当CollectedHeap初始化结束后就会调用post_heap_initialize对已初始化的参数做必要的更新
  virtual void post_heap_initialize() = 0;
};

class ClearedAllSoftRefs : public StackObj {
  bool _clear_all_soft_refs;
  CollectorPolicy* _collector_policy;
 public:
  ClearedAllSoftRefs(bool clear_all_soft_refs,
                     CollectorPolicy* collector_policy) :
    _clear_all_soft_refs(clear_all_soft_refs),
    _collector_policy(collector_policy) {}

  ~ClearedAllSoftRefs() {
    if (_clear_all_soft_refs) {
      _collector_policy->cleared_all_soft_refs();
    }
  }
};

// GenCollectorPolicy继承自CollectorPolicy，表示分代内存使用的CollectorPolicy，同样定义在collectorPolicy.hpp中
class GenCollectorPolicy : public CollectorPolicy {
 protected:
  // _min_gen0_size：gen0的内存最小值
  size_t _min_gen0_size;
  // _initial_gen0_size：gen0的内存初始值
  size_t _initial_gen0_size;
  // _max_gen0_size：gen0的内存最大值
  size_t _max_gen0_size;

  // _gen_alignment and _space_alignment will have the same value most of the
  // time. When using large pages they can differ.
  // _gen_alignment：分代内存分配粒度，_gen_alignment必须被_space_alignment整除，_heap_alignment被_gen_alignment整除
  size_t _gen_alignment;
  
  // _generations一种特殊的Generation实现
  GenerationSpec **_generations;

  // Return true if an allocation should be attempted in the older
  // generation if it fails in the younger generation.  Return
  // false, otherwise.
  virtual bool should_try_older_generation_allocation(size_t word_size) const;

  void initialize_flags();
  void initialize_size_info();

  DEBUG_ONLY(void assert_flags();)
  DEBUG_ONLY(void assert_size_info();)

  // Try to allocate space by expanding the heap.
  virtual HeapWord* expand_heap_and_allocate(size_t size, bool is_tlab);

  // Compute max heap alignment
  size_t compute_max_alignment();

 // Scale the base_size by NewRatio according to
 //     result = base_size / (NewRatio + 1)
 // and align by min_alignment()
 size_t scale_by_NewRatio_aligned(size_t base_size);

 // Bound the value by the given maximum minus the min_alignment
 size_t bound_minus_alignment(size_t desired_size, size_t maximum_size);

 public:
  GenCollectorPolicy();

  // Accessors
  size_t min_gen0_size()     { return _min_gen0_size; }
  size_t initial_gen0_size() { return _initial_gen0_size; }
  size_t max_gen0_size()     { return _max_gen0_size; }
  size_t gen_alignment()     { return _gen_alignment; }

  virtual int number_of_generations() = 0;

  virtual GenerationSpec **generations() {
    assert(_generations != NULL, "Sanity check");
    return _generations;
  }

  virtual GenCollectorPolicy* as_generation_policy() { return this; }

  virtual void initialize_generations() { };

  virtual void initialize_all() {
    CollectorPolicy::initialize_all();
    // initialize_generations()根据用户参数，配置各内存代的管理器。
    initialize_generations();
  }

  size_t young_gen_size_lower_bound();

  HeapWord* mem_allocate_work(size_t size,
                              bool is_tlab,
                              bool* gc_overhead_limit_was_exceeded);

  HeapWord *satisfy_failed_allocation(size_t size, bool is_tlab);

  // Adaptive size policy
  virtual void initialize_size_policy(size_t init_eden_size,
                                      size_t init_promo_size,
                                      size_t init_survivor_size);

  virtual void post_heap_initialize() {
    assert(_max_gen0_size == MaxNewSize, "Should be taken care of by initialize_size_info");
  }
};

// All of hotspot's current collectors are subtypes of this
// class. Currently, these collectors all use the same gen[0],
// but have different gen[1] types. If we add another subtype
// of CollectorPolicy, this class should be broken out into
// its own file.

// TwoGenerationCollectorPolicy继承自GenCollectorPolicy，其定义也在collectorPolicy.hpp中，
// 表示一个只有两个Generation的CollectorPolicy，现有的GenCollectedHeap的所有子类都是只有两个Generation，
// 第一个Generation相同，对应GenCollectorPolicy中新增的gen0的相关属性，第二个Generation的实现各不相同。
class TwoGenerationCollectorPolicy : public GenCollectorPolicy {
 protected:
  size_t _min_gen1_size;
  size_t _initial_gen1_size;
  size_t _max_gen1_size;

  void initialize_flags();
  void initialize_size_info();

  DEBUG_ONLY(void assert_flags();)
  DEBUG_ONLY(void assert_size_info();)

 public:
  TwoGenerationCollectorPolicy() : GenCollectorPolicy(), _min_gen1_size(0),
    _initial_gen1_size(0), _max_gen1_size(0) {}

  // Accessors
  size_t min_gen1_size()     { return _min_gen1_size; }
  size_t initial_gen1_size() { return _initial_gen1_size; }
  size_t max_gen1_size()     { return _max_gen1_size; }

  // Inherited methods
  TwoGenerationCollectorPolicy* as_two_generation_policy() { return this; }

  int number_of_generations()          { return 2; }
  BarrierSet::Name barrier_set_name()  { return BarrierSet::CardTableModRef; }

  virtual CollectorPolicy::Name kind() {
    return CollectorPolicy::TwoGenerationCollectorPolicyKind;
  }

  // Returns true if gen0 sizes were adjusted
  bool adjust_gen0_sizes(size_t* gen0_size_ptr, size_t* gen1_size_ptr,
                         const size_t heap_size);
};

class MarkSweepPolicy : public TwoGenerationCollectorPolicy {
 protected:
  void initialize_alignments();
  // initialize_generations()根据用户参数，配置各内存代的管理器。
  void initialize_generations();

 public:
  MarkSweepPolicy() {}

  MarkSweepPolicy* as_mark_sweep_policy() { return this; }

  void initialize_gc_policy_counters();
};

#endif // SHARE_VM_MEMORY_COLLECTORPOLICY_HPP
