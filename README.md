# keything

A thing that lets you use the arrow keys without moving your fingers from the home row on the keyboard, based on [ProtoTyping].

Whereas [ProtoTyping] works on Mac OS and Windows, this one **only works on Linux**.
It needs root privileges to run.

[ProtoTyping]: https://github.com/pnutus/ProtoTyping

## Usage

When the keything is started, hold down Caps Lock.
Then use the following keys to do Special Things:

- `ijkl` work as arrow keys.
- `h` moves to the beginning of the row.
- `the key to the right of L` (on an US keyboard that's `;`) moves to the end of the row.

When Caps Lock is released, everything returns to normal.

## Installation

Just compile with

```bash
$ ./build.sh
```

Then run with root privileges:

```bash
$ sudo ./keything
```
