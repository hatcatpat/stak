b: (44100 0.1 *) 2 buffer

tog: 1 pulse bi2norm

r: in tog b record

lfo: [(0.013 sin bi2norm 4 *) sin bi2norm (0.003 sin bi2norm 880 *) *]

speed: (1 saw) bi2norm 4 *

out: speed b play
