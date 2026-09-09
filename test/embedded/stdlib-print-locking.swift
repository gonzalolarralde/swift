// RUN: %empty-directory(%t)
// RUN: split-file %s %t
// RUN: %target-clang -c %t/helper.c -pthread -o %t/helper.o
// RUN: %target-swift-frontend %t/main.swift -parse-as-library -wmo -enable-experimental-feature Embedded -import-bridging-header %t/helper.h -c -o %t/main.o
// RUN: %target-embedded-link %target-clang-resource-dir-opt %t/main.o %t/helper.o -pthread -o %t/a.out
// RUN: %target-run %t/a.out > %t/output.txt
// RUN: %{python} %t/check.py %t/output.txt

// REQUIRES: executable_test
// REQUIRES: optimized_stdlib
// REQUIRES: OS=macosx || OS=linux-gnu
// REQUIRES: swift_feature_Embedded

//--- helper.h
void assert_stdout_locked(void);
void run_printing_tests(void (*print_body)(void));

//--- main.swift
struct Described: CustomStringConvertible {
  var description: String {
    assert_stdout_locked()
    print("nested")
    return "described"
  }
}

func printMessages() {
  print("static", terminator: "!\n")
  let string = String(repeating: "x", count: 8)
  print(string, terminator: "?\n")
  print(-123456, terminator: "!\n")
  print(true)
  print(Described())
}

@main
struct Main {
  static func main() {
    run_printing_tests(printMessages)
  }
}

//--- helper.c
#undef NDEBUG
#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

static pthread_mutex_t probe_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t probe_condition = PTHREAD_COND_INITIALIZER;
static int request;
static int stream_was_locked;
static int probe_enabled;
static void (*print_body)(void);

static void *observe_stdout(void *unused) {
  assert(pthread_mutex_lock(&probe_mutex) == 0);
  for (;;) {
    while (request == 0)
      assert(pthread_cond_wait(&probe_condition, &probe_mutex) == 0);
    if (request == 2)
      break;
    stream_was_locked = ftrylockfile(stdout) != 0;
    if (!stream_was_locked)
      funlockfile(stdout);
    request = 0;
    assert(pthread_cond_broadcast(&probe_condition) == 0);
  }
  assert(pthread_mutex_unlock(&probe_mutex) == 0);
  return NULL;
}

void assert_stdout_locked(void) {
  if (!probe_enabled)
    return;
  assert(pthread_mutex_lock(&probe_mutex) == 0);
  request = 1;
  assert(pthread_cond_broadcast(&probe_condition) == 0);
  while (request != 0)
    assert(pthread_cond_wait(&probe_condition, &probe_mutex) == 0);
  int locked = stream_was_locked;
  assert(pthread_mutex_unlock(&probe_mutex) == 0);
  assert(locked && "print must hold the stdout lock while formatting and writing");
}

// Inspect every character, including the terminator, before fputc takes its
// own recursive lock. Yielding also exercises interleaving in the stress run.
int putchar(int character) {
  assert_stdout_locked();
  int result = fputc(character, stdout);
  sched_yield();
  return result;
}

static void *print_many_times(void *unused) {
  for (int i = 0; i < 50; ++i)
    print_body();
  return NULL;
}

void run_printing_tests(void (*body)(void)) {
  alarm(30);
  print_body = body;
  probe_enabled = 1;
  pthread_t observer;
  assert(pthread_create(&observer, NULL, observe_stdout, NULL) == 0);
  print_body();
  probe_enabled = 0;
  assert(pthread_mutex_lock(&probe_mutex) == 0);
  request = 2;
  assert(pthread_cond_broadcast(&probe_condition) == 0);
  assert(pthread_mutex_unlock(&probe_mutex) == 0);
  assert(pthread_join(observer, NULL) == 0);

  pthread_t threads[4];
  for (int i = 0; i < 4; ++i)
    assert(pthread_create(&threads[i], NULL, print_many_times, NULL) == 0);
  for (int i = 0; i < 4; ++i)
    assert(pthread_join(threads[i], NULL) == 0);
  alarm(0);
}

//--- check.py
from collections import Counter
import sys

with open(sys.argv[1]) as output:
    lines = output.read().splitlines()

expected = Counter({line: 201 for line in (
    "static!", "xxxxxxxx?", "-123456!", "true", "nested", "described"
)})
assert Counter(lines) == expected, Counter(lines)
for index, line in enumerate(lines):
    if line == "nested":
        assert lines[index + 1] == "described", lines[index:index + 2]
