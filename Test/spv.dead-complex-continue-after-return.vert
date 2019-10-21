#version 450

layout(location =0 ) in int c;
layout(location =0 ) out int o;

void main() {
  int i = 0;
  o = 1;
  // This has non-trivial continue target.
  for (i=0; i < 5; ++i, o=99) {
    o = 2;
    return;
    o = 3;
  }
  // This is considered reachable since we don't assume
  // the compiler is smart enough to deduce that the loop body
  // is always entered.
  o = 4;
}
