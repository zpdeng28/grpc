
#ifndef UPBC_MESSAGE_LAYOUT_H
#define UPBC_MESSAGE_LAYOUT_H

#include <unordered_map>
#include "absl/base/macros.h"
#include "google/protobuf/descriptor.h"

namespace upbc {

class MessageLayout {
 public:
  struct Size {
    void Add(const Size& other) {
      size32 += other.size32;
      size64 += other.size64;
    }

    void MaxFrom(const Size& other) {
      size32 = std::max(size32, other.size32);
      size64 = std::max(size64, other.size64);
    }

    void AlignUp(const Size& align) {
      size32 = Align(size32, align.size32);
      size64 = Align(size64, align.size64);
    }

    int64_t size32;
    int64_t size64;
  };

  struct SizeAndAlign {
    Size size;
    Size align;

    void MaxFrom(const SizeAndAlign& other) {
      size.MaxFrom(other.size);
      align.MaxFrom(other.align);
    }
  };

  MessageLayout(const google::protobuf::Descriptor* descriptor) {
    ComputeLayout(descriptor);
  }

  Size GetFieldOffset(const google::protobuf::FieldDescriptor* field) const {
    return GetMapValue(field_offsets_, field);
  }

  Size GetOneofCaseOffset(
      const google::protobuf::OneofDescriptor* oneof) const {
    return GetMapValue(oneof_case_offsets_, oneof);
  }

  int GetHasbitIndex(const google::protobuf::FieldDescriptor* field) const {
    return GetMapValue(hasbit_indexes_, field);
  }

  Size message_size() const { return size_; }

  static bool HasHasbit(const google::protobuf::FieldDescriptor* field);
  static SizeAndAlign SizeOfUnwrapped(
      const google::protobuf::FieldDescriptor* field);

 private:
  void ComputeLayout(const google::protobuf::Descriptor* descriptor);
  void PlaceNonOneofFields(const google::protobuf::Descriptor* descriptor);
  void PlaceOneofFields(const google::protobuf::Descriptor* descriptor);
  Size Place(SizeAndAlign size_and_align);

  template <class K, class V>
  static V GetMapValue(const std::unordered_map<K, V>& map, K key) {
    auto iter = map.find(key);
    if (iter == map.end()) {
      fprintf(stderr, "No value for field.\n");
      abort();
    }
    return iter->second;
  }

  static bool IsPowerOfTwo(size_t val) {
    return (val & (val - 1)) == 0;
  }

  static size_t Align(size_t val, size_t align) {
    ABSL_ASSERT(IsPowerOfTwo(align));
    return (val + align - 1) & ~(align - 1);
  }

  static SizeAndAlign SizeOf(const google::protobuf::FieldDescriptor* field);
  static int64_t FieldLayoutRank(
      const google::protobuf::FieldDescriptor* field);

  std::unordered_map<const google::protobuf::FieldDescriptor*, Size>
      field_offsets_;
  std::unordered_map<const google::protobuf::FieldDescriptor*, int>
      hasbit_indexes_;
  std::unordered_map<const google::protobuf::OneofDescriptor*, Size>
      oneof_case_offsets_;
  Size maxalign_;
  Size size_;
};

}  // namespace upbc

#endif  // UPBC_MESSAGE_LAYOUT_H
