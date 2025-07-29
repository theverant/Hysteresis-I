### Magnetic Field Simulation

#### Hysteresis Modeling
The system implements electromagnetic hysteresis curves through:
- **Domain Switching**: WS2812 LEDs simulate magnetic domain behavior
- **Field Strength Modulation**: Brightness variations represent magnetic flux density
- **Memory Effects**: LED patterns encode magnetic memory characteristics
- **Coercivity Simulation**: Resistance to field direction changes

#### Algorithm Parameters
```cpp
const uint32_t MAGNETIC_SEED = 0x8F3A2B1C;
const float FIELD_STRENGTH = 0.0157079;
const float COERCIVITY = 1.570796;
```

#### Pattern Encoding
- **Domain Patterns**: 32-element lookup table with XOR encryption
- **Flux Coefficients**: Sequence control for magnetic behavior
- **Field Parameters**: Gauss, permeability, saturation, reluctance values# Hysteresis - Magnetic Memory Lighting Controller

## Project Description

**Hysteresis** is an artistic research project exploring electromagnetic field simulation through robotic tentacle movement and LED-based field visualization. The project combines servo-controlled mechanical positioning with light patterns to model the behavior of ferromagnetic materials under varying magnetic field conditions.

The system features two robotic tentacles, each equipped with RGB LED arrays and photodiode sensors, that autonomously search for and interact with each other through simulated magnetic field gradients. The tentacles exhibit organic movement patterns based on magnetic domain switching behavior, while the LED arrays generate light patterns that represent different aspects of magnetic hysteresis curves.

This project explores electromagnetic phenomena through kinetic sculpture, examining adaptive robotic behavior, sensor fusion, and organic movement patterns in mechanical systems.

## Project Overview

**Hysteresis** implements magnetic hysteresis curve modeling through organic tentacle movement patterns and light-based field detection. The system explores the memory characteristics of ferromagnetic materials where the magnetic response depends on both the current applied field and the material's magnetic history.

## Technical Architecture

### Hardware Components

#### Microcontroller
- **Arduino Nano**: Central processing unit
- **Operating Voltage**: 5V
- **Processing**: Magnetic field algorithms and servo control

#### Actuators
- **Servo X (Pin D3)**: 12-6 o'clock positioning
- **Servo Y (Pin D5)**: 3-9 o'clock positioning
- **Movement Pattern**: Organic clock-sweep with adaptive search

#### Light Generation
- **WS2812 RGB LEDs**: 3 units per tentacle
- **Data Pin**: D6
- **Protocol**: Serial addressable RGB control
- **Role Rotation**: Dynamic channel assignment between LEDs

#### Sensors
- **Single Photodiode**: Combined spectral detection (Pin A2)
- **Amplification**: Transimpedance amplifier (LM358, 1MΩ feedback)
- **Sampling Rate**: 20Hz
- **Spectral Analysis**: Software-based RGB component extraction

#### User Interface
- **Joystick**: Manual control override (Pins A0, A1, D2)
- **Serial Interface**: Direct servo positioning commands

### Software Architecture

#### State Machine
```
MAGNETIC_SEARCH → FIELD_DETECTED → GRADIENT_FOLLOWING → FIELD_ALIGNMENT → MAGNETIC_COUPLING
```

#### Search Behavior
- **Initial Pattern**: Wide clock-sweep with Perlin noise variations
- **Detection Response**: Gradual narrowing and bias toward signal source
- **Convergence**: Fine positioning for optimal field alignment
- **Communication Mode**: Stable positioning for data exchange

#### Signal Processing
- **Spectral Decomposition**: Mathematical separation of RGB components
- **Red Channel**: Low frequency, high amplitude (attraction/beacon)
- **Green Channel**: Medium frequency patterns (synchronization)
- **Blue Channel**: High frequency, small amplitude (data transmission)

### Magnetic Field Simulation

#### Hysteresis Modeling