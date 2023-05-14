#include "Sustained.h"
template <>
uint64_t Sustained<true>::s_minLength = 64;

template <>
uint64_t Sustained<false>::s_minLength = 1;
