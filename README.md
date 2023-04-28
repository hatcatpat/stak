# stak
## a forth-y livecoding audio doodad

heavily inspired by [sporth](https://paulbatchelor.github.io/proj/cook/what_is_sporth.html), with an aim to be like [ProxySpace](https://doc.sccode.org/Classes/ProxySpace.html)

it is a tiny stack-based language similar to forth
```forth
# howdy there :)
freq: 0.3 sin bi2norm 880 *

wave: freq saw

noise: 1000 1 buffer fill_noise

out: (wave) (1 noise play 0.5 *) +
```

## key terminology:
* **value**: a basic unit used by operations: 
  * a number
  * a pointer to something
  
* **stack**: a stack of values

* **atom**: a basic operation:
  * a string (push a string)
  * a number (push a number)
  * a pointer to a variable (append all values from that variable's stack onto the current);
  * a function (see below)
  
* **procedure**: a list of atoms, and an process type (how often it should be processed)

* **function**: these are compliated atoms that can perform a variety of tasks
  * may have an "init" function, which is called when the atom is created
  * may have a "deinit" function, which is called when the atom is destroyed
  * may have a "process" function, which is called when the atom is processed
  * may have some dynamic data, which can be allocated/freed by the init/deinit functions and used by the process function
  * for example:
    * add: no init, no deinit, no data, process(a,b) = a + b 
    * buffer: init will allocate a buffer, deinit will free the buffer, data stores the buffer_t struct, process(len,chans) = resizes buffer to len and chan
    * sin: init will allocate an oscilator, no deinit (data is automatically freed), data stores the oscil_t struct, process(freq) = increments oscilator by the given frequency

* **variable**: these are defined by "name: body", and consist of a stack and a procedure that is applied to that stack

* **out**: this is a special variable, the top values on its stack are sent to the speakera

* **comments**: they start with a # and take up the whole line

* **brackets/parenthesis/tabs/newline**: these are completely ignored, use them to split up your code however you see fit

## interacting
this section needs work :)

currently, type "r" to reload example.st, "q" to quit

## build requirements
* ansi c compiler
* make
* jack
