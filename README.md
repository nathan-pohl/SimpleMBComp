SimpleMBComp is a multi-band compressor plugin made with the JUCE framework.
It is based off of a youtube course by MatKat Music. 
The differences between this project and the original are mostly in code style, with a few improvements to organization and coding practices thrown in.

[Original Youtube Video](https://youtu.be/H1IvfOfBsVQ)

<img title="Plugin UI" src="/docs/SimpleMBComp.PNG">

This plugin features a low, mid, and high band compressor. The crossover frequencies between the three compressors can be changed to allow each compressor to focus on the range of the users choice. Each compressor has it's own separate Attack, Release, Threshold, and Ratio parameters. The compressor that the user wishes to change parameters for can be selected with the three buttons in the lower left corner. In addition, all three bands can be bypassed, soloed, or muted using the `X`, `S`, and `M` buttons in the bottom right.

The frequency analyzer band shows the stereo input to the plugin, and will show what gain reductions are taking place live with an opaque pinkish color. The frequency analyzer can be disabled with the button on the top left.
