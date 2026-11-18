#pragma once
#include <cstddef>
#include <utility>
#include <vector>

// Собственные обобщённые алгоритмы (не из STL).

template <typename Range, typename Pred>
std::size_t countIf(const Range& r, Pred pred) {
    std::size_t n = 0;
    for (const auto& x : r)
        if (pred(x)) ++n;
    return n;
}

template <typename Range, typename Key>
const typename Range::value_type* maxBy(const Range& r, Key key) {
    const typename Range::value_type* best = nullptr;
    for (const auto& x : r)
        if (!best || key(x) > key(*best)) best = &x;
    return best;
}

template <typename Range, typename Key>
const typename Range::value_type* minBy(const Range& r, Key key) {
    const typename Range::value_type* best = nullptr;
    for (const auto& x : r)
        if (!best || key(x) < key(*best)) best = &x;
    return best;
}

namespace detail {
template <typename T, typename Less>
void mergeSortImpl(std::vector<T>& v, std::vector<T>& tmp, std::size_t lo, std::size_t hi, Less& less) {
    if (hi - lo < 2) return;
    const std::size_t mid = lo + (hi - lo) / 2;
    mergeSortImpl(v, tmp, lo, mid, less);
    mergeSortImpl(v, tmp, mid, hi, less);
    std::size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) tmp[k++] = less(v[j], v[i]) ? std::move(v[j++]) : std::move(v[i++]);
    while (i < mid) tmp[k++] = std::move(v[i++]);
    while (j < hi) tmp[k++] = std::move(v[j++]);
    for (k = lo; k < hi; ++k) v[k] = std::move(tmp[k]);
}
}  // namespace detail

// Устойчивая сортировка слиянием, O(n log n).
template <typename T, typename Less>
void mergeSort(std::vector<T>& v, Less less) {
    if (v.size() < 2) return;
    std::vector<T> tmp(v.size());
    detail::mergeSortImpl(v, tmp, 0, v.size(), less);
}
