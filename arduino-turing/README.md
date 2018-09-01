# Random Sequence Looper

100% inspired by the great Music Thing Modular's [Turing Machine](http://musicthing.co.uk/pages/turing.html) module.

## Inputs & ouputs

### Knobs

From top to bottom:

- **A1**: Sequence length (from 0 to 16 notes).
- **A2**: Note range (from 0 to 24 semitones, i.e. max 2 octaves).
- **A3**: Randomization rate (from frozen to updating on every cycle).

### Jacks

- **D2**: Clock input.
- **A0**: Random gate output. Outputs a gate signal with the probability of 50% based on the clock input.
- **D11**: Gate input – can be self-patched from the output above.
- **DAC**: Pitch CV output. 1V/octave. Uses MCP4725 on pins **A4** and **A5**.


![img_6602](https://user-images.githubusercontent.com/381895/40932624-43a529f8-682f-11e8-96fa-4ee1e227ae96.jpeg)
