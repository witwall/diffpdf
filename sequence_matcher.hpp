#ifndef SEQUENCE_MATCHER_HPP
#define SEQUENCE_MATCHER_HPP
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

#include "generic.hpp"
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>

typedef QStringList Sequence;
typedef QString Element;

class SequenceMatcher;

RangesPair computeRanges(SequenceMatcher *matcher);
RangesPair invertRanges(const Ranges &ranges1, int length1,
                        const Ranges &ranges2, int length2);

struct Match
{
    Match(int i_=0, int j_=0, int size_=0) : i(i_), j(j_), size(size_) {}

    int i;
    int j;
    int size;
};


// A simplified C++ implementation of Python's difflib's SequenceMatcher
class SequenceMatcher
{
public:
    SequenceMatcher(const Sequence &a_=Sequence(),
                    const Sequence &b_=Sequence());

    void set_sequences(const Sequence &a, const Sequence &b)
        { set_sequence1(a); set_sequence2(b); }
    void set_sequence1(const Sequence &sequence);
    void set_sequence2(const Sequence &sequence);

    QList<Match> get_matching_blocks();
    Match find_longest_match(int a_low, int a_high, int b_low, int b_high);

private:
    void chain_b();

    Sequence a;
    Sequence b;
    QHash<Element, QList<int> > b2j;
    QList<Match> matching_blocks;
};

#endif // SEQUENCE_MATCHER_HPP
