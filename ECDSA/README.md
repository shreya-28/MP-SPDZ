This directory contains the code used for the benchmarks by [Dalskov
et al.](https://eprint.iacr.org/2019/889) `*-ecdsa-party.cpp`
contains the high-level programs while the two phases are implemented
`preprocessing.hpp` and `sign.hpp`, respectively.

#### Compilation

- Add `MOD = -DGFP_MOD_SZ=4` to `CONFIG.mine`
- Also consider adding either `CXX = clang++` or `OPTIM = -O2` because GCC 8 or later with `-O3` will produce a segfault when using `mascot-ecdsa-party.x`
- For older hardware, also add `ARCH = -march=native`
- Install [Crypto++](https://www.cryptopp.com) (`libcrypto++-dev` on Ubuntu). We used version 5.6.4, which is the default on Ubuntu 18.04.
- Compile the binaries: `make -j8 ecdsa`
- Or compile the static binaries: `make -j8 ecdsa-static`

#### Running

The following binaries have been used for the paper:

| Protocol | Binary |
| --- | --- |
| MASCOT | `mascot-ecdsa-party.x` |
| Semi-honest OT | `semi-ecdsa-party.x` |
| Malicious Shamir | `mal-shamir-ecdsa-party.x` |
| Semi-honest Shamir | `shamir-ecdsa-party.x` |
| Malicious replicated | `mal-rep-ecdsa-party.x` |
| Semi-honest replicated | `rep-ecdsa-party.x` |

All binaries offer the same interface. With MASCOT for example, run
the following:
```
./mascot-ecsda-party.x -p 0 [-N <number of parties>] [-h <host of party 0>] [-D] [<number of prep tuples>]
./mascot-ecsda-party.x -p 1 [-N <number of parties>] [-h <host of party 0>] [-D] [<number of prep tuples>]
...
```

`-D` activates delayed multiplication, deferring usage of the secret
key until signing.

The number of parties defaults to 2 for OT-based protocols and to 3
for honest-majority protocols.

In addition, there is `fake-spdz-ecsda-party.x`, which runs only the
online phase of SPDZ. You will need to run `Fake-ECDSA.x` beforehands
and then distribute `Player-Data/ECSDA` to all parties.
