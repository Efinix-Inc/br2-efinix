
                     C-Ray Simple Raytracing Tests

           http://www.futuretech.blinkenlights.nl/c-ray.html

              By: John Tsiombikas <nuclear@siggraph.org>

       Test suite compiled by: Ian Mapleson <mapesdhs@yahoo.com>

                       Last Change: 10/Apr/2008


1. Introduction
2. The C-Ray Tests (how to compile)
3. Running the Tests
4. Submitting Results
5. Background
6. Appendix A: Invalid Tests

**********************************************************************

1. Introduction

C-Ray is a simple raytracer written by John Tsiombikas; in his
own words, it is:

  "...an extremely small program I did one day to figure out how
  would the simplest raytracer program look like in the least
  ammount of code lines."

The relevant discussion on Nekochan is at:

  http://forums.nekochan.net/viewtopic.php?f=3&t=15719

The default data set is very small, so C-ray really only tests the pure
floating-point (fp) speed of a CPU core (or multiple CPUs/cores using the
threaded version), ie. RAM speed is not a significant factor. John said:

  This thing only measures 'floating point CPU performance' and
  nothing more, and it's good that nothing else affects the results.
  A real rendering program/scene would be still CPU-limited meaning
  that by far the major part of the time spent would be CPU time in
  the fpu, but it would have more overhead for disk I/O, shader
  parsing, more strain for the memory bandwidth, and various other
  things. So it's a good approximation being a renderer itself, but
  it's definitely not representative."

Nevertheless, the results are certainly interesting:

  http://www.futuretech.blinkenlights.nl/c-ray.html

If you wish to submit your own results, follow the instructions given
below. Send the data to me, not to John, and I will add them to the
relevant tables; please include all requested details.

Comments and feedback welcome!

Ian.

mapesdhs@yahoo.com
sgidepot@blueyonder.co.uk
http://www.futuretech.blinkenlights.nl/sgidepot/

**********************************************************************

2. The C-Ray Tests

Two programs are included in this archive for testing:

c-ray-f:

  This is for single-CPU systems, or for testing just a single core
  of a multi-core CPU.

c-ray-mt:

  This is the multithreaded version for testing multi-CPU systems, and/or
  systems with more than one CPU core. Note that on some systems c-ray-mt
  with just one thread may be faster than c-ray-f. Use whichever version
  gives you the best results in each case. Use the -t option to specify
  the number of threads; without the -t option, only 1 thread is used.

Compile the source files for your target platform with gcc or whatever
compiler you have. Just enter 'make', though feel free to add any arch-
specific optimizations for your compiler in CFLAGS first. By default,
the Makefile is designed for use with GCC. If you are using an SGI and
want to use MIPS Pro to compile the programs, then enter:

  /bin/cp Makefile.mips Makefile

and then enter 'make'.

Note that the c-ray binaries as supplied were compiled for an SGI
Octane2 R12K/R14 system (users of other SGI models should recompile
if possible), while the example x86 binary in the x86 directory was
compiled by John for a 3GHz P4.

If you don't want to use make, then typical compile lines for each
program on SGIs using MIPS Pro are as follows (in this case for an
Octane system - use a different IP number for other SGI systems):

  cc -O3 -mips4 -TARG:platform=ip30 -Ofast=ip30 c-ray-f.c -o c-ray-f -lm

while for the threaded version the pthread library must be included:

  cc -O3 -mips4 -TARG:platform=ip30 -Ofast=ip30 c-ray-mt.c -o c-ray-mt -lm -lpthread

See the 'cc' man page for full details of available optimisation options.

The file 'sgi.txt' has further example compile lines for SGI O2 and
Octane machines, some with extra example optimisation options. Try
them out, see which one works best on your system. Those using GCC
should consult the gcc man page for full details of available options.

NOTE: results for tests done with pre-run profiling/virtualisation
compiler optimisations will NOT be accepted! (see Appendix A for details).

Before running the tests, naturally you should shut down any other
applications, processes, etc. which might interfere with the test.
For example, on SGI systems, I shut down the mediad and sgi_apache
daemons:

  /etc/init.d/mediad stop
  /etc/init.d/sgi_apache stop

Better still, turn off timed, nsd, and rlogin remotely to run the tests.
It should be possible to do the same thing on a Linux/BSD system.

On a Windows machine, shut down all unnecessary processes, close any
antivirus/firewall applications/processes, and it may be worthwhile
forcing any pending idle tasks to complete before running the tests,
ie. select Run from the Start menu and enter:

  Rundll32.exe advapi32.dll,ProcessIdleTasks

Assuming you are now ready to use the binary programs for the tests...

**********************************************************************

3. Running the Tests

There are two data files used for the tests:

'scene' is a simple environment, with just three reflective spheres.
Examine scene.jpg to see the final image.

