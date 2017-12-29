# puppybbs
An update of T Jennings' minimalist, late-1980s Puppy BBS.

So there this California-based guy named Tom Jennings who does a lot
of stuff in the intersection between tech and art.  Once upon a time
he was a driving force behind FidoNet, which was a pre-internet
community of dial-up BBSs.  He's done many cool things since FidoNet.

Check out his cool art at http://www.sensitiveresearch.com/ 

And there there is me, Mike Gran.  I like old things.  One time,
whilst digging through some old CD-ROMS over at textfiles.com, I found
source code for something called Puppy BBS.  I guess Tom wrote it as a
reaction to how complicated BBSs had become back in the late 1980s.
So it is just about the simplest BBS one could imagine.

I found it in here:
http://cd.textfiles.com/simtel/simtel20/MSDOS/FIDO/.index.html

So I thought, hey, does this thing still build and run?  Well, not
exactly.  First off, it uses a MS-DOS C library that handles serial
comms, which, of course, doesn't work on Microsoft Windows 10 or on
Linux.  And even if that library did still exist, I couldn't try it
even if I wanted to.  I mean, if I wanted to try it I would need two
landlines and two dial-up modems so I could call myself.  I do have a
dial-up modem in a box in the garage, but, I'm not going to get
another landline for this nonsense.

But, there is such a thing as a BBS over Telnet.  There are a few that
still exist.

Anyway, I e-mailed Tom and asked if I could hack it up and post it on
Github, and he said okay.  And so this is what that is.

I did already haxor the crap out of it once.  But now, I'm going to do
it again with a bit more rigor.

Note that the license for all this is non-standard and weird and doesn't
really qualify as "free" or "open source" in the modern understanding of
such things.

## Puppy's internal databases

### Topics

The 16 topics are stored as
- name: char[8]
- description: char[24]

### The caller database

The caller is stored as

- name: char[36]
- date: uint16_t[16]. 16-bit MS-DOS date of the most recent message read in
  each topic
- topic: uint16_t. Number from 1 to 16 of last topic selected
- lines: uint8_t. preferred number of screen lines
- cols: uint8_t. preferred number of screen cols
- calls: uint_16_t. number of calls by this user
- extra: uint16_t (unused)

The number of callers is limited to 25 in PUP.SET or 100 in defaults

### The messages database

The message header

- from: char[36]
- to: char[36]
- subj: char[36]
- date: MS-DOS 16-bit date
- time: MS-DOS 16-bit time
- extra: uint16_t (unused)
- attr: uint16_t (unused)
- topic: uint16_t
- topic_map: uint16_t (set to zero but unused)
- message: char[2048 or 2560]

It looks like extra, attr, and topic_map are unused.
TOPIC is 1 to 16: one of the 16 topic keywords categories.

The message size of 2048 was probably chosen as around 80 * 25.

The maximum number of messages is limited to 10 by 2560 in PUP.SET, or 50 by
2048 in defaults, with old ones expiring, so the message database is limited
to 25k in PUP.SET or  100kB max in default. The 25k was probably for MS-DOS
memory handling or floppy disk storage.

### The files database

All downloadable files were stored on the local file system in a
single folder.  There was a file "FILES.PUP" that held a database
of files that could be downloaded.

The "FILES.PUP" file format was
- filename: char[13]
- description: char[40]

If FILENAME began with a space or hyphen then description is
just a comment.

If FILENAME began with ^Z or @, that was the end of the database.

Other part of the files 'database' comes from the filesystem
itself, including creation time and file size.


## On Making a Telnet BBS

So original PuppyBBS was probably a MS-DOS program.  From the original
code, it looks like it started up as a foreground application where
the sysop could interact with it, and it also listened to a single
modem port.

There are a couple of routes to making this into something I could
use.

- original MS-DOS application with one serial-port-attached modem
- inetd-style application on my Fedora GNU/Linux server
- multi-threaded server on my Fedora GNU/Linux server
- service on my Microsoft Windows 10 box

