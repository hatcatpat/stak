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
