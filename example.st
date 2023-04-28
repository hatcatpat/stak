freq_max: (2.5 sin) bi2norm 440 * 440 +

lfo: (1 saw) bi2norm freq_max *

wave: (4 1 oscil) bi2norm 4 *

b: 44100 2 buffer fill_sine

out: [(lfo sin) b play]
     [lfo wave oscil dup]
	 mix