### original MS-DOS application with one serial-port-attached modem

The problem here is testing out a modem, and also that no one will
ever call me.  Kind of pointless, but, awesome.

This has the two-connections-as-once style, with sysop at the console
and a user on the modem.

### inetd-style application on my Fedora Server GNU/Linux box

This is obviously the simplest.  With each new client connection, a
new instance of the application is spawned.  The TCP I/O is converted
to stdin/stdout I/O by the inetd (or systemd) server.

The challenge here is that there needs to be file locking of the
database files, since multiple instantiations of the PuppyBBS
executable may be accessing the files at the same time.

Info on making systemd to act like an inetd spawner.
http://0pointer.de/blog/projects/inetd.html

### multi-threaded server on my Fedora Server GNU/Linux box

Now we get a bit more complicated.  Multiple threads and multiple
connections in a single application.

For info on making a daemon
http://0pointer.de/public/systemd-man/daemon.html

For info on making a server application, I can look back at the
lessons learned from GNU serveez.

### service on Windows 10

This is new to me.  There is info about writing services in C++ at
https://msdn.microsoft.com/en-us/library/windows/desktop/ms686953(v=vs.85).aspx

It seems a bit like coding up an old school DLL back in the day.

This is also pointless, since my Windows 10 box is not exposed to the
world, but it was fun to read about.

## Required algorithms

### Getting user's terminal size

In original Puppy, the user is queried for the number of rows and columns
his terminal supports.  In Pupper we're going to ask for the terminal
instead.  The default terminal size is in the terminfo database.

Note that for ncurses, we need use_env(FALSE) and use_toictl(FALSE) to
be called before any ncurses stuff, so that ncurses uses the
number of rows and columns in the terminfo database, instead of trying
to query it from environment variables or ioctl calls.  Those
aren't relevant here because we're running on a client's terminal over
telnet.

## Replaceable Code

- ascii.h has #ifdefs like 8 = BS, 9 = TAB
- edit.c is an interactive console editor.  Ncurses Forms also
  has an editor.
- quote.c has a date/time parser
- there is initialization data, which could become an ini file, registry,
  or database.  Boost::property_tree?
- there is a message file-based database, which could become a standard
  database.  MariaDB
- the non-portable serial port code could become ASIO to make it both
  portable serial port and TCP
- there is a lot of parsing and string handling.  Boost::string_algo
- there is a lot of filename handling.
- xmodem I/O.  https://github.com/caseykelso/xmodem
- there is a task scheduler. This could also be Boost::ASIO
- what is the best c++ method to parse user input?  getline and regex?
  Is there something easier than regex?
- download() searches files by regex.

### Libraries
-   Standard C++ filesystem library:
    [documentation](http://en.cppreference.com/w/cpp/filesystem)
-   Standard C++ locale library sets up text encoding:
    [documentation](http://en.cppreference.com/w/cpp/locale)
-   Standard C++ messages library stores lists of translations:
    [documentation](http://en.cppreference.com/w/cpp/locale/messages)
-   The MySQL Connector/C++ (GPL):
    [documentation](https://dev.mysql.com/doc/connector-cpp/en/) and
    [code](https://dev.mysql.com/downloads/connector/cpp/)
-   Ncurses (BSD):
    [documentation](http://invisible-island.net/ncurses/ncurses.html) and
    [code](ftp://ftp.invisible-island.net/ncurses/ncurses.tar.gz)
    including the underdocumented C++ bindings
-   Boost [documentation](http://www.boost.org/doc/libs/1_66_0/) and
    [code](https://dl.bintray.com/boostorg/release/1.66.0/source/)
-   lrzsz (GPL2)
    [documentation and code](https://ohse.de/uwe/software/lrzsz.html), but
    see jnavila's patched version on
    [github](https://github.com/jnavila/lrzsz)
-   caseykelso's xmodem on [github](https://github.com/caseykelso/xmodem)