'sphract' is a much more complex scenario, with dozens of spheres in
a fractal pattern (see gen_fract.txt for details of how the scene
description was created). Examine sphract.jpg to see the final image.

There are four tests; the first is the shortest, the data from which
are used for the main table on the results page. The tests are:


  Test   Data File    Image Resolution     Oversampling

   1.    scene        Default 800x600      NONE
   2.    sphract      Default 800x600      NONE
   3.    sphract      1024 x 768           8X
   4.    scene        7500 x 3500          NONE

If you are using a single-CPU system which only has one core, or wish
to test just one CPU/core of a multi-CPU/core system, then run the
tests with c-ray-f, or with c-ray-mt using just 1 thread. For systems
with multiple CPUs/cores, please submit results for just a single
CPU/core aswell as the fastest results for using all CPUs/cores (this
allows one to see how well parallel systems scale).

On UNIX systems, the programs receive the scene description data from
the standard input and send the results (the image created) to the
standard output. To run the first test on a UNIX system, enter:

  cat scene | ./c-ray-f > foo.ppm

On a Windows system, enter the following in a Command window:

  c-ray-f -i scene -o foo.ppm

The output will resemble the following (in this case run on a R14000
550MHz SGI Octane2):

  Rendering took: 1 seconds (1888 milliseconds)

The result to submit is the number of milliseconds. Run each test
several times if possible to observe a typical result. It's up to
you whether you submit a typical result or the fastest overall result.

If you have a multi-CPU/core system, now run the test multithreaded with
c-ray-mt, using the -t option to specify the number of threads, eg.

     UNIX:   cat scene | ./c-ray-mt -t 32 > foo.ppm
  Windows:   c-ray-mt -t 32 -i scene -o foo.ppm

For multi-CPU/core systems, the optimum number of threads varies
greatly from one system to another, though a good estimate is 16
times the number of cores. Try different numbers, eg. 32, 64, 128,
or some inbetween number such as 40, 60, etc. But also try smaller
number too, eg. just 8 threads for a quad-core system.

The maximum number of threads c-ray-mt can use is the vertical
resolution of the output image. As the number of threads increases,
eventually the speedup obtained by the parallel processing will be
outweighed by the overhead cost of managing the threads. Experiment
to find what works best for each test.

Thus, for the other tests, the commands to enter on a UNIX system
would be as follows, using 32 threads just as an example here:

  cat sphract | ./c-ray-f > foo.ppm
  cat sphract | ./c-ray-mt -t 32 > foo.ppm
  cat sphract | ./c-ray-f -s 1024x768 -r 8 > foo.ppm
  cat sphract | ./c-ray-mt -t 32 -s 1024x768 -r 8 > foo.ppm
  cat scene | ./c-ray-f -s 7500x3500 > foo.ppm
  cat scene | ./c-ray-mt -t 32 -s 7500x3500 > foo.ppm

while on a Windows system these would be:

  c-ray-f -i sphract -o foo.ppm
  c-ray-mt -t 32 -i sphract -o foo.ppm
  c-ray-f -s 1024x768 -r 8 -i sphract -o foo.ppm
  c-ray-mt -t 32 -s 1024x768 -r 8 -i sphract -o foo.ppm
  c-ray-f -s 7500x3500 -i scene -o foo.ppm
  c-ray-mt -t 32 -s 7500x3500 -i scene -o foo.ppm

If you don't want to bother experimenting, then the script RUN.full
will execute all the tests, with the c-ray-mt tests done using 32
threads, but remember 32 threads might not be optimal for your system.

**********************************************************************

4. Submitting Results

Do not send any results to John; instead, send all results to me at
both of my email addresses (include "C-Ray" in the subject line):

  mapesdhs@yahoo.com
  sgidepot@blueyonder.co.uk

Apart from the run-times reported by each test, please remember
to state which tests are multithreaded and how many threads were
used. Better still, just copy the on-screen text for each test.

With respect to system information, state the type of system (name
and/or model number), CPU details (name/model, type/speed/cache,
no. of CPUs/cores), OS name/version (eg. for Linux, what name,
kernel version/build), what compiler was used (name/version) and
what extra options if any were employed. The online results page
also shows the host name where available, but this is optional.

Thus, for example, a system description might look like this:

  SUN Fire X2100, Solaris 10, host name 'kobe'
  Sun Studio 10 Compiler
  AMD Opteron 175 2.2 GHz 1MB L2

while for an SGI it might be:

  SGI Octane2, IRIX 6.5.26m
  MIPS Pro 7.4.3m (7.3 EOE)
  Dual-R12000 400MHz (2MB L2)

or a Windows system (in this case my own setup):

  WinXP-32bit PC (SP2), Asrock AM2NF3 motherboard, 4GB DDR2/800 RAM
  Athlon64 X2 6000+ 3.15GHz (overclocked)
  Supplied x86 binary used (1 core only)

