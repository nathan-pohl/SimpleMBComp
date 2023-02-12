/*
  ==============================================================================

    Params.h
    Created: 12 Feb 2023 4:15:41pm
    Author:  Nate

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace Params {
    enum Names {
        LowMidCrossoverFreq,
        MidHighCrossoverFreq,

        ThresholdLowBand,
        ThresholdMidBand,
        ThresholdHighBand,

        AttackLowBand,
        AttackMidBand,
        AttackHighBand,

        ReleaseLowBand,
        ReleaseMidBand,
        ReleaseHighBand,

        RatioLowBand,
        RatioMidBand,
        RatioHighBand,

        BypassedLowBand,
        BypassedMidBand,
        BypassedHighBand,

        MuteLowBand,
        MuteMidBand,
        MuteHighBand,

        SoloLowBand,
        SoloMidBand,
        SoloHighBand,

        GainIn,
        GainOut
    };

    inline const std::map<Names, juce::String>& GetParams() {
        static std::map<Names, juce::String> params = {
            { LowMidCrossoverFreq, "Low-Mid Crossover Freq" },
            { MidHighCrossoverFreq, "Mid-High Crossover Freq"},
            { ThresholdLowBand, "Threshold Low Band"},
            { ThresholdMidBand, "Threshold Mid Band"},
            { ThresholdHighBand, "Threshold High Band"},
            { AttackLowBand, "Attack Low Band"},
            { AttackMidBand, "Attack Mid Band"},
            { AttackHighBand, "Attack High Band"},
            { ReleaseLowBand, "Release Low Band"},
            { ReleaseMidBand, "Release Mid Band"},
            { ReleaseHighBand, "Release High Band"},
            { RatioLowBand, "Ratio Low Band"},
            { RatioMidBand, "Ratio Mid Band"},
            { RatioHighBand, "Ratio High Band"},
            { BypassedLowBand, "Bypassed Low Band"},
            { BypassedMidBand, "Bypassed Mid Band"},
            { BypassedHighBand, "Bypassed High Band"},
            { MuteLowBand, "Mute Low Band"},
            { MuteMidBand, "Mute Mid Band"},
            { MuteHighBand, "Mute High Band"},
            { SoloLowBand, "Solo Low Band"},
            { SoloMidBand, "Solo Mid Band"},
            { SoloHighBand, "Solo High Band"},
            { GainIn, "Gain In"},
            { GainOut, "Gain Out"},
        };
        return params;
    }
}
