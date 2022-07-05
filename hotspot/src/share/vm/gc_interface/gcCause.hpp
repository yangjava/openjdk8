/*
 * Copyright (c) 2002, 2012, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_INTERFACE_GCCAUSE_HPP
#define SHARE_VM_GC_INTERFACE_GCCAUSE_HPP

#include "memory/allocation.hpp"

//
// This class exposes implementation details of the various
// collector(s), and we need to be very careful with it. If
// use of this class grows, we should split it into public
// and implemenation-private "causes".
//

// GCCause的定义在同目录下的gcCause.hpp中，表示导致GC的原因
class GCCause : public AllStatic {
 public:
  // GCCause专门定义了一个枚举表示所有可能的原因
  enum Cause {
    /* public */
    _java_lang_system_gc,
    _full_gc_alot,
    _scavenge_alot,
    _allocation_profiler,
    _jvmti_force_gc,
    _gc_locker,
    _heap_inspection,
    _heap_dump,

    /* implementation independent, but reserved for GC use */
    _no_gc,
    _no_cause_specified,
    _allocation_failure,

    /* implementation specific */

    _tenured_generation_full,
    _metadata_GC_threshold,

    _cms_generation_full,
    _cms_initial_mark,
    _cms_final_remark,
    _cms_concurrent_mark,

    _old_generation_expanded_on_last_scavenge,
    _old_generation_too_full_to_scavenge,
    _adaptive_size_policy,

    _g1_inc_collection_pause,
    _g1_humongous_allocation,

    _last_ditch_collection,
    _last_gc_cause
  };

  inline static bool is_user_requested_gc(GCCause::Cause cause) {
    return (cause == GCCause::_java_lang_system_gc ||
            cause == GCCause::_jvmti_force_gc);
  }

  inline static bool is_serviceability_requested_gc(GCCause::Cause
                                                             cause) {
    return (cause == GCCause::_jvmti_force_gc ||
            cause == GCCause::_heap_inspection ||
            cause == GCCause::_heap_dump);
  }

  // Return a string describing the GCCause.
  // to_string(GCCause::Cause cause)方法用来返回给定Cause的描述字符串
  static const char* to_string(GCCause::Cause cause);
};

// Helper class for doing logging that includes the GC Cause
// as a string.
// GCCauseString是一个工具类，用来打印包含GCCause::Cause的日志
class GCCauseString : StackObj {
 private:
   static const int _length = 128;
   char _buffer[_length];
   int _position;

 public:
   GCCauseString(const char* prefix, GCCause::Cause cause) {
    // PrintGCCause表示是否打印GC原因，默认为tru
     if (PrintGCCause) {
      // jio_snprintf返回char数组中被占用的元素的个数，_buffer是起始空闲位置，_length是空闲的元素个数
      _position = jio_snprintf(_buffer, _length, "%s (%s) ", prefix, GCCause::to_string(cause));
     } else {
      _position = jio_snprintf(_buffer, _length, "%s ", prefix);
     }
     assert(_position >= 0 && _position <= _length,
       err_msg("Need to increase the buffer size in GCCauseString? %d", _position));
   }
   
   // 将str插入到_buffer中已有的字符串的后面
   GCCauseString& append(const char* str) {
     int res = jio_snprintf(_buffer + _position, _length - _position, "%s", str);
     _position += res;
     assert(res >= 0 && _position <= _length,
       err_msg("Need to increase the buffer size in GCCauseString? %d", res));
     return *this;
   }
   
   // 运算符重载，将其强转成const char*时返回_buffer
   operator const char*() {
     return _buffer;
   }
};

#endif // SHARE_VM_GC_INTERFACE_GCCAUSE_HPP