You can also mention the system's RAM, disk, or anything else, but
they're not essential.

Note for SGI users: you can find out what eoe/dev compiler versions
your system has installed by entering:

  versions -b | grep compiler_

eg. on my Octane2 this gives:

  I  compiler_dev   05/14/2004  Base Compiler Development Environment, 7.3
  I  compiler_eoe   07/13/2005  IRIX Standard Execution Environment (Base Compiler Headers and Libraries, 7.4.3m)

**********************************************************************

5. Background

When I asked John about why he created C-Ray, he said:

  This is just an extremely small program I did one day to figure
  out how would the simplest raytracer program look like in the
  least amount of code lines :)  It's not useful for anything apart
  from benchmarking.

  As part of my BSc dissertation project I did a really big and
  feature-full ray tracer, which could be useful, supporting:
  programmable shading, network rendering, monte carlo rendering
  algorithms, etc. But it's big and buggy, slow, and incomplete
  because I was rushing to finish it the last minute before the
  deadline (as always) :)

  So I scrapped the damn thing after that, and I'm starting from
  scratch with a new design if I finish it and it proves to be
  sucessful I'll let you know :)"


I also asked John why the best c-ray-mt results seem to be obtained
with a number of threads that is much larger than the number of
CPUs/cores in a system, to which he replied:

  Ian wrote:
  > I also suspect more threads means the balanced load between
  > the two CPUs is less affected by the possible differences in
  > complexity between threads.

  Bingo, each thread takes a bunch of scanlines, if the relative
  complexity of the rendering calculations between the bunches(sic)
  is not equal, then one thread may spend much more time calculating
  than another thread. Of course that doesn't necessarily mean that
  one "CPU" ends up calculating much more than the other since the
  threads are not "bound" to any CPU, each CPU takes one of the
  available ready-to-run threads each timeslice. Anyway having more
  threads evens it out. I would guess about 4 times as many threads
  as the CPUs [multiplied by the no. of cores per CPU] would be enough.


I also asked John if there was any element of overhead processing
to handle the results of the multiple threads. He said:

  There's no such overhead. Each thread gets a pointer to the
  appropriate location of the framebuffer, and stores every pixel
  as it is calculated directly. Also any processing afterwards
  (output of the PPM image) is done *after* timing stops.


See the Nekochan thread for more discussion about C-Ray, including
further comments by John.

**********************************************************************

6. Appendix A: Invalid Tests

Results for tests done with pre-run profiling/virtualisation compiler
optimisations will NOT be accepted! What does this mean? Read on...

Someone emailed me to say they had been able to halve the test run
times by using the following compilation/execution sequence:

  XC="gcc -O3 -ffast-math -fomit-frame-pointer c-ray-f.c
  -finline-limit=10000 -ftree-vectorize -fwhole-program
  -fbranch-probabilities -ffunction-sections -o c-ray-f -lm" && $XC
  -fprofile-generate && ./run > /dev/null 2>&1 && $XC -fprofile-use
  && ./run && ./run && ./run && ./run

with ./run containing:

  #!/bin/sh
  cat scene | ./c-ray-f > foo2.ppm

The explanation was as follows:

  What this does is to create an executable with profiling and
  virtualization instructions, execute it in order to create "real life"
  information and then recompile it using that information to better
  optimize it without the need of "guessing" (which is what usually a
  plain -O3 does). Doing this speeds up the execution a lot. Another
  important thing was to increase the inline limit from 600 (or whatever
  the default value is) to something big (like 10000) so more functions
  will be inlined instead of called. The usage of "-fwhole-program" tells
  to gcc that the .c file is the one and only source code file for the
  program, so it will use all functions as static and will make all
  "inline-able" functions, inline. This is another great speedup :-).

My problem with this is that, by definition, the test has to be run
multiple times in order to provide execution profiling data used to
optimise the test for the final run. Thus, the real total run time spent
to obtain the final result was not that much less or perhaps longer than
just running the test a single time without this sort of execution
profiling.

I asked John about these optimisations; he said:

  Hahaha :)
  Yes I bet the biggest advantage was the profiling run and use of that
  profiling information, and maybe also the whole program optimizations.
  Both of which can't be done in real-life programs :)

  The profiling information helps the compiler issue branch prediction
  hints to the processor, and also do prefetches. The first one makes a
  lot of difference in modern x86 CPUs with their huge execution
  pipelines. If the branch prediction fails, you end up flushing the
  pipeline and backtracking tens of instructions. Explicit prefetching,
  would also make a big difference if the data set was bigger. I don't
  think it helps here.

This method of optimisation is certainly interesting, but I don't think
it's appropriate for the C-Ray tests.

