/*
    Copyright © 2011-13 Qtrac Ltd. All rights reserved.
    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 2 of
    the License, or (at your option) any later version. This program is
    distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
    for more details.
*/

#include "sequence_matcher.hpp"
#include <QSet>


RangesPair computeRanges(SequenceMatcher *matcher)
{
    Ranges ranges1;
    Ranges ranges2;
    QList<Match> matches = matcher->get_matching_blocks();
    foreach (const Match &match, matches) {
        if (match.size == 0)
            continue;
        ranges1 |= unorderedRange(match.i + match.size, match.i);
        ranges2 |= unorderedRange(match.j + match.size, match.j);
    }
    return qMakePair(ranges1, ranges2);
}


RangesPair invertRanges(const Ranges &ranges1, int length1,
                        const Ranges &ranges2, int length2)
{
    const Ranges newRanges1 = unorderedRange(length1) - ranges1;
    const Ranges newRanges2 = unorderedRange(length2) - ranges2;
    return qMakePair(newRanges1, newRanges2);
}


bool matchLessThan(const Match &a, const Match &b)
{
    if (a.i != b.i)
        return a.i < b.i;
    if (a.j != b.j)
        return a.j < b.j;
    return a.size < b.size;
}


struct Offsets
{
    Offsets(int a_low_=0, int a_high_=0, int b_low_=0, int b_high_=0)
        : a_low(a_low_), a_high(a_high_), b_low(b_low_), b_high(b_high_) {}

    int a_low;
    int a_high;
    int b_low;
    int b_high;
};


SequenceMatcher::SequenceMatcher(const Sequence &a_, const Sequence &b_)
    : a(a_), b(b_)
{
    set_sequences(a, b);
}


void SequenceMatcher::set_sequence1(const Sequence &sequence)
{
    a = sequence;
    matching_blocks.clear();
}


void SequenceMatcher::set_sequence2(const Sequence &sequence)
{
    b = sequence;
    matching_blocks.clear();
    chain_b();
}


void SequenceMatcher::chain_b()
{
    const int N = b.count();
    b2j.clear();
    QSet<Element> popular;

    for (int i = 0; i < N; ++i) {
        const Element &element = b.at(i);
        if (b2j.contains(element)) {
            QList<int> &indexes = b2j[element];
            if (N >= 200 && indexes.count() * 100 > N) {
                popular.insert(element);
                indexes.clear();
            }
            else
                indexes.append(i);
        }
        else
            b2j[element].append(i);
    }

    foreach (const Element &element, popular)
        b2j.remove(element);
}


QList<Match> SequenceMatcher::get_matching_blocks()
{
    if (!matching_blocks.isEmpty())
        return matching_blocks;

    const int LengthA = a.count();
    const int LengthB = b.count();
    QList<Offsets> offsets;
    offsets << Offsets(0, LengthA, 0, LengthB);
    while (!offsets.isEmpty()) {
        const Offsets &offset = offsets.takeLast();
        const int a_low = offset.a_low;
        const int a_high = offset.a_high;
        const int b_low = offset.b_low;
        const int b_high = offset.b_high;
        const Match match = find_longest_match(a_low, a_high, b_low,
                                               b_high);
        const int i = match.i;
        const int j = match.j;
        const int k = match.size;
        if (k) {
            matching_blocks.append(match);
            if (a_low < i && b_low < j)
                offsets.append(Offsets(a_low, i, b_low, j));
            if (i + k < a_high && j + k < b_high)
                offsets.append(Offsets(i + k, a_high, j + k, b_high));
        }
    }
    qSort(matching_blocks.begin(), matching_blocks.end(), matchLessThan);

    int i1 = 0;
    int j1 = 0;
    int k1 = 0;
    QList<Match> non_adjacent;
    foreach (const Match match, matching_blocks) {
        const int i2 = match.i;
        const int j2 = match.j;
        const int k2 = match.size;
        if (i1 + k1 == i2 && j1 + k1 == j2)
            k1 += k2;
        else {
            if (k1)
                non_adjacent.append(Match(i1, j1, k1));
            i1 = i2;
            j1 = j2;
            k1 = k2;
        }
    }
    if (k1)
        non_adjacent.append(Match(i1, j1, k1));
    non_adjacent.append(Match(LengthA, LengthB, 0));
    matching_blocks = non_adjacent;
    return matching_blocks;
}


Match SequenceMatcher::find_longest_match(int a_low, int a_high,
                                          int b_low, int b_high)
{
    int best_i = a_low;
    int best_j = b_low;
    int best_size = 0;
    QHash<int, int> j2len;
    for (int i = a_low; i < a_high; ++i) {
        QHash<int, int> newj2len;
        foreach (int j, b2j.value(a[i])) {
            if (j < b_low)
                continue;
            if (j >= b_high)
                break;
            const int k = j2len.value(j - 1, 0) + 1;
            newj2len[j] = k;
            if (k > best_size) {
                best_i = i - k + 1;
                best_j = j - k + 1;
                best_size = k;
            }
        }
        j2len = newj2len;
    }

    while (best_i > a_low && best_j > b_low &&
           a[best_i - 1] == b[best_j - 1]) {
        --best_i;
        --best_j;
        ++best_size;
    }
    while (best_i + best_size < a_high && best_j + best_size < b_high &&
           a[best_i + best_size] == b[best_j + best_size])
        ++best_size;

    return Match(best_i, best_j, best_size);
}
