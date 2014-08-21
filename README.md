Rainbow
=======

Yet another rainbow table generation and search tool software.

Rainbow tables
--------------

We consider the problem of finding back a value from its hash (in the
current implementation, MD5). Formally, we want to perform a preimage
attack. Since hash functions are designed to be resistant to preimage
attacks, we are often reduced to brute forcing: consider every single
possible value, hash it and match it against the target hash. This is
guaranteed to work but can take tremendeous amounts of time.

For feasible targets, we can speed up the process by storing the result
of all these hashes in a (sorted) table. That way, whenever a new hash
comes in, we simply have to look it up in the table. However, lookup
tables tend to be huge and impractical.

    C -> 12b2
    B -> 3e4f
    A -> 467f
    D -> 8801

Here, the hash 0e4f would be easily mapped to the value B.

The root idea of rainbow tables is to find a middle point between brute
force cracking and lookup tables. Instead of storing every value/hash
couples, they are grouped in "chains" each identified by one initial
value and one final hash. Basically, the hash of the initial value
is mapped to a new value, which is hashed in turn, mapped, and so on,
a fixed number of times, until a final hash.

    A -> 467f -> D -> 8801 -> H -> 6939
    B -> 3e4f -> C -> 12b2 -> A -> 3e4f

In the example above, we choose to map hashes to values by taking the
first character of the hash and taking the corresponding letter of
the alphabet (e.g. `7 -> G`). Now, notice that we can freely choose the
initial values but the middle are already determined; some may not appear
(e.g. E) and some may appear twice (e.g. A). However, we do control the
mapping function so that we can optimize the repartition.

In this case, all that is actually stored is:

    A --> 6939
    B --> 3e4f

Now, if we are given some hash target (e.g. 12b2), we apply the map-hash
process until we find a matching value in the table and consider the chain
it belongs to. We then follow the chain until the value. In the example:

    12b2 -> A -> 3e4f

From the rainbow table, 3e4f corresponds to the chain beginning with B:

    B -> 3e4f -> C

The mapping operation used in rainbow tables is called reduction.

Parameters
----------

From what we have done, when manipulating a rainbow table, we have to
consider the following parameters:

* key space: this is the set of all the values we may have to consider (e.g. 8 characters words)
* chain length: this is the number of time we proceed to reduction-hashing from an initial value
* table size: the number of chains in the table
* reduction seed: to better cover the key space while avoiding duplicates,
  it is best to generate several rainbow tables with different reduction
  functions; the reduction seed is a number which basically sets a
  reduction function
* part number: rainbow tables are way smaller than lookup tables but
  still take quite some space; some systems do not handle large files
  properly so it is usual to split rainbow tables in several parts

The keyspace is usually defined by the number and the set of characters
(charset) that makes a value. The charset is hardcoded in this program
but can be changed easily.

Examples
--------

Generate a single (weak) rainbow table for 6 character words:

    $ ./rtgen 6 0 1000 500000 1 0 alpha4.rt

Attempt to crack a value with this table:

    $ echo -n 6c02ec | md5sum                                      
    750f4b11bbd880f9fb9bcd0c24b7b473  -
    $ ./rtcrack -x 750f4b11bbd880f9fb9bcd0c24b7b473 alpha4.rt
    750f4b11bbd880f9fb9bcd0c24b7b473 6c02ec

You can test how efficient a table is by cracking for random values:

    $ ./rtcrack -r 1000 alpha4.rt 
    204 / 1000

A bash script makes it easy to generate rainbow tables for several
reduction seeds and in several parts:

    $ ./gen.sh 6 1000 1000000 0 50 1
    $ ./rtcrack -r 1000 rt/alnum_6_1000_1000000_1/*
    992 / 1000

Licence
-------

This program is distributed under the GPL licence (see
[LICENCE.md](LICENCE.md) file). The credits for markdown formatting goes
to https://github.com/IQAndreas/markdown-licenses
