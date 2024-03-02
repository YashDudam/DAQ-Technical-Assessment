# Brainstorming

This file is used to document your thoughts, approaches and research conducted across all tasks in the Technical Assessment.

## Firmware
I tried compiling the dbcppp library but for the life of me it did not work. At first it wouldn't build, but I eventually fixed that by
building it in a linux container. But then I got a linker error that I couldn't fix at all. I tried using CMake which I have limited
experience with. I tried compiling it manually with clang. I tried consulting friends that have more experience with C++. Whatever I
threw at the problem would simply not work and I already wasted two days on it, so I decided to parse the dbc file myself.

The actual parsing itself wasn't too difficult if not a bit tedious. I believe I implemented it as cleanly as possible and made good
design decisions overall. However, I could improve the code safety of the solution by using the pimpL idiom to hide implementation details
of the dbc::format class. I believe memory management could have been done better by using pass by reference for strings and crucially,
use unique pointers for the map of signals. I did try using a unique pointer, but I kept getting memory leaks or compile errors and I
was under a bit of a time crunch so I couldn't sort it out more.

Overall, I would have preferred to use that external library because it's a much more robust solution and in large projects, building my
own parser and maintaining it is simply not an option. I also wish I had the time to implement the pimpL idiom although i did hide the
implementation of dbc::signal, but that was done in a C style way rather than a C++ way. Using a unique_ptr would have also been ideal
since dbc::format has sole ownership of signal. Good memory management is crucial in a language like C++.

I still have a lot to learn with C++ development, and I'm sure the C++ course running in T2 will help heaps with good practises. For now
though, I'm excited to learn more and get better.

## Telemetry
The streaming service was crashing because of incorrect JSON strings causing an exception to be thrown by JSON.parse(). The solution
was to simply catch it and move onto the next data packet.

The second task was simply tracking if its been five seconds since the last temp exceeded and was simple enough to implement.

The third task was just a couple cases that checked the state as the componenent rerendered.

Overall, I wish I could have improved the UI and explored that a bit more but unfortunately I was quite busy this week and had to leave
it to the last minute.

## Cloud
I have no experience working with cloud architecture and due to some unforeseen circumstances I wasn't able to attempt this task at all :(.
However, when I have some free time on my hands I'm excited to look into how developing cloud architecture works.
