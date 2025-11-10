// DOC: 02-project-setup.md # Import JUCE Library
import * as Juce from "./juce/index.js";

// Verify JUCE backend is connected
console.log("JUCE WebView connected:", window.__JUCE__.backend);

// DOC: 04-parameter-binding.md # JavaScript Slider Binding

// Wait for DOM to load
document.addEventListener("DOMContentLoaded", () => {
    // Get HTML elements
    const slider = document.getElementById("gainSlider");
    const valueDisplay = document.getElementById("gainValue");

    // DOC: 04-parameter-binding.md # Get Slider State
    // PATTERN: Get parameter state from JUCE
    const sliderState = Juce.getSliderState("GAIN");

    // Set slider step from parameter properties
    slider.step = 1 / sliderState.properties.numSteps;

    // DOC: 04-parameter-binding.md # Bind UI → Parameter
    // PATTERN: User moves slider → update parameter
    slider.oninput = function() {
        sliderState.setNormalisedValue(this.value);
        updateValueDisplay(this.value);
    };

    // DOC: 04-parameter-binding.md # Bind Parameter → UI
    // PATTERN: Parameter changes → update slider
    // (e.g., from automation, preset load, DAW control)
    sliderState.valueChangedEvent.addListener(() => {
        const value = sliderState.getNormalisedValue();
        slider.value = value;
        updateValueDisplay(value);
    });

    // Initialize display
    updateValueDisplay(slider.value);

    // Helper: Update percentage display
    function updateValueDisplay(value) {
        const percentage = Math.round(value * 100);
        valueDisplay.textContent = `${percentage}%`;
    }
});
