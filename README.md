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

Note that the license for all this is non-standard and weird and
doesn't really qualify as "free" or "open source" in the modern
understanding of such things.

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

## Replaceable Code

- ascii.h has #ifdefs like 8 = BS, 9 = TAB
- edit.c is an interactive console editor
- quote.c has a date/time parser



