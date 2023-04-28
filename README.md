# stak
## a forth-y livecoding audio doodad

heavily inspired by [sporth](https://paulbatchelor.github.io/proj/cook/what_is_sporth.html), with an aim to be like [ProxySpace](https://doc.sccode.org/Classes/ProxySpace.html)

it is a tiny stack-based language similar to forth
```forth
freq: 0.3 sine bi2norm 880 *

wave: freq saw

out: wave (0.1 sine) pan
```

* **variables** are defined by "name: body", and consist of a procedure (i.e., a list of atoms that will be evaluated) and a stack
* **ugens** are an atom that have a persistent state, they are evaluated every sample and are used to generate/modify audio signals
* **out** is a special variable, the top values on its stack are sent to the speaker
* **comments** start with a # and take up the whole line
* **brackets/parenthesis/tabs/newline** are completely ignored, use them to split up your code however you see fit

stereo audio is really simple in stack-based languages, you can use:
* **dup** to duplicated the top value on the stack
* **pan** to pan a mono signal into a stereo signal
* **mix** to combine 2 stereo signals into 1 stereo signal

## interacting
this section needs work :)

at the moment, the stak will just parse "test.st" when it loads

you can interact with it further by sending a udp message to 127.0.0.1:2000, the stak_send.sh script gives an example of how to do this using netcat. the advantage of a udp server over stdin is that you can send messages from any other program (i.e., vim), without having to rely on a terminal multiplexer (like tmux or screen).

## build requirements
* ansi c compiler
* make
* jack
