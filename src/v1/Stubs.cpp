//
// kbx81's tube clock
// ---------------------------------------------------------------------------
// (c)2017 by kbx81. See LICENSE for details.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>
//

// Minimal terminate handler replacing libstdc++'s __verbose_terminate_handler.
// The default pulls in __cxa_demangle and stdio code -- useless on bare-metal
// where there's no console to print to anyway.
namespace __gnu_cxx {
void __verbose_terminate_handler() {
  while (1) {
  }
}
}  // namespace __gnu_cxx

// Break the virtual-destructor → operator delete → free → malloc chain.
// Every class with a virtual destructor embeds a "deleting destructor" in its
// vtable (C++ ABI requirement), so these symbols must exist even though they
// are never actually invoked — all View instances are statically allocated and
// delete is never called on them at runtime.
// Without these stubs: Application.o's vtables reference operator delete(void*,
// unsigned int), which pulls in del_op.o → free → malloc.o → mallocr.o →
// impure.o → findfp.o → stdio.o → sbrk.o, costing ~1,564 bytes of RAM.
void operator delete(void *) noexcept {}
void operator delete[](void *) noexcept {}
void operator delete(void *, unsigned int) noexcept {}
void operator delete[](void *, unsigned int) noexcept {}

// Break the static-constructor → __aeabi_atexit → atexit table chain.
// Static objects with virtual destructors call __aeabi_atexit at startup to
// register their destructors. This pulls in the atexit table (__atexit0,
// 400 bytes BSS), mutex data, and locking primitives — all useless since the
// firmware never exits and destructors are never called.
// __cxa_atexit is stubbed as well to guard against direct calls (e.g. from
// static local variables with non-trivial destructors added in the future).
extern "C" {
  int __aeabi_atexit(void *, void (*)(void *), void *) { return 0; }
  int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
}

// Stub for the C runtime 'finish' routine, pulled in when static C++ objects
// with virtual destructors register via __cxa_atexit / __libc_fini_array.
// The firmware never exits so this is never called.
extern "C" void _fini() {}

// Ensure _sbrk always fails so malloc can never silently consume heap space.
// If any future dependency accidentally calls malloc, it gets nullptr
// immediately rather than growing into the stack-heap gap.
extern "C" void *_sbrk(int) { return reinterpret_cast<void *>(-1); }

// Minimal syscall stubs. These override the weak versions in libg.a that emit
// linker warnings about unimplemented syscalls. The firmware has no OS, so
// all I/O syscalls are no-ops that return an error.
extern "C" {
  int _close(int) { return -1; }
  int _lseek(int, int, int) { return -1; }
  int _read(int, char *, int) { return -1; }
  int _write(int, char *, int) { return -1; }
}
