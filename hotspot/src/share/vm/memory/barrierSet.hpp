/*
 * Copyright (c) 2000, 2012, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_MEMORY_BARRIERSET_HPP
#define SHARE_VM_MEMORY_BARRIERSET_HPP

#include "memory/memRegion.hpp"
#include "oops/oopsHierarchy.hpp"

// This class provides the interface between a barrier implementation and
// the rest of the system.

// BarrierSet表示一个数据读写动作的栅栏，跟高速缓存中用来在不同CPU之间同步数据的的Barrier（内存屏障）完全不同，BarrierSet的功能类似于一个拦截器，在读写动作实际作用于内存前执行某些前置或者后置动作，其定义在hotspot src/share/vm/memory/barrierSet.hpp中。
class BarrierSet: public CHeapObj<mtGC> {
  friend class VMStructs;
public:
  // BarrierSet定义了一个枚举Name来描述不同类型的子类
  enum Name {
    ModRef,
    CardTableModRef,
    CardTableExtension,
    G1SATBCT,
    G1SATBCTLogging,
    Other,
    Uninit
  };

  enum Flags {
    None                = 0,
    TargetUninitialized = 1
  };
protected:
  int _max_covered_regions;
  Name _kind;

public:

  BarrierSet() { _kind = Uninit; }
  // To get around prohibition on RTTI.
  BarrierSet::Name kind() { return _kind; }
  virtual bool is_a(BarrierSet::Name bsn) = 0;

  // These operations indicate what kind of barriers the BarrierSet has.
  // 方法名中的ref表示引用类型的数据 prim表示基本类型的数据
  // has_read_ref_barrier表示该BarrierSet是在读取引用类型数据时执行的
  virtual bool has_read_ref_barrier() = 0;
  // has_read_prim_barrier表示该BarrierSet是在读取基本类型数据时执行的
  virtual bool has_read_prim_barrier() = 0;
  // has_write_ref_barrier表示该BarrierSet是在写入引用类型数据时执行的
  virtual bool has_write_ref_barrier() = 0;
  // has_write_ref_pre_barrier表示该BarrierSet是在写入引用类型数据前预先执行的
  virtual bool has_write_ref_pre_barrier() = 0;
  virtual bool has_write_prim_barrier() = 0;

  // These functions indicate whether a particular access of the given
  // kinds requires a barrier.
  virtual bool read_ref_needs_barrier(void* field) = 0;
  virtual bool read_prim_needs_barrier(HeapWord* field, size_t bytes) = 0;
  virtual bool write_prim_needs_barrier(HeapWord* field, size_t bytes,
                                        juint val1, juint val2) = 0;

  // The first four operations provide a direct implementation of the
  // barrier set.  An interpreter loop, for example, could call these
  // directly, as appropriate.

  // Invoke the barrier, if any, necessary when reading the given ref field.
  virtual void read_ref_field(void* field) = 0;

  // Invoke the barrier, if any, necessary when reading the given primitive
  // "field" of "bytes" bytes in "obj".
  virtual void read_prim_field(HeapWord* field, size_t bytes) = 0;

  // Invoke the barrier, if any, necessary when writing "new_val" into the
  // ref field at "offset" in "obj".
  // (For efficiency reasons, this operation is specialized for certain
  // barrier types.  Semantically, it should be thought of as a call to the
  // virtual "_work" function below, which must implement the barrier.)
  // First the pre-write versions...
  template <class T> inline void write_ref_field_pre(T* field, oop new_val);
private:
  // Keep this private so as to catch violations at build time.
  virtual void write_ref_field_pre_work(     void* field, oop new_val) { guarantee(false, "Not needed"); };
protected:
  virtual void write_ref_field_pre_work(      oop* field, oop new_val) {};
  virtual void write_ref_field_pre_work(narrowOop* field, oop new_val) {};
public:

  // ...then the post-write version.
  inline void write_ref_field(void* field, oop new_val);
protected:
  virtual void write_ref_field_work(void* field, oop new_val) = 0;
public:

  // Invoke the barrier, if any, necessary when writing the "bytes"-byte
  // value(s) "val1" (and "val2") into the primitive "field".
  virtual void write_prim_field(HeapWord* field, size_t bytes,
                                juint val1, juint val2) = 0;

  // Operations on arrays, or general regions (e.g., for "clone") may be
  // optimized by some barriers.

  // The first six operations tell whether such an optimization exists for
  // the particular barrier.
  virtual bool has_read_ref_array_opt() = 0;
  virtual bool has_read_prim_array_opt() = 0;
  virtual bool has_write_ref_array_pre_opt() { return true; }
  virtual bool has_write_ref_array_opt() = 0;
  virtual bool has_write_prim_array_opt() = 0;

  virtual bool has_read_region_opt() = 0;
  virtual bool has_write_region_opt() = 0;

  // These operations should assert false unless the correponding operation
  // above returns true.  Otherwise, they should perform an appropriate
  // barrier for an array whose elements are all in the given memory region.
  virtual void read_ref_array(MemRegion mr) = 0;
  virtual void read_prim_array(MemRegion mr) = 0;

  // Below length is the # array elements being written
  virtual void write_ref_array_pre(oop* dst, int length,
                                   bool dest_uninitialized = false) {}
  virtual void write_ref_array_pre(narrowOop* dst, int length,
                                   bool dest_uninitialized = false) {}
  // Below count is the # array elements being written, starting
  // at the address "start", which may not necessarily be HeapWord-aligned
  inline void write_ref_array(HeapWord* start, size_t count);

  // Static versions, suitable for calling from generated code;
  // count is # array elements being written, starting with "start",
  // which may not necessarily be HeapWord-aligned.
  static void static_write_ref_array_pre(HeapWord* start, size_t count);
  static void static_write_ref_array_post(HeapWord* start, size_t count);

protected:
  virtual void write_ref_array_work(MemRegion mr) = 0;
public:
  virtual void write_prim_array(MemRegion mr) = 0;

  virtual void read_region(MemRegion mr) = 0;

  // (For efficiency reasons, this operation is specialized for certain
  // barrier types.  Semantically, it should be thought of as a call to the
  // virtual "_work" function below, which must implement the barrier.)
  inline void write_region(MemRegion mr);
protected:
  virtual void write_region_work(MemRegion mr) = 0;
public:

  // Some barrier sets create tables whose elements correspond to parts of
  // the heap; the CardTableModRefBS is an example.  Such barrier sets will
  // normally reserve space for such tables, and commit parts of the table
  // "covering" parts of the heap that are committed.  The constructor is
  // passed the maximum number of independently committable subregions to
  // be covered, and the "resize_covoered_region" function allows the
  // sub-parts of the heap to inform the barrier set of changes of their
  // sizes.
  BarrierSet(int max_covered_regions) :
    _max_covered_regions(max_covered_regions) {}

  // Inform the BarrierSet that the the covered heap region that starts
  // with "base" has been changed to have the given size (possibly from 0,
  // for initialization.)
  virtual void resize_covered_region(MemRegion new_region) = 0;

  // If the barrier set imposes any alignment restrictions on boundaries
  // within the heap, this function tells whether they are met.
  virtual bool is_aligned(HeapWord* addr) = 0;

  // Print a description of the memory for the barrier set
  virtual void print_on(outputStream* st) const = 0;
};

#endif // SHARE_VM_MEMORY_BARRIERSET_HPP
