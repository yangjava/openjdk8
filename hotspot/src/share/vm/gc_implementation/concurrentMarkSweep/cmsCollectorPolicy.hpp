/*
 * Copyright (c) 2007, 2010, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_IMPLEMENTATION_CONCURRENTMARKSWEEP_CMSCOLLECTORPOLICY_HPP
#define SHARE_VM_GC_IMPLEMENTATION_CONCURRENTMARKSWEEP_CMSCOLLECTORPOLICY_HPP

#include "memory/collectorPolicy.hpp"
// ConcurrentMarkSweepPolicy在hotspot/src/share/vm/gc_implementation/concurrentMarkSweep/cmsCollectorPolicy.hpp中定义的，
// 是UseConcMarkSweepGC为true但是UseAdaptiveSizePolicy为false时使用的CollectorPolicy实现。该类没有添加新的属性，重点关注initialize_alignments 和 initialize_generations方法的实现。
// 这两方法在父类中都没有提供有效实现，两个都是在GenCollectorPolicy::initialize_all方法中调用的，先调用initialize_alignments，最后调用initialize_generations。
class ConcurrentMarkSweepPolicy : public TwoGenerationCollectorPolicy {
 protected:
  void initialize_alignments();
  void initialize_generations();

 public:
  ConcurrentMarkSweepPolicy() {}

  ConcurrentMarkSweepPolicy* as_concurrent_mark_sweep_policy() { return this; }

  void initialize_gc_policy_counters();

  virtual void initialize_size_policy(size_t init_eden_size,
                                      size_t init_promo_size,
                                      size_t init_survivor_size);

  // Returns true if the incremental mode is enabled.
  virtual bool has_soft_ended_eden();
};

class ASConcurrentMarkSweepPolicy : public ConcurrentMarkSweepPolicy {
 public:

  // Initialize the jstat counters.  This method requires a
  // size policy.  The size policy is expected to be created
  // after the generations are fully initialized so the
  // initialization of the counters need to be done post
  // the initialization of the generations.
  void initialize_gc_policy_counters();

  virtual CollectorPolicy::Name kind() {
    return CollectorPolicy::ASConcurrentMarkSweepPolicyKind;
  }
};

#endif // SHARE_VM_GC_IMPLEMENTATION_CONCURRENTMARKSWEEP_CMSCOLLECTORPOLICY_HPP
