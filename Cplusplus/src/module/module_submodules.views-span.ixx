// module submodules
export module views.span;
import std;

export namespace views {
// Implicitly includes one '\0'
std::span<const char> hello_world_span() { return "hello world-span"; }
}  // namespace views