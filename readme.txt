libghetto - A no-frills TIFF file reading library
=================================================

1. Introduction
Many camera raw image formats are derived from TIFF 6.0 or TIFF/EP with EXIF
tags. As a part of an ongoing effort to provide high-speed, low-overhead and
freely-licensed software to access this type of imagery, libghetto was born.
The goal of libghetto was to provide a relatively clean set of interfaces to
read and manipulate TIFF imagery and related data structures, without
introducing too much complexity to the process of supporting a particular
RAW camera format.

2. Distribution
Most users will just need libghetto.so dropped somewhere the dynamic linker
can find it. On Windows (when I get around to making it work on Windows),
ghetto.dll should be somewhere in the library search path.

Developers are going to need ghetto.h and ghetto_fp.h ONLY. The ghetto_priv.h
header should never be used outside of libghetto's code itself. ghetto.h is
never going to change, and ghetto_fp.h is seldom going to change, but all
bets are off for functions, macros and preprocessor defines in the private
header.

3. Developers, Developers, Developers
Some design notes that might help programmers using libghetto:

- The "fp" infrastructure allows replacing the I/O handles used underneath
  libghetto for file access. This should relatively transparently allow the
  use of non-standard I/O functions (such as reading a ZIPped TIFF image).
  Of course, this is all application-level logic. If you simply call
  tiff_open, libghetto will default to using the stdio I/O functions. To
  see how to implement your own I/O function handlers, check out the file
  ghetto_fp.c. To use your own I/O function handlers, call tiff_open_ex
  with a pointer to your I/O handlers structure.

- The "ifd" structure is only loosely tied to the actual file pointer. The
  file pointer is largely used for accessing data associated with a tag.
  The tiff_ifd_t structure is strictly entirely in memory, so traversing an
  IFD without fetching data is typically a fairly efficient process.

- libghetto is designed to minimize excessive seeks and reads. Structures
  and data associated with tags should be read in once at most, typically.
  If you're finding you need to re-read some data during the use of
  libghetto, either you're doing something wrong or you've found a flaw in
  the API. Either way, drop me a line and we can see what is going on.

4. Licence
A two-clause BSD variant:
  Copyright (c) 2011, Phil Vachon <phil@cowpig.ca>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  - Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

  - Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

5. Questions? Comments? Bugfixes? Bug reports? 
Contact the author, Phil Vachon at phil@cowpig.ca.
