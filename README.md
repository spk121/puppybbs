# puppybbs
An update of T Jennings' minimalist, late-1980s Puppy BBS.

So there this California-based guy named Tom Jennings who does a lot
of stuff in the intersection between tech and art.  Once upon a time
he was a driving force behind FidoNet, which was a pre-internet
community of dial-up BBSs.  He's done many cool things since FidoNet.

Check out his cool art at <http://www.sensitiveresearch.com/>

And there there is me, Mike Gran.  I like old things.  One time,
whilst digging through some old CD-ROMS over at
[textfiles.com](http://textfiles.com), I found source code for
something called Puppy BBS.  I guess Tom wrote it as a reaction to how
complicated BBSs had become back in the late 1980s.  So it is just
about the simplest BBS one could imagine.

I found it in here:
<http://cd.textfiles.com/simtel/simtel20/MSDOS/FIDO/.index.html>

So I thought, hey, does this thing still build and run?  Well, not
exactly.  First off, it uses a MS-DOS C library that handles serial
comms, which, of course, doesn't work on Microsoft Windows 10 or on
Linux.  And even if that library did still exist, I couldn't try it
even if I wanted to.  I mean, if I wanted to try it I would need two
landlines and two dial-up modems so I could call myself.  I do have a
dial-up modem in a box in the garage, but, I'm not going to get
another landline for this nonsense.

Anyway, I e-mailed Tom and asked if I could hack it up and post it on
Github, and he said okay.  And so this is what that is.

## Organization

In the *original* directory is the code more or less how I found
it.

In the *src* directory is a version of puppybbs that builds on Linux.

Does it work?  I don't know.  I'll never have two land lines with
which to test it.  But it does launch so you can get a sense of it.

Note that the license for all this is non-standard and weird and doesn't
really qualify as "free" or "open source" in the modern understanding of
such things.

## Puppy's internal databases

### The global state

Most of the global state lives in `struct _pup`.  This is initialized
by an INI file, and the INI file is overwritten when values like `top`
or `tries` change.  An INI file that gets modified by the program
itself is unfortunate.

| Variable    | Format        | Init       | Description                                      |
|-------------|---------------|------------|--------------------------------------------------|
| `callers`   | `int32_t`     | persistent | running total of number of callers to the system |
| `quote_pos` | `int32_t`     | persistent | current position in quotes file                  |
| `id`        | `_node`       | unused     | a Fidonet node id                                |
| `nlimit`    | `int16_t`     | INI set    | max connection length in minutes                 |
| `klimit`    | `int16_t`     | unused     | max kB download per connection                   |
| `top`       | `int16_t`     | persistent | ID of top of circular message buffer             |
| `msgnbr`    | `int16_t`     | ?          | current highest message number                   |
| `messages`  | `int16_t`     | INI set    | total number of messages allowed                 |
| `msgsize`   | `int16_t`     | INI set    | max chars per message                            |
| `topic`     | `_topic[16] ` | INI set    | names a descriptions of topics                   |
| `maxbaud`   | `int16_t`     | INI set    | max baud rate of a connection                    |
| `mdmstr`    | `char[80]`    | INI set    | initialization string for the modem              |
| `cd_bit`    | `uint16_t`    | INI set    | modem Carrier Detect mask                        |
| `iodev`     | `int16_t`     | INI set    | the COM port number                              |
| `tries`     | `int16_t`     | unused     | FidoNet dial attempts w/o connects               |
| `connects`  | `int16_t`     | unused     | FidoNet dial attempts w/ connects                |
| `sched`     | `_sched[35]`  | unused     | Event table for periodic events                  |
| `filepref`  | `char[80]`    | INI set    | path of upload/download files                    |


### Quotes

The quotes database is the QUOTES.PUP file.  The quotes are stored as
variable length plain text, and are separated from one another by a
blank line.

### Topics

The 16 topics are stored as

| Variable | Format     | Init    | Description            |
|----------|------------|---------|------------------------|
| `name`   | `char[8]`  | INI set | short topic name       |
| `desc`   | `char[24]` | INI set | long topic description |


### The caller database

The caller is stored as

| Variable | Format         | Init   | Description                                            |
|----------|----------------|--------|--------------------------------------------------------|
| `name`   | `char[36]`     |        | user name                                              |
| `date`   | `uint16_t[16]` |        | MS-DOS dates of most recent message read in each topic |
| `topic`  | `uint16_t`     |        | number 1 to 16 of last topic selected                  |
| `lines`  | `uint8_t`      |        | preferred number of screen lines                       |
| `cols`   | `uint8_t`      |        | preferred number of screen cols                        |
| `calls`  | `uint16_t`     |        | number of calls by this user                           |
| `extra`  | `uint16_t`     | unused |                                                        |

The number of callers is limited to 25 in PUP.SET or 100 in defaults


### The messages database

The message header

| Variable    | Format       | Init   | Description             |
|-------------|--------------|--------|-------------------------|
| `from`      | `char[36]`   |        | Name of author          |
| `to`        | `char[36]`   |        | Name of recipient       |
| `subj`      | `char[36]`   |        | Topic of message        |
| `date`      | `uint16_t`   |        | MS-DOS 16-bit date      |
| `time`      | `uint16_t`   |        | MS-DOS 16-bit time      |
| `extra`     | `uint16_t`   | unused |                         |
| `attr`      | `uint16_t`   | unused | bitmask                 |
| `topic`     | `uint16_t`   |        | topics                  |
| `topic_map` | `uint16_t`   | unused | ?                       |
| `message`   | `char[2048]` |        | The text of the message |
|             |              |        |                         |

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
