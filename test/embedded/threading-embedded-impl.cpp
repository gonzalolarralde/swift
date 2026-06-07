// RUN: %empty-directory(%t)
// RUN: %target-clang -x c++ -std=c++17 -fsyntax-only -DSWIFT_THREADING_EMBEDDED -I %swift_obj_root/include %s

// REQUIRES: swift_in_compiler
// REQUIRES: swift_feature_Embedded
// REQUIRES: swift_embedded_platform

#include <cstdint>

#include "swift/Threading/Impl.h"

namespace {

void exerciseEmbeddedThreading() {
  swift::threading_impl::mutex_handle mutex{};
  swift::threading_impl::mutex_init(mutex);
  swift::threading_impl::mutex_lock(mutex);
  swift::threading_impl::mutex_unlock(mutex);

  swift::threading_impl::cond_handle condition{};
  swift::threading_impl::cond_init(condition);
  swift::threading_impl::cond_broadcast(condition);
  swift::threading_impl::cond_destroy(condition);

  swift::threading_impl::once_t predicate{};
  swift::threading_impl::once_impl(
      predicate,
      [](void *value) {
        reinterpret_cast<std::uint8_t *>(value)[0] = 0;
      },
      nullptr);
}

} // namespace

int main(int, char **) {
  exerciseEmbeddedThreading();
  return 0;
}
